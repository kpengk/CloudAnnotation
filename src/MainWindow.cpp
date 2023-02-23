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

namespace {
double interpolate(double value, double y0, double x0, double y1, double x1) {
    if (value < x0)
        return y0;
    if (value > x1)
        return y1;
    return (value - x0) * (y1 - y0) / (x1 - x0) + y0;
}

double jet_base(double value) {
    if (value <= -0.75)
        return 0.0;
    else if (value <= -0.25)
        return interpolate(value, 0.0, -0.75, 1.0, -0.25);
    else if (value <= 0.25)
        return 1.0;
    else if (value <= 0.75)
        return interpolate(value, 1.0, 0.25, 0.0, 0.75);
    else
        return 0.0;
}

// value: (-0.125, 1.125]
std::array<double, 3> map_color(double value) {
    return std::array{
        jet_base(value * 2.0 - 1.5), // red
        jet_base(value * 2.0 - 1.0), // green
        jet_base(value * 2.0 - 0.5)  // blue
    };
}

constexpr std::array<double, 3> color_01(int r, int g, int b) { return std::array{r / 255.0, g / 255.0, b / 255.0}; }
} // namespace

std::vector<std::array<float, column_count>> read_data(std::string_view filename) {
    csv2::Reader<csv2::delimiter<','>, csv2::quote_character<'"'>, csv2::first_row_is_header<true>,
                 csv2::trim_policy::trim_whitespace>
        csv;

    if (!csv.mmap(filename)) {
        spdlog::error("Fail to open file. filename:{}", filename);
        return {};
    }
    const auto header = csv.header();
    if (csv.cols() == 5) { // X Y Z error object
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
            // error to rgb
            const auto rgb = map_color(row_val[3]);
            // Reassign
            row_val[3] = rgb[0] * 255.0F;
            row_val[4] = rgb[1] * 255.0F;
            row_val[5] = rgb[2] * 255.0F;
            result.push_back(row_val);
        }

        return result;
    } else if (csv.cols() == 6) { // X Y Z R G B
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
    } else {
        spdlog::error("The number of columns is not equal to {}.", column_count);
        return {};
    }
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow())
    , segmentation_tool_{} {
    ui->setupUi(this);

    connectActions();

    ui->splitter->setStretchFactor(1, 3);
    ui->tableView->setModel(new CloudTableModel(this));

    ui->toolBarView->hide();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::moveEvent(QMoveEvent* event) {
    if (segmentation_tool_ && segmentation_tool_->isVisible())
        repositionOverlayDialog(segmentation_tool_, Qt::TopRightCorner);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    if (segmentation_tool_ && segmentation_tool_->isVisible())
        repositionOverlayDialog(segmentation_tool_, Qt::TopRightCorner);
}

CloudTableModel* MainWindow::tableViewModel() {
    static auto model{qobject_cast<CloudTableModel*>(ui->tableView->model())};
    return model;
}

void MainWindow::connectActions() {
    //"File" menu
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::doActionLoadFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::doActionSaveFile);
    connect(ui->actionMerge, &QAction::triggered, this, []() {});
    connect(ui->actionDelete, &QAction::triggered, this, []() {});

    //"Tools"
    connect(ui->actionSegment, &QAction::triggered, this, &MainWindow::activateSegmentationMode);

    // widget
    connect(ui->tableView, &QTableView::clicked, this, &MainWindow::doTableViewClicked);
}

bool MainWindow::haveSelection() { return ui->tableView->currentIndex().row() >= 0; }

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

    ui->toolBarFile->setEnabled(false);
    ui->toolBarView->setEnabled(false);
    ui->toolBarTagging->setEnabled(false);
    ui->tableView->setEnabled(false);

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
        // segmentation_tool_->applySegmentation(this, result);

        if (auto model = tableViewModel(); model) {
            const CloudProperty property{.name = QString(tr("object-%1")).arg(model->rowCount()),
                                         .visible = true,
                                         .label = -1,
                                         .vertices = ui->cloudWidget->visibleVertices()};
            model->appendCloud(property);
        }
    }

    ui->toolBarFile->setEnabled(true);
    ui->toolBarView->setEnabled(true);
    ui->toolBarTagging->setEnabled(true);
    ui->tableView->setEnabled(true);
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
