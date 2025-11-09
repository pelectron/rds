#include "rds/qt/register.hpp"
#include "qassert.h"
#include "qcontainerfwd.h"
#include "qlogging.h"
#include "rds/device.hpp"
#include "rds/qt/column.hpp"
#include "rds/qt/common.hpp"
#include "rds/qt/device.hpp"
#include "rds/qt/editors.hpp"
#include "rds/qt/group.hpp"
#include "rds/register.hpp"
#include <cstdint>

namespace rds {

QRegister::QRegister(QGroup *parent, rds::Register *reg)
    : reg_(reg), parent_(parent) {
  Q_ASSERT(reg != nullptr);
  Q_ASSERT(parent != nullptr);
  for (auto &f : reg_->fields)
    fields_.push_back(std::unique_ptr<QField>(new QField(this, f.get())));
  connect(this, &Node::formatChanged, this, &QRegister::onFormatChanged);
}

bool QRegister::addValue(const EnumeratedValue &val) {
  if (not reg_)
    return false;
  if (reg_->add_value(val)) {
    emit valueAdded(val);
    return true;
  }
  return false;
}

bool QRegister::addValues(const QList<EnumeratedValue> &vals) {
  if (not reg_)
    return false;
  std::vector<EnumeratedValue> vs{vals.begin(), vals.end()};
  if (reg_->add_values(vs)) {
    for (const auto &v : vals)
      emit valueAdded(v);
    return true;
  }
  return false;
}

void QRegister::removeValue(const QString &name) {
  auto it = std::find_if(
      reg_->values.begin(), reg_->values.end(),
      [&name](const EnumeratedValue &v) { return v.name == name; });
  if (it == reg_->values.end())
    return;
  auto v = *it;
  reg_->values.erase(it);
  valueRemoved(v);
}

void QRegister::removeValue(uint64_t val) {
  auto it =
      std::find_if(reg_->values.begin(), reg_->values.end(),
                   [&val](const EnumeratedValue &v) { return v.value == val; });
  if (it == reg_->values.end())
    return;
  auto v = *it;
  reg_->values.erase(it);
  valueRemoved(v);
}

QList<EnumeratedValue> QRegister::values() const {
  QList<EnumeratedValue> ret;
  for (const auto &v : reg_->values)
    ret.append(v);
  return ret;
}

void QRegister::reset() {
  if (reg_)
    setValue(reg_->initial);
}

QGroup *QRegister::group() { return parent_; }

const QGroup *QRegister::group() const { return parent_; }

QString QRegister::name() const {
  return reg_ ? QString::fromStdString(reg_->name) : QString{};
}

QString QRegister::displayName() const {
  return reg_ ? QString::fromStdString(reg_->display_name) : QString{};
}

QString QRegister::description() const {
  return reg_ ? QString::fromStdString(reg_->description) : QString{};
}

QString QRegister::backup() const {
  return reg_ ? QString::fromStdString(reg_->backup) : QString{};
}

QString QRegister::unit() const {
  return reg_ ? QString::fromStdString(reg_->unit) : QString{};
}

double QRegister::zeroCodeValue() const {
  return reg_ ? reg_->zero_code_value : 0;
}

double QRegister::step() const { return reg_ ? reg_->step : 0; }

uint64_t QRegister::addr() const { return reg_ ? reg_->addr : 0; }

uint64_t QRegister::size() const { return reg_ ? reg_->size : 0; }

uint64_t QRegister::zerosMask() const { return reg_ ? reg_->zeros_mask : 0; }

uint64_t QRegister::onesMask() const { return reg_ ? reg_->ones_mask : 0; }

uint64_t QRegister::xMask() const { return reg_ ? reg_->x_mask : 0; }

uint64_t QRegister::initial() const { return reg_ ? reg_->initial : 0; }

Access QRegister::access() const { return reg_ ? reg_->access : Access{}; }

bool QRegister::isSigned() const { return reg_ ? reg_->is_signed : false; }

uint64_t QRegister::numBits() const { return reg_ ? reg_->num_bits() : 0; }

QVariant QRegister::extraData(const QString &name) const {
  if (not reg_)
    return {};
  auto n = name.toStdString();
  if (auto it = reg_->extra_data.find(n); it != reg_->extra_data.end())
    return to_qvariant(it->second);
  return {};
}

void QRegister::removeExtraData(const QString &name) {
  auto n = name.toStdString();
  if (auto it = reg_->extra_data.find(n); it != reg_->extra_data.end()) {
    reg_->extra_data.erase(it);
    extraDataChanged(name, {});
  }
}
void QRegister::setAccessor(Accessor a) {
  if (not a)
    return;
  access_ = std::move(a);
}
void QRegister::setName(const QString &name) {
  if (name == reg_->name)
    return;
  reg_->name = name.toStdString();
  emit nameChanged(name);
}

void QRegister::setDisplayName(const QString &name) {
  if (name == reg_->display_name)
    return;
  reg_->display_name = name.toStdString();
  emit displayNameChanged(name);
}

void QRegister::setDescription(const QString &description) {
  if (description == reg_->description)
    return;
  reg_->description = description.toStdString();
  emit descriptionChanged(description);
}

void QRegister::setBackup(const QString &backup) {
  if (backup == reg_->backup)
    return;
  reg_->backup = backup.toStdString();
  emit descriptionChanged(backup);
}

void QRegister::setUnit(const QString &unit) {
  if (unit == reg_->unit)
    return;
  reg_->unit = unit.toStdString();
  emit unitChanged(unit);
}

void QRegister::setZeroCodeValue(double value) {
  if (value == reg_->zero_code_value)
    return;
  reg_->zero_code_value = value;
  zeroCodeValueChanged(value);
}

void QRegister::setStep(double step) {
  if (step == reg_->step)
    return;
  reg_->step = step;
  zeroCodeValueChanged(step);
}

void QRegister::setInitial(uint64_t initial) {
  if (initial == reg_->initial)
    return;
  reg_->initial = initial;
  emit initialChanged(initial);
}

void QRegister::setAddr(uint64_t addr) {
  if (addr == reg_->addr)
    return;
  reg_->addr = addr;
  emit addrChanged(addr);
}

void QRegister::setSize(uint64_t size) {
  if (size == reg_->size)
    return;
  reg_->size = size;
  emit sizeChanged(size);
}

void QRegister::setZerosMask(uint64_t mask) {
  if (mask == reg_->zeros_mask)
    return;
  reg_->zeros_mask = mask;
  emit zerosMaskChanged(mask);
}

void QRegister::setOnesMask(uint64_t mask) {
  if (mask == reg_->ones_mask)
    return;
  reg_->ones_mask = mask;
  emit onesMaskChanged(mask);
}

void QRegister::setXMask(uint64_t mask) {
  if (mask == reg_->x_mask)
    return;
  reg_->x_mask = mask;
  emit xMaskChanged(mask);
}

void QRegister::setAccess(Access access) {
  if (access == reg_->access)
    return;
  reg_->access = access;
  emit accessChanged(access);
}

void QRegister::setIsSigned(bool is_signed) {
  if (is_signed == reg_->is_signed)
    return;
  reg_->is_signed = is_signed;
  emit isSignedChanged(is_signed);
}

void QRegister::setExtraData(const QString &name, const QVariant &data) {
  if (not reg_)
    return;
  if (auto v = to_simple_value(data)) {
    auto n = name.toStdString();
    if (reg_->extra_data.contains(n)) {
      if (reg_->extra_data.at(n) == *v)
        return;
    }
    reg_->extra_data[n] = *v;
    emit extraDataChanged(name, data);
  }
}

Node *QRegister::child(int row) {
  return row < fields_.size() and row >= 0 ? fields_[row].get() : nullptr;
}

int QRegister::childCount() const { return fields_.size(); }

QVariant QRegister::data(const QColumn &column, int role) const {
  if (column.id == DataId::Custom and column.register_data)
    return column.register_data(*this, role);
  else
    return access_(*this, column, role);
}

int QRegister::row() const {
  int i = 0;
  for (const auto &r : parent_->registers_)
    if (r.get() == this)
      return i;
    else
      ++i;
  return i;
}

Node *QRegister::parentItem() { return parent_; }

QWidget *QRegister::createEditor(QWidget *parent,
                                 const QStyleOptionViewItem &option) {
  if (not reg_->values.empty()) {
    auto *cb = new QComboBox;
    int i = 0;
    for (const auto &v : reg_->values) {
      cb->addItem(QString::fromStdString(v.name), v.value);
      if (v.value == reg_->value)
        cb->setCurrentIndex(i);
      else
        ++i;
    }
    return cb;
  } else if (reg_->is_signed) {
    auto *fb = new SignedSpinBox(parent);
    fb->setFormat(format_);
    fb->setValue(reg_->value);
    fb->setMaximum((1 << (reg_->num_bits() - 1)) - 1);
    fb->setMinimum(-(1 << (reg_->num_bits() - 1)));
    return fb;
  } else {
    qDebug() << "Register::createEditor UnsignedSpinBox";
    auto *fb = new UnsignedSpinBox(parent);
    fb->setFormat(format_);
    fb->setValue(reg_->value);
    fb->setMaximum((1u << reg_->num_bits()) - 1u);
    return fb;
  }
}

void QRegister::setEditorData(QWidget *editor) const {
  if (not reg_->values.empty()) {
    int i = 0;
    for (const auto &v : reg_->values) {
      if (v.value == reg_->value)
        reinterpret_cast<QComboBox *>(editor)->setCurrentIndex(i);
      else
        ++i;
    }
  } else if (reg_->is_signed) {
    reinterpret_cast<SignedSpinBox *>(editor)->setValue(
        static_cast<int64_t>(reg_->value));
  } else {
    reinterpret_cast<UnsignedSpinBox *>(editor)->setValue(reg_->value);
  }
}

void QRegister::setModelData(QWidget *editor) {
  if (not reg_->values.empty()) {
    reg_->set_value(
        reg_->values[reinterpret_cast<QComboBox *>(editor)->currentIndex()]
            .value);
  } else if (reg_->is_signed) {
    reg_->set_value(static_cast<uint64_t>(
        reinterpret_cast<SignedSpinBox *>(editor)->value()));
  } else {
    reg_->set_value(reinterpret_cast<UnsignedSpinBox *>(editor)->value());
  }
}

void QRegister::setValue(uint64_t v) {
  if (not reg_ or v == reg_->value)
    return;
  if (reg_->set_value(v))
    emit valueChanged(v);
}

uint64_t QRegister::value() const { return reg_ ? reg_->value : 0; }

bool QRegister::hasField(const QString &name) const {
  return std::find_if(fields_.begin(), fields_.end(),
                      [&name](const std::unique_ptr<QField> &f) {
                        return f->field_->name == name;
                      }) != fields_.end();
}

QField *QRegister::getField(const QString &name) {
  auto it = std::find_if(fields_.begin(), fields_.end(),
                         [&name](const std::unique_ptr<QField> &f) {
                           return f->field_->name == name;
                         });
  if (it == fields_.end())
    return nullptr;
  else
    return it->get();
}

const QField *QRegister::getField(const QString &name) const {
  auto it = std::find_if(fields_.begin(), fields_.end(),
                         [&name](const std::unique_ptr<QField> &f) {
                           return f->field_->name == name;
                         });
  if (it == fields_.end())
    return nullptr;
  else
    return it->get();
}

QField *QRegister::addField(const QString &name, const uint64_t msb,
                            const uint64_t lsb) {
  if (not reg_)
    return nullptr;

  auto n = name.toStdString();
  auto *field = reg_->add_field(n, msb, lsb);
  if (not field)
    return nullptr;

  int index =
      std::find_if(reg_->fields.begin(), reg_->fields.end(),
                   [field](const auto &f) { return f.get() == field; }) -
      reg_->fields.begin();
  auto it = fields_.insert(std::next(fields_.begin(), index),
                           std::unique_ptr<QField>(new QField(this, field)));
  emit fieldAdded(it->get());
  return it->get();
}

void QRegister::removeField(QField *field) {
  auto it = std::find_if(fields_.begin(), fields_.end(),
                         [field](const auto &f) { return f.get() == field; });
  if (it == fields_.end())
    return;
  emit fieldRemoved(it->get());
  fields_.erase(it);
}

void QRegister::removeField(const QString &name) {
  auto it = std::find_if(fields_.begin(), fields_.end(),
                         [name](const auto &g) { return g->name() == name; });
  if (it == fields_.end())
    return;
  removeField(it->get());
}

QList<const QField *> QRegister::fields() const {
  QList<const QField *> ret;
  for (auto &f : fields_)
    ret.append(f.get());
  return ret;
}

QField *QRegister::addField(const QString &name, uint64_t position) {
  return addField(name, position, position);
}

void QRegister::onFormatChanged(Fmt fmt) {
  for (const auto &f : fields_)
    f->setFormat(fmt);
}

QVariant QRegister::default_register_access(const QRegister &reg,
                                            const QColumn &column, int role) {
  QString tmp;
  rds::Device *dev;
  int i;
  using enum DataId;
  switch (column.id) {
  case Name:
    return reg.name();
  case AddrPos:
    tmp = QString::number(reg.addr(), 16).toUpper();
    dev = reg.parent_->parent_->dev_.get();
    if (dev->register_width % 4 == 0) {
      i = dev->register_width / 4 - tmp.size();
    } else {
      i = (dev->register_width / 4 + 1) * 4 - tmp.size();
    }
    if (i < 0)
      i = 0;
    return "0x" + QString::fromStdString(std::string(std::size_t(i), '0')) +
           tmp;
  case Value:
    switch (role) {
    case Qt::EditRole:
      if (reg.isSigned())
        return qlonglong(reg.value());
      else
        return qulonglong(reg.value());
    case Qt::DisplayRole:
      if (reg.isSigned())
        return to_string(qlonglong(reg.value()), reg.format());
      else
        return to_string(qulonglong(reg.value()), reg.format());
    default:
      return {};
    }
  case Initial:
    return to_string(reg.initial(), reg.format());
  case Unit:
    return reg.unit();
  case Description:
    return reg.description();
  case ZeroCodeValue:
    return reg.zeroCodeValue();
  case Step:
    return reg.step();
  case ExtraData:
    return reg.extraData(column.data_name);
  case Custom:
    if (column.register_data)
      return column.register_data(reg, role);
  default:
    break;
  }
  return {};
}
} // namespace rds
