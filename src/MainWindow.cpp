#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "AboutDialog.hpp"
#include "CloudTableModel.hpp"
#include "Config/ConfigManage.hpp"
#include "GraphicalSegmentationTool.hpp"
#include "Reader/CsvPiontReader.hpp"

#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
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

#include <OverlayDialog.hpp>
#include <spdlog/spdlog.h>

#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QRadioButton>
#include <QScopedPointer>
#include <QtConcurrent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow())
    , segmentation_tool_{}
    , table_model_{new CloudTableModel(this)}
    , category_name_group_{new QButtonGroup(this)}
    , cloud_raw_data_{} {
    ui->setupUi(this);
    ui->splitter->setStretchFactor(1, 3);
    ui->tableView->setModel(table_model_);
    ui->toolBarView->hide();

    connectActions();
    initCategoryLabels();
}

MainWindow::~MainWindow() {
    delete ui;
    if (cloud_raw_data_)
        delete cloud_raw_data_;
}

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

    QScopedPointer<QProgressDialog> progress{new QProgressDialog};
    progress->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    progress->setWindowTitle("Load");
    progress->setAutoClose(true);
    progress->setLabelText("Load data ...");
    progress->setRange(0, 100);
    progress->setCancelButton(nullptr);

    // read data
    QtConcurrent::run([&]() {
        const std::string path = selected_file.toLocal8Bit().constData();
        if (cloud_raw_data_) {
            delete cloud_raw_data_;
            cloud_raw_data_ = nullptr;
        }
        CsvPiontReader reader;
        connect(&reader, &CsvPiontReader::progressChanged, progress.data(), &QProgressDialog::setValue);
        cloud_raw_data_ = reader.read_data(path);
    });
    progress->exec();

    if (!cloud_raw_data_) {
        return;
    }
    spdlog::debug("Point count:{}", cloud_raw_data_->pointCount());

    ui->cloudWidget->updatePoints(cloud_raw_data_);
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
        const auto head = cloud_raw_data_->header();
        for (int i = 0; i < head.size(); ++i) {
            points_ts << head.at(i);
            if (i + 1 < head.size())
                points_ts << ",";
            else
                points_ts << "\n";
        }

        auto iter = vtk::TakeSmartPointer(item.vertices->NewIterator());
        for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal(); iter->GoToNextCell()) {
            const int id = iter->GetCurrentCell()->GetId(0);
            points_ts << cloud_raw_data_->toCSV(id) << "\n";
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
