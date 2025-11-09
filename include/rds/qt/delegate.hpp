#ifndef RDS_QT_DELEGATE_HPP
#define RDS_QT_DELEGATE_HPP

#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

namespace rds {

class Delegate : public QStyledItemDelegate {
  Q_OBJECT;
  QSortFilterProxyModel *model{nullptr};

public:
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const override;
  void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const override;
  void setFilterModel(QSortFilterProxyModel *m) { model = m; }
};
} // namespace rds

#endif
