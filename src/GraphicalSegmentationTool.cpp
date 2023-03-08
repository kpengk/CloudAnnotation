#include "GraphicalSegmentationTool.hpp"

#include <spdlog/spdlog.h>

#include "PointCloudWidget.hpp"

#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkCoordinate.h>
#include <vtkPoints.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>

#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

// System
#include <assert.h>

#if defined(_OPENMP)
// OpenMP
#include <omp.h>
#endif

QPolygonF toVtkDisplay(const QPolygonF& polyline, int display_height) {
    QPolygonF out;
    out.reserve(polyline.size());
    for (const auto point : polyline) {
        out.append(QPointF(point.x(), display_height - point.y()));
    }
    return out;
}

QPolygonF point_zoom(const QPolygonF& polyline, float scaling_x, float scaling_y) {
    QPolygonF out;
    out.reserve(polyline.size());
    for (const auto point : polyline) {
        out.append(QPointF(point.x() * scaling_x, point.y() * scaling_y));
    }
    return out;
}

GraphicalSegmentationTool::GraphicalSegmentationTool(QWidget* parent)
    : OverlayDialog(parent)
    , Ui::GraphicalSegmentationDlg()
    , state_(0)
    , rectangular_mode_(false) {
    // Set QDialog background as transparent (DGM: doesn't work over an OpenGL context)
    // setAttribute(Qt::WA_NoSystemBackground);

    setupUi(this);

    connect(pauseButton, &QToolButton::toggled, this, &GraphicalSegmentationTool::pauseSegmentationMode);
    connect(inButton, &QToolButton::clicked, this, &GraphicalSegmentationTool::segmentIn);
    connect(outButton, &QToolButton::clicked, this, &GraphicalSegmentationTool::segmentOut);
    connect(undoButton, &QToolButton::clicked, this, &GraphicalSegmentationTool::undo);
    connect(redoButton, &QToolButton::clicked, this, &GraphicalSegmentationTool::redo);
    connect(resetButton, &QToolButton::clicked, this, &GraphicalSegmentationTool::reset);
    connect(validButton, &QToolButton::clicked, this, &GraphicalSegmentationTool::apply);
    connect(cancelButton, &QToolButton::clicked, this, &GraphicalSegmentationTool::cancel);

    // selection modes
    connect(actionSetPolylineSelection, &QAction::triggered, this, &GraphicalSegmentationTool::setPolylineMode);
    connect(actionSetRectangularSelection, &QAction::triggered, this, &GraphicalSegmentationTool::setRectangleMode);

    // add shortcuts
    addOverriddenShortcut(Qt::Key_Space);  // space bar for the "pause" button
    addOverriddenShortcut(Qt::Key_Escape); // escape key for the "cancel" button
    addOverriddenShortcut(Qt::Key_Return); // return key for the "apply" button
    addOverriddenShortcut(Qt::Key_Tab);    // tab key to switch between rectangular and polygonal selection modes
    addOverriddenShortcut(Qt::Key_I);      //'I' key for the "segment in" button
    addOverriddenShortcut(Qt::Key_O);      //'O' key for the "segment out" button
    connect(this, &OverlayDialog::shortcutTriggered, this, &GraphicalSegmentationTool::onShortcutTriggered);
    addOverriddenStandShortcut(QKeySequence::Undo); //'Ctrl + Z' key for the "undo" button
    addOverriddenStandShortcut(QKeySequence::Redo); //'Ctrl + Y' key for the "redo" button
    connect(this, &OverlayDialog::standShortcutTriggered, this, &GraphicalSegmentationTool::onStandShortcutTriggered);

    QMenu* selectionModeMenu = new QMenu(this);
    selectionModeMenu->addAction(actionSetPolylineSelection);
    selectionModeMenu->addAction(actionSetRectangularSelection);
    selectionModelButton->setDefaultAction(actionSetPolylineSelection);
    selectionModelButton->setMenu(selectionModeMenu);
}

GraphicalSegmentationTool::~GraphicalSegmentationTool() {}

