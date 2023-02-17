#pragma once
#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(const QString& version, QWidget* parent = nullptr);
    ~AboutDialog();
    const QString& versionStr() const { return version_str_; }
    QString versionLongStr(bool includeOS) const;

private:
    Ui::AboutDialog* ui;
    const QString version_str_;
};
