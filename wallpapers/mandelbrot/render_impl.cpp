// Copyright 2008-2010 by Benoît Jacob <jacob.benoit.1@gmail.com>
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License or (at your option) version 3 or any later version
// accepted by the membership of KDE e.V. (or its successor approved
// by the membership of KDE e.V.), which shall act as a proxy 
// defined in Section 14 of version 3 of the license.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "mandelbrot.h"
#include "mix.h"
#include <Eigen/Array>
#include <limits>

#ifdef THIS_PATH_WITH_SSE2_EXPLICTLY_ENABLED
namespace with_SSE2_explicitly_enabled_if_x86 {
#else
namespace with_arch_defaults {
#endif

/* The number of iterations to do before testing for divergence. The higher, the faster. However,
 * at each iteration we square z, and z may initially have modulus up to 2. So we're very quickly hitting the
 * exponent limit of floating point numbers. Taking into account that we want this number to be a multiple of 4,
 * as we are going to peel the inner loop by 4, the values below are the largest possible.
 * As they are so small, we make them compile-time constants to allow to compiler to unroll if it decides to.
 */
template<typename T> struct iter_before_test { enum { ret = 4 }; };
template<> struct iter_before_test<double> { enum { ret = 8 }; };

template<typename Real>
struct mandelbrot_render_tile_impl
{
  // number of reals in a SIMD packet.
  // Examples:
  //  * no vectorization: then packet_size == 1 always
  //  * SSE or AltiVec: then packet_size is 4 if Real==float and is 2 if Real==double
  enum { packet_size = Eigen::ei_packet_traits<Real>::size };

  // with Eigen, if we use vectors of the right size, operations on them will be compiled
  // as operations on SIMD packets
  typedef Eigen::Matrix<Real, packet_size, 1> Packet;
  typedef Eigen::Matrix<int, packet_size, 1> Packeti;

  // normally we wouldn't need to do that, but since some compilers can miss optimizations,
  // in the most critical code, let's play directly with SIMD registers. It's completely equivalent
  // to Eigen vectors of the right size, just less subtle for the compiler to get right.
  typedef typename Eigen::ei_packet_traits<Real>::type LowlevelPacket;

  // how many iterations we do before testing for divergence. See comment above for struct iter_before_test.
  // must be a multiple of 4 as we'll peel the inner loop by 4
  enum { iter_before_test = iter_before_test<Real>::ret };

  Real resolution;
  int supersampling;
  int max_iter;
  float log_max_iter;
  float tmin;
  float log_of_2;
  float log_of_2log2;
  Real square_bailout_radius, log_log_square_bailout_radius;
  Color3 rgb1, rgb2, rgb3, hsv1, hsv2, hsv3;
  Mandelbrot *mandelbrot;
  QImage *image;
  const MandelbrotTile& tile;
  bool found_exterior_point;

