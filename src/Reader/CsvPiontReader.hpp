#pragma once
#include <QObject>
#include <csv2/reader.hpp>

class NormalizedProgress;
class AbstractPointCloudContainer;

class CsvPiontReader : public QObject {
    Q_OBJECT

public:
    CsvPiontReader();

    AbstractPointCloudContainer* read_data(std::string_view filename);

private:
    AbstractPointCloudContainer* read_data_rgb(NormalizedProgress* nprogress);
    AbstractPointCloudContainer* read_data_error(NormalizedProgress* nprogress);

private:
    csv2::Reader<csv2::delimiter<','>, csv2::quote_character<'"'>, csv2::first_row_is_header<true>,
                 csv2::trim_policy::trim_whitespace>
        csv_;
};
