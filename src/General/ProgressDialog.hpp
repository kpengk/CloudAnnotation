#pragma once
#include "GenericProgressCallback.hpp"

#include <QAtomicInt>
#include <QProgressDialog>

//! Graphical progress indicator (thread-safe)
/** Implements the GenericProgressCallback interface, in order
    to be passed to the CoreLib algorithms (check the
    CoreLib documentation for more information about the
    inherited methods).
**/
class ProgressDialog : public QProgressDialog, public GenericProgressCallback {
    Q_OBJECT

public:
    //! Default constructor
    /** By default, a cancel button is always displayed on the
        progress interface. It is only possible to activate or
        deactivate this button. Sadly, the fact that this button is
        activated doesn't mean it will be possible to stop the ongoing
        process: it depends only on the client algorithm implementation.
        \param cancelButton activates or deactivates the cancel button
        \param parent parent widget
    **/
    ProgressDialog(bool cancelButton = false, QWidget* parent = nullptr);

    //! Destructor (virtual)
    virtual ~ProgressDialog() {}

    // inherited method
    virtual void update(float percent) override;
    inline virtual void setMethodTitle(const char* methodTitle) override { setMethodTitle(QString(methodTitle)); }
    inline virtual void setInfo(const char* infoStr) override { setInfo(QString(infoStr)); }
    inline virtual bool isCancelRequested() override { return wasCanceled(); }
    virtual void start() override;
    virtual void stop() override;

    //! setMethodTitle with a QString as argument
    virtual void setMethodTitle(QString methodTitle);
    //! setInfo with a QString as argument
    virtual void setInfo(QString infoStr);

protected:
    //! Refreshes the progress
    /** Should only be called in the main Qt thread!
        This slot is automatically called by 'update' (in Qt::QueuedConnection mode).
    **/
    void refresh();

Q_SIGNALS:
    //! Schedules a call to refresh
    void scheduleRefresh();

protected:
    //! Current progress value (percent)
    QAtomicInt current_value_;

    //! Last displayed progress value (percent)
    QAtomicInt last_refresh_value_;
};
