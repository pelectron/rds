#ifndef RDS_QT_REGISTER_HPP
#define RDS_QT_REGISTER_HPP

#include "qcontainerfwd.h"
#include "qtmetamacros.h"
#include "qvariant.h"
#include "rds/qt/common.hpp"
#include "rds/qt/field.hpp"
#include "rds/register.hpp"

#include <cstdint>
#include <memory>
#include <vector>

Q_DECLARE_OPAQUE_POINTER(rds::QGroup *);

namespace rds {

/**
 * @addtogroup qrds
 * @class QRegister
 * The Qt version of rds::Register.
 *
 */
class QRegister final : public Node {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(QRegister);

public:
  using Accessor = std::function<QVariant(const QRegister &reg,
                                          const QColumn &column, int role)>;
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
  Q_PROPERTY(
      uint64_t value READ value WRITE setValue NOTIFY valueChanged RESET reset);
  Q_PROPERTY(
      uint64_t initial READ initial WRITE setInitial NOTIFY initialChanged);
  Q_PROPERTY(uint64_t addr READ addr WRITE setAddr NOTIFY addrChanged);
  Q_PROPERTY(uint64_t size READ size WRITE setSize NOTIFY sizeChanged);
  Q_PROPERTY(uint64_t zerosMask READ zerosMask WRITE setZerosMask NOTIFY
                 zerosMaskChanged);
  Q_PROPERTY(
      uint64_t onesMask READ onesMask WRITE setOnesMask NOTIFY onesMaskChanged);
  Q_PROPERTY(uint64_t xMask READ xMask WRITE setXMask NOTIFY xMaskChanged);
  Q_PROPERTY(Access access READ access WRITE setAccess NOTIFY accessChanged);
  Q_PROPERTY(
      bool isSigned READ isSigned WRITE setIsSigned NOTIFY isSignedChanged);
  Q_PROPERTY(QList<const QField *> fields READ fields);
  Q_PROPERTY(QGroup *group READ group);
  Q_PROPERTY(QList<Value> values READ values);

  uint64_t numBits() const;

  void setValue(uint64_t v) override;
  uint64_t value() const override;
  void reset() override;

  /**
   * adds enumerated values to the register.
   * Returns true if the value could be added, else false.
   * The value cannot be added if another value with the same name or value as
   * val has beeen added before.
   * @param val the value to be added
   * @param vals the values to be added
   * @{
   */
  bool addValue(const EnumeratedValue &val);
  bool addValues(const QList<EnumeratedValue> &vals);
  /**
   * @}
   */
  void removeValue(const QString &name);
  void removeValue(uint64_t val);
  QList<EnumeratedValue> values() const;

  QGroup *group();
  const QGroup *group() const;

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

  void setAddr(uint64_t addr);
  uint64_t addr() const;

  void setSize(uint64_t size);
  uint64_t size() const;

  void setZerosMask(uint64_t mask);
  uint64_t zerosMask() const;

  void setOnesMask(uint64_t mask);
  uint64_t onesMask() const;

  void setXMask(uint64_t mask);
  uint64_t xMask() const;

  void setAccess(Access access);
  Access access() const;

  void setIsSigned(bool is_signed);
  bool isSigned() const;

  void setExtraData(const QString &name, const QVariant &data);
  QVariant extraData(const QString &name) const;
  void removeExtraData(const QString &name);

  void setAccessor(Accessor a);

  /**
   * returns true if this register contains a field with the specified name
   */
  bool hasField(const QString &name) const;
  /**
   * gets the field with the specified name. Returns nullptr if the such a
   * field does not exist.
   */
  QField *getField(const QString &name);
  /**
   * gets the field with the specified name. Returns nullptr if the such a
   * field does not exist.
   */
  const QField *getField(const QString &name) const;

  /**
   * add a field to the register. This will return a pointer to the added Field,
   * or nullptr in case of failure. This will fail if a field with the same name
   * or an overlapping field has already been added, or if the field is outside
   * the valid bitrange for the register.
   * @param name the name
   * @param position the bit position
   * @param msb the msb position in the register
   * @param lsb the lsb position in the reigster
   * @{
   */
  QField *addField(const QString &name, uint64_t position);
  QField *addField(const QString &name, const uint64_t msb, const uint64_t lsb);
  /**
   * @}
   */
  void removeField(QField *field);
  void removeField(const QString &name);

  QList<const QField *> fields() const;
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
  void addrChanged(uint64_t addr);
  void sizeChanged(uint64_t size);
  void zerosMaskChanged(uint64_t mask);
  void onesMaskChanged(uint64_t mask);
  void xMaskChanged(uint64_t mask);
  void accessChanged(Access access);
  void isSignedChanged(bool is_signed);
  void extraDataChanged(const QString &name, const QVariant &data);
  void valueAdded(const EnumeratedValue &v);
  void valueRemoved(const EnumeratedValue &v);
  void fieldAdded(QField *field);
  void fieldRemoved(QField *field);

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
  friend class QGroup;
  friend class QField;
  friend class QColumn;
  QRegister(QGroup *parent, rds::Register *reg);
  static QVariant default_register_access(const QRegister &reg,
                                          const QColumn &column, int role);
  void onFormatChanged(Fmt fmt);
  ::rds::Register *reg_;
  QGroup *parent_;
  std::vector<std::unique_ptr<QField>> fields_;
  Accessor access_{&default_register_access};
};
} // namespace rds

#endif
