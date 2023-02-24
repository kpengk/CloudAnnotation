#include "FileIO.hpp"
#include <QFile>

std::string read_file(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        auto msg = QString("File (%1) open fail: %2.").arg(filename, file.errorString());
        qCritical("%s", msg.toStdString().c_str());
        return {};
    }

    return file.readAll().toStdString();
}

bool write_file(const QString& filename, const std::string& text) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        const QString msg = QString("File (%1) open fail: %2.").arg(filename, file.errorString());
        qCritical("%s", msg.toStdString().c_str());
        return false;
    }

    const auto data = QString::fromStdString(text).toUtf8();
    file.write(data);
    return true;
}
