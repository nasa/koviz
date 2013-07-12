#include "thread.h"

#include <cmath>

static bool intLessThan(int a, int b)
{
    return a < b;
}


void Thread::_do_stats()
{
    if ( jobs.size() == 0 ) {
        return;
    }

    qSort(jobs.begin(),jobs.end(),jobAvgTimeGreaterThan);

    // Frequency (cycle time) of thread
    freq = 1.0e20;
    foreach ( Job* job, jobs ) {
        if ( job->freq() < 0.000001 ) continue;
        if ( job->freq() < freq ) {
            freq = job->freq();
        }
    }
    if (freq == 1.0e20) {
        freq = 0.0;
    }

    Job* job0 = jobs.at(0);
    int npoints = job0->npoints();
    double* t = job0->timestamps();
    double tnext = t[0] + freq;
    double sum_time = 0.0;
    double frame_time = 0.0;
    max_runtime = 0.0;
    int last_frameidx = 0 ;
    bool is_frame_change = true;
    int nframes = 0 ;
    for ( int tidx = 0; tidx < npoints; ++tidx) {

        if ( freq < 0.000001 ) {
            is_frame_change = true;
        } else if ( tnext < t[tidx] ) {
            tnext += freq;
            if ( frame_time/1000000.0 > freq ) {
                num_overruns++;
            }
            is_frame_change = true;
        }

        if ( is_frame_change ) {
            if ( frame_time > max_runtime ) {
                max_runtime = frame_time;
                tidx_max_runtime = last_frameidx;
            }
            _frameidx2runtime[last_frameidx] = frame_time/1000000.0;
            sum_time += frame_time;
            nframes++;
            frame_time = 0.0;
            last_frameidx = tidx;
            is_frame_change = false;
        }

        foreach ( Job* job, jobs ) {
            frame_time += job->runtime()[tidx];
        }
    }
    max_runtime /= 1000000.0;
    avg_runtime = sum_time/nframes/1000000.0;
    if ( freq > 0.0000001 ) {
        avg_load = 100.0*avg_runtime/freq;
        max_load = 100.0*max_runtime/freq;
    }

    // Stddev
    foreach ( int tidx, _frameidx2runtime.keys() ) {
        double rt = _frameidx2runtime[tidx];
        double vv = (avg_runtime-rt)*(avg_runtime-rt);
        stdev += vv;
    }
    stdev = sqrt(stdev/nframes);

}

double Thread::runtime(int tidx) const
{
    double rt = -1.0;

    for ( int ii = tidx; ii >= 0 ; --ii) {
        if ( _frameidx2runtime.contains(ii) ) {
            rt = _frameidx2runtime[ii];
            break;
        }
    }

    return rt;
}

double Thread::runtime(double timestamp) const
{
    int tidx = getIndexAtTime(jobs.at(0)->npoints(),
                              jobs.at(0)->timestamps(),
                              timestamp);

    double rt = runtime(tidx);
    return rt;
}




int Thread::nframes() const
{
    return _frameidx2runtime.keys().size();
}

double Thread::avg_job_runtime(Job *job) const
{
    return job->avg_runtime()*job->npoints()/nframes();
}

//
// Returns a percent average load of job on thread
// A load of 0.0 can mean it's negligible
// If there's only one job on a thread, load is 100% if there's any load
//
double Thread::avg_job_load(Job *job) const
{
    double load = 0.0;

    if ( avg_runtime > 0.000001 ) {
        if ( jobs.length() == 1 ) {
            // Fix round off error.
            // If the job has an average above zero
            // and the thread has a single job
            // this job took 100%, so force it to 100.0
            load = 100.0;
        } else {
            load = 100.0*avg_job_runtime(job)/avg_runtime;
        }
    }

    return load;

}

Threads::Threads(const QList<Job*>& jobs) : _jobs(jobs)
{
    foreach ( Job* job, _jobs ) {

        int tid = job->thread_id();
        if ( ! _ids.contains(tid) ) {
            _ids.append(tid);
            Thread* thread = new Thread();
            thread->thread_id = tid;
            _threads.insert(tid,thread);
        }

        Thread* thread = _threads.value(tid);
        thread->jobs.append(job);
    }

    qSort(_ids.begin(),_ids.end(),intLessThan);

    foreach ( Thread* thread, _threads.values() ) {
        thread->_do_stats();
    }

}

Threads::~Threads()
{
    foreach ( Thread* thread, _threads.values() ) {
        delete thread;
    }
}

Thread Threads::get(int id) const
{
    return *(_threads[id]);
}

QList<Thread> Threads::list() const
{
    QList<Thread> list ;

    foreach ( Thread* thread, _threads.values() ) {
        list.append(*thread);
    }

    return list;
}

QList<int> Threads::ids() const
{
    return _ids;
}
