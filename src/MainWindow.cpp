#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "GraphicalSegmentationTool.hpp"

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
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include <csv2/reader.hpp>
#include <spdlog/spdlog.h>
#include <OverlayDialog.hpp>

#include "CloudTableModel.hpp"
#include "AboutDialog.hpp"

#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>

constexpr int column_count{6};

std::vector<std::array<float, column_count>> read_data(std::string_view filename) {
    csv2::Reader<csv2::delimiter<','>, csv2::quote_character<'"'>, csv2::first_row_is_header<true>,
                 csv2::trim_policy::trim_whitespace>
        csv;

    if (!csv.mmap(filename)) {
        spdlog::error("Fail to open file. filename:{}", filename);
        return {};
    }
    const auto header = csv.header();
    if (csv.cols() != column_count) {
        spdlog::error("The number of columns is not equal to {}.", column_count);
        return {};
    }

    std::vector<std::array<float, column_count>> result;
    result.reserve(csv.rows());
    // X Y Z R G B
    for (const auto row : csv) {
        std::array<float, column_count> row_val;
        std::size_t id{};
        for (const auto cell : row) {
            std::string value;
            cell.read_value(value);
            row_val[id] = std::stof(value);
            ++id;
        }
        result.push_back(row_val);
    }

    return result;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow())
    , segmentation_tool_{} {
    ui->setupUi(this);

    connectActions();

    ui->splitter->setStretchFactor(1, 3);
    ui->tableView->setModel(new CloudTableModel(this));
}

MainWindow::~MainWindow() { delete ui; }

CloudTableModel* MainWindow::tableViewModel() {
    static auto model{qobject_cast<CloudTableModel*>(ui->tableView->model())};
    return model;
}

void MainWindow::connectActions() {
    //"File" menu
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::doActionLoadFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::doActionSaveFile);
    connect(ui->actionQuit, &QAction::triggered, this, &QWidget::close);

    //"About" menu entry
    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::doActionShowHelpDialog);
    connect(ui->actionAbout, &QAction::triggered, this, [this]() {
        auto aboutDialog = new AboutDialog("0.1.0", this);
        aboutDialog->exec();
    });

    //"Tools"
    //connect(ui->actionBoundingBox, &QAction::triggered, ui->cloudWidget, &PointCloudWidget::setBoundingBoxVisible);
    connect(ui->actionSegment, &QAction::triggered, this, &MainWindow::activateSegmentationMode);

    // widget
    connect(ui->tableView, &QTableView::clicked, this, &MainWindow::doTableViewClicked);
}

bool MainWindow::haveSelection() { return true; }

void MainWindow::activateSegmentationMode() {
    if (!haveSelection())
        return;

    if (!segmentation_tool_) {
        segmentation_tool_ = new GraphicalSegmentationTool(this);
        connect(segmentation_tool_, &OverlayDialog::processFinished, this, &MainWindow::deactivateSegmentationMode);

        repositionOverlayDialog(segmentation_tool_, Qt::TopRightCorner);
    }

    segmentation_tool_->linkWith(ui->cloudWidget);
    segmentation_tool_->setEntity(ui->cloudWidget->allPoints().Get(), ui->cloudWidget->visibleVertices());

    //ui->toolBarView->setDisabled(false);

    if (!segmentation_tool_->start())
        deactivateSegmentationMode(false);
    else
        repositionOverlayDialog(segmentation_tool_, Qt::TopRightCorner);
}

void MainWindow::deactivateSegmentationMode(bool state) {
    if (!segmentation_tool_) {
        assert(false);
        return;
    }

    bool deleteHiddenParts = false;

    // shall we apply segmentation?
    if (state) {
        //segmentation_tool_->applySegmentation(this, result);

        if (auto model = tableViewModel(); model) {
            const CloudProperty property{.name = QDateTime::currentDateTime().toString("yyyy-MM-dd_HHmmss"),
                                   .visible = true,
                                   .label = -1,
                                   .vertices = ui->cloudWidget->visibleVertices()};
            model->appendCloud(property);
        }
    }

    // we enable all windows
    //enableAll();
}

void MainWindow::repositionOverlayDialog(OverlayDialog* dlg, Qt::Corner position) {
    int dx = 0;
    int dy = 0;
    static const int margin = 5;
    switch (position) {
        case Qt::TopLeftCorner:
            dx = margin;
            dy = margin;
            break;
        case Qt::TopRightCorner:
            dx = std::max(margin, ui->cloudWidget->width() - dlg->width() - margin);
            dy = margin;
            break;
        case Qt::BottomLeftCorner:
            dx = margin;
            dy = std::max(margin, ui->cloudWidget->height() - dlg->height() - margin);
            break;
        case Qt::BottomRightCorner:
            dx = std::max(margin, ui->cloudWidget->width() - dlg->width() - margin);
            dy = std::max(margin, ui->cloudWidget->height() - dlg->height() - margin);
            break;
    }

    // show();
    dlg->move(ui->cloudWidget->mapToGlobal(QPoint(dx, dy)));
    dlg->raise();
}

void MainWindow::doActionLoadFile() {
    // file choosing dialog
    QString selected_file = QFileDialog::getOpenFileName(this, tr("Open file(s)"), QString(), "CSV (*.csv)");
    if (selected_file.isEmpty())
        return;

    // read data
    const std::string path = selected_file.toStdString();
    const auto cloud = read_data(path);
    spdlog::debug("Point count:{}", cloud.size());

    ui->cloudWidget->updatePoints(cloud);
    if (auto model = tableViewModel(); model) {
        const CloudProperty property{.name = QFileInfo(selected_file).baseName(),
                               .visible = true,
                               .label = -1,
                               .vertices = ui->cloudWidget->visibleVertices()};
        model->clearCloud();
        model->appendCloud(property);
    }
}

void MainWindow::doActionSaveFile() {}

void MainWindow::doActionShowHelpDialog() {
    QMessageBox messageBox;
    messageBox.setTextFormat(Qt::RichText);
    messageBox.setWindowTitle("Documentation");
    messageBox.setText("Please look at the <a href='http://www.example.org/doc/wiki'>wiki</a>");
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.exec();
}

void MainWindow::doTableViewClicked(const QModelIndex& index) {
    if (!index.isValid())
        return;

    if (auto model = tableViewModel(); model) {
        const CloudProperty& cloud = model->cloudAt(index.row());
        ui->cloudWidget->setVisibleVertices(cloud.vertices);
    }
}
