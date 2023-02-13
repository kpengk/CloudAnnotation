#include "OverlayDialog.hpp"
#include "PointCloudWidget.hpp"

// Qt
#include <QApplication>
#include <QEvent>
#include <QKeyEvent>

// system
#include <cassert>

OverlayDialog::OverlayDialog(QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , associated_win_(nullptr)
    , processing_(false) {}

OverlayDialog::~OverlayDialog() { onLinkedWindowDeletion(); }

bool OverlayDialog::linkWith(PointCloudWidget* win) {
    if (processing_) {
        qWarning("[OverlayDialog] Can't change associated window while running/displayed!");
        return false;
    }

    // same dialog? nothing to do
    if (associated_win_ == win) {
        return true;
    }

    if (associated_win_) {
        // we automatically detach the former dialog
        {
            QWidgetList top_widgets = QApplication::topLevelWidgets();
            for (QWidget* widget : top_widgets) { widget->removeEventFilter(this); }
        }
        associated_win_->disconnect(this);
        associated_win_ = nullptr;
    }

    associated_win_ = win;
    if (associated_win_) {
        QWidgetList top_widgets = QApplication::topLevelWidgets();
        for (QWidget* widget : top_widgets) { widget->installEventFilter(this); }
        connect(associated_win_, &QObject::destroyed, this, &OverlayDialog::onLinkedWindowDeletion);
    }

    return true;
}

void OverlayDialog::onLinkedWindowDeletion(QObject* object /*=nullptr*/) {
    if (processing_)
        stop(false);

    linkWith(nullptr);
}

bool OverlayDialog::start() {
    if (processing_)
        return false;

    processing_ = true;

    // auto-show
    show();

    return true;
}

void OverlayDialog::stop(bool accepted) {
    processing_ = false;

    // auto-hide
    hide();

    linkWith(nullptr);

    emit processFinished(accepted);
}

void OverlayDialog::reject() {
    QDialog::reject();

    stop(false);
}

void OverlayDialog::addOverriddenShortcut(Qt::Key key) { overridden_keys_.push_back(key); }

bool OverlayDialog::eventFilter(QObject* obj, QEvent* e) {
    if (e->type() == QEvent::KeyPress) {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(e);

        if (overridden_keys_.contains(key_event->key())) {
            emit shortcutTriggered(key_event->key());
            return true;
        } else {
            return QDialog::eventFilter(obj, e);
        }
    } else {
        if (e->type() == QEvent::Show) {
            emit shown();
        }

        // standard event processing
        return QDialog::eventFilter(obj, e);
    }
}
