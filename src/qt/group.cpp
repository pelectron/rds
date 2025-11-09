#include "rds/qt/group.hpp"
#include "qassert.h"
#include "rds/qt/column.hpp"
#include "rds/qt/common.hpp"
#include "rds/qt/device.hpp"
#include <memory>

namespace rds {

QGroup::QGroup(QDevice *parent, rds::Group *group)
    : group_(group), parent_(parent) {
  Q_ASSERT(group != nullptr);
  Q_ASSERT(parent != nullptr);
  connect(this, &Node::formatChanged, this, &QGroup::onFormatChanged);
  for (auto &r : group->registers) {
    registers_.push_back(
        std::unique_ptr<QRegister>(new QRegister(this, r.get())));
  }
}

void QGroup::reset() {}

QDevice *QGroup::device() { return parent_; }

const QDevice *QGroup::device() const { return parent_; }

Node *QGroup::child(int row) {
  return row < registers_.size() and row >= 0 ? registers_[row].get() : nullptr;
}

int QGroup::childCount() const { return registers_.size(); }

QVariant QGroup::data(const QColumn &column, int role) const {
  using enum DataId;
  switch (column.id) {
  case Name:
    return name();
    break;
  case AddrPos:
    return to_string(group_->base_addr, Fmt::hex);
    break;
  case Description:
    break;
  default:
    break;
  }
  return {};
}

int QGroup::row() const {
  int i = 0;
  for (const auto &g : parent_->groups_)
    if (g.get() == this)
      return i;
    else
      ++i;
  return i;
}

Node *QGroup::parentItem() { return parent_; }

QWidget *QGroup::createEditor(QWidget *, const QStyleOptionViewItem &) {
  return nullptr;
}

void QGroup::setEditorData(QWidget *) const {}

void QGroup::setModelData(QWidget *) {}

void QGroup::setName(const QString &name) {
  if (not group_ or group_->name == name)
    return;
  group_->name = name.toStdString();
  emit nameChanged(name);
}

QString QGroup::name() const { return QString::fromStdString(group_->name); }

void QGroup::setDescription(const QString &description) {
  // TODO: add description to group
}

QString QGroup::description() const { return {}; }

void QGroup::setBaseAddr(uint64_t base_addr) {
  if (not group_ or group_->base_addr == base_addr)
    return;
  group_->base_addr = base_addr;
  emit baseAddrChanged(base_addr);
}

uint64_t QGroup::baseAddr() const { return group_->base_addr; }

void QGroup::setSize(uint64_t size) {
  if (not group_ or group_->size == size)
    return;
  group_->size = size;
  emit sizeChanged(size);
}

uint64_t QGroup::size() const { return group_->size; }

void QGroup::setDefault(const QString &name, const QVariant &value) {
  if (not group_)
    return;
  if (auto v = to_simple_value(value)) {
    auto n = name.toStdString();
    if (group_->defaults.contains(n)) {
      if (group_->defaults.at(n) == *v)
        return;
    }
    group_->defaults[n] = *v;
    emit defaultsChanged(name, value);
  }
}

QVariant QGroup::defaults(const QString &name) const {
  if (auto it = group_->defaults.find(name.toStdString());
      it != group_->defaults.end())
    return to_qvariant(it->second);
  return {};
}

bool QGroup::hasRegister(const QString &name) const {
  return getRegister(name) != nullptr;
}

QRegister *QGroup::getRegister(const QString &name) {
  auto it = std::find_if(registers_.begin(), registers_.end(),
                         [name](const std::unique_ptr<QRegister> &r) {
                           return r->reg_->name == name;
                         });
  if (it == registers_.end())
    return nullptr;
  else
    return it->get();
}

const QRegister *QGroup::getRegister(const QString &name) const {
  auto it = std::find_if(registers_.begin(), registers_.end(),
                         [name](const std::unique_ptr<QRegister> &r) {
                           return r->reg_->name == name;
                         });
  if (it == registers_.end())
    return nullptr;
  else
    return it->get();
}

QRegister *QGroup::getRegister(uint64_t addr) {
  auto it = std::find_if(registers_.begin(), registers_.end(),
                         [addr](const std::unique_ptr<QRegister> &r) {
                           return r->reg_->addr == addr;
                         });
  if (it == registers_.end())
    return nullptr;
  else
    return it->get();
}

const QRegister *QGroup::getRegister(uint64_t addr) const {
  auto it = std::find_if(registers_.begin(), registers_.end(),
                         [addr](const std::unique_ptr<QRegister> &r) {
                           return r->reg_->addr == addr;
                         });
  if (it == registers_.end())
    return nullptr;
  else
    return it->get();
}

QRegister *QGroup::addRegister(const QString &name, uint64_t addr,
                               uint64_t size) {
  if (not group_)
    return nullptr;

  auto n = name.toStdString();
  auto *reg = group_->add_reg(n, addr, size);
  if (not reg)
    return nullptr;

  int index = std::find_if(group_->registers.begin(), group_->registers.end(),
                           [reg](const auto &r) { return r.get() == reg; }) -
              group_->registers.begin();
  auto it =
      registers_.insert(std::next(registers_.begin(), index),
                        std::unique_ptr<QRegister>(new QRegister(this, reg)));
  emit registerAdded(it->get());
  return it->get();
}

void QGroup::removeRegister(QRegister *reg) {

  auto it = std::find_if(registers_.begin(), registers_.end(),
                         [reg](const auto &r) { return r.get() == reg; });
  if (it == registers_.end())
    return;
  emit registerRemoved(reg);
  registers_.erase(it);
}

void QGroup::removeRegister(const QString &name) {
  auto it = std::find_if(registers_.begin(), registers_.end(),
                         [name](const auto &r) { return r->name() == name; });
  if (it == registers_.end())
    return;
  removeRegister(it->get());
}

QList<const QRegister *> QGroup::registers() const {
  QList<const QRegister *> ret;
  for (const auto &r : registers_)
    ret.append(r.get());
  return ret;
}

void QGroup::onFormatChanged(Fmt fmt) {
  for (auto &r : registers_)
    r->setFormat(fmt);
}
} // namespace rds
