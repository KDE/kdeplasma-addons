// Copyright 2008-2009 by Benoît Jacob <jacob.benoit.1@gmail.com>
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

/** This is the main rendering function. It renders only the current tile of the Mandelbrot wallpaper.
  * Moreover, if the Mandelbrot wallpaper is using n threads, then this function will only render every n-th scanline.
  * The parameter interleaving_number then controls which scanline to start from, so typically different threads will use
  * different values of interleaving_number.
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
  int interleaving_number,
  QImage *image
)
{
// if we're compiling the path with SSE2 explicitly enabled, since it's only used on x86 (and not even on x86-64 since it has SSE2 by
// default), let's only compile the code if it's going to be used!

#if defined(HAVE_PATH_WITH_SSE2_EXPLICTLY_ENABLED) || !defined(THIS_PATH_WITH_SSE2_EXPLICTLY_ENABLED)

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

  // the current tile of mandelbrot. Used to determine the viewpoint. We won't directly render to this tile in
  // mandelbrot's image, we'll only render to the 'image' parameter passed to this function.
  const MandelbrotTile& tile = mandelbrot->tile();

  const Real real_start = Real(tile.affix().x());
  const Real imaginary_start = Real(tile.affix().y());

  // the supersampling factor. It's only used to determine the resolution.
  const Real supersampling = image->width() / mandelbrot->tile().destination().width();

  // the resolution, i.e. the distance in the complex plane between adjacent pixels.
  const Real resolution = Real(mandelbrot->resolution()) / supersampling;

  /***************
  Henceforth, we can completely forget about supersampling -- it's implicit.
  Supersampling only means that 'image' is bigger and 'resolution' is smaller than they would otherwise be,
  but from now on we don't need to care.
  ***************/

  int interleaving_count = mandelbrot->renderThreadCount();

  // how many iterations we do on each sample before declaring that it doesn't diverge
  const int max_iter = mandelbrot->maxIter();

  // precompute some constants that will be used in the coloring computations
  const float log_max_iter = std::log(float(max_iter));
  float tmin;
  if(mandelbrot->min_iter_divergence() != 0 && mandelbrot->min_iter_divergence() != max_iter)
  {
    tmin = std::log(Real(mandelbrot->min_iter_divergence())) / log_max_iter;
  }
  else tmin = 0.f;
  const float gamma = mandelbrot->gamma();
  const float tshift_to_gamma = 0.3f;
  const float tshift = std::pow(tshift_to_gamma, 1.f/gamma);
  const float tshift_plus_1_to_gamma = std::pow(tshift+1.f, gamma);
  
  const float log_of_2 = std::log(2.f);
  const float log_of_2log2 = std::log(2.f*log_of_2);

  Color3 rgb1, rgb2, rgb3, hsv1, hsv2, hsv3;
  mandelbrot->color1().getRgbF(&rgb1[0], &rgb1[1], &rgb1[2]);
  mandelbrot->color1().getHsvF(&hsv1[0], &hsv1[1], &hsv1[2]);
  mandelbrot->color2().getRgbF(&rgb2[0], &rgb2[1], &rgb2[2]);
  mandelbrot->color2().getHsvF(&hsv2[0], &hsv2[1], &hsv2[2]);
  mandelbrot->color3().getRgbF(&rgb3[0], &rgb3[1], &rgb3[2]);
  mandelbrot->color3().getHsvF(&hsv3[0], &hsv3[1], &hsv3[2]);

  /****** Beginning of rendering *********/

  // iterate over scanlines. Notice how each thread works on a different set of scanlines -- since each thread uses
  // a different value for interleaving_number
  for(int y = interleaving_number; y < image->height(); y += interleaving_count)
  {
    // pointer to the first pixel in the scanline to render
    unsigned char *pixel = const_cast<unsigned char*>(static_cast<const QImage *>(image)->scanLine(y));

    // for each pixel, we're going to do the iteration z := z^2 + c where z and c are complex numbers, 
    // starting with z = c = complex coord of the pixel. pzr and pzi denote the real and imaginary parts of z.
    // pcr and pci denote the real and imaginary parts of c.
    Packet pzi_start, pci_start;
    for(int i = 0; i < packet_size; i++) pzi_start[i] = pci_start[i] = imaginary_start + y * resolution;

    // iterate over pixels in the current scanline, by steps of 'packet_size'.
    for(int x = 0; x < image->width(); x += packet_size)
    {
      if(mandelbrot->abortRenderingAsSoonAsPossible()) return;

      // initial values before we start iterating
      Packet pcr, pci = pci_start, pzr, pzi = pzi_start, pzr_buf;
      for(int i = 0; i < packet_size; i++) pzr[i] = pcr[i] = real_start + (x+i) * resolution;

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
        pzabs2 = pzr.cwise().square();
        pzabs2 += pzi.cwise().square();
        for(int i = 0; i < packet_size; i++) {
          if(!(pixel_diverge[i])) {
            if(pzabs2[i] > 4) {
              pixel_diverge[i] = 1;
              pzr_before_diverge[i] = pzr_previous[i];
              pzi_before_diverge[i] = pzi_previous[i];
              count_not_yet_diverged--;
            }
            else pixel_iter[i] += iter_before_test;
          }
        }
        j++;
      }
      while(j < max_iter/iter_before_test && count_not_yet_diverged);

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
      Packet_to_float escape_modulus = Packet_to_float::Zero();
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
            if(pzabs2[i] > 4) {
              pixel_diverge[i] = 1;
              escape_modulus[i] = (float)pzabs2[i];
              count_not_yet_diverged--;
            }
            else pixel_iter[i]++;
          }
        }
        j++;
      }
      while(j < iter_before_test && count_not_yet_diverged);

      /* Now we know exactly the number of iterations before divergence, and the escape modulus. */ 


      /* Third step: compute pixel colors. */

      int pixels_to_write = std::min(int(packet_size), image->width()-x);

      Packet_to_float log_log_escape_modulus = escape_modulus.template cast<float>();
      log_log_escape_modulus = log_log_escape_modulus.cwise().log().cwise().log();

      Packet_to_float shift, normalized_iter_count;

      for(int i = 0; i < packet_size; i++)
      {
        shift[i] = (-log_log_escape_modulus[i]+log_of_2log2)/log_of_2;
        shift[i]=CLAMP(shift[i],-1.f,0.f);
        normalized_iter_count[i] = pixel_iter[i] + shift[i];
        if(normalized_iter_count[i]<=1.f) normalized_iter_count[i]=1.f;
      }

      Packet_to_float log_normalized_iter_count = normalized_iter_count.cwise().log();

      Packet_to_float t;
      t = log_normalized_iter_count / log_max_iter;

      for(int i = 0; i < pixels_to_write; i++)
      {
        // Now, remember that in MandelbrotRenderThread, we did a little statistical analysis on some samples
        // to determine roughly what would be the smallest count of iterations before divergence.
        // At the beginning of the present function, we used that to compute 'tmin'.
        // Now we use it to make the gradient actually start at t=tmin. Lower values of t just give black.
        // In other words, we are ensuring that no matter what the viewpoint, the darkest part of the image
        // will always be black (at least if our statistical analysis went well).
        // In other words, we are trying to get optimal contrast. This is important as otherwise there could be
        // almost no contrast at all and an interesting viewpoint could give an almost blank image.
        if(t[i] <= tmin) t[i] = 0.f;
        else t[i] = (t[i]-tmin)/(1.f-tmin);
        t[i] = CLAMP(t[i], 0.f, 1.f);
      }

      Packet_to_float t_plus_tshift = t.cwise()+tshift;
      Packet_to_float t_plus_tshift_to_gamma = (t_plus_tshift.cwise().log() * gamma).cwise().exp();


      // Another homemade formula. It seems that some amount of gamma correction is beneficial.
      // However it must be avoided for t near zero as x^gamma is non differentiable at x=0.
      // So we just shift t a little bit to avoid 0.
      t = (t_plus_tshift_to_gamma.cwise()-tshift_to_gamma) / (tshift_plus_1_to_gamma - tshift_to_gamma);

      for(int i = 0; i < pixels_to_write; i++)
      {
        Color3 rgb;
        if(t[i] < 0.5f) {
          rgb = 2*t[i]*rgb3;
        }
        else if(t[i] < 0.75f) {
          rgb = mix(rgb2, hsv2, rgb3, hsv3, 4*t[i] - 2.f);
        }
        else {
          rgb = mix(rgb1, hsv1, rgb2, hsv2,  4*t[i] - 3.f);
        }
        
        pixel[0] = (unsigned char)(255*rgb[2]);
        pixel[1] = (unsigned char)(255*rgb[1]);
        pixel[2] = (unsigned char)(255*rgb[0]);
        pixel+=4;
      }
    }
  }
#endif
}

template void mandelbrot_render_tile<float>(
  Mandelbrot *mandelbrot,
  int interleaving_number,
  QImage *image
);

template void mandelbrot_render_tile<double>(
  Mandelbrot *mandelbrot,
  int interleaving_number,
  QImage *image
);

}
