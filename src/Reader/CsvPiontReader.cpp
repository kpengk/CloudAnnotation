#include "CsvPiontReader.hpp"
#include "Container/ErrorPointCloudContainer.hpp"
#include "Container/StandardPointCloudContainer.hpp"
#include "General/ProgressDialog.hpp"
#include "General/NormalizedProgress.hpp"

#include <spdlog/spdlog.h>

 CsvPiontReader::CsvPiontReader() {}

AbstractPointCloudContainer* CsvPiontReader::read_data(std::string_view filename) {
    if (!csv_.mmap(filename)) {
        spdlog::error("Fail to open file. filename:{}", filename);
        return nullptr;
    }

    const int point_count = csv_.rows();
    ProgressDialog dlg(true);
    dlg.setMethodTitle(tr("Open CSV data [%1]").arg(QString::fromLocal8Bit(filename)));
    dlg.setInfo(tr("Number of points: %1").arg(point_count));
    dlg.start();
    NormalizedProgress nprogress(&dlg, point_count);

    const auto header = csv_.header();
    if (csv_.cols() == 5) { // X Y Z error object
        return read_data_error(&nprogress);
    } else if (csv_.cols() == 6) { // X Y Z R G B
        return read_data_rgb(&nprogress);
    } else {
        spdlog::error("Unknown data type.");
        return nullptr;
    }
}

AbstractPointCloudContainer* CsvPiontReader::read_data_rgb(NormalizedProgress* nprogress) {
    // X Y Z R G B
    StandardPointCloudContainer* clouds = new StandardPointCloudContainer();
    const int point_count = csv_.rows();
    clouds->reserve(point_count);
    // clouds->setHeader(QStringList{"x", "y", "z", "r", "g", "b"});
    for (const auto& row : csv_) {
        std::array<float, 6> row_val;
        int col{};
        for (const auto& cell : row) {
            std::string value;
            cell.read_value(value);
            row_val[col] = std::stof(value);
            ++col;
        }
        clouds->append(Point3D(row_val[0], row_val[1], row_val[2]), Color(row_val[3], row_val[4], row_val[5]));

        if (nprogress && !nprogress->oneStep()) {
            break;
        }
    }

    return clouds;
}

AbstractPointCloudContainer* CsvPiontReader::read_data_error(NormalizedProgress* nprogress) {
    // X Y Z error object
    ErrorPointCloudContainer* clouds = new ErrorPointCloudContainer();
    const int point_count = csv_.rows();
    clouds->reserve(point_count);
    // clouds->setHeader(QStringList{"x", "y", "z", "error"});
    for (const auto& row : csv_) {
        std::array<float, 5> row_val;
        int col{};
        for (const auto& cell : row) {
            std::string value;
            cell.read_value(value);
            row_val[col] = std::stof(value);
            ++col;
        }
        if (row_val[4] > 0) {
            break;
        }
        clouds->append(Point3D(row_val[0], row_val[1], row_val[2]), row_val[3]);

        if (nprogress && !nprogress->oneStep()) {
            break;
        }
    }

    return clouds;
}
