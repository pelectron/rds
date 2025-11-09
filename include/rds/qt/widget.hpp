#ifndef RDS_QT_WIDGET_HPP
#define RDS_QT_WIDGET_HPP

#include "rds/common.hpp"
#include "rds/qt/common.hpp"
#include "rds/qt/delegate.hpp"
#include "rds/qt/filter.hpp"
#include "rds/qt/io.hpp"

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>

namespace rds {

/**
 * This is the actual widget that displays a device.
 *
 * Its main purpose is to display an rds::Device.
 *
 * Depending on the options set, the following features are available:
 *  - loading a device from disk
 *  - saving a device to disk
 *  - writing the contents to the physical device
 *  - reading the contents from the physical device
 *  - sorting and filtering
 *  - a status bar
 *
 *  To write and read contents from/to the physical device, an IO object needs
 * to be implemented and set on the Widget. See rds::IO for more details.
 * @addtogroup qrds
 */
class Widget : public QWidget {
  Q_OBJECT;
  Q_DISABLE_COPY(Widget);

public:
  /**
   * constructs a Widget.
   * @param options the Widget options. These can also be set with
   * ``setOptions()``.
   * @param io the io object that allows communication. This can also be set
   * with ``setIO`()``.
   * @param parent the parent QWidget.
   */
  Widget(Options options = Options::None, IO *io = nullptr,
         QWidget *parent = nullptr);

  ~Widget();

  /**
   * Set the widget features/options.
   * All options are set at once. Calling setOptions a second time will override
   * the options of the first call.
   *
   * @param opts the potions to be enabled
   */
  void setOptions(Options opts);

  /**
   * loads a device description from a file.
   *
   * @param path the file path
   */
  void load_file(const QString &path);

  /**
   * save a device to a file
   *
   * @param path the file path
   */
  void save_file(const QString &path);

  /**
   * returns a pointer to the device of the widget.
   * @note the device's lifetime is managed by the widget. Do not delete the
   * pointer returned by this method. If you want to delete the device, use
   * ``setDevice(std::unique_ptr<rds::QDevice>{});``.
   */
  QDevice *device();

  /**
   * returns a const pointer to the device of the widget.
   */
  const QDevice *device() const;

  /**
   * set the device to be displayed.
   *
   * @param device the device
   */
  void setDevice(std::unique_ptr<QDevice> &&device);

  /**
   * returns the currently selected contents of the device.
   * TODO: respect user selection
   */
  rds::Contents contents() const;

  /**
   * sets the register contents.
   */
  void updateContents(const rds::Contents &contents);

  /**
   * sets the value format, i.e. the format used for displaying the value field,
   * for every register and field in the device.
   * @param format the integer format to use.
   */
  void setFormat(Fmt format);

  /// writes the currently selected contents to the physical device
  void write();

  /// reads the currently selected content from the physical device
  void read();

  /**
   * sets the IO object to use for read and write operations. Without setting an
   * IO object, read and write operations always fail.
   *
   * @note the Widget takes care of the lifetime management. Do not delete an IO
   * object that is set on a Widget. Use ``releaseIO()`` if you want to delete
   * the IO object yourself.
   *
   * @param io the io object. May be nullptr to destroy the current IO object.
   */
  void setIO(IO *io);

  /**
   * returns a pointer the IO object.
   * @note the Widget takes care of the lifetime management. Do not delete an IO
   * object that is set on a Widget. Use ``releaseIO()`` if you want to delete
   * the IO object yourself.
   */
  IO *getIO();

  /**
   * releases ownership of the IO object and returns a pointer to it.
   *
   * @note the object pointed to by the return value must be deleted manually.
   *
   * @return pointer to the IO object, or nullptr if no object was set.
   */
  IO *releaseIO();

  /**
   * clears the Widget. This will reset everything except the options (also
   * unloading any file that may loaded).
   */
  void clear();

signals:

  /**
   * This signal is emitted every time a file is successfully saved.
   *
   * @param path the path to where the device was saved to.
   */
  void fileSaved(const QString &path);

  /**
   * This signal is emitted when a device is loaded from disk successfully.
   *
   * @param path the path to the file the deivce was laoded from.
   */
  void fileLoaded(const QString &path);

  /**
   * This signal is emitted every time the Widget's write button is pressed.
   * @param w the widget
   */
  void writePressed(Widget *w);

  /**
   * This signal is emitted when the widegt's read button is pressed.
   *
   * @param w
   */
  void readPressed(Widget *w);

  void error(Error e);

private slots:
  void onSavePressed();
  void onLoadPressed();
  void onWritePressed();
  void onReadPressed();
  void onCustomContextMenu(const QPoint &);

private:
  QVBoxLayout *layout_ = nullptr;
  QTreeView *view_ = nullptr;
  QModel *model_ = nullptr;
  Filter *filter_ = nullptr;
  Delegate *delegate_ = nullptr;
  QHBoxLayout *button_layout_ = nullptr;
  QPushButton *load_ = nullptr;
  QPushButton *save_ = nullptr;
  QPushButton *write_ = nullptr;
  QPushButton *read_ = nullptr;
  QStatusBar *status_ = nullptr;
  QProgressBar *progress_ = nullptr;
  IO *io_ = nullptr;
  QMenu *context_menu_ = nullptr;
  QAction *fmt_int_ = nullptr;
  QAction *fmt_hex_ = nullptr;
  QAction *fmt_bin_ = nullptr;
  Options options_{Options::None};
};

} // namespace rds

#endif