void GraphicalSegmentationTool::onShortcutTriggered(int key) {
    switch (key) {
        case Qt::Key_Space:
            // toggle pause mode
            pauseSegmentationMode(!pauseButton->isChecked());
            // pauseButton->toggle();
            return;
        case Qt::Key_I: segmentIn(); return;
        case Qt::Key_O: segmentOut(); return;
        case Qt::Key_Return:
            if (!segment_steps_.is_initial_state())
                apply();
            // validButton->click();
            return;
        case Qt::Key_Escape:
            cancel();
            // cancelButton->click();
            return;
        case Qt::Key_Tab:
            if (rectangular_mode_)
                setPolylineMode();
            else
                setRectangleMode();
            return;

        default:
            // nothing to do
            break;
    }
}

void GraphicalSegmentationTool::onStandShortcutTriggered(int key) {
    switch (key) {
        case QKeySequence::Undo: undo(); return;
        case QKeySequence::Redo: redo(); return;
        default:
            // nothing to do
            break;
    }
}

bool GraphicalSegmentationTool::linkWith(PointCloudWidget* win) {
    auto* oldWin = associated_win_;

    if (!OverlayDialog::linkWith(win)) {
        return false;
    }

    if (oldWin) {
        oldWin->disconnect(this);
    }

    if (associated_win_) {
        connect(associated_win_, &PointCloudWidget::leftButtonClicked, this,
                &GraphicalSegmentationTool::addPointToPolyline);
        connect(associated_win_, &PointCloudWidget::rightButtonClicked, this,
                &GraphicalSegmentationTool::closePolyLine);
        connect(associated_win_, &PointCloudWidget::mouseMoved, this, &GraphicalSegmentationTool::updatePolyLine);
        connect(associated_win_, &PointCloudWidget::buttonReleased, this, &GraphicalSegmentationTool::closeRectangle);
    }

    return true;
}

bool GraphicalSegmentationTool::start() {
    if (!associated_win_) {
        spdlog::warn("[Graphical Segmentation Tool] No associated window!");
        return false;
    }

    segmentation_poly_.clear();
    // the user must not close this window!
    pauseSegmentationMode(false);
    reset();

    return OverlayDialog::start();
}

void GraphicalSegmentationTool::stop(bool accepted) {
    if (associated_win_) {
        associated_win_->displayNewMessage("Segmentation [OFF]", PointCloudWidget::UPPER_CENTER_MESSAGE, false, 2);
        associated_win_->setInteractionMode(PointCloudWidget::MODE_TRANSFORM_CAMERA);
    }

    OverlayDialog::stop(accepted);
}

void GraphicalSegmentationTool::setEntity(vtkPoints* points, vtkSmartPointer<vtkCellArray> vertices) {
    points_ = points;
    segment_steps_.reset(vertices);
}

vtkSmartPointer<vtkCellArray> GraphicalSegmentationTool::entity() const { return *segment_steps_.current(); }

vtkSmartPointer<vtkCellArray> GraphicalSegmentationTool::applySegmentation(vtkSmartPointer<vtkCellArray> vertices,
                                                                           const QPolygonF& polygon, SegmentType type) {
    vtkNew<vtkCoordinate> corrdinate;
    corrdinate->SetCoordinateSystemToWorld();

    vtkRenderer* renderer = associated_win_->renderer();
    vtkSmartPointer<vtkCellArray> result = vtkSmartPointer<vtkCellArray>::New();
#if 0
    vtkNew<vtkIdList> pts; 
    for (int i = 0; i < vertices->GetNumberOfCells(); ++i) {
        vertices->GetCellAtId(i, pts);
        const int id = pts->GetId(0);
#else
    auto iter = vtk::TakeSmartPointer(vertices->NewIterator());
    for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell()) {
        const int id = iter->GetCurrentCell()->GetId(0);
#endif
    const double* word_pos = points_->GetPoint(id);
    corrdinate->SetValue(word_pos);
    const double* display_pos = corrdinate->GetComputedDoubleDisplayValue(renderer);
    const bool contains_point = polygon.containsPoint(QPointF(display_pos[0], display_pos[1]), Qt::OddEvenFill);
    if (type == SegmentType::CutInside && contains_point) {
        result->InsertNextCell(1); // 加入顶点信息
        result->InsertCellPoint(id);
    } else if (type == SegmentType::CutOutside && !contains_point) {
        result->InsertNextCell(1); // 加入顶点信息
        result->InsertCellPoint(id);
    }
}

return result;
}

