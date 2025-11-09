#include "rds/qt/io.hpp"
#include "qprogressbar.h"

namespace rds {

IO::IO(QStatusBar *status, QProgressBar *progress, QObject *parent)
    : QObject(parent), progress_(progress) {
  if (progress_)
    connect(progress_, &QProgressBar::valueChanged, this, &IO::progressChanged);
}

IO::~IO() {}

void IO::update(int value) {
  if (progress_)
    progress_->setValue(value);
}

int IO::progress() const { return progress_ ? progress_->value() : 0; }

void IO::showMessage(const QString &msg, int timeout) {
  if (status_)
    status_->showMessage(msg, timeout);
}

QString IO::error_string(Error e) {
  switch (e) {
  case IO::Error::None:
    return "";
  case IO::Error::NotConnected:
    return "IO Error: Protocol";
  case IO::Error::InvalidAddress:
    return "IO Error: nvalid Address";
  case IO::Error::ConnectionAborted:
    return "IO Error: Connection Aborted";
  case IO::Error::ProtocolError:
    return "IO Error: Protocol";
  case IO::Error::InvalidOperation:
    return "IO Error: Invalid Operation";
  default:
    return "IO Error: Unknown";
  }
}
} // namespace rds
