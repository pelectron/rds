#include "rds/register.hpp"
#include "rds/detail/rds.hpp"
#include "rds/device.hpp"

#include <memory>

namespace rds {

Register::Register(std::string_view name, uint64_t addr, uint64_t size,
                   Group *group)
    : group(group), name(name.data(), name.size()),
      display_name(name.data(), name.size()), addr(addr), size(size) {}

Register::Register(const Register &other)
    : name(other.name), display_name(other.display_name),
      description(other.description), backup(other.backup), unit(other.unit),
      zero_code_value(other.zero_code_value), step(other.step),
      initial(other.initial), value(other.value), addr(other.addr),
      size(other.size), zeros_mask(other.zeros_mask),
      ones_mask(other.ones_mask), x_mask(other.x_mask), access(other.access),
      is_signed(other.is_signed), values(other.values),
      extra_data(other.extra_data) {
  for (const auto &f : other.fields)
    fields.push_back(std::make_unique<Field>(*f));
  for (auto &f : fields)
    f->reg = this;
}

Register::Register(Register &&other)
    : name(std::move(other.name)), display_name(std::move(other.display_name)),
      description(std::move(other.description)),
      backup(std::move(other.backup)), unit(std::move(other.unit)),
      zero_code_value(other.zero_code_value), step(other.step),
      initial(other.initial), value(other.value), addr(other.addr),
      size(other.size), zeros_mask(other.zeros_mask),
      ones_mask(other.ones_mask), x_mask(other.x_mask), access(other.access),
      is_signed(other.is_signed), values(std::move(other.values)),
      fields(std::move(other.fields)), extra_data(std::move(other.extra_data)) {
  for (auto &f : fields)
    f->reg = this;
}

Register &Register::operator=(const Register &other) {
  name = other.name;
  display_name = other.display_name;
  description = other.description;
  backup = other.backup;
  unit = other.unit;
  zero_code_value = other.zero_code_value;
  step = other.step;
  initial = other.initial;
  value = other.value;
  addr = other.addr;
  size = other.size;
  zeros_mask = other.zeros_mask;
  ones_mask = other.ones_mask;
  x_mask = other.x_mask;
  access = other.access;
  is_signed = other.is_signed;
  values = other.values;
  extra_data = other.extra_data;
  for (auto &f : other.fields) {
    fields.push_back(std::make_unique<Field>(*f));
    f->reg = this;
  }
  return *this;
}

Register &Register::operator=(Register &&other) {
  name = std::move(other.name);
  display_name = std::move(other.display_name);
  description = std::move(other.description);
  backup = std::move(other.backup);
  unit = std::move(other.unit);
  zero_code_value = other.zero_code_value;
  step = other.step;
  initial = other.initial;
  value = other.value;
  addr = other.addr;
  size = other.size;
  zeros_mask = other.zeros_mask;
  ones_mask = other.ones_mask;
  x_mask = other.x_mask;
  access = other.access;
  is_signed = other.is_signed;
  values = std::move(other.values);
  fields = std::move(other.fields);
  extra_data = std::move(other.extra_data);
  for (auto &f : fields)
    f->reg = this;
  return *this;
}

double Register::unit_value() const {
  const auto n = num_bits();
  if (not is_signed or (value & (1u << (n - 1))) == 0)
    return zero_code_value + step * value;
  // value is negative
  return -1.0 * (((~value + 1) & detail::make_mask(n - 1, 0)) * step) +
         zero_code_value;
}

uint64_t Register::num_bits() const {
  return size * (group ? group->device ? group->device->register_width : 8 : 8);
}

bool Register::has_field(std::string_view name) const {
  return std::find_if(fields.begin(), fields.end(),
                      [&name](const std::unique_ptr<Field> &f) {
                        return f->name == name;
                      }) != fields.end();
}

Field *Register::field(std::string_view name) {
  auto it = std::find_if(
      fields.begin(), fields.end(),
      [&name](const std::unique_ptr<Field> &f) { return f->name == name; });
  if (it == fields.end())
    return nullptr;
  else
    return it->get();
}

const Field *Register::field(std::string_view name) const {
  auto it = std::find_if(
      fields.begin(), fields.end(),
      [&name](const std::unique_ptr<Field> &f) { return f->name == name; });
  if (it == fields.end())
    return nullptr;
  else
    return it->get();
}

Field &Register::operator[](std::string_view name) { return *field(name); }

const Field &Register::operator[](std::string_view name) const {
  return *field(name);
}

Field *Register::add_field(std::unique_ptr<Field> &&f) {
  if (not f.get())
    return nullptr;

  if (not detail::is_valid_field(num_bits(), fields, *f))
    return nullptr;

  return fields
      .insert(std::find_if(fields.begin(), fields.end(),
                           [&f](const std::unique_ptr<Field> &field) {
                             return f->lsb < field->lsb;
                           }),
              std::move(f))
      ->get();
}

Field *Register::add_field(std::string_view name, uint64_t position) {
  return add_field(name, position, position);
}

Field *Register::add_field(const std::string &name, uint64_t position) {
  return add_field(std::string_view{name}, position);
}

Field *Register::add_field(const char *name, uint64_t position) {
  return add_field(std::string_view{name}, position);
}

Field *Register::add_field(std::string_view name, const uint64_t msb,
                           const uint64_t lsb) {
  if (not detail::is_valid_field(num_bits(), fields, Field{name, msb, lsb}))
    return nullptr;

  auto *field =
      fields
          .insert(std::find_if(fields.begin(), fields.end(),
                               [lsb](const std::unique_ptr<Field> &f) {
                                 return lsb < f->lsb;
                               }),
                  std::make_unique<Field>())
          ->get();
  field->reg = this;
  field->name = name.data();
  field->msb = msb;
  field->lsb = lsb;
  return field;
}

Field *Register::add_field(const std::string &name, const uint64_t msb,
                           const uint64_t lsb) {
  return add_field(std::string_view{name}, msb, lsb);
}

Field *Register::add_field(const char *name, const uint64_t msb,
                           const uint64_t lsb) {
  return add_field(std::string_view{name}, msb, lsb);
}

bool Register::set_value(uint64_t val) {
  const auto set_field_values = [this, val]() {
    for (auto &f : fields) {
      auto mask = detail::make_mask(f->msb, f->lsb);
      const auto field_value = (val & mask) >> f->lsb;
      if (f->values.empty()) {
        f->value = field_value;
      } else {
        for (const auto &v : f->values) {
          if (v.value == field_value) {
            f->value = field_value;
            continue;
          }
        }
        return false;
      }
    }
    return true;
  };

  if ((val & zeros_mask) != 0)
    return false;

  if ((val & ones_mask) != ones_mask)
    return false;

  if ((val & detail::make_mask(num_bits() - 1, 0)) != val)
    return false;

  if (not values.empty()) {
    for (const auto &v : values) {
      if (v.value == val) {
        value = val;
        return set_field_values();
      }
    }
    return false;
  } else {
    value = val;
    return set_field_values();
  }
}

bool Register::add_value(const EnumeratedValue &val) {
  return ::rds::detail::add_value(values, val, num_bits());
}

bool Register::add_values(const std::vector<EnumeratedValue> &vals) {
  return ::rds::detail::add_values(values, vals, num_bits());
}

bool Register::has_data(std::string_view name) const {
  return extra_data.contains({name.data(), name.size()});
}

bool Register::has_data(const std::string &name) const {
  return extra_data.contains(name);
}

bool Register::has_data(const char *name) const {
  return extra_data.contains(name);
}

bool Register::add_data(std::string_view name, const Value &value) {
  auto [_, inserted] = extra_data.insert({{name.data(), name.size()}, value});
  return inserted;
}

bool Register::add_data(const std::string &name, const Value &value) {
  auto [_, inserted] = extra_data.insert({name, value});
  return inserted;
}

bool Register::add_data(const char *name, const Value &value) {
  auto [_, inserted] = extra_data.insert({name, value});
  return inserted;
}

void Register::set_data(std::string_view name, const Value &value) {
  extra_data.insert_or_assign({name.data(), name.size()}, value);
}

void Register::set_data(const std::string &name, const Value &value) {
  extra_data.insert_or_assign(name, value);
}

void Register::set_data(const char *name, const Value &value) {
  extra_data.insert_or_assign(name, value);
}

Value &Register::data(std::string_view name) {
  return extra_data.at({name.data(), name.size()});
}

Value &Register::data(const std::string &name) { return extra_data.at(name); }

Value &Register::data(const char *name) { return extra_data.at(name); }

const Value &Register::data(std::string_view name) const {
  return extra_data.at({name.data(), name.size()});
}

const Value &Register::data(const std::string &name) const {
  return extra_data.at(name);
}

const Value &Register::data(const char *name) const {
  return extra_data.at(name);
}

void Register::reset() { set_value(initial); }

Register::iterator Register::begin() { return fields.begin(); }

Register::const_iterator Register::begin() const { return fields.begin(); }

Register::iterator Register::end() { return fields.end(); }

Register::const_iterator Register::end() const { return fields.end(); }

} // namespace rds
