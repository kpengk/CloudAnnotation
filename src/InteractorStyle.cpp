#include "InteractorStyle.hpp"

vtkStandardNewMacro(InteractorStyle);

InteractorStyle::InteractorStyle()
    : vtkInteractorStyleTrackballCamera()
    , interactive_{true} {}

InteractorStyle::~InteractorStyle() {}

void InteractorStyle::OnMouseMove() {
    if (interactive_) {
        vtkInteractorStyleTrackballCamera::OnMouseMove();
    }
}

void InteractorStyle::OnLeftButtonDown() {
    if (interactive_) {
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }
}

void InteractorStyle::OnLeftButtonUp() {
    if (interactive_) {
        vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    }
}

void InteractorStyle::OnMiddleButtonDown() {
    if (interactive_) {
        vtkInteractorStyleTrackballCamera::OnMiddleButtonDown();
    }
}

void InteractorStyle::OnMiddleButtonUp() {
    if (interactive_) {
        vtkInteractorStyleTrackballCamera::OnMiddleButtonUp();
    }
}

void InteractorStyle::OnRightButtonDown() {
    if (interactive_) {
        vtkInteractorStyleTrackballCamera::OnRightButtonDown();
    }
}

void InteractorStyle::OnRightButtonUp() {
    if (interactive_) {
        vtkInteractorStyleTrackballCamera::OnRightButtonUp();
    }
}

void InteractorStyle::OnMouseWheelForward() {
    if (interactive_) {
        vtkInteractorStyleTrackballCamera::OnMouseWheelForward();
    }
}

void InteractorStyle::OnMouseWheelBackward() {
    if (interactive_) {
        vtkInteractorStyleTrackballCamera::OnMouseWheelBackward();
    }
}

void InteractorStyle::setInteractiveEnable(bool enable) { interactive_ = enable; }
