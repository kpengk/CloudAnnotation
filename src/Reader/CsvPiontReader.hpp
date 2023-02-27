#pragma once
#include "Container/ErrorPointCloudContainer.hpp"
#include "Container/StandardPointCloudContainer.hpp"

#include <QObject>
#include <csv2/reader.hpp>
#include <spdlog/spdlog.h>

class CsvPiontReader : public QObject {
    Q_OBJECT

signals:
    void progressChanged(int val);

public:
    CsvPiontReader()
        : progress_{} {}

    AbstractPointCloudContainer* read_data(std::string_view filename) {
        if (!csv_.mmap(filename)) {
            spdlog::error("Fail to open file. filename:{}", filename);
            return nullptr;
        }
        const auto header = csv_.header();
        if (csv_.cols() == 5) { // X Y Z error object
            return read_data_error();
        } else if (csv_.cols() == 6) { // X Y Z R G B
            return read_data_rgb();
        } else {
            spdlog::error("Unknown data type.");
            emit progressChanged(100);
            return nullptr;
        }
    }

private:
    AbstractPointCloudContainer* read_data_rgb() {
        // X Y Z R G B
        StandardPointCloudContainer* clouds = new StandardPointCloudContainer();
        const int point_count = csv_.rows();
        clouds->reserve(point_count);
        // clouds->setHeader(QStringList{"x", "y", "z", "r", "g", "b"});
        int count{};
        progress_ = 0;
        emit progressChanged(progress_);

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
            ++count;
            const int p = static_cast<float>(count) / point_count * 100.0F;
            if (p != progress_) {
                progress_ = p;
                emit progressChanged(progress_);
            }
        }

        progress_ = 100;
        emit progressChanged(progress_);
        return clouds;
    }

    AbstractPointCloudContainer* read_data_error() {
        // X Y Z error object
        ErrorPointCloudContainer* clouds = new ErrorPointCloudContainer();
        const int point_count = csv_.rows();
        clouds->reserve(point_count);
        // clouds->setHeader(QStringList{"x", "y", "z", "error"});
        int count{};
        progress_ = 0;
        emit progressChanged(progress_);

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
            ++count;
            const int p = static_cast<float>(count) / point_count * 100.0F;
            if (p != progress_) {
                progress_ = p;
                emit progressChanged(progress_);
            }
        }

        progress_ = 100;
        emit progressChanged(progress_);
        return clouds;
    }

private:
    csv2::Reader<csv2::delimiter<','>, csv2::quote_character<'"'>, csv2::first_row_is_header<true>,
                 csv2::trim_policy::trim_whitespace>
        csv_;
    int progress_;
};
