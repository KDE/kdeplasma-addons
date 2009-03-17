/**********************************************************************/
/*   TimeMon (c)  1994  Helmut Maierhofer                             */
/*   KDE-ified M. Maierhofer 1998                                     */
/**********************************************************************/

/*
 * sample.cc
 *
 * Definitions for the system dependent sampling class.
 */


#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <fstream>
#include <stdio.h>

#ifdef __osf__
#include <sys/table.h>
#elif defined(USE_SOLARIS)
#include <kstat.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/swap.h>
#endif

#include <fcntl.h>

#include <qwidget.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "timemon.h"
#include "sample.h"

// -- global definitions -------------------------------------------------

#if defined(__osf__) || defined(USE_SOLARIS)
extern "C" int getpagesize();   // argh, have to define prototype!
#endif

#ifdef __linux__
// -- global constants ---------------------------------------------------
#define STAT_NAME "/proc/stat"
#define MEMINFO_NAME "/proc/meminfo"
#endif

// -- KSample::Sample definition -----------------------------------------

// Fill sample with some default values (e.g. used in preview widget
// in configuration)
void KSample::Sample::fill(unsigned scale)
{
  user = scale * 40; user /= 100;
  nice = scale * 25; user /= 100;
  kernel = scale * 10; kernel /= 100;
  iowait = scale * 5; iowait /= 100;
  cpus = 1;
  buffers = scale * 20; buffers /= 100;
  used = scale * 30; used /= 100;
  cached = scale * 20; cached /= 100;
  sused = scale * 25; sused /= 100;
}

// -- KSample definition -------------------------------------------------

// Initialise the member variables and try to open the standard files in
// the proc filesystem; for other platforms perform equivalent initialisation
KSample::KSample(KTimeMon *t, bool a, unsigned p, unsigned s, unsigned c) :
  timemon(t),
#ifdef __linux__
  memFD(-1), statFD(-1),
#elif defined (USE_SOLARIS)
  kc(0), warned(false),
#endif
  pageScale(p), swapScale(s), cxScale(c), autoscale(a)
{
#ifdef __linux__
  memstats[0].name = "SwapTotal:";
  memstats[0].stat = &sample.stotal;
  memstats[1].name = "MemTotal:";
  memstats[1].stat = &sample.mtotal;
  memstats[2].name = "MemFree:";
  memstats[2].stat = &sample.free;
  memstats[3].name = "Buffers:";
  memstats[3].stat = &sample.buffers;
  memstats[4].name = "Cached:";
  memstats[4].stat = &sample.cached;
  memstats[5].name = "SwapFree:";
  memstats[5].stat = &sample.sfree;
  memstats[6].name = 0;
  memstats[6].stat = 0;

  if ((memFD = open(MEMINFO_NAME, O_RDONLY)) == -1) {
    KMessageBox::error((QWidget *)timemon,
                       i18n("Unable to open the file '%1'. The diagnostics are:\n%2.\n"
                            "This file is required to determine current memory usage.\n"
                            "Maybe your proc filesystem is non-Linux standard?").arg(MEMINFO_NAME).arg(strerror(errno)));
    exit(1);
  }

  fcntl( memFD,F_SETFD, FD_CLOEXEC );

  if ((statFD = open(STAT_NAME, O_RDONLY)) == -1) {
    KMessageBox::error((QWidget *)timemon,
                       i18n("Unable to open the file '%1'. The diagnostics are:\n%2.\n"
                            "This file is required to determine current system info. "
                            "Maybe your proc filesystem is non-Linux standard?").arg(MEMINFO_NAME).arg(strerror(errno)));
    exit(1);
  }

  fcntl( statFD,F_SETFD, FD_CLOEXEC );

#elif defined (USE_SOLARIS)
  if ((kc = kstat_open()) == 0) {
    KMessageBox::error((QWidget *)timemon, i18n("Unable to initialize the 'kstat' library. "
                                     "This library is used for accessing kernel information. "
                                     "The diagnostics are:\n%1.\n"
                                     "Are you really running Solaris? "
                                     "Please contact the maintainer at mueller@kde.org "
                                     "who will try to figure out what went wrong.").arg(strerror(errno)));
    exit(1);
  }
#endif

#if defined(USE_SOLARIS) || defined(__osf__)
  pagesPerMB = (1024*1024) / getpagesize();
  if (pagesPerMB == 0) pagesPerMB = 1; // paranoia sanity check
#endif

  readSample();
  updateSample();
}

// Get rid of the resources we acquired in the constructor.
KSample::~KSample()
{
#ifdef __linux__
  close(memFD);
  close(statFD);
#elif defined (USE_SOLARIS)
  if (kc != 0) kstat_close(kc);
#endif
}

