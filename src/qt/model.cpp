#include "rds/qt/model.hpp"
#include "qabstractitemmodel.h"
#include "qlogging.h"
#include "qnamespace.h"
#include "rds.hpp"
#include "rds/qt/column.hpp"
#include "rds/qt/common.hpp"
#include <memory>

namespace rds {

QModel::QModel(QObject *parent)
    : QAbstractItemModel(parent),
      columns_({DataId::Name, DataId::AddrPos, DataId::Value, cols::unit,
                DataId::Initial, DataId::Description}) {
  int i = 0;
  for (const auto &c : columns_) {
    setHeaderData(i, Qt::Orientation::Horizontal, c.header_label,
                  Qt::DisplayRole);
    setHeaderData(i, Qt::Orientation::Horizontal, c.data_name, Qt::EditRole);
    ++i;
  }
}

QModel::~QModel() {}

void QModel::set_headers(const std::vector<QColumn> &columns) {
  this->removeColumns(0, this->columns_.size());
  this->columns_ = columns;
  int i = 0;
  for (const auto &c : columns) {
    setHeaderData(i, Qt::Orientation::Horizontal, c.header_label,
                  Qt::DisplayRole);
    setHeaderData(i, Qt::Orientation::Horizontal, c.data_name, Qt::EditRole);
    ++i;
  }
}

QVariant QModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() or not(role == Qt::EditRole or role == Qt::DisplayRole))
    return {};
  if (role == Qt::EditRole and (columns_.size() > index.column() and
                                columns_[index.column()].id != DataId::Value))
    return {};

  const auto *item = static_cast<const Node *>(index.internalPointer());
  return item->data(columns_[index.column()], role);
}

bool QModel::setData(const QModelIndex &index, const QVariant &value,
                     int role) {
  if (not index.isValid() or
      (columns_.size() > index.column() and
       not(columns_.at(index.column()).id == DataId::Value or
           columns_.at(index.column()).id == DataId::ExtraData)) or
      role != Qt::EditRole)
    return false;
  bool ok;
  auto val = value.toULongLong(&ok);
  auto *node = reinterpret_cast<Node *>(index.internalPointer());
  if (not ok) {
    auto val = value.toLongLong(&ok);
    if (not ok)
      return false;
    node->setValue(val);
  } else {
    node->setValue(val);
  }
  emit dataChanged(index, index, {role});
  return true;
}

Qt::ItemFlags QModel::flags(const QModelIndex &index) const {
  return index.isValid()
             ? columns_.size() > index.column() and
                       columns_.at(index.column()).id == DataId::Value
                   ? Qt::ItemFlag::ItemIsEditable |
                         QAbstractItemModel::flags(index)
                   : QAbstractItemModel::flags(index)
             : Qt::ItemFlags(Qt::NoItemFlags);
}

bool QModel::setHeaderData(int section, Qt::Orientation orientation,
                           const QVariant &value, int role) {
  if (section > columns_.size())
    return false;

  if (section == columns_.size()) {
    qDebug() << "append column header";
    beginInsertColumns(index(0, 0, {}), section - 1, section);
    switch (role) {
    case Qt::DisplayRole:
      columns_.push_back(QColumn());
      columns_.at(section).header_label = value.toString();
      break;
    case Qt::EditRole:
      columns_.push_back(QColumn());
      columns_.at(section).data_name = value.toString();
      break;
    default:
      return false;
    }
    endInsertColumns();
    headerDataChanged(orientation, section, section);
    return true;
  }

  switch (role) {
  case Qt::DisplayRole:
    columns_.at(section).header_label = value.toString();
    break;
  case Qt::EditRole:
    columns_.at(section).data_name = value.toString();
    break;
  default:
    return false;
  }
  qDebug() << "change column header";
  headerDataChanged(orientation, section, section);
  return true;
}

QVariant QModel::headerData(int section, Qt::Orientation orientation,
                            int role) const {
  if (section >= columns_.size() or
      not(role == Qt::DisplayRole or role != Qt::EditRole))
    return {};
  switch (role) {
  case Qt::DisplayRole:
    return columns_.at(section).header_label;
    break;
  case Qt::EditRole:
    return columns_.at(section).data_name;
  default:
    return {};
  }
}

QModelIndex QModel::index(int row, int column,
                          const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent))
    return {};

  Node *parentItem = parent.isValid()
                         ? static_cast<Node *>(parent.internalPointer())
                         : device_.get();
  if (not parentItem)
    return {};

  if (auto *childItem = parentItem->child(row))
    return createIndex(row, column, childItem);
  return {};
}

QModelIndex QModel::parent(const QModelIndex &index) const {
  if (!index.isValid())
    return {};

  auto *childItem = static_cast<Node *>(index.internalPointer());
  Node *parentItem = childItem->parentItem();

  return parentItem != device_.get()
             ? createIndex(parentItem->row(), 0, parentItem)
             : QModelIndex{};
}

int QModel::rowCount(const QModelIndex &parent) const {
  if (parent.column() > 0)
    return 0;

  const Node *parentItem =
      parent.isValid() ? static_cast<const Node *>(parent.internalPointer())
                       : device_.get();

  return parentItem ? parentItem->childCount() : 0;
}

int QModel::columnCount(const QModelIndex &parent) const {
  return columns_.size();
}

QDevice *QModel::device() { return device_.get(); }

const QDevice *QModel::device() const { return device_.get(); }

void QModel::setDevice(std::unique_ptr<QDevice> &&device) {
  beginResetModel();
  device_ = std::move(device);
  endResetModel();
}

void QModel::load_file(const QString &path) {
  beginResetModel();
  const auto p = path.toStdString();
  device_ = std::make_unique<QDevice>(rds::device_from_file(p));
  endResetModel();
}

void QModel::save_file(const QString &path) {
  if (not device_ or not device_->dev_)
    return;

  rds::device_to_file(*device_->dev_, path.toStdString());
}

void QModel::setFormat(Fmt format) {
  if (not device_)
    return;
  int row = 0;
  QModelIndex br;
  for (auto &group : device_->groups_) {
    for (auto &reg : group->registers_) {
      reg->format_ = format;
      int frow = 0;
      for (auto &field : reg->fields_) {
        field->format_ = format;
        br = index(frow, columnCount(), index(row, columnCount(), {}));
        ++frow;
      }
      ++row;
    }
  }
  // dataChanged(index(0, 0, {}), index(0, 0, {}));
}

void QModel::clear() {
  beginResetModel();
  device_.reset();
  endResetModel();
}

rds::Contents QModel::contents() const {
  if (device_ == nullptr)
    return {};
  rds::Contents contents;
  for (const auto &g : device_->dev_->groups) {
    Groupdata gd;
    gd.base_addr = g->base_addr;
    for (const auto &reg : g->registers) {
      gd.registers.push_back({.address = reg->addr, .value = reg->value});
    }
    contents.groups.push_back(std::move(gd));
  }
  return contents;
}

void QModel::updateContents(const rds::Contents &contents) {
  if (device_ == nullptr)
    return;
  for (const auto &group : contents.groups) {
    rds::Group *g = device_->dev_->group(group.base_addr);
    for (const auto &r : group.registers) {
      auto *reg = g->reg(r.address);
      reg->set_value(r.value);
    }
    dataChanged(index(0, 0, {}), index(0, 0, {}));
  }
}
} // namespace rds
