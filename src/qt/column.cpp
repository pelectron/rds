#include "rds/qt/column.hpp"

namespace rds {

QColumn::QColumn(DataId id) : id(id) {
  using enum DataId;
  switch (id) {
  case Name:
    header_label = "Name";
    break;
  case AddrPos:
    header_label = data_name = "Addr/Pos";
    break;
  case Value:
    header_label = data_name = "Value";
    break;
  case Initial:
    header_label = data_name = "Initial";
    break;
  case Unit:
    header_label = data_name = "Unit";
    break;
  case Description:
    header_label = data_name = "Description";
    break;
  case ZeroCodeValue:
    header_label = data_name = "Zero Code Value";
    break;
  case Step:
    header_label = data_name = "step";
    break;
  default:
    assert(id <= Step);
  }
}

QColumn::QColumn(std::string_view label, std::string_view name)
    : id(DataId::ExtraData), header_label(label.data()),
      data_name(name.data()) {}

QColumn::QColumn(std::string_view label, FieldAccess get_field_data,
                 RegisterAccess get_register_data)
    : id(DataId::Custom), header_label(label.data()), data_name(label.data()),
      field_data(std::move(get_field_data)),
      register_data(std::move(get_register_data)) {}
} // namespace rds
