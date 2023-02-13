#pragma once
#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkActor.h>
#include <vtkPlanes.h>
#include <vtkBoxWidget2.h>
#include <vtkObjectFactory.h>


// 包围盒
/**
 * @brief The CBvtkBoxWidget3DCallback class
 * vtkBoxWidget2D 的观察者，响应 InteractionEvent 事件，设置当前的Planes到vtkVolume的Planes。
 */
class CBvtkBoxWidget3DCallback final : public vtkCommand {
public:
    static CBvtkBoxWidget3DCallback* New();
    vtkTypeMacro(CBvtkBoxWidget3DCallback, vtkCommand);

    CBvtkBoxWidget3DCallback();
    ~CBvtkBoxWidget3DCallback();
    vtkActor* GetActor() const;
    void SetActor(vtkActor* actor);
    void Execute(vtkObject* caller, unsigned long event_id, void* call_data) override;

private:
    vtkActor* actor_;
};


class CBvtkBoxWidget3D : public vtkBoxWidget2 {
public:
    static CBvtkBoxWidget3D* New();
    vtkTypeMacro(CBvtkBoxWidget3D, vtkBoxWidget2);
    CBvtkBoxWidget3D();
    ~CBvtkBoxWidget3D();
    void SetBounds(double bounds[6]);
};
