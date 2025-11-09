#include "rds/field.hpp"
#include "rds/detail/rds.hpp"
#include "rds/register.hpp"

#include <cstdint>
#include <memory>

namespace rds {

Field::Field(std::string_view name, uint64_t msb, uint64_t lsb, Register *reg)
    : reg(reg), name(name.data(), name.size()),
      display_name(name.data(), name.size()), msb(msb), lsb(lsb) {}

Field::Field(const Field &other)
    : name(other.name), display_name(other.display_name),
      description(other.description), backup(other.backup), unit(other.unit),
      zero_code_value(other.zero_code_value), step(other.step),
      initial(other.initial), value(other.value), msb(other.msb),
      lsb(other.lsb), access(other.access), is_signed(other.is_signed),
      values(other.values), extra_data(other.extra_data) {
  // TODO: variants
}

Field::Field(Field &&other)
    : name(std::move(other.name)), display_name(std::move(other.display_name)),
      description(std::move(other.description)),
      backup(std::move(other.backup)), unit(std::move(other.unit)),
      zero_code_value(other.zero_code_value), step(other.step),
      initial(other.initial), value(other.value), msb(other.msb),
      lsb(other.lsb), access(other.access), is_signed(other.is_signed),
      values(std::move(other.values)), extra_data(std::move(other.extra_data)) {
  // TODO: variants
}

Field &Field::operator=(const Field &other) {
  Field f(other);
  *this = std::move(f);
  return *this;
}

Field &Field::operator=(Field &&other) {
  name = std::move(other.name);
  display_name = std::move(other.display_name);
  description = std::move(other.description);
  backup = std::move(other.backup);
  unit = std::move(other.unit);
  zero_code_value = other.zero_code_value;
  step = other.step;
  initial = other.initial;
  value = other.value;
  msb = other.msb;
  lsb = other.lsb;
  access = other.access;
  is_signed = other.is_signed;
  values = std::move(other.values);
  extra_data = std::move(other.extra_data);
  // TODO: variants
  return *this;
}

bool Field::set_value(uint64_t val) {
  const auto mask = detail::make_mask(msb, lsb);
  const auto set_register_value = [this, val, mask]() -> bool {
    if (not reg)
      return true;

    reg->value = (reg->value & ~mask) | (val << lsb);
    return true;
  };

  if (val & ~(mask >> lsb))
    return false;

  if (not values.empty()) {
    for (const auto &v : values) {
      if (v.value == val) {
        value = val;
        return set_register_value();
      }
    }
    return false;
  } else {
    value = val;
    return set_register_value();
  }
}

double Field::unit_value() const {
  if (not is_signed or (value & (1u << msb)) == 0)
    return zero_code_value + step * value;
  // value is negative
  return -1.0 * (((~value + 1) & detail::make_mask(msb, lsb)) * step) +
         zero_code_value;
}

bool Field::add_value(const EnumeratedValue &val) {
  return ::rds::detail::add_value(values, val, msb - lsb + 1);
}

bool Field::add_values(const std::vector<EnumeratedValue> &vals) {
  return ::rds::detail::add_values(values, vals, msb - lsb + 1);
}

bool Field::has_data(std::string_view name) const {
  return extra_data.contains({name.data(), name.size()});
}

bool Field::has_data(const std::string &name) const {
  return extra_data.contains(name);
}

bool Field::has_data(const char *name) const {
  return extra_data.contains(name);
}

bool Field::add_data(std::string_view name, const Value &value) {
  auto [_, inserted] = extra_data.insert({{name.data(), name.size()}, value});
  return inserted;
}

bool Field::add_data(const std::string &name, const Value &value) {
  auto [_, inserted] = extra_data.insert({name, value});
  return inserted;
}

bool Field::add_data(const char *name, const Value &value) {
  auto [_, inserted] = extra_data.insert({name, value});
  return inserted;
}

void Field::set_data(std::string_view name, const Value &value) {
  extra_data.insert_or_assign({name.data(), name.size()}, value);
}

void Field::set_data(const std::string &name, const Value &value) {
  extra_data.insert_or_assign(name, value);
}

void Field::set_data(const char *name, const Value &value) {
  extra_data.insert_or_assign(name, value);
}

Value &Field::data(std::string_view name) {
  return extra_data.at({name.data(), name.size()});
}

Value &Field::data(const std::string &name) { return extra_data.at(name); }

Value &Field::data(const char *name) { return extra_data.at(name); }

const Value &Field::data(std::string_view name) const {
  return extra_data.at({name.data(), name.size()});
}

const Value &Field::data(const std::string &name) const {
  return extra_data.at(name);
}

const Value &Field::data(const char *name) const { return extra_data.at(name); }

void Field::reset() { value = initial; }

std::unique_ptr<Field> Field::clone() const {
  return std::make_unique<Field>(*this);
}

} // namespace rds
