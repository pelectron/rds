#ifndef RDS_QT_EDITORS_HPP
#define RDS_QT_EDITORS_HPP

#include "qcombobox.h"
#include "rds/common.hpp"
#include "rds/qt/common.hpp"
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QLineEdit>

namespace rds {

class SignedSpinBox : public QAbstractSpinBox {
  Q_OBJECT

  Q_PROPERTY(qlonglong minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(qlonglong maximum READ maximum WRITE setMaximum)

  Q_PROPERTY(
      qlonglong value READ value WRITE setValue NOTIFY valueChanged USER true)
public:
  explicit SignedSpinBox(QWidget *parent = 0) : QAbstractSpinBox(parent) {
    connect(this->lineEdit(), &QLineEdit::textEdited, this,
            &SignedSpinBox::onEditFinished);
    connect(this, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));
    setButtonSymbols(ButtonSymbols::PlusMinus);
  };

  ~SignedSpinBox() {};

  qlonglong value() const { return value_; };

  qlonglong minimum() const { return min_; };

  void setMinimum(qlonglong min) { min_ = min; }

  qlonglong maximum() const { return max_; };

  void setMaximum(qlonglong max) { max_ = max; }

  void setRange(qlonglong min, qlonglong max) {
    setMinimum(min);
    setMaximum(max);
  }

  Fmt format() const { return format_; }

  void setFormat(Fmt format) {
    if (format == format_)
      return;
    format_ = format;
    lineEdit()->setText(textFromValue(value_));
  }

  void stepBy(int steps) override {
    auto new_value = value_ + steps;

    if (new_value < min_)
      new_value = min_;
    else if (new_value > max_)
      new_value = max_;

    lineEdit()->setText(textFromValue(new_value));
    setValue(new_value);
  }

protected:
  // bool event(QEvent *event);
  QValidator::State validate(QString &input, int &pos) const override {
    if (auto v = from_string<qlonglong>(input);
        v.has_value() and (v.value() >= min_ and v.value() <= max_)) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  virtual qlonglong valueFromText(const QString &text) const {
    return from_string<qlonglong>(text).value_or(0);
  }

  virtual QString textFromValue(qlonglong val) const {
    return to_string(val, format_);
  }

  // void fixup(QString &str) const override {
  //   if (str == "0B")
  //     str = "0B0";
  //   else if (str == "0b")
  //     str = "0b0";
  //   else if (str == "0X")
  //     str = "0X0";
  //   else if (str == "0x")
  //     str = "0x0";
  // }

  QAbstractSpinBox::StepEnabled stepEnabled() const override {
    return StepUpEnabled | StepDownEnabled;
  }

public Q_SLOTS:
  void setValue(qlonglong val) {
    if (value_ != val) {
      lineEdit()->setText(textFromValue(val));
      value_ = val;
      emit valueChanged(val);
    }
  }

  void onEditFinished() {
    if (auto v = from_string<qlonglong>(lineEdit()->text());
        v.has_value() and value_ != v.value()) {
      value_ = v.value();
      lineEdit()->setText(textFromValue(value_));
      emit valueChanged(value_);
    } else
      lineEdit()->setText(textFromValue(value_));
  }

Q_SIGNALS:
  void valueChanged(qlonglong v);

private:
  qlonglong value_ = 0;
  qlonglong min_ = std::numeric_limits<qlonglong>::min();
  qlonglong max_ = std::numeric_limits<qlonglong>::max();
  Fmt format_ = Fmt::integer;
};

class UnsignedSpinBox : public QAbstractSpinBox {
  Q_OBJECT

  Q_PROPERTY(qulonglong minimum READ minimum WRITE setMinimum)
  Q_PROPERTY(qulonglong maximum READ maximum WRITE setMaximum)

  Q_PROPERTY(
      qulonglong value READ value WRITE setValue NOTIFY valueChanged USER true)
public:
  explicit UnsignedSpinBox(QWidget *parent = 0)
      : QAbstractSpinBox(parent), format_(Fmt::hex) {
    connect(lineEdit(), SIGNAL(textEdited(QString)), this,
            SLOT(onEditFinished()));
    connect(this, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));
    setButtonSymbols(ButtonSymbols::PlusMinus);
  };

  ~UnsignedSpinBox() {};

  qulonglong value() const { return value_; };

  qulonglong minimum() const { return min_; };

  void setMinimum(qulonglong min) { min_ = min; }

  qulonglong maximum() const { return max_; };

  void setMaximum(qulonglong max) { max_ = max; }

  void setRange(qulonglong min, qulonglong max) {
    setMinimum(min);
    setMaximum(max);
  }

  Fmt format() const { return format_; }

  void setFormat(Fmt format) {
    if (format == format_)
      return;
    format_ = format;
    lineEdit()->setText(textFromValue(value_));
  }

  void stepBy(int steps) override {
    auto new_value = value_ + steps;

    if (new_value < min_)
      new_value = min_;
    else if (new_value > max_)
      new_value = max_;

    lineEdit()->setText(textFromValue(new_value));
    setValue(new_value);
  }

protected:
  // bool event(QEvent *event);
  QValidator::State validate(QString &input, int &pos) const override {
    if (auto v = from_string<qulonglong>(input);
        v.has_value() and (v.value() >= min_ and v.value() <= max_)) {
      return QValidator::Acceptable;
    }
    return QValidator::Invalid;
  }

  virtual qulonglong valueFromText(const QString &text) const {
    return from_string<qulonglong>(text).value_or(0);
  }

  virtual QString textFromValue(qulonglong val) const {
    return to_string(val, format_);
  }

  QAbstractSpinBox::StepEnabled stepEnabled() const override {
    return StepUpEnabled | StepDownEnabled;
  }

public Q_SLOTS:
  void setValue(qulonglong val) {
    if (value_ != val) {
      lineEdit()->setText(textFromValue(val));
      value_ = val;
      emit valueChanged(val);
    }
  }

  void onEditFinished() {
    if (auto v = from_string<qulonglong>(lineEdit()->text());
        v.has_value() and value_ != v.value()) {
      value_ = v.value();
      lineEdit()->setText(textFromValue(value_));
      emit valueChanged(value_);
    } else
      lineEdit()->setText(textFromValue(value_));
  }

Q_SIGNALS:
  void valueChanged(qulonglong v);

private:
  qulonglong value_ = 0;
  qulonglong min_ = std::numeric_limits<qulonglong>::min();
  qulonglong max_ = std::numeric_limits<qulonglong>::max();
  Fmt format_ = Fmt::integer;
};

inline QComboBox *to_combo_box(const std::vector<EnumeratedValue> &values) {
  auto *cb = new QComboBox;
  for (const auto &v : values)
    cb->addItem(QString::fromStdString(v.name) + " (" +
                    QString::number(v.value) + ")",
                v.value);
  return cb;
}

} // namespace rds

#endif
