#ifndef RDS_QT_FILTER_HPP
#define RDS_QT_FILTER_HPP

#include "qabstractitemmodel.h"
#include "qabstractitemview.h"
#include "qproperty.h"
#include "qsortfilterproxymodel.h"
#include "qtmetamacros.h"
#include <QAbstractItemModel>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QProperty>
#include <QSortFilterProxyModel>
#include <QWidget>

namespace rds {
Q_NAMESPACE;

enum class FilterType { None = 0, Fixed = 1, Wildcard = 2, Regex = 3 };

Q_ENUM_NS(FilterType);

/**
 * This is an input widget for filtering Qt Views.
 *
 * Usage:
 * Either construct the filter with a pointer to a view:
 *  - Filter*f = new Filter(view_ptr);
 * or set the view with setView
 *  - f->setView(view_ptr);
 * If the view does not have a model when adding it to the filter, the model
 * must then be set with the through the filters setModel function.
 * - f->setModel(model_ptr);
 *
 * Internally, a QSortFilterProxyModel is used for filtering.
 */
class Filter : public QWidget {
  Q_OBJECT;

public:
  Q_PROPERTY(FilterType type READ type WRITE setType NOTIFY typeChanged);
  Q_PROPERTY(
      QString text READ text WRITE setText NOTIFY textChanged RESET clearText);
  Q_PROPERTY(int column READ column WRITE setColumn NOTIFY columnChanged);
  Q_PROPERTY(Qt::CaseSensitivity caseSensitivity READ caseSensitivity WRITE
                 setCaseSensitivity NOTIFY caseSensitivityChanged);
  Q_PROPERTY(bool regex READ regex WRITE setRegex NOTIFY regexChanged);
  Q_PROPERTY(
      bool wildcard READ wildcard WRITE setWildcard NOTIFY wildcardChanged);

  Filter(QAbstractItemView *view = nullptr, QWidget *parent = nullptr);

  void setView(QAbstractItemView *view);
  QAbstractItemView *view() const;

  void setModel(QAbstractItemModel *model);
  QAbstractItemModel *model() const;
  QSortFilterProxyModel *proxy() const;

  void setType(FilterType type);
  FilterType type() const;

  void setText(const QString &text);
  QString text() const;
  void clearText();

  void setColumn(int column);
  int column();

  void setCaseSensitivity(Qt::CaseSensitivity sensitivity);
  Qt::CaseSensitivity caseSensitivity() const;

  void setRegex(bool enable);
  bool regex() const;

  void setWildcard(bool enable);
  bool wildcard() const;

  void clear();
signals:
  void textChanged(const QString &text);
  void columnChanged(int column);
  void typeChanged(FilterType type);
  void caseSensitivityChanged(Qt::CaseSensitivity sensitivity);
  void regexChanged(bool enabled);
  void wildcardChanged(bool enabled);

private slots:
  void onIndexChanged(int idx);
  void onWildCardClicked(bool checked);
  void onRegexClicked(bool checked);
  void onCaseSensitiveClicked(bool checked);
  void onHeaderDataChanged(Qt::Orientation orientation, int first, int last);
  void onSourceModelChange();

private:
  void setupUi();
  void connectView();
  void disconnectView();
  void connectModel();
  void disconnectModel();
  void setupColumnSel(QAbstractItemModel *model);
  void resetColumnSel();
  void filter();

  QPropertyNotifier on_index_changed_{};
  QAbstractItemView *view_ = nullptr;
  QSortFilterProxyModel *proxy_ = nullptr;
  QHBoxLayout *layout_ = nullptr;
  QLineEdit *line_edit_ = nullptr;
  QComboBox *column_sel_ = nullptr;
  QCheckBox *case_sensitive_ = nullptr;
  QCheckBox *wildcard_ = nullptr;
  QCheckBox *regex_ = nullptr;
  Qt::CaseSensitivity sensitivity_{Qt::CaseInsensitive};
  FilterType type_{FilterType::Fixed};
};
} // namespace rds

#endif
