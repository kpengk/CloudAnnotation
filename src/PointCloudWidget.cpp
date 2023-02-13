#include "PointCloudWidget.hpp"

#include <Common/timer.hpp>

#include "GraphicalSegmentationTool.hpp"
#include "BoundingBox.hpp"
#include "InteractorStyle.hpp"

#include <vtkColorTransferFunction.h>
#include <vtkContourValues.h>
#include <vtkImageData.h>
#include <vtkMetaImageReader.h>
#include <vtkNamedColors.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkVolumeProperty.h>
#include <vtkWidgetRepresentation.h>
#include <QVTKOpenGLNativeWidget.h>

#include <vtkCellArray.h>
#include <vtkCubeAxesActor.h>
#include <vtkOutlineFilter.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkUnsignedCharArray.h>

#include <OverlayDialog.hpp>
#include <csv2/reader.hpp>
#include <spdlog/spdlog.h>

#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>

class PointCloudWidget::PointCloudWidgetPrivate {
public:
    PointCloudWidgetPrivate()
        : points_{vtkSmartPointer<vtkPoints>::New()}
        , points_colors_{vtkSmartPointer<vtkUnsignedCharArray>::New()}
        , vertices_{vtkSmartPointer<vtkCellArray>::New()}
        , poly_data_{vtkSmartPointer<vtkPolyData>::New()}
        , mapper_{vtkSmartPointer<vtkPolyDataMapper>::New()}
        , actor_{vtkSmartPointer<vtkActor>::New()}
        , axes_actor_{vtkSmartPointer<vtkCubeAxesActor>::New()}
        , renderer_{vtkSmartPointer<vtkRenderer>::New()}
        , box_widget_{vtkSmartPointer<CBvtkBoxWidget3D>::New()}
        , style_{vtkSmartPointer<InteractorStyle>::New()} {}

private:
    vtkSmartPointer<vtkPoints> points_;                   // 顶点
    vtkSmartPointer<vtkUnsignedCharArray> points_colors_; // 顶点颜色

    vtkSmartPointer<vtkCellArray> vertices_;    // 需要绘制的顶点索引
    vtkSmartPointer<vtkPolyData> poly_data_;    // 多边形数据集
    vtkSmartPointer<vtkPolyDataMapper> mapper_; // 图形数据到渲染图元的转换

    vtkSmartPointer<vtkActor> actor_;
    vtkSmartPointer<vtkCubeAxesActor> axes_actor_;

    vtkSmartPointer<vtkRenderer> renderer_;

    vtkSmartPointer<CBvtkBoxWidget3D> box_widget_;

    vtkSmartPointer<InteractorStyle> style_;

    friend class PointCloudWidget;
};

PointCloudWidget::PointCloudWidget(QWidget* parent)
    : QVTKOpenGLNativeWidget(parent)
    , d{std::make_unique<PointCloudWidgetPrivate>()} {
    // 指定(颜色)数组中每个元组的大小
    d->points_colors_->SetNumberOfComponents(3);

    initGeometry();
    initAxis();

    // 设置背景色
    const vtkColor3d bgcolor{0.18, 0.22, 0.25};
    d->renderer_->SetBackground(bgcolor.GetData());
    d->renderer_->ResetCamera();
    d->renderer_->ResetCameraClippingRange();

    // 渲染
    auto render_window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    setRenderWindow(render_window);
    renderWindow()->AddRenderer(d->renderer_);
    renderWindow()->GetInteractor()->SetInteractorStyle(d->style_);

    // 包围盒
    auto boxCallback = vtkSmartPointer<CBvtkBoxWidget3DCallback>::New();
    boxCallback->SetActor(d->actor_);

    d->box_widget_ = vtkSmartPointer<CBvtkBoxWidget3D>::New();
    d->box_widget_->SetInteractor(renderWindow()->GetInteractor());
    d->box_widget_->SetBounds(d->actor_->GetBounds());
    d->box_widget_->AddObserver(vtkCommand::InteractionEvent, boxCallback);
    d->box_widget_->Off();

    renderWindow()->Render();
}