void GraphicalSegmentationTool::updatePolyLine(int x, int y, Qt::MouseButtons buttons) {
    // process not started yet?
    if ((state_ & RUNNING) == 0) {
        return;
    }
    if (!associated_win_) {
        assert(false);
        return;
    }

    unsigned vertCount = segmentation_poly_.size();

    if (state_ & RECTANGLE) {
        // we need 4 points for the rectangle!
        if (vertCount != 4)
            segmentation_poly_.resize(4);

        const auto p0 = segmentation_poly_[0];
        segmentation_poly_[1] = QPointF(p0.x(), y);
        segmentation_poly_[2] = QPointF(x, y);
        segmentation_poly_[3] = QPointF(x, p0.y());
    } else if (state_ & POLYLINE) {
        if (vertCount < 2)
            return;
        // we replace last point by the current one
        segmentation_poly_.back() = QPointF(x, y);
    }

    associated_win_->setPolyline(segmentation_poly_);
    associated_win_->update();
}

void GraphicalSegmentationTool::addPointToPolylineExt(int x, int y, bool allowClicksOutside) {
    if ((state_ & STARTED) == 0) {
        return;
    }
    if (!associated_win_) {
        assert(false);
        return;
    }

    if (!allowClicksOutside && (x < 0 || y < 0 || x >= associated_win_->width() || y >= associated_win_->height())) {
        // ignore clicks outside of the 3D view
        return;
    }

    unsigned vertCount = segmentation_poly_.size();

    // particular case: we close the rectangular selection by a 2nd click
    if (rectangular_mode_ && vertCount == 4 && (state_ & RUNNING))
        return;

    // CTRL key pressed at the same time?
    bool ctrlKeyPressed =
        rectangular_mode_ || ((QApplication::keyboardModifiers() & Qt::ControlModifier) == Qt::ControlModifier);

    // start new polyline?
    if (((state_ & RUNNING) == 0) || vertCount == 0 || ctrlKeyPressed) {
        // reset state
        state_ = (ctrlKeyPressed ? RECTANGLE : POLYLINE);
        state_ |= STARTED;
        run();

        // reset polyline
        segmentation_poly_.clear();
        segmentation_poly_.reserve(2);
        // we add the same point twice (the last point will be used for display only)
        segmentation_poly_.append(QPointF(x, y));
        segmentation_poly_.append(QPointF(x, y));
    } else { // next points in "polyline mode" only
        // we were already in 'polyline' mode?
        if (state_ & POLYLINE) {
            // and add a new (equivalent) one
            segmentation_poly_.append(QPointF(x, y));
        } else {           // we must change mode
            assert(false); // we shouldn't fall here?!
            stopRunning();
            addPointToPolylineExt(x, y, allowClicksOutside);
            return;
        }
    }

    // DGM: to increase the poll rate of the mouse movements in RenderWidget::mouseMoveEvent
    // we have to completely grab the mouse focus!
    //(the only way to take back the control is to right-click now...)
    associated_win_->setPolyline(segmentation_poly_);
    associated_win_->update();
}

void GraphicalSegmentationTool::closeRectangle() {
    // only for rectangle selection in RUNNING mode
    if ((state_ & RECTANGLE) == 0 || (state_ & RUNNING) == 0)
        return;

    unsigned vertCount = segmentation_poly_.size();
    if (vertCount < 4) {
        // first point only? we keep the real time update mechanism
        if (rectangular_mode_)
            return;
        segmentation_poly_.clear();
    } else {
        // close
        segmentation_poly_.append(segmentation_poly_.first());
    }
    // stop
    stopRunning();

    if (associated_win_) {
        associated_win_->setPolyline(segmentation_poly_);
        associated_win_->update();
    }
}

