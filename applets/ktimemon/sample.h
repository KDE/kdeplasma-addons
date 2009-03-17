/* -*- C++ -*- */

/**********************************************************************/
/*   TimeMon (c)  1994  Helmut Maierhofer			      */
/*   KDE-ified M. Maierhofer 1998                                     */
/**********************************************************************/

/*
 * sample.h
 *
 * Definitions for the system dependent sampling class (currently relies
 * on the linux /proc filesystem).
 */

#ifndef SAMPLE_H
#define SAMPLE_H

// -- global constants ---------------------------------------------------

#define MAX_CPU 16		// max number of CPUS in an SMP machine
				// we get the status for

// -- forward declaration ------------------------------------------------
class KTimeMon;

#ifdef USE_SOLARIS
struct kstat_ctl;
#endif

// -- class declaration --------------------------------------------------

/*
 * KSample
 *
 * This class is responsible for reading the /proc file system and parsing
 * the system information.
 */
class KSample {
public:
    // -- Sample declaration -----------------------------------------------
    struct Sample {
        unsigned long cputotal;
        unsigned long user, nice, kernel, iowait, idle;
        int cpus;
        unsigned long smptotal[MAX_CPU], smpbusy[MAX_CPU];
        unsigned long mtotal, free, buffers, cached, mkernel, used;
        unsigned long stotal, sused, sfree;

        void fill(unsigned scale);	// fill sample with some fake values
    };

    struct MemStats {
        const char *name;
        unsigned long *stat;
    };

    KSample(KTimeMon *timemon, bool autoScale, unsigned pageScale,
            unsigned swapScale, unsigned ctxScale);
    virtual ~KSample();

    void setScaling(bool autoScale, unsigned pageScale,
                    unsigned swapScale, unsigned ctxScale);

    Sample getSample(unsigned scale); // returns the current sample
    Sample getRawSample();	// returns unscaled sample
    void updateSample();		// updates the internally stored sample

private:
    void readSample();		// reads a new sample from /proc
    unsigned long doScale(unsigned long value, unsigned scale1,
                                 unsigned long scale2);
				// converts pages to MB
    void makeMBytes(unsigned long &pages);

    void fatal(const QString& msg);
    void nonfatal(const QString& msg);

    KTimeMon *timemon;
#ifdef __linux__
    int memFD, statFD;
#elif defined(USE_SOLARIS)
    struct kstat_ctl *kc;
    bool warned;
#endif
#if defined(USE_SOLARIS) || defined(__osf__)
    unsigned long pagesPerMB;
#endif
    Sample sample, oldSample;
    unsigned pageScale, swapScale, cxScale;
    bool autoscale;
    struct MemStats memstats[7];
};

#endif // SAMPLE_H
