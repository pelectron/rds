#ifndef RDS_QT_FIELD_HPP
#define RDS_QT_FIELD_HPP

#include "qcontainerfwd.h"
#include "qlist.h"
#include "qtmetamacros.h"
#include "rds/common.hpp"
#include "rds/field.hpp"
#include "rds/qt/node.hpp"

#include <cstdint>

Q_DECLARE_OPAQUE_POINTER(rds::QRegister *);

namespace rds {

/**
 * @brief
 *
 * @addtogroup qrds
 */
class QField final : public Node {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(QField);

public:
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);
  Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY
                 displayNameChanged);
  Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY
                 descriptionChanged);
  Q_PROPERTY(QString backup READ backup WRITE setBackup NOTIFY backupChanged);
  Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged);
  Q_PROPERTY(double zeroCodeValue READ zeroCodeValue WRITE setZeroCodeValue
                 NOTIFY zeroCodeValueChanged);
  Q_PROPERTY(double step READ step WRITE setStep NOTIFY stepChanged);
  Q_PROPERTY(uint64_t msb READ msb WRITE setMsb NOTIFY msbChanged);
  Q_PROPERTY(uint64_t lsb READ lsb WRITE setLsb NOTIFY lsbChanged);
  Q_PROPERTY(
      uint64_t value READ value WRITE setValue NOTIFY valueChanged RESET reset);
  Q_PROPERTY(
      uint64_t initial READ initial WRITE setInitial NOTIFY initialChanged);
  Q_PROPERTY(Access access READ access WRITE setAccess NOTIFY accessChanged);
  Q_PROPERTY(
      bool isSigned READ isSigned WRITE setIsSigned NOTIFY isSignedChanged);
  Q_PROPERTY(QRegister *reg READ reg);
  Q_PROPERTY(QList<Value> values READ values);

  using Accessor = std::function<QVariant(const QField &field,
                                          const QColumn &column, int role)>;

  QRegister *reg();
  const QRegister *reg() const;

  void setValue(uint64_t v) override;
  uint64_t value() const override;
  void reset() override;

  void setName(const QString &name);
  QString name() const;

  void setDisplayName(const QString &name);
  QString displayName() const;

  void setDescription(const QString &description);
  QString description() const;

  void setBackup(const QString &backup);
  QString backup() const;

  void setUnit(const QString &unit);
  QString unit() const;

  void setZeroCodeValue(double value);
  double zeroCodeValue() const;

  void setStep(double step);
  double step() const;

  void setInitial(uint64_t initial);
  uint64_t initial() const;

  void setMsb(uint64_t msb);
  uint64_t msb() const;

  void setLsb(uint64_t lsb);
  uint64_t lsb() const;

  void setAccess(Access access);
  Access access() const;

  void setIsSigned(bool is_signed);
  bool isSigned() const;

  void setExtraData(const QString &name, const QVariant &data);
  QVariant extraData(const QString &name) const;
  void removeExtraData(const QString &name);

  bool addValue(const EnumeratedValue &val);
  bool addValues(const QList<EnumeratedValue> &vals);
  void removeValue(const QString &name);
  void removeValue(uint64_t val);
  QList<EnumeratedValue> values() const;

  void setAccessor(Accessor a);
signals:
  void valueChanged(uint64_t value);
  void nameChanged(const QString &name);
  void displayNameChanged(const QString &name);
  void descriptionChanged(const QString &description);
  void backupChanged(const QString &backup);
  void unitChanged(const QString &unit);
  void zeroCodeValueChanged(double value);
  void stepChanged(double step);
  void initialChanged(uint64_t initial);
  void msbChanged(uint64_t msb);
  void lsbChanged(uint64_t lsb);
  void accessChanged(Access access);
  void isSignedChanged(bool is_signed);
  void extraDataChanged(const QString &name, const QVariant &data);
  void valueAdded(const EnumeratedValue &v);
  void valueRemoved(const EnumeratedValue &v);

protected:
  Node *child(int row) override;
  int childCount() const override;
  QVariant data(const QColumn &column, int role) const override;
  int row() const override;
  Node *parentItem() override;
  QWidget *createEditor(QWidget *parent,
                        const QStyleOptionViewItem &option) override;
  void setEditorData(QWidget *editor) const override;
  void setModelData(QWidget *editor) override;

private:
  friend class QModel;
  friend class QRegister;
  friend class QGroup;
  friend class QColumn;
  QField(QRegister *parent, rds::Field *field);
  static QVariant default_field_access(const QField &field,
                                       const QColumn &column, int role);
  ::rds::Field *field_;
  QRegister *parent_;
  Accessor access_{&default_field_access};
};
} // namespace rds

#endif
