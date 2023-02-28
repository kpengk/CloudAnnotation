#include "ProgressDialog.hpp"
// Qt
#include <QCoreApplication>
#include <QProgressBar>
#include <QPushButton>

ProgressDialog::ProgressDialog(bool showCancelButton, QWidget* parent /*=nullptr*/)
    : QProgressDialog(parent)
    , current_value_(0)
    , last_refresh_value_(-1) {
    // Make sure the dialog doesn't steal focus
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlag(Qt::WindowDoesNotAcceptFocus);

    setAutoClose(true);

    resize(400, 200);
    setRange(0, 100);
    setMinimumWidth(400);

    if (showCancelButton) {
        QPushButton* cancelButton = new QPushButton(tr("Cancel"));
        cancelButton->setDefault(false);
        cancelButton->setFocusPolicy(Qt::NoFocus);
        setCancelButton(cancelButton);
    } else {
        setCancelButton(nullptr);
    }

    connect(this, &ProgressDialog::scheduleRefresh, this, &ProgressDialog::refresh,
            Qt::QueuedConnection); // can't use DirectConnection here!
}

void ProgressDialog::refresh() {
    const int value = current_value_;
    if (last_refresh_value_ != value) {
        last_refresh_value_ = value;
        setValue(value); // See Qt doc: if the progress dialog is modal, setValue() calls QApplication::processEvents()
    }
}

void ProgressDialog::update(float percent) {
    // thread-safe
    const int value = static_cast<int>(percent);
    if (value != current_value_) {
        current_value_ = value;
        emit scheduleRefresh();
        QCoreApplication::processEvents();
    }
}

void ProgressDialog::setMethodTitle(QString methodTitle) { setWindowTitle(methodTitle); }

void ProgressDialog::setInfo(QString infoStr) {
    setLabelText(infoStr);
    if (isVisible()) {
        QProgressDialog::update();
        QCoreApplication::processEvents();
    }
}

void ProgressDialog::start() {
    last_refresh_value_ = -1;
    show();
    QCoreApplication::processEvents();
}

void ProgressDialog::stop() {
    hide();
    QCoreApplication::processEvents();
}
