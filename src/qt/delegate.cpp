#include "rds/qt/delegate.hpp"
#include "rds/qt/node.hpp"

namespace rds {

QWidget *Delegate::createEditor(QWidget *parent,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
  if (not index.isValid())
    return nullptr;

  QModelIndex i = model ? model->mapToSource(index) : index;
  auto *node = reinterpret_cast<Node *>(i.internalPointer());
  return node->createEditor(parent, option);
}

void Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  if (not index.isValid() or not editor)
    return;
  QModelIndex i = model ? model->mapToSource(index) : index;
  reinterpret_cast<const Node *>(i.internalPointer())->setEditorData(editor);
}

void Delegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                            const QModelIndex &index) const {
  if (not index.isValid() or not model or not editor)
    return;
  QModelIndex i = this->model ? this->model->mapToSource(index) : index;
  reinterpret_cast<Node *>(i.internalPointer())->setModelData(editor);
  emit model->dataChanged(index, index);
}

} // namespace rds
