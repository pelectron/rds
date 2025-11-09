#ifndef RDS_QT_GROUP_HPP
#define RDS_QT_GROUP_HPP

#include "qcontainerfwd.h"
#include "qtmetamacros.h"
#include "qvariant.h"
#include "rds/group.hpp"
#include "rds/qt/register.hpp"

#include <cstdint>
#include <memory>
#include <vector>

Q_DECLARE_OPAQUE_POINTER(rds::QDevice *);

namespace rds {

/**
 * @class QGroup
 * The Qt version of rds::Group.
 *
 * @addtogroup qrds qrds
 */
class QGroup final : public Node {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(QGroup);

public:
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);
  Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY
                 descriptionChanged);
  Q_PROPERTY(
      uint64_t baseAddr READ baseAddr WRITE setBaseAddr NOTIFY baseAddrChanged);
  Q_PROPERTY(uint64_t size READ size WRITE setSize NOTIFY sizeChanged);
  Q_PROPERTY(QList<const QRegister *> registers READ registers);
  Q_PROPERTY(QDevice *device READ device);

  void reset() override;

  QDevice *device();
  const QDevice *device() const;

  void setName(const QString &name);
  QString name() const;
  void setDescription(const QString &description);
  QString description() const;
  void setBaseAddr(uint64_t base_addr);
  uint64_t baseAddr() const;
  void setSize(uint64_t size);
  uint64_t size() const;
  void setDefault(const QString &name, const QVariant &value);
  QVariant defaults(const QString &name) const;

  bool hasRegister(const QString &name) const;
  QRegister *getRegister(const QString &name);
  const QRegister *getRegister(const QString &name) const;
  QRegister *getRegister(uint64_t addr);
  const QRegister *getRegister(uint64_t addr) const;
  QRegister *addRegister(const QString &name, uint64_t addr, uint64_t size = 1);

  void removeRegister(QRegister *r);
  void removeRegister(const QString &name);

  QList<const QRegister *> registers() const;
signals:
  void nameChanged(const QString &name);
  void descriptionChanged(const QString &description);
  void baseAddrChanged(uint64_t base_addr);
  void sizeChanged(uint64_t size);
  void defaultsChanged(const QString &name, const QVariant &data);
  void registerAdded(QRegister *reg);
  void registerRemoved(QRegister *reg);

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
  friend class QDevice;
  friend class QRegister;
  friend class QField;
  QGroup(QDevice *parent, rds::Group *group);
  void onFormatChanged(Fmt fmt);

  ::rds::Group *group_;
  QDevice *parent_;
  std::vector<std::unique_ptr<QRegister>> registers_;
};
} // namespace rds

#endif
