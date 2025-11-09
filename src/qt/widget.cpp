#include "rds/qt/widget.hpp"
#include "rds/qt/common.hpp"
#include "rds/qt/delegate.hpp"
#include "rds/qt/model.hpp"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QMenu>

namespace rds {

Widget::Widget(Options options, IO *io, QWidget *parent)
    : QWidget(parent), model_(new QModel), delegate_(new Delegate), io_(io),
      context_menu_(new QMenu), fmt_int_(new QAction("normal")),
      fmt_hex_(new QAction("hex")), fmt_bin_(new QAction("binary")) {
  setOptions(options);
}

Widget::~Widget() {
  if (delegate_)
    delegate_->deleteLater();
  if (model_)
    model_->deleteLater();
  if (context_menu_)
    context_menu_->deleteLater();
  if (fmt_int_)
    fmt_int_->deleteLater();
  if (fmt_hex_)
    fmt_hex_->deleteLater();
  if (fmt_bin_)
    fmt_bin_->deleteLater();
}

void Widget::load_file(const QString &path) {
  try {
    model_->load_file(path);
    if (status_)
      status_->showMessage("Loaded from '" + path + "'");
    // tree_view->hideColumn(1);
    emit fileLoaded(path);
  } catch (const std::exception &e) {
    if (status_)
      status_->showMessage("Failed to load file '" + path + "': " + e.what());
  }
}

void Widget::save_file(const QString &path) {
  try {
    model_->save_file(path);
    if (status_)
      status_->showMessage("Saved to '" + path + "'");
    emit fileSaved(path);
  } catch (const std::exception &e) {
    if (status_)
      status_->showMessage("Failed to save file to '" + path +
                           "': " + e.what());
    return;
  }
}

QDevice *Widget::device() { return model_->device_.get(); }

const QDevice *Widget::device() const { return model_->device_.get(); }

void Widget::setDevice(std::unique_ptr<QDevice> &&device) {
  model_->setDevice(std::move(device));
}

rds::Contents Widget::contents() const { return model_->contents(); }

void Widget::updateContents(const rds::Contents &contents) {
  return model_->updateContents(contents);
}

void Widget::setFormat(Fmt format) { model_->setFormat(format); }

void Widget::write() {
  if (not io_) {
    if (status_)
      status_->showMessage("IO Error: IO unavailable", 5000);
    return;
  }

  IO::Error e = io_->write(contents());

  if (not status_)
    return;

  if (e == IO::Error::None) {
    status_->showMessage("Write Successfull", 5000);
  } else {
    status_->showMessage(io_->error_string(e));
  }
}

void Widget::read() {
  if (not io_) {
    if (status_)
      status_->showMessage("IO Error: IO unavailable", 5000);
    return;
  }

  auto c = contents();
  IO::Error e = io_->read(c);
  if (status_) {
    if (e == IO::Error::None) {
      updateContents(c);
      status_->showMessage("Read Successfull", 5000);
    } else {
      status_->showMessage(io_->error_string(e), 5000);
    }
  }
}

void Widget::setIO(IO *io) {
  if (io_)
    io_->deleteLater();

  io_ = io;

  if (io_)
    io_->setParent(this);
}

IO *Widget::getIO() { return io_; }

IO *Widget::releaseIO() {
  auto ret = io_;
  io_ = nullptr;
  return ret;
}

void Widget::setOptions(Options options) {
  if (options_ == options)
    return;

  if (layout_) {
    layout_->deleteLater();
    button_layout_ = nullptr;
    load_ = nullptr;
    save_ = nullptr;
    write_ = nullptr;
    read_ = nullptr;
    filter_ = nullptr;
    view_ = nullptr;
    status_ = nullptr;
    progress_ = nullptr;
  }

  view_ = new QTreeView;
  connect(view_, &QTreeView::customContextMenuRequested, this,
          &Widget::onCustomContextMenu);
  layout_ = new QVBoxLayout;
  this->setLayout(layout_);

  if (contains_any_of(options, Options::LoadButton | Options::SaveButton |
                                   Options::WriteButton |
                                   Options::ReadButton)) {
    button_layout_ = new QHBoxLayout;
    layout_->addLayout(button_layout_);
    if (contains(options, Options::LoadButton)) {
      load_ = new QPushButton("Load From File...");
      button_layout_->addWidget(load_);
      connect(load_, &QPushButton::clicked, this, &Widget::onLoadPressed);
    }

    if (contains(options, Options::SaveButton)) {
      save_ = new QPushButton("Save To File...");
      button_layout_->addWidget(save_);
      connect(save_, &QPushButton::clicked, this, &Widget::onSavePressed);
    }

    if (contains(options, Options::ReadButton)) {
      read_ = new QPushButton("Read...");
      button_layout_->addWidget(read_);
      connect(read_, &QPushButton::clicked, this, &Widget::onReadPressed);
    }

    if (contains(options, Options::WriteButton)) {
      write_ = new QPushButton("Write...");
      button_layout_->addWidget(write_);
      connect(write_, &QPushButton::clicked, this, &Widget::onWritePressed);
    }
  }

  if (contains(options, Options::FilterSort)) {
    filter_ = new Filter(view_);
    filter_->setModel(model_);
    delegate_->setFilterModel(filter_->proxy());
    layout_->addWidget(filter_);
  } else {
    view_->setModel(model_);
    delegate_->setFilterModel(nullptr);
  }

  view_->setItemDelegate(delegate_);
  view_->setSelectionMode(QAbstractItemView::ExtendedSelection);
  view_->setSelectionBehavior(QAbstractItemView::SelectItems);
  view_->setContextMenuPolicy(Qt::CustomContextMenu);
  layout_->addWidget(view_);

  if (contains(options, Options::StatusBar)) {
    status_ = new QStatusBar();
    status_->showMessage("No File Loaded");
    layout_->addWidget(status_);
  }

  if (contains(options, Options::StatusBar | Options::ProgessBar)) {
    progress_ = new QProgressBar;
    progress_->setRange(0, 100);
    progress_->setValue(0);
    progress_->setEnabled(false);
    status_->addWidget(progress_);
  }

  if (io_) {
    io_->status_ = status_;
    io_->progress_ = progress_;
  }

  options_ = options;
}

void Widget::onSavePressed() {
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::AnyFile);
  QStringList fileNames;
  if (not dialog.exec())
    return;
  fileNames = dialog.selectedFiles();
  if (fileNames.isEmpty())
    return clear();

