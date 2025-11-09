#ifndef RDS_QT_IO_HPP
#define RDS_QT_IO_HPP

#include "rds/common.hpp"
#include "rds/qt/common.hpp"

#include <QProgressBar>
#include <QStatusBar>
#include <cstdint>

namespace rds {

/**
 * This class is the extension point for IO operations.
 * It is used by rds::Widget to actually write and read memory content of an
 * embedded device.
 *
 * To enable IO use in the Widget, derive from rds::IO, implement the
 * read, write, and error_string methods, and pass an instance of IO to the
 * constructor of Widget or set the IO with the Widget's ``setIO`` method.
 *
 * IO also has the ability to send messages and show progress on the viewers
 * status/progress bar. To show messages, use `showMessage`.
 *
 * To update the progress bar, use `update`.
 */
class IO : public QObject {
  Q_OBJECT;
  Q_PROPERTY(int progress READ progress WRITE update NOTIFY progressChanged);

public:
  enum class Error : std::uint32_t {
    None,
    NotConnected,
    InvalidAddress,
    ConnectionAborted,
    ProtocolError,
    InvalidOperation,
  };

  IO(QStatusBar *status = nullptr, QProgressBar *progress = nullptr,
     QObject *parent = nullptr);

  virtual ~IO();

  /// updates the progress bar. value must be between 0 and 100 to indicate 0 to
  /// 100%.
  void update(int value);

  /// gets the current progress value.
  int progress() const;

  /**
   * @brief show a message for a duration of timeout milliseconds.
   *
   * @param msg the message text
   * @param timeout how long to show the message for in milliseconds. A value of
   * 0 means no timeout, i.e. the message is displayed unitl the next
   * showMessage call.
   */
  void showMessage(const QString &msg, int timeout = 0);

  /**
   * write the contents to the embedded device.
   *
   * @param contents the memory contents
   * @return the error, if any occured.
   */
  virtual Error write(const rds::Contents &contents) = 0;

  /**
   * @brief reads content of the embdedded device.
   *
   * This reads the registers indicated by contents and fills in the values read
   * from the embedded device.
   * @param contents the memory to read.
   * @return the error, if any occured.
   */
  virtual Error read(rds::Contents &contents) = 0;

  /**
   * returns the error message that should be displayed for e.
   *
   * @param e the error
   */
  virtual QString error_string(Error e);

signals:
  void progressChanged(int progress);

private:
  friend class Widget;
  QStatusBar *status_;
  QProgressBar *progress_;
};

} // namespace rds

#endif
