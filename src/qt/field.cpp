#include "rds/qt/field.hpp"
#include "qassert.h"
#include "qcontainerfwd.h"
#include "qlogging.h"
#include "rds/common.hpp"
#include "rds/qt/column.hpp"
#include "rds/qt/common.hpp"
#include "rds/qt/editors.hpp"
#include "rds/qt/register.hpp"
#include "rds/util.hpp"

namespace rds {

QField::QField(QRegister *parent, rds::Field *field)
    : field_(field), parent_(parent) {
  Q_ASSERT(field != nullptr);
  Q_ASSERT(parent != nullptr);
}

QRegister *QField::reg() { return parent_; }

const QRegister *QField::reg() const { return parent_; }

void QField::reset() {
  if (field_)
    setValue(field_->initial);
}

Node *QField::child(int row) { return nullptr; }

int QField::childCount() const { return 0; }

QVariant QField::data(const QColumn &column, int role) const {
  if (column.id == DataId::Custom and column.field_data)
    return column.field_data(*this, role);
  else if (access_)
    return access_(*this, column, role);
  else
    return {};
}

int QField::row() const {
  int i = 0;
  for (const auto &f : parent_->fields_)
    if (f.get() == this)
      return i;
    else
      ++i;
  return i;
}

Node *QField::parentItem() { return parent_; }

QWidget *QField::createEditor(QWidget *parent,
                              const QStyleOptionViewItem &option) {
  if (not field_->values.empty()) {
    auto *cb = new QComboBox;
    int i = 0;
    for (const auto &v : field_->values) {
      cb->addItem(QString::fromStdString(v.name));
      if (v.value == field_->value)
        cb->setCurrentIndex(i);
      else
        ++i;
    }
    return cb;
  } else if (field_->is_signed) {
    auto *fb = new SignedSpinBox(parent);
    fb->setFormat(format_);
    fb->setValue(field_->value);
    fb->setMaximum((1 << (field_->msb - field_->lsb)) - 1);
    fb->setMinimum(-(1 << (field_->msb - field_->lsb)));
    return fb;
  } else {
    auto *fb = new UnsignedSpinBox(parent);
    fb->setFormat(format_);
    fb->setValue(field_->value);
    fb->setMaximum((1u << (field_->msb - field_->lsb + 1u)) - 1u);
    return fb;
  }
}

void QField::setEditorData(QWidget *editor) const {
  if (not field_->values.empty()) {
    int i = 0;
    for (const auto &v : field_->values) {
      if (v.value == field_->value)
        reinterpret_cast<QComboBox *>(editor)->setCurrentIndex(i);
      else
        ++i;
    }
  } else if (field_->is_signed) {
    reinterpret_cast<SignedSpinBox *>(editor)->setValue(
        static_cast<int64_t>(field_->value));
  } else {
    reinterpret_cast<UnsignedSpinBox *>(editor)->setValue(field_->value);
  }
}

void QField::setModelData(QWidget *editor) {
  if (not field_->values.empty()) {
    field_->set_value(
        field_->values[reinterpret_cast<QComboBox *>(editor)->currentIndex()]
            .value);
  } else if (field_->is_signed) {
    field_->set_value(static_cast<uint64_t>(
        reinterpret_cast<SignedSpinBox *>(editor)->value()));
  } else {
    field_->set_value(reinterpret_cast<UnsignedSpinBox *>(editor)->value());
  }
}

void QField::setValue(uint64_t v) {
  if (not field_ or field_->value == v)
    return;
  if (field_->set_value(v))
    emit valueChanged(v);
}

uint64_t QField::value() const { return field_ ? field_->value : 0; }

QString QField::name() const {
  return field_ ? QString::fromStdString(field_->name) : QString{};
}

QString QField::displayName() const {
  return field_ ? QString::fromStdString(field_->display_name) : QString{};
}

QString QField::description() const {
  return field_ ? QString::fromStdString(field_->description) : QString{};
}

QString QField::backup() const {
  return field_ ? QString::fromStdString(field_->backup) : QString{};
}

QString QField::unit() const {
  return field_ ? QString::fromStdString(field_->unit) : QString{};
}

double QField::zeroCodeValue() const {
  return field_ ? field_->zero_code_value : 0;
}

double QField::step() const { return field_ ? field_->step : 0; }

uint64_t QField::initial() const { return field_ ? field_->initial : 0; }

uint64_t QField::msb() const { return field_ ? field_->msb : 0; }

uint64_t QField::lsb() const { return field_ ? field_->lsb : 0; }

Access QField::access() const { return field_ ? field_->access : Access{}; }

bool QField::isSigned() const { return field_ ? field_->is_signed : false; }

void QField::setExtraData(const QString &name, const QVariant &data) {
  if (not field_)
    return;
  if (auto v = to_simple_value(data)) {
    auto n = name.toStdString();
    if (field_->extra_data.contains(n)) {
      if (field_->extra_data.at(n) == *v)
        return;
    }
    field_->extra_data[n] = *v;
    emit extraDataChanged(name, data);
  }
}

QVariant QField::extraData(const QString &name) const {
  if (not field_)
    return {};
  auto n = name.toStdString();
  if (auto it = field_->extra_data.find(n); it != field_->extra_data.end())
    return to_qvariant(it->second);
  return {};
}

void QField::removeExtraData(const QString &name) {
  auto n = name.toStdString();
  if (auto it = field_->extra_data.find(n); it != field_->extra_data.end()) {
    field_->extra_data.erase(it);
    extraDataChanged(name, {});
  }
}

bool QField::addValue(const EnumeratedValue &val) {
  if (not field_)
    return false;
  if (field_->add_value(val)) {
    emit valueAdded(val);
    return true;
  }
  return false;
}

bool QField::addValues(const QList<EnumeratedValue> &vals) {
  if (not field_)
    return false;

  std::vector<EnumeratedValue> vs{vals.begin(), vals.end()};
  if (field_->add_values(vs)) {
    for (const auto &v : vs)
      emit valueAdded(v);
    return true;
  }
  return false;
}

void QField::removeValue(const QString &name) {
  auto it = std::find_if(
      field_->values.begin(), field_->values.end(),
      [&name](const EnumeratedValue &v) { return v.name == name; });
  if (it == field_->values.end())
    return;
  auto v = *it;
  field_->values.erase(it);
  valueRemoved(v);
}

void QField::removeValue(uint64_t val) {
  auto it =
      std::find_if(field_->values.begin(), field_->values.end(),
                   [&val](const EnumeratedValue &v) { return v.value == val; });
  if (it == field_->values.end())
    return;
  auto v = *it;
  field_->values.erase(it);
  valueRemoved(v);
}

QList<EnumeratedValue> QField::values() const {
  QList<EnumeratedValue> ret;
  for (const auto &v : field_->values)
    ret.append(v);
  return ret;
}

void QField::setAccessor(Accessor a) {
  if (not a)
    return;
  access_ = std::move(a);
}
void QField::setName(const QString &name) {
  if (name == field_->name)
    return;
  field_->name = name.toStdString();
  emit nameChanged(name);
}

void QField::setDisplayName(const QString &name) {
  if (name == field_->display_name)
    return;
  field_->display_name = name.toStdString();
  emit displayNameChanged(name);
}

void QField::setDescription(const QString &description) {
  if (description == field_->description)
    return;
  field_->description = description.toStdString();
  emit descriptionChanged(description);
}

void QField::setBackup(const QString &backup) {
  if (backup == field_->backup)
    return;
  field_->backup = backup.toStdString();
  emit descriptionChanged(backup);
}

void QField::setUnit(const QString &unit) {
  if (unit == field_->unit)
    return;
  field_->unit = unit.toStdString();
  emit unitChanged(unit);
}

void QField::setZeroCodeValue(double value) {
  if (value == field_->zero_code_value)
    return;
  field_->zero_code_value = value;
  zeroCodeValueChanged(value);
}

void QField::setStep(double step) {
  if (step == field_->step)
    return;
  field_->step = step;
  zeroCodeValueChanged(step);
}

void QField::setInitial(uint64_t initial) {
  if (initial == field_->initial)
    return;
  field_->initial = initial;
  emit initialChanged(initial);
}

void QField::setMsb(uint64_t msb) {
  if (msb == field_->msb)
    return;
  // TODO: sort fields
  field_->msb = msb;
  emit msbChanged(msb);
}

void QField::setLsb(uint64_t lsb) {
  if (lsb == field_->lsb)
    return;
  // TODO: sort fields
  field_->lsb = lsb;
  emit lsbChanged(lsb);
}

void QField::setAccess(Access access) {
  if (access == field_->access)
    return;
  field_->access = access;
  emit accessChanged(access);
}

void QField::setIsSigned(bool is_signed) {
  if (is_signed == field_->is_signed)
    return;
  field_->is_signed = is_signed;
  emit isSignedChanged(is_signed);
}

QVariant QField::default_field_access(const QField &field,
                                      const QColumn &column, int role) {
  using enum DataId;
  switch (column.id) {
  case Name:
    return QString::fromStdString(field.field_->name);
    break;
  case AddrPos:
    if (field.field_->msb == field.field_->lsb)
      return "[" + QString::number(field.field_->msb) + "]";
    else
      return "[" + QString::number(field.field_->msb) + ":" +
             QString::number(field.field_->lsb) + "]";
    break;
  case Value:
    switch (role) {
    case Qt::EditRole:
      if (field.field_->is_signed)
        return qlonglong(field.field_->value);
      else
        return qulonglong(field.field_->value);
    case Qt::DisplayRole:
      if (field.field_->is_signed)
        return to_string(qlonglong(field.field_->value), field.format());
      else
        return to_string(qulonglong(field.field_->value), field.format());
    default:
      return {};
    }
  case Initial:
    return to_string(field.field_->initial, field.format());
  case Unit:
    return QString::fromStdString(field.field_->unit);
    break;
  case Description:
    return QString::fromStdString(field.field_->description);
    break;
  case ZeroCodeValue:
    return field.field_->zero_code_value;
    break;
  case Step:
    return field.field_->step;
    break;
  case ExtraData:
    qDebug() << "field extra data: " << column.data_name;
    if (auto it = field.field_->extra_data.find(column.data_name.toStdString());
        it != field.field_->extra_data.end()) {
      return std::visit(
          rds::util::overload{[](const auto &v) { return QVariant{v}; },
                              [](const std::string &s) {
                                return QVariant{QString::fromStdString(s)};
                              }},
          it->second);
    }
    break;
  case Custom:
    if (column.field_data)
      return column.field_data(field, role);
  }
  return {};
}
} // namespace rds
