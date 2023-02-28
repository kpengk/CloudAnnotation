#include "NormalizedProgress.hpp"
#include "GenericProgressCallback.hpp"

// system
#include <cassert>
#include <mutex>

// Use a class "wrapper" to avoid having to include <mutex> in header
class StdMutex : public std::mutex {};

NormalizedProgress::NormalizedProgress(GenericProgressCallback* callback, unsigned totalSteps, unsigned totalPercentage)
    : percent_(0)
    , step_(1)
    , percent_add_(1.0f)
    , counter_(0)
    , mutex_(new StdMutex)
    , progress_callback_(callback) {
    scale(totalSteps, totalPercentage);
}

NormalizedProgress::~NormalizedProgress() {
    delete mutex_;
    mutex_ = nullptr;
}

void NormalizedProgress::scale(unsigned totalSteps, unsigned totalPercentage, bool updateCurrentProgress) {
    if (!progress_callback_) {
        return;
    }

    if (totalSteps == 0 || totalPercentage == 0) {
        step_ = 1;
        percent_add_ = 0;
        return;
    }

    if (totalSteps >= 2 * totalPercentage) {
        step_ = static_cast<unsigned>(ceil(static_cast<float>(totalSteps) / totalPercentage));
        assert(step_ != 0 && step_ < totalSteps);
        percent_add_ = static_cast<float>(totalPercentage) / (totalSteps / step_);
    } else {
        step_ = 1;
        percent_add_ = static_cast<float>(totalPercentage) / totalSteps;
    }

    mutex_->lock();
    if (updateCurrentProgress) {
        percent_ = (static_cast<float>(totalPercentage) / totalSteps) * counter_;
    } else {
        counter_ = 0;
    }
    mutex_->unlock();
}

void NormalizedProgress::reset() {
    mutex_->lock();
    percent_ = 0;
    counter_ = 0;
    if (progress_callback_) {
        progress_callback_->update(0);
    }
    mutex_->unlock();
}

bool NormalizedProgress::oneStep() {
    if (!progress_callback_) {
        return true;
    }

    mutex_->lock();
    if ((++counter_ % step_) == 0) {
        percent_ += percent_add_;
        progress_callback_->update(percent_);
    }
    mutex_->unlock();

    return !progress_callback_->isCancelRequested();
}

bool NormalizedProgress::steps(unsigned n) {
    if (!progress_callback_) {
        return true;
    }

    mutex_->lock();
    counter_ += n;
    const unsigned d1 = counter_ / step_;
    const unsigned d2 = (counter_ + n) / step_;
    if (d2 != d1) {
        percent_ += static_cast<float>(d2 - d1) * percent_add_;
        progress_callback_->update(percent_);
    }
    mutex_->unlock();

    return !progress_callback_->isCancelRequested();
}