PointCloudWidget::~PointCloudWidget() {}

void PointCloudWidget::updatePoints(const std::vector<std::array<float, 6>>& cloud) {
    timer t;
    d->points_->Resize(0);
    d->points_colors_->Resize(0);
    d->vertices_->Reset();

    // 物体
    const int max_point_count = cloud.size();

    // 读点云数据信息
    for (int n = 0; n < max_point_count; ++n) {
        const auto& value = cloud[n];

        d->points_->InsertNextPoint(value[0], value[1], value[2]); // 加入点信息

        unsigned char color[] = {value[3], value[4], value[5]};
        d->points_colors_->InsertNextTypedTuple(color);

        d->vertices_->InsertNextCell(1); // 用于渲染点集
        d->vertices_->InsertCellPoint(n);
    }

    printf("Time: %lf\n", t.elapsed());

    d->poly_data_->SetPoints(d->points_);                         // 设置点集
    d->poly_data_->SetVerts(d->vertices_);                        // 设置渲染顶点
    d->poly_data_->GetPointData()->SetScalars(d->points_colors_); //设置顶点颜色

    d->axes_actor_->SetBounds(d->points_->GetBounds());

    d->poly_data_->Modified();
    d->renderer_->ResetCamera();
    renderWindow()->Render();
}

void PointCloudWidget::setVisibleVertices(vtkSmartPointer<vtkCellArray> vertices) { 
    d->vertices_ = vertices;
    d->poly_data_->SetVerts(d->vertices_);

    d->poly_data_->Modified();
    d->renderer_->ResetCamera();
    renderWindow()->Render();
}

vtkSmartPointer<vtkCellArray> PointCloudWidget::visibleVertices() { return d->vertices_; }

vtkSmartPointer<vtkPoints> PointCloudWidget::allPoints() { return d->points_; }

vtkRenderer* PointCloudWidget::renderer() const { return d->renderer_; }

void PointCloudWidget::setAxesVisible(bool visible) {
    d->axes_actor_->SetVisibility(visible);
    renderWindow()->Render();
}

void PointCloudWidget::setBoundingBoxVisible(bool visible) {
    if (visible)
        d->box_widget_->On();
    else
        d->box_widget_->Off();
}

void PointCloudWidget::setPointSize(int value) {
    d->actor_->GetProperty()->SetPointSize(value);
    renderWindow()->Render();
}

void PointCloudWidget::mousePressEvent(QMouseEvent* event) {
    if (interaction_flags_ == INTERACT_SEND_ALL_SIGNALS) {
        if (event->button() == Qt::LeftButton) {
            emit leftButtonClicked(event->x(), event->y());
        } else if (event->button() == Qt::RightButton) {
            emit rightButtonClicked(event->x(), event->y());
        }
    }
    QWidget::mousePressEvent(event);
}

void PointCloudWidget::mouseMoveEvent(QMouseEvent* event) {
    if (interaction_flags_ == INTERACT_SEND_ALL_SIGNALS) {
        emit mouseMoved(event->x(), event->y(), event->button());
    }
    QWidget::mouseMoveEvent(event);
}

void PointCloudWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (interaction_flags_ == INTERACT_SEND_ALL_SIGNALS) {
        emit buttonReleased();
    }
    QWidget::mouseReleaseEvent(event);
}

void PointCloudWidget::setPolyline(const QPolygonF& poly) {
    polyline_ = poly;
    update();
}

void PointCloudWidget::paintGL() {
    // clean the outdated messages
    {
        std::list<MessageToDisplay>::iterator it = messages_to_display_.begin();
        qint64 current_time_sec = timer_.elapsed() / 1000;
        // ccLog::PrintDebug(QString("[paintGL] Current time: %1 s.").arg(currentTime_sec));

        while (it != messages_to_display_.end()) {
            // no more valid? we delete the message
            if (it->message_validity_sec < current_time_sec) {
                it = messages_to_display_.erase(it);
            } else {
                ++it;
            }
        }
    }

    QVTKOpenGLNativeWidget::paintGL();
    QPainter painter(this);
    if (interaction_flags_ == INTERACT_SEND_ALL_SIGNALS && !polyline_.isEmpty()) {
        drawArea(&painter);
    }
    drawMessage(&painter);

    renderWindow()->Render();
}

