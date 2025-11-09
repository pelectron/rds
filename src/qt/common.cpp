#include "rds/qt/common.hpp"
#include "qvariant.h"
#include "rds/util.hpp"

namespace rds {

QVariant to_qvariant(const SimpleValue &v) {
  return std::visit(
      rds::util::overload{[](const auto &v) { return QVariant{v}; },
                          [](const std::string &s) {
                            return QVariant{QString::fromStdString(s)};
                          }},
      v);
}

std::optional<SimpleValue> to_simple_value(const QVariant &v) {
  if (not v.isValid())
    return std::nullopt;
  switch (v.metaType().id()) {
  case QMetaType::Bool:
    return v.toBool();
  case QMetaType::Short:
    [[fallthrough]];
  case QMetaType::Int:
    [[fallthrough]];
  case QMetaType::Long:
    [[fallthrough]];
  case QMetaType::LongLong:
    return v.toLongLong();
  case QMetaType::UShort:
    [[fallthrough]];
  case QMetaType::UInt:
    [[fallthrough]];
  case QMetaType::ULong:
    [[fallthrough]];
  case QMetaType::ULongLong:
    return v.toULongLong();
  case QMetaType::Float:
    [[fallthrough]];
  case QMetaType::Double:
    return v.toDouble();
  case QMetaType::QString:
    return v.toString().toStdString();
  default:
    return std::nullopt;
  }
}
} // namespace rds