// Set the appropriate scaling parameters
void KSample::setScaling(bool a, unsigned p, unsigned s, unsigned c)
{
    autoscale = a;
    pageScale = p;
    swapScale = s;
    cxScale = c;
}

// -----------------------------------------------------------------------------
// Show a message box with the given message and terminate the application.

void KSample::fatal(const QString& msg)
{
    timemon->stop();

    KMessageBox::error((QWidget *)timemon, msg);
//    exit(1);
}


// -----------------------------------------------------------------------------
// Show a message box with the given message and don't terminate the app ;-)

void KSample::nonfatal(const QString& msg)
{
    timemon->stop();

    KMessageBox::sorry((QWidget *)timemon, msg);
    timemon->cont();
}


// -----------------------------------------------------------------------------
// Read a new sample from the files or whatever resource the OS implements

/* For 2.5 kernels */
static inline void
scan_one(const char* buff, const char *key, unsigned long int* val)
{
        const char *b = strstr(buff, key);
        if (b) {
                b = strstr(b, " ");
                if (b)
                        sscanf(b, " %lu", val);
        }
}

void KSample::readSample()
{
    sample.cpus = 0;            // just to make sure...

#ifdef __linux__                // linux makes it simple: use the /proc if
    int l;
    char buffer[4096];

    lseek(memFD, 0, 0);
    if ((l = read(memFD, buffer, sizeof(buffer) - 1)) < 0)
    {
        fatal(i18n("Unable to read the memory usage file '%1'.\n"
                   "The diagnostics are: %2").arg(MEMINFO_NAME).arg(strerror(errno)));
    }
    buffer[l] = '\0';
    l = 0;
    char *p;
    while (memstats[l].name != 0) {
        p = strstr(buffer, memstats[l].name);
        if (p == 0 ||
            sscanf(p + strlen(memstats[l].name), "%lu kB", memstats[l].stat) < 1)
            fatal(i18n("The memory usage file '%1' seems to use a "
                       "different file format than expected.\n"
                       "Maybe your version of the proc filesystem is "
                       "incompatible with supported versions. "
		       "Please contact the developer at http://bugs.kde.org/ who will try to sort this out.").arg(MEMINFO_NAME));
        l++;
    }

    if ( ( p = strstr(buffer, "Slab:") ) ) {
        unsigned long slabs;
        sscanf(p + 5, "%lu kB", &slabs);
        sample.mkernel = slabs;
    }

    /* read the data for the cpu stats */
    lseek(statFD, 0, 0);
    if ((l = read(statFD, buffer, sizeof(buffer)-1)) < 0)
        fatal(i18n("Unable to read the system usage file '%1'.\n"
                   "The diagnostics are: %2").arg(STAT_NAME).arg(strerror(errno)));

    buffer[l] = '\0';

    bool ok = (sscanf(buffer, "cpu %lu %lu %lu %lu %lu", &sample.user,
                      &sample.nice, &sample.kernel, &sample.idle, &sample.iowait) == 5);

    if (ok) {
        for (l = 0; l < MAX_CPU; l++) { // get individual stat for SMP machines
            char cpuname[10];
            sprintf(cpuname, "cpu%d", l);

            if ((p = strstr(buffer, cpuname)) == NULL) break;

            unsigned long u, n, k, i;
            ok = sscanf(p, "cpu%*d %lu %lu %lu %lu", &u, &n, &k, &i);
            if (!ok) break;

            sample.smptotal[l] = u+n+k+i;
            sample.smpbusy[l] = sample.smptotal[l] - i;
        }
    }
    sample.cpus = l;

#elif defined(__osf__)          // in OSF/2, we can use table()

    QString msg = i18n("Unable to obtain system information.\n"
                       "The table(2) system call returned an error "
                       "for table %1.\n"
                       "Please contact the maintainer at mueller@kde.org "
		       "who will try to figure out what went wrong.");

    struct tbl_sysinfo sysinfo;
    if (table(TBL_SYSINFO, 0, &sysinfo, 1, sizeof(sysinfo)) != 1)
        fatal(msg.arg("TBL_SYSINFO"));

    sample.user = sysinfo.si_user;
    sample.nice = sysinfo.si_nice;
    sample.kernel = sysinfo.si_sys;
    sample.iowait = sysinfo.wait;
    sample.idle = sysinfo.si_idle;

    struct tbl_vmstats vmstats;
    if (table(TBL_VMSTATS, 0, &vmstats, 1, sizeof(vmstats)) != 1)
        fatal(msg.arg("TBL_VMSTATS"));

    sample.mtotal = vmstats.free_count + vmstats.active_count +
        vmstats.inactive_count + vmstats.wire_count;
    sample.free = vmstats.free_count;
    sample.buffers = vmstats.inactive_count; // pages not used for some time
    sample.cached = vmstats.wire_count; // kernel/driver memory

    struct tbl_swapinfo swapinfo;
    if (table(TBL_SWAPINFO, -1, &swapinfo, 1, sizeof(swapinfo)) != 1)
        fatal(msg.arg("TBL_SWAPINFO"));

    sample.stotal = swapinfo.size;
    sample.sfree = swapinfo.free;

#elif defined(USE_SOLARIS)
    kstat_t *ksp;

    sample.cpus = 0;
    for (ksp = kc->kc_chain; ksp != 0; ksp = ksp->ks_next) {
        if (strncmp(ksp->ks_name, "cpu_stat", 8) != 0) continue;
        sample.cpus++;
    }

    if (sample.cpus == 0)
        fatal(i18n("Unable to find any entries for CPU statistics "
                   "in the 'kstat' library. Are you running a non-standard "
                   "version of Solaris?\n"
                   "Please contact the maintainer via http://bugs.kde.org/ who will try to sort this out."));

    sample.user = sample.nice = sample.kernel = sample.iowait = sample.idle = 0;
    sample.stotal = sample.sfree = 0;

    int cpus = 0;
    for (ksp = kc->kc_chain; ksp != 0; ksp = ksp->ks_next) {
        if (strncmp(ksp->ks_name, "cpu_stat", 8) != 0) continue;
        cpus++;

        cpu_stat_t cstat;
        if (kstat_read(kc, ksp, 0) == -1 || // update from kernel
            kstat_read(kc, ksp, &cstat) == -1) // and read into buffer
            fatal(i18n("Unable to read the CPU statistics entry "
                       "from the 'kstat' library. The diagnostics are '%1'.\n"
                       "Please contact the maintainer via http://bugs.kde.org/ who will try to sort this out.").arg(strerror(errno)));

        // fields are: idle user kernel iowait (no nice info?)
        sample.user += cstat.cpu_sysinfo.cpu[1] / sample.cpus;
        sample.nice += 0;
        sample.kernel += cstat.cpu_sysinfo.cpu[2] / sample.cpus;
        sample.iowait += cstat.cpu_sysinfo.cpu[3] / sample.cpus;
        sample.idle += cstat.cpu_sysinfo.cpu[0] / sample.cpus;
    }

    if (cpus != sample.cpus)
        fatal(i18n("The number of CPUs appears to have changed at "
                   "very short notice, or the 'kstat' library returns "
                   "inconsistent results (%1 vs. %2 CPUs).\n"
                   "Please contact the maintainer via http://bugs.kde.org/ who will try to sort this out.").arg(sample.cpus).arg(cpus));

    // availrmem = pages of core for user-proc ( == physmem - kernelmem)
    // freemem = no of free pages
    // physmem == total mem in 4KB blocks

    errno = 0;
    if ((ksp = kstat_lookup(kc, "unix", -1, "system_pages")) == 0 ||
        kstat_read(kc, ksp, 0) == -1)
        fatal(i18n("Unable to read the memory statistics entry "
                   "from the 'kstat' library. The diagnostics are '%1'\n"
                   "You might want to contact the maintainer at "
                   "http://bugs.kde.org/ who will try to sort this out.").arg(strerror(errno)));

    int i;
    unsigned long physmem = 0, freemem = 0, availrmem = 0;

    kstat_named_t *kn = (kstat_named_t *)ksp->ks_data;
    for (i = 0; i < (int) ksp->ks_ndata; i++) {
        if (strcmp(kn->name, "physmem") == 0) physmem = kn->value.ul;
        else if (strcmp(kn->name, "freemem") == 0) freemem = kn->value.ul;
        else if (strcmp(kn->name, "availrmem") == 0) availrmem = kn->value.ul;
        kn++;
    }

    if (physmem == 0)           // sanity check, this should always be > 0
        fatal(i18n("There seems to be a problem with KTimeMon's handling "
                   "of the 'kstat' library: 0 bytes of physical memory determined!\n"
                   "Free memory is %1, available memory is %2.\n"
                   "Please contact the maintainer at mueller@kde.org who will try to sort this out.").arg(freemem).arg(availrmem));

    sample.mtotal = physmem;
    sample.free = freemem;
    sample.buffers = 0;
    sample.cached = physmem - availrmem; // memory used by the kernel

    int swapentries;
    if ((swapentries = swapctl(SC_GETNSWP, 0)) == -1)
        fatal(i18n("Unable to determine the number of "
                   "swap spaces. The diagnostics are '%1'.\n"
                   "Please contact the maintainer at http://bugs.kde.org/ who will try to sort this out.").arg(strerror(errno)));

    if (swapentries != 0) {
                                // 2* to get some space for padding??
        swaptbl_t *stbl = (swaptbl_t *) malloc(2*sizeof(int) + swapentries *
                                               sizeof(struct swapent));
        if (stbl == 0)
            fatal(i18n("KTimeMon ran out of memory while "
                       "trying to determine the swap usage.\n"
                       "Attempted to allocate %1 bytes of memory (2 * %2 + %3 * %4).\n"
                       "Please contact the maintainer at http://bugs.kde.org/ who will try to sort this out.")
                  .arg(2 * sizeof(int) + swapentries * sizeof(struct swapent))
                  .arg(sizeof(int)).arg(swapentries).arg(sizeof(struct swapent)));

        char path[1024];
        stbl->swt_n = swapentries;
        for (i = 0; i < swapentries; i++) stbl->swt_ent[i].ste_path = path;

        if ((swapentries = swapctl(SC_LIST, stbl)) == -1)
            fatal(i18n("Unable to determine the swap usage.\n"
                       "The diagnostics are '%1'.\n"
                       "Please contact the maintainer at http://bugs.kde.org/ who will try to sort this out.").arg(strerror(errno)));


        if (!warned && swapentries != stbl->swt_n) {
            warned = true;
            nonfatal(i18n("Information was requested for "
                          "%1 swap spaces, but only %2 swap entries were returned.\n"
                          "KTimeMon will attempt to continue.\n"
                          "Please contact the maintainer at http://bugs.kde.org/ who will try to sort this out.").arg(stbl->swt_n).arg(swapentries));
        }

        for (i = 0; i < swapentries; i++) {
            sample.stotal += stbl->swt_ent[i].ste_pages;
            sample.sfree += stbl->swt_ent[i].ste_free;
        }

        free(stbl);
    }

#else
#warning This type of system is not supported
    sample.stotal = sample.sfree = 0;
#endif

    sample.cputotal =
      sample.user + sample.nice + sample.kernel + sample.iowait + sample.idle;
    sample.used = sample.mtotal - sample.mkernel - sample.free - sample.buffers - sample.cached;
    sample.sused = sample.stotal - sample.sfree;
}