void PointCloudWidget::setInteractionMode(InteractionFlags flags) {
    interaction_flags_ = flags;
    switch (flags) {
        case PointCloudWidget::INTERACT_NONE: break;
        case PointCloudWidget::MODE_TRANSFORM_CAMERA: d->style_->setInteractiveEnable(true); break;
        case PointCloudWidget::INTERACT_SEND_ALL_SIGNALS: d->style_->setInteractiveEnable(false); break;
        default: break;
    }
}

void PointCloudWidget::displayNewMessage(const QString& message, MessagePosition pos, bool append,
                                     int display_max_delay_sec) {
    if (message.isEmpty()) {
        if (!append) {
            std::list<MessageToDisplay>::iterator it = messages_to_display_.begin();
            while (it != messages_to_display_.end()) {
                // same position? we remove the message
                if (it->position == pos)
                    it = messages_to_display_.erase(it);
                else
                    ++it;
            }
        } else {
            qWarning("[ccGLWindow::displayNewMessage] Appending an empty message has no effect!");
        }
        return;
    }

    // shall we replace the equivalent message(if any)?
    if (!append) {
        for (std::list<MessageToDisplay>::iterator it = messages_to_display_.begin(); it != messages_to_display_.end();) {
            // same type? we remove it
            if (it->position == pos)
                it = messages_to_display_.erase(it);
            else
                ++it;
        }
    } else {
        if (pos == SCREEN_CENTER_MESSAGE) {
            qWarning("[ccGLWindow::displayNewMessage] Append is not supported for center screen messages!");
        }
    }

    MessageToDisplay mess;
    mess.message = message;
    mess.message_validity_sec = timer_.elapsed() / 1000 + display_max_delay_sec;
    mess.position = pos;
    messages_to_display_.push_back(mess);

    // ccLog::Print(QString("[displayNewMessage] New message valid until %1 s.").arg(mess.messageValidity_sec));
}

void PointCloudWidget::drawMessage(QPainter* painter) {
    // current messages (if valid)
    if (messages_to_display_.empty()) {
        return;
    }

    int ll_currentHeight = height() - 10; // lower left
    int uc_currentHeight = 10;            // upper center

    for (const auto& message : messages_to_display_) {
        switch (message.position) {
            case LOWER_LEFT_MESSAGE: {
                painter->drawText(QPoint(10, ll_currentHeight), message.message);
                int messageHeight = QFontMetrics(painter->font()).height();
                ll_currentHeight -= (messageHeight * 5) / 4; // add a 25% margin
            } break;

            case UPPER_CENTER_MESSAGE: {
                QRect rect = QFontMetrics(painter->font()).boundingRect(message.message);
                // take the GL filter banner into account!
                int x = (width() - rect.width()) / 2;
                int y = uc_currentHeight + rect.height();
                painter->drawText(QPoint(x, y), message.message);
                uc_currentHeight += (rect.height() * 5) / 4; // add a 25% margin
            } break;

            case SCREEN_CENTER_MESSAGE: {
                QFont newFont(painter->font()); // no need to take zoom into account!
                newFont.setPointSize(12 * devicePixelRatio());
                QRect rect = QFontMetrics(newFont).boundingRect(message.message);
                // only one message supported in the screen center (for the moment ;)
                painter->drawText((width() - rect.width()) / 2, (height() - rect.height()) / 2, message.message);
            } break;
        }
    }
}

void PointCloudWidget::drawArea(QPainter* painter) {
    if (polyline_.size() <= 1) {
        return;
    }

    painter->setPen(QPen(Qt::green));
    painter->setBrush(QBrush(Qt::green, Qt::Dense4Pattern));
    painter->drawPolygon(polyline_);
}

