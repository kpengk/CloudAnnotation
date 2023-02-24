#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "GraphicalSegmentationTool.hpp"

#include <vtkCellArray.h>
#include <vtkColorTransferFunction.h>
#include <vtkContourValues.h>
#include <vtkImageData.h>
#include <vtkMetaImageReader.h>
#include <vtkNamedColors.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkVolumeProperty.h>
#include <vtkWidgetRepresentation.h>
#include <vtkCellArrayIterator.h>

#include <OverlayDialog.hpp>
#include <csv2/reader.hpp>
#include <spdlog/spdlog.h>

#include "AboutDialog.hpp"
#include "CloudTableModel.hpp"
#include "Config/ConfigManage.hpp"

#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QRadioButton>

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
    , segmentation_tool_{}
    , table_model_{new CloudTableModel(this)}
    , category_name_group_{new QButtonGroup(this)} {
    ui->setupUi(this);
    ui->splitter->setStretchFactor(1, 3);
    ui->tableView->setModel(table_model_);
    ui->toolBarView->hide();

    connectActions();
    initCategoryLabels();
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

        const CloudProperty property{.name = QString(tr("cloud-%1")).arg(table_model_->rowCount()),
                                     .category = 0,
                                     .visible = true,
                                     .vertices = ui->cloudWidget->visibleVertices()};
        table_model_->appendCloud(property);
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
    const QString selected_file = QFileDialog::getOpenFileName(this, tr("Open file(s)"), QString(), "CSV (*.csv)");
    if (selected_file.isEmpty())
        return;

    // read data
    const std::string path = selected_file.toStdString();
    const auto cloud = read_data(path);
    cloud_raw_data_ = cloud;
    spdlog::debug("Point count:{}", cloud.size());

    ui->cloudWidget->updatePoints(cloud);
    const CloudProperty property{.name = QFileInfo(selected_file).baseName(),
                                 .category = 0,
                                 .visible = true,
                                 .vertices = ui->cloudWidget->visibleVertices()};
    table_model_->clearCloud();
    table_model_->appendCloud(property);
}

void MainWindow::doActionSaveFile() {
    const QString selected_dir = QFileDialog::getExistingDirectory(this);
    if (selected_dir.isEmpty())
        return;

    const auto& items = table_model_->cloudItems();
    if (items.isEmpty()) {
        return;
    }

    QFile info_file(selected_dir + "/cloud_annotaions.csv");
    if (!info_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream info_ts(&info_file);
    info_ts << "files,category\n";
    for (int id = 0; id < items.size(); ++id) {
        if (items.at(id).category > 0) {
            info_ts << QString("cloud-%1.csv,%2\n").arg(id).arg(table_model_->categoryName(items.at(id).category));
        }
    }

    for (int id = 0; id < items.size(); ++id) {
        const auto& item = items.at(id);
        if (item.category <= 0) {
            continue;
        }

        QFile points_file(selected_dir + QString("/cloud-%1.csv").arg(id));
        if (!points_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            continue;
        }
        QTextStream points_ts(&points_file);
        points_ts << "x,y,z,r,g,b\n";
        auto iter = vtk::TakeSmartPointer(item.vertices->NewIterator());
        for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell()) {
            const int id = iter->GetCurrentCell()->GetId(0);
            const auto property = cloud_raw_data_.at(id);
            points_ts << QString("%1,%2,%3,%4,%5,%6\n")
                             .arg(property[0])
                             .arg(property[1])
                             .arg(property[2])
                             .arg(property[3])
                             .arg(property[4])
                             .arg(property[5]);
        }
    }
}

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

    const CloudProperty& cloud = table_model_->cloudAt(index.row());
    ui->cloudWidget->setVisibleVertices(cloud.vertices);

    auto btn = category_name_group_->button(cloud.category);
    if (btn) {
        auto radio_btn = qobject_cast<QRadioButton*>(btn);
        if (radio_btn) {
            radio_btn->setChecked(true);
        }
    }
}

void MainWindow::initCategoryLabels() {
    const auto& names = ConfigManage::instance()->category_names();
    QStringList category_names = {"N/A"};
    for (const auto& name : names) {
        category_names.append(QString::fromStdString(name));
    }

    table_model_->setCategoryNames(category_names);

    int id{};
    for (const auto& name : category_names) {
        auto radio = new QRadioButton(name, this);
        ui->labelVLayout->addWidget(radio);
        category_name_group_->addButton(radio, id++);
    }
    ui->labelVLayout->addStretch();

    connect(category_name_group_, &QButtonGroup::idClicked, this, [this](int id) {
        const QModelIndex index = ui->tableView->currentIndex();
        if (!index.isValid())
            return;

        CloudProperty cloud = table_model_->cloudAt(index.row());
        cloud.category = id;
        table_model_->setCloud(index.row(), cloud);
    });
}