// Read a new sample after copying the old one.
void KSample::updateSample()
{
    oldSample = sample;
    readSample();
}

// Convert v to a value representing megabytes.
inline void KSample::makeMBytes(unsigned long &v)
{
#ifdef __linux__
    v /= 1024;                  // can it be simpler ;-)
#elif defined (__osf__) || defined(USE_SOLARIS)
    v /= pagesPerMB;
#endif
}

// Return unscaled sample
KSample::Sample KSample::getRawSample()
{
    Sample diff = sample;

    diff.cputotal -= oldSample.cputotal;

    diff.user -= oldSample.user;
    diff.nice -= oldSample.nice;
    diff.kernel -= oldSample.kernel;
    diff.iowait -= oldSample.iowait;

    for (int i = 0; i < diff.cpus; i++) {
        diff.smptotal[i] -= oldSample.smptotal[i];
        diff.smpbusy[i] -= oldSample.smpbusy[i];
    }

    return diff;
}

// Better scaling, round according to first decimal
inline unsigned long KSample::doScale(unsigned long value, unsigned scale1,
                                      unsigned long scale2)
{
    if (scale2 == 0) scale2 = (unsigned long)~0; // avoid SEGVs

    unsigned long v = value * scale1 * 10;
    v /= scale2;
    unsigned r = v % 10;
    v /= 10;
    if (r > 4) v++;
    return v;
}

// Provide the difference from the last to the current sample, scale it
// and return it.
KSample::Sample KSample::getSample(unsigned scale)
{
    Sample s = getRawSample();

    s.user = doScale(s.user, scale, s.cputotal);
    s.nice = doScale(s.nice, scale, s.cputotal);
    s.kernel = doScale(s.kernel, scale, s.cputotal);
    s.iowait = doScale(s.iowait, scale, s.cputotal);

    for (int i = 0; i < s.cpus; i++)
        s.smpbusy[i] = doScale(s.smpbusy[i], scale, s.smptotal[i]);

    s.cached = doScale(s.cached, scale, s.mtotal);
    s.buffers = doScale(s.buffers, scale, s.mtotal);
    s.used = doScale(s.used, scale, s.mtotal);
    s.mkernel = doScale(s.mkernel, scale, s.mtotal);
    makeMBytes(s.mtotal);

    s.sused = doScale(s.sused, scale, s.stotal);
    makeMBytes(s.stotal);

    return s;
}

