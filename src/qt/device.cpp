#include "rds/qt/device.hpp"
#include "rds/qt/column.hpp"
#include "rds/qt/common.hpp"
#include <memory>

namespace rds {

QDevice::QDevice(std::unique_ptr<rds::Device> d) {
  connect(this, &Node::formatChanged, this, &QDevice::formatChanged);
  setDevice(std::move(d));
}

void QDevice::reset() {
  for (auto &g : groups_)
    g->reset();
}

Node *QDevice::child(int row) {
  return row < groups_.size() and row >= 0 ? groups_[row].get() : nullptr;
}

int QDevice::childCount() const { return groups_.size(); }

QVariant QDevice::data(const QColumn &column, int role) const {
  using enum DataId;
  switch (column.id) {
  case Name:
    return QString::fromStdString(dev_->name);
  case AddrPos:
    break;
  case Description:
    // return QString::fromStdString(group_->description);
    break;
  default:
    break;
  }
  return {};
}

int QDevice::row() const { return 0; }

Node *QDevice::parentItem() { return nullptr; }

QWidget *QDevice::createEditor(QWidget *, const QStyleOptionViewItem &) {
  return nullptr;
}

void QDevice::setEditorData(QWidget *) const {}

void QDevice::setModelData(QWidget *) {}

void QDevice::setDevice(std::unique_ptr<rds::Device> &&d) {
  groups_.clear();
  dev_ = std::move(d);
  if (not dev_.get())
    return;
  for (auto &g : dev_->groups) {
    groups_.push_back(std::unique_ptr<QGroup>{new QGroup(this, g.get())});
  }
}

void QDevice::setName(const QString &name) {
  if (not dev_ or dev_->name == name)
    return;
  dev_->name = name.toStdString();
  emit nameChanged(name);
}

QString QDevice::name() const {
  return dev_ ? QString::fromStdString(dev_->name) : QString{};
}

void QDevice::setVersion(int64_t v) {
  if (not dev_ or dev_->version == v)
    return;
  dev_->version = v;
  emit versionCanged(v);
}

int64_t QDevice::version() const { return dev_ ? dev_->version : 0; }

void QDevice::setRegisterWidth(int64_t num_bits) {
  if (not dev_ or dev_->register_width == num_bits)
    return;
  dev_->register_width = num_bits;
  emit registerWidthChanged(num_bits);
}

int64_t QDevice::registerWidth() const {
  return dev_ ? dev_->register_width : 0;
}

void QDevice::setNumPages(int64_t n) {
  if (not dev_ or dev_->num_pages == n)
    return;
  dev_->num_pages = n;
  emit numPagesChanged(n);
}

int64_t QDevice::numPages() const { return dev_ ? dev_->num_pages : 0; }

void QDevice::setRegistersPerPage(int64_t n) {
  if (not dev_ or dev_->registers_per_page == n)
    return;
  dev_->registers_per_page = n;
  emit registersPerPageChanged(n);
}

int64_t QDevice::registersPerPage() const {
  return dev_ ? dev_->registers_per_page : 0;
}

void QDevice::setExtraData(const QString &name, const QVariant &data) {
  if (not dev_)
    return;
  if (auto v = to_simple_value(data)) {
    auto n = name.toStdString();
    if (dev_->extra_data.contains(n)) {
      if (dev_->extra_data.at(n) == *v)
        return;
    }
    dev_->extra_data[n] = *v;
    emit extraDataChanged(name, data);
  }
}

QVariant QDevice::extraData(const QString &name) {
  if (not dev_)
    return {};
  if (auto it = dev_->extra_data.find(name.toStdString());
      it != dev_->extra_data.end())
    return to_qvariant(it->second);
  return {};
}

QList<const QGroup *> QDevice::groups() const {
  QList<const QGroup *> ret{};
  for (const auto &g : groups_)
    ret.append(g.get());
  return ret;
}

QGroup *QDevice::getGroup(const QString &name) {
  auto it = std::find_if(groups_.begin(), groups_.end(),
                         [name](const std::unique_ptr<QGroup> &group) {
                           return group->name() == name;
                         });
  if (it == groups_.end())
    return nullptr;
  else
    return it->get();
}

const QGroup *QDevice::getGroup(const QString &name) const {
  auto it = std::find_if(groups_.begin(), groups_.end(),
                         [name](const std::unique_ptr<QGroup> &group) {
                           return group->group_->name == name;
                         });
  if (it == groups_.end())
    return nullptr;
  else
    return it->get();
}

QGroup *QDevice::getGroup(uint64_t base_addr) {
  auto it = std::find_if(groups_.begin(), groups_.end(),
                         [base_addr](const std::unique_ptr<QGroup> &group) {
                           return group->group_->base_addr == base_addr;
                         });
  if (it == groups_.end())
    return nullptr;
  else
    return it->get();
}

const QGroup *QDevice::getGroup(uint64_t base_addr) const {
  auto it = std::find_if(groups_.begin(), groups_.end(),
                         [base_addr](const std::unique_ptr<QGroup> &group) {
                           return group->group_->base_addr == base_addr;
                         });
  if (it == groups_.end())
    return nullptr;
  else
    return it->get();
}

QGroup *QDevice::addGroup(const QString &name, uint64_t base_addr,
                          uint64_t size) {
  if (not dev_)
    return nullptr;

  const auto n = name.toStdString();
  auto *const group = dev_->add_group(n, base_addr, size);
  if (not group)
    return nullptr;

  int index =
      std::find_if(dev_->groups.begin(), dev_->groups.end(),
                   [group](const auto &g) { return g.get() == group; }) -
      dev_->groups.begin();
  auto it = groups_.insert(std::next(groups_.begin(), index),
                           std::unique_ptr<QGroup>(new QGroup(this, group)));
  emit groupAdded(it->get());
  return it->get();
}

void QDevice::removeGroup(QGroup *group) {
  auto it = std::find_if(groups_.begin(), groups_.end(),
                         [group](const auto &g) { return g.get() == group; });
  if (it == groups_.end())
    return;
  emit groupRemoved(it->get());
  groups_.erase(it);
}

void QDevice::removeGroup(const QString &name) {
  auto it = std::find_if(groups_.begin(), groups_.end(),
                         [name](const auto &g) { return g->name() == name; });
  if (it == groups_.end())
    return;
  emit groupRemoved(it->get());
  groups_.erase(it);
}

void QDevice::onFormatChanged(Fmt fmt) {
  for (auto &g : groups_)
    g->setFormat(fmt);
}
} // namespace rds
