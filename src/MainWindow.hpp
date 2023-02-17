#pragma once
#include <QMainWindow>
#include "BoundingBox.hpp"
#include "GraphicalSegmentationTool.hpp"

#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkUnsignedCharArray.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <QPolygonF>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
};
QT_END_NAMESPACE

struct CloudPoint {
    double x;
    double y;
    double z;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class CloudTableModel;
class OverlayDialog;
class GraphicalSegmentationTool;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    CloudTableModel* tableViewModel();
    void connectActions();
    bool haveSelection();
    void activateSegmentationMode();
    void deactivateSegmentationMode(bool state);
    void repositionOverlayDialog(OverlayDialog* dlg, Qt::Corner position);

    void doActionLoadFile();
    void doActionSaveFile();
    void doActionShowHelpDialog();
    void doTableViewClicked(const QModelIndex& index);

private:
    Ui::MainWindow* ui;
    GraphicalSegmentationTool* segmentation_tool_;
};