  save_file(fileNames[0]);
}

void Widget::onLoadPressed() {
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::ExistingFile);
  QStringList fileNames;
  if (not dialog.exec())
    return;
  fileNames = dialog.selectedFiles();
  if (fileNames.isEmpty())
    return clear();

  load_file(fileNames[0]);
}

void Widget::onWritePressed() {
  emit writePressed(this);
  this->write();
}

void Widget::onReadPressed() {
  emit readPressed(this);
  this->read();
}

void Widget::onCustomContextMenu(const QPoint &p) {
  qDebug() << "onCustomContextMenu" << p;
  QModelIndex i = view_->indexAt(p);
  if (filter_)
    i = filter_->proxy()->mapToSource(i);
  Node *node = i.isValid() ? reinterpret_cast<Node *>(i.internalPointer())
                           : model_->device_.get();
  if (node == nullptr)
    return;

  QAction *a = context_menu_->exec(view_->viewport()->mapToGlobal(p));
  if (a == fmt_int_) {
    node->setFormat(Fmt::integer);
  } else if (a == fmt_hex_) {
    node->setFormat(Fmt::hex);
  } else if (a == fmt_bin_) {
    node->setFormat(Fmt::binary);
  } else {
    return;
  }
}

void Widget::clear() {
  model_->clear();
  if (filter_)
    filter_->clear();
  status_->showMessage("No File Loaded");
}

} // namespace rds
