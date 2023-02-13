#include "BoundingBox.hpp"
#include <vtkBoxRepresentation.h>
#include <vtkMapper.h>

vtkStandardNewMacro(CBvtkBoxWidget3DCallback);

 CBvtkBoxWidget3DCallback::CBvtkBoxWidget3DCallback()
    : actor_{} {}

 CBvtkBoxWidget3DCallback::~CBvtkBoxWidget3DCallback() = default;

 vtkActor* CBvtkBoxWidget3DCallback::GetActor() const { return actor_; }

void CBvtkBoxWidget3DCallback::SetActor(vtkActor* actor) { actor_ = actor; }

void CBvtkBoxWidget3DCallback::Execute(vtkObject* caller, unsigned long event_id, void* call_data) {
    auto* const box_widget = vtkBoxWidget2::SafeDownCast(caller);
    auto* const box_representation = vtkBoxRepresentation::SafeDownCast(box_widget->GetRepresentation());
    box_representation->SetInsideOut(1);
    vtkNew<vtkPlanes> planes_;
    box_representation->GetPlanes(planes_);
    actor_->GetMapper()->SetClippingPlanes(planes_);
}

vtkStandardNewMacro(CBvtkBoxWidget3D);

CBvtkBoxWidget3D::CBvtkBoxWidget3D() {
    CreateDefaultRepresentation();
    GetRepresentation()->SetPlaceFactor(1);// Default is 0.5
}

CBvtkBoxWidget3D::~CBvtkBoxWidget3D() {}

void CBvtkBoxWidget3D::SetBounds(double bounds[6]) { GetRepresentation()->PlaceWidget(bounds); }
