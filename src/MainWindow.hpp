#pragma once
#include <QMainWindow>
#include "BoundingBox.hpp"
#include "GraphicalSegmentationTool.hpp"
#include "PointCloudContainer.hpp"

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
#include <QButtonGroup>
#include <QStringList>

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

protected:
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void initCategoryLabels();
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
    CloudTableModel* table_model_;
    QButtonGroup* category_name_group_;
    PointCloudContainer cloud_raw_data_;
};
