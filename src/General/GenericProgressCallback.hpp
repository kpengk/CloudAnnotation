#pragma once

//! A generic progress indicator interface to notify algorithms progress to the client application
class GenericProgressCallback {
public:
    //! Default destructor
    virtual ~GenericProgressCallback() = default;

    //! Notifies the algorithm progress
    /** The notification is sent by the running algorithm (on the library side).
        This virtual method shouldn't be called too often, as the real process
        behind it is unspecified and may be time consuming. Ideally it shouldn't
        be called more than a few hundreds time.
        \param percent current progress, between 0.0 and 100.0
    **/
    virtual void update(float percent) = 0;

    //! Notifies the algorithm title
    /** The notification is sent by the ongoing algorithm (on the library side).
        \param methodTitle the algorithm title
    **/
    virtual void setMethodTitle(const char* methodTitle) = 0;

    //! Notifies some information about the ongoing process
    /** The notification is sent by the ongoing algorithm (on the library side).
        \param infoStr some textual information about the ongoing process
    **/
    virtual void setInfo(const char* infoStr) = 0;

    //! Notifies the fact that every information has been sent and that the process begins
    /** Once start() is called, the progress bar and other informations could be displayed (for example).
     **/
    virtual void start() = 0;

    //! Notifies the fact that the process has ended
    /** Once end() is called, the progress bar and other informations could be hidden (for example).
     **/
    virtual void stop() = 0;

    //! Checks if the process should be canceled
    /** This method is called by some process from time to time to know if it
        should halt before its normal ending. This is a way for the client application
        to cancel an ongoing process (but it won't work with all algorithms).
        Process results may be incomplete/void. The cancel requirement mechanism must
        be implemented (typically a simple "cancel()" method that will be called by the
        client application).
    **/
    virtual bool isCancelRequested() = 0;

    //! Returns whether the dialog title and info can be updated or not
    virtual bool textCanBeEdited() const { return true; }
};
