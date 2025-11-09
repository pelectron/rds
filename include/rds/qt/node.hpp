#ifndef RDS_QT_NODE_HPP
#define RDS_QT_NODE_HPP

#include "rds/qt/common.hpp"

#include <QObject>
#include <QStyleOption>
#include <QVariant>
#include <cstdint>

namespace rds {

class Node : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Node);

public:
  Q_PROPERTY(Fmt format READ format WRITE setFormat NOTIFY formatChanged);
  Q_PROPERTY(
      uint64_t value READ value WRITE setValue NOTIFY valueChanged RESET reset);

  Node(Fmt fmt = Fmt::hex) : format_(fmt) {}

  virtual ~Node() = 0;
  virtual void reset() = 0;
  virtual void setValue(uint64_t v) {}
  virtual uint64_t value() const { return 0; }

  void setFormat(Fmt fmt);
  Fmt format() const;
signals:
  void formatChanged(Fmt format);
  void valueChanged(uint64_t value);

protected:
  virtual Node *child(int row) = 0;
  virtual int childCount() const = 0;
  virtual QVariant data(const QColumn &column, int role) const = 0;
  virtual int row() const = 0;
  virtual Node *parentItem() = 0;
  virtual QWidget *createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option) = 0;
  virtual void setEditorData(QWidget *editor) const = 0;
  virtual void setModelData(QWidget *editor) = 0;
  friend class QModel;
  friend class QDevice;
  friend class QGroup;
  friend class QRegister;
  friend class QField;
  friend class Delegate;
  Fmt format_;
};
} // namespace rds

#endif
