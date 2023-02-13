#pragma once
#include <QDialog>
#include <QList>

class PointCloudWidget;

//! Generic overlay dialog interface
class OverlayDialog : public QDialog {
    Q_OBJECT

signals:

    //! Signal emitted when process is finished
    /** \param accepted specifies how the process finished (accepted or not)
     **/
    void processFinished(bool accepted);

    //! Signal emitted when an overridden key shortcut is pressed
    /** See ccOverlayDialog::addOverriddenShortcut
     **/
    void shortcutTriggered(int key);

    //! Signal emitted when a 'show' event is detected
    void shown();

public:
    explicit OverlayDialog(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::Tool);

    //! Destructor
    ~OverlayDialog() override;

    //! Links the overlay dialog with a MDI window
    /** Warning: link can't be modified while dialog is displayed/process is running!
            \return success
    **/
    virtual bool linkWith(PointCloudWidget* win);

    //! Starts process
    /** \return success
     **/
    virtual bool start();

    //! Stops process/dialog
    /** Automatically emits the 'processFinished' signal (with input state as argument).
            \param accepted process/dialog result
    **/
    virtual void stop(bool accepted);

    // reimplemented from QDialog
    void reject() override;

    //! Adds a keyboard shortcut (single key) that will be overridden from the associated window
    /** When an overridden key is pressed, the shortcutTriggered(int) signal is emitted.
     **/
    void addOverriddenShortcut(Qt::Key key);

    //! Returns whether the tool is currently started or not
    bool started() const { return processing_; }

protected:
    //! Slot called when the linked window is deleted (calls 'onClose')
    virtual void onLinkedWindowDeletion(QObject* object = nullptr);

protected:
    // inherited from QObject
    bool eventFilter(QObject* obj, QEvent* e) override;

    PointCloudWidget* associated_win_;

    //! Running/processing state
    bool processing_;

    //! Overridden keys
    QList<int> overridden_keys_;
};
