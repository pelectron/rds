#ifndef RDS_QT_DEVICE_HPP
#define RDS_QT_DEVICE_HPP

#include "qtmetamacros.h"
#include "qvariant.h"
#include "rds/device.hpp"
#include "rds/qt/group.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace rds {

/**
 * @class QDevice
 * @brief
 * @addtogroup qrds
 *
 */
class QDevice final : public Node {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(QDevice);

public:
  Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);

  Q_PROPERTY(
      int64_t version READ version WRITE setVersion NOTIFY versionCanged);

  Q_PROPERTY(int64_t registerWidth READ registerWidth WRITE setRegisterWidth
                 NOTIFY registerWidthChanged);

  Q_PROPERTY(
      int64_t numPages READ numPages WRITE setNumPages NOTIFY numPagesChanged);

  Q_PROPERTY(int64_t registersPerPage READ registersPerPage WRITE
                 setRegistersPerPage NOTIFY registersPerPageChanged);

  Q_PROPERTY(QList<const QGroup *> groups READ groups);

  QDevice(std::unique_ptr<rds::Device> d = {});

  void setDevice(std::unique_ptr<rds::Device> &&d);

  void reset() override;

  void setName(const QString &name);
  QString name() const;

  void setVersion(int64_t v);
  int64_t version() const;

  void setRegisterWidth(int64_t num_bits);
  int64_t registerWidth() const;

  void setNumPages(int64_t n);
  int64_t numPages() const;

  void setRegistersPerPage(int64_t n);
  int64_t registersPerPage() const;

  void setExtraData(const QString &name, const QVariant &data);
  QVariant extraData(const QString &name);

  QList<const QGroup *> groups() const;

  QGroup *getGroup(const QString &name);
  const QGroup *getGroup(const QString &name) const;

  QGroup *getGroup(uint64_t base_addr);
  const QGroup *getGroup(uint64_t base_addr) const;

  QGroup *addGroup(const QString &name, uint64_t base_addr, uint64_t size);

  void removeGroup(QGroup *group);
  void removeGroup(const QString &name);
signals:
  void nameChanged(const QString &name);
  void versionCanged(int64_t version);
  void numPagesChanged(int64_t n);
  void registerWidthChanged(int64_t num_bits);
  void registersPerPageChanged(int64_t n);
  void extraDataChanged(const QString &name, const QVariant &data);
  void groupAdded(QGroup *group);
  void groupRemoved(QGroup *group);

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
  void onFormatChanged(Fmt fmt);
  friend class QModel;
  friend class QGroup;
  friend class QRegister;
  friend class QField;
  friend class Widget;
  std::unique_ptr<rds::Device> dev_;
  std::vector<std::unique_ptr<QGroup>> groups_;
};
} // namespace rds

#endif
