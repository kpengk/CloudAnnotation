#pragma once

class StdMutex;
class GenericProgressCallback;

//! Efficient management of progress based on a total number of steps different than 100
/** DGM: can now be associated to a null 'callback' pointer to simplify the client code.
 **/
class NormalizedProgress {
public:
    //! Default constructor
    /** \param callback associated GenericProgressCallback instance (can be null)
        \param totalSteps total number of steps (> 0)
        \param totalPercentage equivalent percentage (> 0)
    **/
    NormalizedProgress(GenericProgressCallback* callback, unsigned totalSteps, unsigned totalPercentage = 100);

    //! Destructor
    virtual ~NormalizedProgress();

    //! Scales inner parameters so that 'totalSteps' calls of the 'oneStep' method correspond to 'totalPercentage'
    //! percents
    void scale(unsigned totalSteps, unsigned totalPercentage = 100, bool updateCurrentProgress = false);

    //! Resets progress state
    void reset();

    //! Increments total progress value of a single unit
    bool oneStep();

    //! Increments total progress value of more than a single unit
    bool steps(unsigned n);

protected:
    //! Total progress value (in percent)
    float percent_;

    //! Number of necessary calls to 'oneStep' to actually call progress callback
    unsigned step_;

    //! Percentage added to total progress value at each step
    float percent_add_;

    //! Current number of calls to 'oneStep'
    unsigned counter_;

    //! Mutex to manage concurrent calls to oneStep()
    StdMutex* mutex_;

    //! associated GenericProgressCallback
    GenericProgressCallback* progress_callback_;
};