void PointCloudWidget::initGeometry() {
    // 映射器负责将几何体推入图形库。如果定义了标量或其他属性，它还可以进行颜色映射。
    d->mapper_->SetInputData(d->poly_data_);

    // actor是一种分组机制：除了几何体（mapper），它还具有属性、变换矩阵和/或纹理贴图
    d->actor_->SetMapper(d->mapper_);
    // d->actor_->GetProperty()->SetColor(0.87, 0.87, 0.87);
    d->actor_->GetProperty()->SetPointSize(1);

    d->renderer_->AddActor(d->actor_);

    // Draw bounding box
    vtkSmartPointer<vtkOutlineFilter> out_line_filter = vtkSmartPointer<vtkOutlineFilter>::New();
    out_line_filter->SetInputData(d->poly_data_);

    vtkSmartPointer<vtkPolyDataMapper> out_line_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    out_line_mapper->SetInputConnection(out_line_filter->GetOutputPort());

    vtkSmartPointer<vtkActor> outlineActor = vtkSmartPointer<vtkActor>::New();
    outlineActor->SetMapper(out_line_mapper);
    outlineActor->GetProperty()->SetColor(0.8, 0.8, 0.8);
    d->renderer_->AddActor(outlineActor);
}

void PointCloudWidget::initAxis() {
    d->axes_actor_->SetCamera(d->renderer_->GetActiveCamera());
    // 设置x、y、z轴的起始和终止值
    d->axes_actor_->SetBounds(d->points_->GetBounds());
    // 设置坐标轴线的宽度
    d->axes_actor_->GetXAxesLinesProperty()->SetLineWidth(0.5);
    d->axes_actor_->GetYAxesLinesProperty()->SetLineWidth(0.5);
    d->axes_actor_->GetZAxesLinesProperty()->SetLineWidth(0.5);
    // 设置标题和标签文本的屏幕大小。默认值为10.0。
    d->axes_actor_->SetScreenSize(6);
    // 指定标签与轴之间的距离。默认值为20.0。
    d->axes_actor_->SetLabelOffset(5);
    //显示坐标轴
    d->axes_actor_->SetVisibility(true);
    //指定一种模式来控制轴的绘制方式
    d->axes_actor_->SetFlyMode(0);
    //设置惯性因子，该惯性因子控制轴切换位置的频率（从一个轴跳到另一个轴）
    // d->axes_actor_->SetInertia(1);

    //网格设置
    //开启x、y、z轴的网格线绘制
    d->axes_actor_->DrawXGridlinesOn();
    d->axes_actor_->DrawYGridlinesOn();
    d->axes_actor_->DrawZGridlinesOn();
    //设置x、y、z轴的内部网格线不绘制
    d->axes_actor_->SetDrawXInnerGridlines(false);
    d->axes_actor_->SetDrawYInnerGridlines(false);
    d->axes_actor_->SetDrawZInnerGridlines(false);
    //设置x、y、z轴网格线的颜色
    d->axes_actor_->GetXAxesGridlinesProperty()->SetColor(0.5, 0.5, 0.5);
    d->axes_actor_->GetYAxesGridlinesProperty()->SetColor(0.5, 0.5, 0.5);
    d->axes_actor_->GetZAxesGridlinesProperty()->SetColor(0.5, 0.5, 0.5);
    //指定网格线呈现的样式
    d->axes_actor_->SetGridLineLocation(2);

    //刻度的设置
    //不显示x、y、z轴的次刻度
    d->axes_actor_->XAxisMinorTickVisibilityOff();
    d->axes_actor_->YAxisMinorTickVisibilityOff();
    d->axes_actor_->ZAxisMinorTickVisibilityOff();
    //设置刻度标签的显示方式(参数1为false，刻度标签按0-200000显示；为true时，按0-200显示)
    d->axes_actor_->SetLabelScaling(false, 0, 0, 0);
    //设置刻度线显示的位置(内部、外部、两侧)
    d->axes_actor_->SetTickLocation(1);

    d->renderer_->AddActor(d->axes_actor_);
}