  mandelbrot_render_tile_impl(Mandelbrot *m, const MandelbrotTile& t)
   : mandelbrot(m), tile(t) { init(); }
  void init();
  void computePacket(int x, int y, Color3 *pixels);
};

template<typename Real>
void mandelbrot_render_tile_impl<Real>::init()
{
  // remember if we already rendered some exterior point. will be useful for the check for fully interior tiles
  found_exterior_point = false;
  
  // the supersampling factor.
  supersampling = mandelbrot->supersampling();

  // the resolution, i.e. the distance in the complex plane between adjacent pixels.
  resolution = Real(mandelbrot->resolution()) / supersampling;

  /***************
  Henceforth, we can completely forget about supersampling -- it's implicit.
  Supersampling only means that 'image' is bigger and 'resolution' is smaller than they would otherwise be,
  but from now on we don't need to care.
  ***************/

  // how many iterations we do on each sample before declaring that it doesn't diverge
  max_iter = mandelbrot->maxIter();

  // precompute some constants that will be used in the coloring computations
  log_max_iter = std::log(float(max_iter));
  if(mandelbrot->min_iter_divergence() != 0 && mandelbrot->min_iter_divergence() != max_iter)
  {
    tmin = std::log(float(mandelbrot->min_iter_divergence())) / log_max_iter;
  }
  else tmin = 0.f;

  square_bailout_radius = 20; // value found in papers on continuous escape time formulas, changing it can degrade the smoothness.
  log_log_square_bailout_radius = std::log(std::log(square_bailout_radius));
  
  log_of_2 = std::log(2.f);
  log_of_2log2 = std::log(2.f*log_of_2);

  mandelbrot->color1().getRgbF(&rgb1[0], &rgb1[1], &rgb1[2]);
  mandelbrot->color1().getHsvF(&hsv1[0], &hsv1[1], &hsv1[2]);
  mandelbrot->color2().getRgbF(&rgb2[0], &rgb2[1], &rgb2[2]);
  mandelbrot->color2().getHsvF(&hsv2[0], &hsv2[1], &hsv2[2]);
  mandelbrot->color3().getRgbF(&rgb3[0], &rgb3[1], &rgb3[2]);
  mandelbrot->color3().getHsvF(&hsv3[0], &hsv3[1], &hsv3[2]);
}

template<typename Real>
void mandelbrot_render_tile_impl<Real>::computePacket(int x, int y, Color3 *pixels)
{
  // for each pixel, we're going to do the iteration z := z^2 + c where z and c are complex numbers,
  // starting with z = c = complex coord of the pixel. pzr and pzi denote the real and imaginary parts of z.
  // pcr and pci denote the real and imaginary parts of c.
  Packet pcr, pci, pzr, pzi, pzr_buf;

  for(int i = 0; i < packet_size; i++) {
    pzi[i] = pci[i] = tile.affix().y() + y * resolution;
    pzr[i] = pcr[i] = tile.affix().x() + (x+i) * resolution;
  }

  /* first step: do iterations by batches of 'iter_before_test'. Testing only every 'iter_before_test'
    * iterations allows to go faster.
    */
  int j = 0;
  Packet pzabs2;
  Packeti pixel_iter = Packeti::Zero(), // number of iteration per pixel in the packet
          pixel_diverge = Packeti::Zero(); // whether or not each pixel has already diverged
  int count_not_yet_diverged = packet_size;

  Packet pzr_previous, pzi_previous,
          pzr_before_diverge = Packet::Zero(), pzi_before_diverge = Packet::Zero();
  do
  {
    pzr_previous = pzr;
    pzi_previous = pzi;

    /* perform iter_before_test iterations */
    for(int i = 0; i < iter_before_test/4; i++) // we peel the inner loop by 4
    {
      LowlevelPacket lpzr, lpzi;
      for(int repeat = 0; repeat < 4; repeat++)
      {
        lpzr = Eigen::ei_pload(pzr.data());
        lpzi = Eigen::ei_pload(pzi.data());
        Eigen::ei_pstore(pzr.data(),
                          Eigen::ei_padd(
                            Eigen::ei_psub(
                              Eigen::ei_pmul(lpzr,lpzr),
                              Eigen::ei_pmul(lpzi,lpzi)
                            ),
                            Eigen::ei_pload(pcr.data())
                          )
                        );
        Eigen::ei_pstore(pzi.data(),
                          Eigen::ei_padd(
                            Eigen::ei_pmul(
                              Eigen::ei_padd(lpzr,lpzr),
                              lpzi
                            ),
                            Eigen::ei_pload(pci.data())
                          )
                        );
      }
    }

    /* test for divergence */
    pzabs2 = pzr.cwise().square();
    pzabs2 += pzi.cwise().square();
    for(int i = 0; i < packet_size; i++) {
      if(!(pixel_diverge[i])) {
        if(pzabs2[i] > square_bailout_radius) {
          pixel_diverge[i] = 1;
          pzr_before_diverge[i] = pzr_previous[i];
          pzi_before_diverge[i] = pzi_previous[i];
          count_not_yet_diverged--;
        }
        else pixel_iter[i] += iter_before_test;
      }
    }

    j += iter_before_test;
  }
  while(j < max_iter && count_not_yet_diverged);

  /* Second step: we know the iteration count before divergence for each pixel but only up to precision
    * 'iter_before_test'. We now want to get the exact iteration count before divergence,
    * so we iterate again starting from where we were before divergence, and now we test at every iteration.
    */
  j = 0;
  pzr = pzr_before_diverge;
  pzi = pzi_before_diverge;
  pixel_diverge = Packeti::Zero();
  count_not_yet_diverged = packet_size;
  typedef Eigen::Matrix<float,packet_size,1> Packet_to_float;
  Packet_to_float square_escape_modulus = Packet_to_float::Zero();
  do
  {
    pzr_buf = pzr;
    pzr = pzr.cwise().square();
    pzr -= pzi.cwise().square();
    pzr += pcr;
    pzi = (2*pzr_buf).cwise()*pzi;
    pzi += pci;
    pzabs2 = pzr.cwise().square();
    pzabs2 += pzi.cwise().square();
    for(int i = 0; i < packet_size; i++) {
      if(!(pixel_diverge[i])) {
        if(pzabs2[i] > square_bailout_radius) {
          pixel_diverge[i] = 1;
          square_escape_modulus[i] = (float)pzabs2[i];
          count_not_yet_diverged--;
        }
        else pixel_iter[i]++;
      }
    }
    j++;
  }
  while(j < iter_before_test && count_not_yet_diverged);

  if(count_not_yet_diverged < packet_size) found_exterior_point = true;

  /* Now we know exactly the number of iterations before divergence, and the escape modulus. */

  /* Third step: compute pixel colors. */

  for(int i = 0; i < packet_size; i++)
  {
    Real log_log_escape_modulus = Real(0);
    if(square_escape_modulus[i] > Real(1))
    {
      Real log_escape_modulus = std::log(square_escape_modulus[i]);
      if(log_escape_modulus > Real(1))
        log_log_escape_modulus = std::log(log_escape_modulus);
    }
    Real normalized_iter_count = pixel_iter[i] + (log_log_square_bailout_radius - log_log_escape_modulus) / log_of_2;
    Real log_normalized_iter_count = (normalized_iter_count > Real(1)) ? std::log(normalized_iter_count) : Real(0);
    Real t = log_normalized_iter_count / log_max_iter;
    
    // Now, remember that we did a little statistical analysis on some samples
    // to determine roughly what would be the smallest count of iterations before divergence.
    // At the beginning of the present function, we used that to compute 'tmin'.
    // Now we use it to make the gradient actually start at t=tmin. Lower values of t just give black.
    // In other words, we are ensuring that no matter what the viewpoint, the darkest part of the image
    // will always be black (at least if our statistical analysis went well).
    // In other words, we are trying to get optimal contrast. This is important as otherwise there could be
    // almost no contrast at all and an interesting viewpoint could give an almost blank image.
    t = (t-tmin)/(1.f-tmin);
    t = CLAMP(t, Real(0), Real(1));

    float threshold1 = 0.09f;
    float threshold2 = 0.3f;
    if(t < threshold1) {
      pixels[i] = (t/threshold1)*rgb3;
    }
    else if(t < threshold2) {
      pixels[i] = mix(rgb2, hsv2, rgb3, hsv3, (t - threshold1) / (threshold2 - threshold1));
    }
    else {
      pixels[i] = mix(rgb1, hsv1, rgb2, hsv2,  (t - threshold2) / (1.f - threshold2));
    }
  }
}


/** This is the main rendering function. It renders only the current tile of the Mandelbrot wallpaper.
  *
  * The image parameter is the image to render the tile to. The size of this image determines how many pixels we render.
  * So to get supersampled rendering, just pass a larger image here, and then scale it down to the real size using smooth
  * scaling.
  *
  * This function only writes to 'image', it does not at all write to mandelbrot->image() directly. The mandelbrot parameter
  * is only used to determine the parameters of the rendering such as viewpoint, colors, etc.
  */
template<typename Real> void mandelbrot_render_tile(
  Mandelbrot *mandelbrot,
  const MandelbrotTile& tile
)
{
// if we're compiling the path with SSE2 explicitly enabled, since it's only used on x86 (and not even on x86-64 since it has SSE2 by
// default), let's only compile the code if it's going to be used!

#if defined(HAVE_PATH_WITH_SSE2_EXPLICTLY_ENABLED) || !defined(THIS_PATH_WITH_SSE2_EXPLICTLY_ENABLED)

  enum { packet_size = Eigen::ei_packet_traits<Real>::size };
  Color3 dummy_buffer[packet_size];
  
  mandelbrot_render_tile_impl<Real> renderer(mandelbrot, tile);

  int supersampling = renderer.supersampling;
  int supersampled_packet_size = supersampling * packet_size;
  int tile_x = tile.destination().x();
  int tile_y = tile.destination().y();
  int tile_width = tile.destination().width();
  int tile_height = tile.destination().height();
  int supersampled_tile_width = supersampling * tile_width;
  int supersampled_tile_height = supersampling * tile_height;

  // first render a part of the border to check if the tile is probably entirely inside the interior of the Mandelbrot set
  for(int y = 1; y < supersampled_tile_height-1; y+=4)  // render every 4th pixel on the border
  {
    renderer.computePacket(0,y,dummy_buffer);
    renderer.computePacket(supersampled_tile_width - packet_size, y, dummy_buffer);
    // abort if required
    if(mandelbrot->abortRenderingAsSoonAsPossible()) return;
  }
  for(int x = 0; x < supersampled_tile_width; x+= 4*packet_size)
  {
    renderer.computePacket(x,0,dummy_buffer);
    renderer.computePacket(x,supersampled_tile_height-1,dummy_buffer);
    // abort if required
    if(mandelbrot->abortRenderingAsSoonAsPossible()) return;
  }
  // render the bottom-right packet: due to our rendering only every 4-th packet on the border, we could be
  // missing this corner of the tile.
  renderer.computePacket(supersampled_tile_width-packet_size,supersampled_tile_height-1,dummy_buffer);

  // now, if the tile looks like it's entirely inside the interior, just assume that's the case,
  // so fill it (not using a QPainter so as to avoid having to lock a mutex in case the image is being accessed
  // by another QPainter in the GUI thread) and return.
  if(!(renderer.found_exterior_point))
  {
    for(int y = 0; y < tile_height; y++)
    {
      for(int x = 0; x < tile_width; x++)
      {
        unsigned char *pixel
          = const_cast<unsigned char*>(
              static_cast<const QImage *>(mandelbrot->image())->scanLine(tile_y+y)
            ) + 4*(tile_x+x);
        pixel[0] = mandelbrot->color1().blue();
        pixel[1] = mandelbrot->color1().green();
        pixel[2] = mandelbrot->color1().red();
        pixel[3] = 255;
      }
    }
    return;
  }
  
  // ok now do the actual rendering. not much point trying to reuse the part of the border we've already rendered,
  // it's few pixels and it would take some nontrivial code.

  qreal one_over_supersampling_squared = qreal(1) / (supersampling*supersampling);
  
  for(int y = 0; y < tile_height; y++)
  {
    for(int x = 0; x < tile_width; x += packet_size)
    {
      Color3 supersampled_buffer[MAX_SUPERSAMPLING][packet_size * MAX_SUPERSAMPLING];

      for(int y2 = 0; y2 < supersampling; y2++)
      {
        for(int x2 = 0; x2 < supersampled_packet_size; x2 += packet_size)
        {
          renderer.computePacket(supersampling*x+x2,supersampling*y+y2,&supersampled_buffer[y2][x2]);
          // abort if required
          if(mandelbrot->abortRenderingAsSoonAsPossible()) return;
        }
      }

      int pixels_to_write = std::min(int(packet_size), tile_width-x);
      for(int i = 0; i < pixels_to_write; i++)
      {
        Color3 color = Color3::Zero();
        for(int y2 = 0; y2 < supersampling; y2++)
        {
          for(int x2 = 0; x2 < supersampling; x2++)
          {
            color += supersampled_buffer[y2][x2 + i*supersampling];
          }
        }
        color *= one_over_supersampling_squared;
        unsigned char *pixel
          = const_cast<unsigned char*>(
              static_cast<const QImage *>(mandelbrot->image())->scanLine(tile_y+y)
            ) + 4*(tile_x+x+i);
        pixel[0] = qreal_to_uchar_color_channel(color[2]);
        pixel[1] = qreal_to_uchar_color_channel(color[1]);
        pixel[2] = qreal_to_uchar_color_channel(color[0]);
        pixel[3] = 255;
      }
    }
  }
#else
  Q_UNUSED(mandelbrot);
  Q_UNUSED(tile);
#endif
}

template void mandelbrot_render_tile<float>(
  Mandelbrot *mandelbrot,
  const MandelbrotTile& tile
);

template void mandelbrot_render_tile<double>(
  Mandelbrot *mandelbrot,
  const MandelbrotTile& tile
);

}
