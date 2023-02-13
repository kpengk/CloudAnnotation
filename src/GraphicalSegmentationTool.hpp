#pragma once
#include "OverlayDialog.hpp"
#include <ui_GraphicalSegmentationDlg.h>

#include <vtkSmartPointer.h>

#include <QSet>
#include <vector>

class vtkPoints;
class vtkCellArray;

enum class SegmentType
{
    CutInside,
    CutOutside
};

using VertexIndex = std::vector<int>;

//! Graphical segmentation mechanism (with polyline)
class GraphicalSegmentationTool : public OverlayDialog, public Ui::GraphicalSegmentationDlg {
    Q_OBJECT

public:
    //! Default constructor
    explicit GraphicalSegmentationTool(QWidget* parent);
    //! Destructor
    virtual ~GraphicalSegmentationTool();

    //! Get a pointer to the polyline that has been segmented
    const QPolygonF& getPolyLine() const { return segmentation_poly_; }

    // inherited from ccOverlayDialog
    virtual bool linkWith(PointCloudWidget* win) override;
    virtual bool start() override;
    virtual void stop(bool accepted) override;

    void setEntity(vtkPoints* points, vtkSmartPointer<vtkCellArray> vertices);
    vtkSmartPointer<vtkCellArray> entity() const;

    // 切割模型
    vtkSmartPointer<vtkCellArray> applySegmentation(vtkSmartPointer<vtkCellArray> vertices, const QPolygonF& polygon,
                                                    SegmentType type);

protected:
    void segmentIn();
    void segmentOut();
    void segment(SegmentType type);
    void reset();
    void apply();
    void cancel();
    inline void addPointToPolyline(int x, int y) { return addPointToPolylineExt(x, y, false); }
    void addPointToPolylineExt(int x, int y, bool allowClicksOutside);
    void closePolyLine(int x = 0, int y = 0); // arguments for compatibility with ccGlWindow::rightButtonClicked signal
    void closeRectangle();
    void updatePolyLine(int x, int y, Qt::MouseButtons buttons);
    void run();
    void stopRunning();
    void pauseSegmentationMode(bool);
    void setClassificationValue();
    void doSetPolylineSelection();
    void doSetRectangularSelection();

    //! To capture overridden shortcuts (pause button, etc.)
    void onShortcutTriggered(int);
protected:
    //! Whether something has changed or not (for proper 'cancel')
    bool something_has_changed_;

    //! Process states
    enum ProcessStates
    {
        POLYLINE = 1,
        RECTANGLE = 2,
        //...			= 4,
        //...			= 8,
        //...			= 16,
        PAUSED = 32,
        STARTED = 64,
        RUNNING = 128,
    };

    //! Current process state
    unsigned state_;

    //! Segmentation polyline
    QPolygonF segmentation_poly_;

    //! Selection mode
    bool rectangular_mode_;

    vtkPoints* points_;
    std::vector<vtkSmartPointer<vtkCellArray>> segment_history_;
};
