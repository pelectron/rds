#ifndef RDS_QT_MODEL_HPP
#define RDS_QT_MODEL_HPP

#include "rds/qt/common.hpp"
#include "rds/qt/device.hpp"

#include <QAbstractItemModel>
#include <QObject>
#include <memory>
#include <vector>

namespace rds {

/**
 * @brief
 *
 * @addtogroup qrds
 */
class QModel : public QAbstractItemModel {
  Q_OBJECT;
  Q_DISABLE_COPY_MOVE(QModel);

public:
  explicit QModel(QObject *parent = nullptr);
  ~QModel() override;
  void set_headers(const std::vector<QColumn> &columns);
  QVariant data(const QModelIndex &index, int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  bool setHeaderData(int section, Qt::Orientation orientation,
                     const QVariant &value, int role = Qt::EditRole) override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent = {}) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  int rowCount(const QModelIndex &parent = {}) const override;
  int columnCount(const QModelIndex &parent = {}) const override;

  QDevice *device();
  const QDevice *device() const;
  void setDevice(std::unique_ptr<QDevice> &&device);

  void load_file(const QString &path);
  void save_file(const QString &path);
  void setFormat(Fmt format);
  void clear();
  rds::Contents contents() const;
  void updateContents(const rds::Contents &contents);

private slots:
  void setFormat(Fmt fmt, const QModelIndex &idx) {
    if (not idx.isValid())
      return;
    reinterpret_cast<Node *>(idx.internalPointer())->setFormat(fmt);
    dataChanged(idx, idx);
  }

  void setIntegerFormat() { setFormat(Fmt::integer); }
  void setHexFormat() { setFormat(Fmt::hex); }
  void setBinaryFormat() { setFormat(Fmt::binary); }

private:
  friend class Widget;
  friend class QDevice;
  friend class QGroup;
  friend class QRegister;
  friend class QField;
  std::unique_ptr<::rds::QDevice> device_;
  std::vector<QColumn> columns_;
};
} // namespace rds

#endif
