#pragma once
#include <vector>

template <typename T> class StepRecorder {
public:
    StepRecorder()
        : iter_{steps_.end()} {}

    void reset(const T& op) {
        steps_.clear();
        steps_.push_back(op);
        iter_ = steps_.begin();
    }

    void reset(T&& op) {
        steps_.clear();
        steps_.emplace_back(std::forward(op));
        iter_ = steps_.begin();
    }

    bool can_undo() const { return iter_ != steps_.begin() && iter_ != steps_.end(); }

    bool can_redo() const { return iter_ != steps_.end() && iter_ + 1 != steps_.end(); }

    std::vector<T>::iterator undo() {
        if (can_undo()) {
            return --iter_;
        } else {
            return iter_;
        }
    }

    std::vector<T>::iterator redo() {
        if (can_redo()) {
            return ++iter_;
        } else {
            return iter_;
        }
    }

    std::vector<T>::iterator to_begin() {
        if (can_undo()) {
            iter_ = steps_.begin();
        }
        return iter_;
    }

    std::vector<T>::iterator push(const T& op) {
        steps_.erase(iter_ + 1, steps_.end());
        steps_.push_back(op);
        iter_ = --steps_.end();
        return iter_;
    }

    std::vector<T>::iterator push(T&& op) {
        steps_.erase(iter_ + 1, steps_.end());
        steps_.emplace_back(op);
        iter_ = --steps_.end();
        return iter_;
    }

    std::vector<T>::iterator current() const { return iter_; }

    bool is_initial_state() const { return iter_ == steps_.begin() || iter_ == steps_.end(); }

private:
    std::vector<T> steps_;
    std::vector<T>::iterator iter_;
};
