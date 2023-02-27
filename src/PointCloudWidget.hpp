#pragma once
#include <vtkSmartPointer.h>
#include <QVTKOpenGLNativeWidget.h>

#include <memory>
#include <vector>
#include <array>

#include <QPolygonF>
#include <QElapsedTimer>

class vtkRenderer;
class vtkPoints;
class vtkCellArray;
class AbstractPointCloudContainer;

class PointCloudWidget : public QVTKOpenGLNativeWidget {
    Q_OBJECT

signals:
    void leftButtonClicked(int x, int y);
    void rightButtonClicked(int x, int y);
    void mouseMoved(int x, int y, Qt::MouseButtons buttons);
    void buttonReleased();

public:
    enum InteractionFlags
    {
        // no interaction
        INTERACT_NONE = 0,

        // camera interactions
        MODE_TRANSFORM_CAMERA = 1,

        // signals
        INTERACT_SIG_RB_CLICKED = 2,       // right button clicked
        INTERACT_SIG_LB_CLICKED = 4,       // left button clicked
        INTERACT_SIG_MOUSE_MOVED = 8,      // mouse moved (only if a button is clicked)
        INTERACT_SIG_BUTTON_RELEASED = 16, // mouse button released
        INTERACT_SIG_MB_CLICKED = 32,      // middle button clicked
        INTERACT_SEND_ALL_SIGNALS = INTERACT_SIG_RB_CLICKED | INTERACT_SIG_LB_CLICKED | INTERACT_SIG_MOUSE_MOVED |
                                    INTERACT_SIG_BUTTON_RELEASED | INTERACT_SIG_MB_CLICKED
    };

    enum MessagePosition
    {
        LOWER_LEFT_MESSAGE,
        UPPER_CENTER_MESSAGE,
        SCREEN_CENTER_MESSAGE,
    };

public:
    explicit PointCloudWidget(QWidget* parent = nullptr);
    ~PointCloudWidget();
    void updatePoints(const AbstractPointCloudContainer* clouds);
    void setVisibleVertices(vtkSmartPointer<vtkCellArray> vertices);
    vtkSmartPointer<vtkCellArray> visibleVertices();
    vtkSmartPointer<vtkPoints> allPoints();

    vtkRenderer* renderer() const;

    void setPointSize(int value);
    void setAxesVisible(bool visible);
    void setBoundingBoxVisible(bool visible);

    void setInteractionMode(InteractionFlags flags);
    void setPolyline(const QPolygonF& poly);
    void displayNewMessage(const QString& message, MessagePosition pos, bool append = false,
                           int display_max_delay_sec = 2);

protected:
    void paintGL() override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    // 几何体
    void initGeometry();
    // 坐标轴
    void initAxis();

    void drawMessage(QPainter* painter);
    void drawArea(QPainter* painter);

private:
    class PointCloudWidgetPrivate;
    std::unique_ptr<PointCloudWidgetPrivate> d;

    struct MessageToDisplay {
        MessageToDisplay()
            : message_validity_sec(0)
            , position(LOWER_LEFT_MESSAGE) {}

        //! Message
        QString message;
        //! Message end time (sec)
        qint64 message_validity_sec;
        //! Message position on screen
        MessagePosition position;
    };

    QPolygonF polyline_;
    InteractionFlags interaction_flags_;

    std::list<MessageToDisplay> messages_to_display_;
    QElapsedTimer timer_;
};
