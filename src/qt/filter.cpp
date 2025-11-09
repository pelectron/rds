#include "rds/qt/filter.hpp"
#include <QLabel>

namespace rds {

Filter::Filter(QAbstractItemView *view, QWidget *parent)
    : QWidget(parent), proxy_(new QSortFilterProxyModel(this)),
      type_(FilterType::Fixed) {
  setupUi();
  setView(view);
}

void Filter::setView(QAbstractItemView *view) {
  if (view == view_)
    return;

  if (view == nullptr) {
    disconnectView();
    view_ = nullptr;
  } else if (view_ == nullptr) {
    view_ = view;
    connectView();
  } else {
    disconnectView();
    view_ = view;
    connectView();
  }
}

QAbstractItemView *Filter::view() const { return view_; }

void Filter::setModel(QAbstractItemModel *model) {
  proxy_->setSourceModel(model);
}

QAbstractItemModel *Filter::model() const { return proxy_->sourceModel(); }

QSortFilterProxyModel *Filter::proxy() const { return proxy_; }

void Filter::setType(FilterType type) {
  const auto old = type_;
  if (type == old)
    return;

  type_ = type;
  emit typeChanged(type_);
}

FilterType Filter::type() const { return type_; }

void Filter::setText(const QString &text) { line_edit_->setText(text); }

QString Filter::text() const { return line_edit_->text(); }

void Filter::clearText() { line_edit_->clear(); }

int Filter::column() { return column_sel_->currentIndex() - 1; }

void Filter::setColumn(int column) {
  auto old = column_sel_->currentIndex() - 1;
  if (old == column)
    return;
  column_sel_->setCurrentIndex(column + 1);
  emit columnChanged(column);
}

Qt::CaseSensitivity Filter::caseSensitivity() const { return sensitivity_; }

void Filter::setCaseSensitivity(Qt::CaseSensitivity sensitivity) {
  if (sensitivity == sensitivity_)
    return;
  sensitivity_ = sensitivity;
  if (sensitivity_ == Qt::CaseInsensitive)
    case_sensitive_->setChecked(false);
  else
    case_sensitive_->setChecked(true);
  emit caseSensitivityChanged(sensitivity_);
}

bool Filter::regex() const { return type_ == FilterType::Regex; }

void Filter::setRegex(bool enable) { regex_->setChecked(enable); }

bool Filter::wildcard() const { return type_ == FilterType::Wildcard; }

void Filter::setWildcard(bool enable) { wildcard_->setChecked(enable); }

void Filter::clear() {
  setType(FilterType::Fixed);
  setCaseSensitivity(Qt::CaseInsensitive);
  line_edit_->clear();
}

void Filter::onIndexChanged(int idx) { emit columnChanged(idx - 1); }

void Filter::onWildCardClicked(bool checked) {
  if (checked) {
    regex_->setChecked(false);
    setType(FilterType::Wildcard);
  } else if (not regex_->isChecked()) {
    setType(FilterType::Fixed);
  }
}

void Filter::onRegexClicked(bool checked) {
  if (checked) {
    setType(FilterType::Regex);
    wildcard_->setChecked(false);
  } else if (not wildcard_->isChecked()) {
    setType(FilterType::Fixed);
  }
}

void Filter::onCaseSensitiveClicked(bool checked) {
  if (checked) {
    this->setCaseSensitivity(Qt::CaseSensitive);
  } else {
    this->setCaseSensitivity(Qt::CaseInsensitive);
  }
}

void Filter::onHeaderDataChanged(Qt::Orientation orientation, int first,
                                 int last) {
  if (orientation == Qt::Vertical)
    return;
  for (; first <= last; ++first) {
    if (column_sel_->count() <= first) {
      column_sel_->addItem(
          proxy_->headerData(first, orientation, Qt::DisplayRole).toString());
    } else {
      column_sel_->setItemText(
          first,
          proxy_->headerData(first, orientation, Qt::DisplayRole).toString());
    }
  }
}

void Filter::onSourceModelChange() {
  if (not proxy_ or not proxy_->sourceModel()) {
    resetColumnSel();
  } else {
    setupColumnSel(proxy_->sourceModel());
  }
}

void Filter::setupUi() {
  if (type_ == FilterType::None)
    return;

  layout_ = new QHBoxLayout;

  // line edit
  line_edit_ = new QLineEdit;
  line_edit_->setClearButtonEnabled(true);

  // column selection
  column_sel_ = new QComboBox;
  column_sel_->addItem("all");
  connect(column_sel_, &QComboBox::currentIndexChanged, this,
          &Filter::onIndexChanged);

  // case sensitive option
  case_sensitive_ = new QCheckBox("case sensitive");
  connect(case_sensitive_, &QCheckBox::clicked, this,
          &Filter::onCaseSensitiveClicked);

  // wildcard option
  wildcard_ = new QCheckBox("wildcard");
  connect(wildcard_, &QCheckBox::clicked, this, &Filter::onWildCardClicked);
  connect(wildcard_, &QCheckBox::clicked, this, &Filter::wildcardChanged);

  // regex option
  regex_ = new QCheckBox("regex");
  connect(regex_, &QCheckBox::clicked, this, &Filter::onRegexClicked);
  connect(regex_, &QCheckBox::clicked, this, &Filter::regexChanged);

  // setup filter triggers
  connect(this, &Filter::typeChanged, this, &Filter::filter);
  connect(this, &Filter::columnChanged, this, &Filter::filter);
  connect(line_edit_, &QLineEdit::textChanged, this, &Filter::filter);
  connect(line_edit_, &QLineEdit::textEdited, this, &Filter::filter);
  connect(line_edit_, &QLineEdit::returnPressed, this, &Filter::filter);

  layout_->addWidget(new QLabel("Filter:"));
  layout_->addWidget(line_edit_);
  layout_->addWidget(new QLabel("Filter Key:"));
  layout_->addWidget(column_sel_);
  layout_->addWidget(case_sensitive_);
  layout_->addWidget(wildcard_);
  layout_->addWidget(regex_);
  this->setLayout(layout_);
}

void Filter::connectView() {
  if (view_ == nullptr)
    return;

  proxy_->setSourceModel(view_->model());
  view_->setModel(proxy_);
  view_->setProperty("sortingEnabled", true);
  connectModel();
}

void Filter::disconnectView() {
  if (view_ == nullptr)
    return;
  auto source_model = proxy_->sourceModel();
  proxy_->setSourceModel(nullptr);
  view_->setModel(source_model);
  disconnectModel();
}

void Filter::connectModel() {
  proxy_->setRecursiveFilteringEnabled(true);
  proxy_->setFilterCaseSensitivity(sensitivity_);
  proxy_->setSortCaseSensitivity(sensitivity_);

  // two way bind case sensitivity
  connect(this, &Filter::caseSensitivityChanged, proxy_,
          &QSortFilterProxyModel::setFilterCaseSensitivity);
  connect(this, &Filter::caseSensitivityChanged, proxy_,
          &QSortFilterProxyModel::setSortCaseSensitivity);
  connect(proxy_, &QSortFilterProxyModel::filterCaseSensitivityChanged, this,
          &Filter::setCaseSensitivity);
  connect(proxy_, &QSortFilterProxyModel::sortCaseSensitivityChanged, this,
          &Filter::setCaseSensitivity);

  // two way filter key column
  connect(this, &Filter::columnChanged, proxy_,
          &QSortFilterProxyModel::setFilterKeyColumn);
  auto index_ = proxy_->bindableFilterKeyColumn();
  on_index_changed_ = index_.addNotifier([this]() {
    column_sel_->setCurrentIndex(proxy_->filterKeyColumn() + 1);
  });

  // setup column selection
  setupColumnSel(proxy_->sourceModel());
  connect(proxy_, &QSortFilterProxyModel::headerDataChanged, this,
          &Filter::onHeaderDataChanged);
  connect(proxy_, &QSortFilterProxyModel::modelReset, this,
          &Filter::onSourceModelChange);
}

void Filter::disconnectModel() {
  if (proxy_ == nullptr)
    return;

  // disconnect case sensitivity
  disconnect(this, &Filter::caseSensitivityChanged, proxy_,
             &QSortFilterProxyModel::setFilterCaseSensitivity);
  disconnect(this, &Filter::caseSensitivityChanged, proxy_,
             &QSortFilterProxyModel::setSortCaseSensitivity);
  disconnect(proxy_, &QSortFilterProxyModel::filterCaseSensitivityChanged, this,
             &Filter::setCaseSensitivity);
  disconnect(proxy_, &QSortFilterProxyModel::sortCaseSensitivityChanged, this,
             &Filter::setCaseSensitivity);

  // disconnect filter key column
  disconnect(this, &Filter::columnChanged, proxy_,
             &QSortFilterProxyModel::setFilterKeyColumn);
  on_index_changed_.setSource({});

  resetColumnSel();
  disconnect(proxy_, &QSortFilterProxyModel::headerDataChanged, this,
             &Filter::onHeaderDataChanged);
  disconnect(proxy_, &QSortFilterProxyModel::modelReset, this,
             &Filter::onSourceModelChange);
}

void Filter::setupColumnSel(QAbstractItemModel *model) {
  if (model == nullptr)
    return;

  resetColumnSel();

  int i = 0;
  while (true) {
    auto data =
        model->headerData(i++, Qt::Orientation::Horizontal, Qt::DisplayRole);
    if (not data.isValid())
      break;
    column_sel_->addItem(data.toString());
  }
  column_sel_->setCurrentIndex(proxy_->filterKeyColumn() + 1);
}

void Filter::resetColumnSel() {
  column_sel_->setCurrentIndex(0);
  for (int i = column_sel_->count() - 1; i > 0; --i) {
    column_sel_->removeItem(i);
  }
}
void Filter::filter() {
  if (proxy_ == nullptr or type_ == FilterType::None)
    return;

  switch (type_) {
  case FilterType::Fixed:
    proxy_->setFilterFixedString(line_edit_->text());
    break;
  case FilterType::Regex:
    proxy_->setFilterRegularExpression(line_edit_->text());
    break;
  case FilterType::Wildcard:
    proxy_->setFilterWildcard(line_edit_->text());
    break;
  default:
    return;
  }
}
} // namespace rds