void GraphicalSegmentationTool::closePolyLine(int, int) {
    // only for polyline in RUNNING mode
    if ((state_ & POLYLINE) == 0 || (state_ & RUNNING) == 0)
        return;

    unsigned vertCount = segmentation_poly_.size();
    if (vertCount < 4) {
        segmentation_poly_.clear();
    } else {
        // remove last point!
        segmentation_poly_.resize(vertCount - 1); // can't fail --> smaller
        // close
        segmentation_poly_.append(segmentation_poly_.first());
    }

    // stop
    stopRunning();

    if (associated_win_) {
        associated_win_->setPolyline(segmentation_poly_);
        associated_win_->update();
    }
}

void GraphicalSegmentationTool::segmentIn() { segment(SegmentType::CutInside); }

void GraphicalSegmentationTool::segmentOut() { segment(SegmentType::CutOutside); }

void GraphicalSegmentationTool::segment(SegmentType type) {
    if (!associated_win_) {
        assert(false);
        return;
    }

    if (segmentation_poly_.isEmpty()) {
        spdlog::error("No polyline defined!");
        return;
    }

    if (!segmentation_poly_.isClosed()) {
        spdlog::error("Define and/or close the segmentation polygon first! (right click to close)");
        return;
    }

    // we must close the polyline if we are in RUNNING mode
    if ((state_ & POLYLINE) != 0 && (state_ & RUNNING) != 0) {
        QPoint mouse_pos = associated_win_->mapFromGlobal(QCursor::pos());
        spdlog::warn("Polyline was not closed - we'll close it with the current mouse cursor position: ({} ; {})",
                     mouse_pos.x(), mouse_pos.y());
        addPointToPolylineExt(mouse_pos.x(), mouse_pos.y(), true);
        closePolyLine(0, 0);
    }

    segmentation_poly_.removeLast();

    const QSize window_size = associated_win_->size();
    const int* render_window_size = associated_win_->renderWindow()->GetSize();
    // VTK二维坐标系原点在左下角，QT坐标系原点在左上角
    const auto polyline = toVtkDisplay(segmentation_poly_, window_size.height());
    // 针对windows高分屏缩放，Qt窗口大小识别错误。如缩放125%，实际窗口大小：(733, 633)，Window size:(586, 506), render window size:(733, 633).
    const auto cut_line = point_zoom(polyline, static_cast<float>(render_window_size[0]) / window_size.width(),
                                     static_cast<float>(render_window_size[1]) / window_size.height());
    spdlog::debug("Window size:({}, {}), render window size:({}, {}).", window_size.width(), window_size.height(),
                  render_window_size[0], render_window_size[1]);

    auto vertices = applySegmentation(*segment_steps_.current(), cut_line, type);
    segment_steps_.push(vertices);

    associated_win_->setVisibleVertices(*segment_steps_.current());
    associated_win_->update();

    validButton->setEnabled(true);
    resetButton->setEnabled(true);
    undoButton->setEnabled(true);
    redoButton->setEnabled(false);
    pauseSegmentationMode(true);
}

void GraphicalSegmentationTool::undo() {
    if (!segment_steps_.is_initial_state()) {
        segment_steps_.undo();
        associated_win_->setVisibleVertices(*segment_steps_.current());
    }
    if (associated_win_) {
        associated_win_->update();
        associated_win_->releaseMouse();
    }

    if (segment_steps_.can_undo()) {
        validButton->setEnabled(true);
    } else {
        undoButton->setEnabled(false);
        resetButton->setEnabled(false);
        validButton->setEnabled(false);
    }
    redoButton->setEnabled(true);
}

void GraphicalSegmentationTool::redo() {
    if (segment_steps_.can_redo()) {
        segment_steps_.redo();
        associated_win_->setVisibleVertices(*segment_steps_.current());
    }
    if (associated_win_) {
        associated_win_->update();
        associated_win_->releaseMouse();
    }
    
    if (segment_steps_.can_redo()) {
    } else {
        redoButton->setEnabled(false);
    }

    undoButton->setEnabled(true);
    resetButton->setEnabled(true);
    validButton->setEnabled(true);
}

void GraphicalSegmentationTool::reset() {
    if (!segment_steps_.is_initial_state()) {
        segment_steps_.to_begin();
        associated_win_->setVisibleVertices(*segment_steps_.current());
    }
    if (associated_win_) {
        associated_win_->update();
        associated_win_->releaseMouse();
    }

    undoButton->setEnabled(false);
    redoButton->setEnabled(segment_steps_.can_redo());
    resetButton->setEnabled(false);
    validButton->setEnabled(false);
}

void GraphicalSegmentationTool::run() {
    state_ |= RUNNING;
    buttonsFrame->setEnabled(false); // we disable the buttons when running
}

void GraphicalSegmentationTool::stopRunning() {
    state_ &= (~RUNNING);
    buttonsFrame->setEnabled(true); // we restore the buttons when running is stopped
}

void GraphicalSegmentationTool::pauseSegmentationMode(bool state) {
    if (!associated_win_)
        return;

    if (state /*=activate pause mode*/) {
        stopRunning();
        state_ = PAUSED;
        if (!segmentation_poly_.isEmpty()) {
            segmentation_poly_.clear();
        }

        associated_win_->setInteractionMode(PointCloudWidget::MODE_TRANSFORM_CAMERA);
        associated_win_->displayNewMessage("Segmentation [PAUSED]", PointCloudWidget::UPPER_CENTER_MESSAGE, false,
                                           3600);
        associated_win_->displayNewMessage("Unpause to segment again", PointCloudWidget::UPPER_CENTER_MESSAGE, true,
                                           3600);
    } else {
        state_ = STARTED;
        associated_win_->setInteractionMode(PointCloudWidget::INTERACT_SEND_ALL_SIGNALS);
        if (rectangular_mode_) {
            associated_win_->displayNewMessage("Segmentation [ON] (rectangular selection)",
                                               PointCloudWidget::UPPER_CENTER_MESSAGE, false, 3600);
            associated_win_->displayNewMessage("Left click: set opposite corners",
                                               PointCloudWidget::UPPER_CENTER_MESSAGE);
        } else {
            associated_win_->displayNewMessage("Segmentation [ON] (polygonal selection)",
                                               PointCloudWidget::UPPER_CENTER_MESSAGE, false, 3600);
            associated_win_->displayNewMessage("Left click: add contour points / Right click: close",
                                               PointCloudWidget::UPPER_CENTER_MESSAGE, true, 3600);
        }
    }

    // update mini-GUI
    pauseButton->blockSignals(true);
    pauseButton->setChecked(state);
    pauseButton->blockSignals(false);

    associated_win_->setPolyline(segmentation_poly_);
    associated_win_->update();
}

void GraphicalSegmentationTool::setPolylineMode() {
    if (!rectangular_mode_)
        return;

    selectionModelButton->setDefaultAction(actionSetPolylineSelection);

    rectangular_mode_ = false;
    if (state_ != PAUSED) {
        pauseSegmentationMode(true);
        pauseSegmentationMode(false);
    }

    associated_win_->displayNewMessage(QString(), PointCloudWidget::UPPER_CENTER_MESSAGE); // clear the area
    associated_win_->displayNewMessage("Segmentation [ON] (rectangular selection)",
                                       PointCloudWidget::UPPER_CENTER_MESSAGE, false, 3600);
    associated_win_->displayNewMessage("Right click: set opposite corners", PointCloudWidget::UPPER_CENTER_MESSAGE,
                                       true, 3600);
}

void GraphicalSegmentationTool::setRectangleMode() {
    if (rectangular_mode_)
        return;

    selectionModelButton->setDefaultAction(actionSetRectangularSelection);

    rectangular_mode_ = true;
    if (state_ != PAUSED) {
        pauseSegmentationMode(true);
        pauseSegmentationMode(false);
    }

    associated_win_->displayNewMessage(QString(), PointCloudWidget::UPPER_CENTER_MESSAGE); // clear the area
    associated_win_->displayNewMessage("Segmentation [ON] (rectangular selection)",
                                       PointCloudWidget::UPPER_CENTER_MESSAGE, false, 3600);
    associated_win_->displayNewMessage("Right click: set opposite corners", PointCloudWidget::UPPER_CENTER_MESSAGE,
                                       true, 3600);
}

void GraphicalSegmentationTool::apply() { stop(true); }

void GraphicalSegmentationTool::cancel() {
    reset();
    stop(false);
}
