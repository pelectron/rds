#include "rds/group.hpp"
#include "rds/detail/rds.hpp"

#include <memory>

namespace rds {

Group::Group(std::string_view name, uint64_t base_addr, uint64_t size,
             Device *device)
    : device(device), name(name.data(), name.size()), base_addr(base_addr),
      size(size) {}

Group::Group(const Group &other)
    : name(other.name), base_addr(other.base_addr), size(other.size),
      defaults(other.defaults) {
  for (const auto &r : other.registers)
    registers.push_back(std::make_unique<Register>(*r));
  for (auto &r : registers)
    r->group = this;
}

Group::Group(Group &&other)
    : name(std::move(other.name)), base_addr(other.base_addr), size(other.size),
      defaults(std::move(other.defaults)),
      registers(std::move(other.registers)) {
  for (auto &r : registers)
    r->group = this;
}

Group &Group::operator=(const Group &other) {
  std::vector<std::unique_ptr<Register>> rs;
  for (const auto &r : other.registers)
    rs.push_back(std::make_unique<Register>(*r));
  for (auto &r : registers)
    r->group = this;
  name = other.name;
  base_addr = other.base_addr;
  size = other.size;
  defaults = other.defaults;
  registers = std::move(rs);
  return *this;
}

Group &Group::operator=(Group &&other) {
  name = std::move(other.name);
  base_addr = other.base_addr;
  size = other.size;
  defaults = other.defaults;
  registers = std::move(other.registers);
  for (auto &r : registers)
    r->group = this;
  return *this;
}

bool Group::has_register(std::string_view name) const {
  return std::find_if(registers.begin(), registers.end(),
                      [&name](const std::unique_ptr<Register> &r) {
                        return r->name == name;
                      }) != registers.end();
}

Register *Group::reg(std::string_view name) {
  auto it = std::find_if(
      registers.begin(), registers.end(),
      [name](const std::unique_ptr<Register> &r) { return r->name == name; });
  if (it == registers.end())
    return nullptr;
  else
    return it->get();
}

const Register *Group::reg(std::string_view name) const {
  auto it = std::find_if(
      registers.begin(), registers.end(),
      [&name](const std::unique_ptr<Register> &r) { return r->name == name; });
  if (it == registers.end())
    return nullptr;
  else
    return it->get();
}

Register *Group::reg(uint64_t addr) {
  auto it = std::find_if(
      registers.begin(), registers.end(),
      [addr](const std::unique_ptr<Register> &r) { return r->addr == addr; });
  if (it == registers.end())
    return nullptr;
  else
    return it->get();
}

const Register *Group::reg(uint64_t addr) const {
  auto it = std::find_if(
      registers.begin(), registers.end(),
      [addr](const std::unique_ptr<Register> &r) { return r->addr == addr; });
  if (it == registers.end())
    return nullptr;
  else
    return it->get();
}

Register &Group::operator[](std::string_view name) { return *reg(name); }

const Register &Group::operator[](std::string_view name) const {
  return *reg(name);
}

Register *Group::add_reg(std::string_view name, uint64_t addr, uint64_t size) {
  if (not detail::is_valid_register(
          registers, Register{detail::to_string(name), addr, size}))
    return nullptr;

  auto *reg =
      registers
          .insert(std::find_if(registers.begin(), registers.end(),
                               [addr](const std::unique_ptr<Register> &r) {
                                 return addr < r->addr;
                               }),
                  std::make_unique<Register>())
          ->get();
  reg->group = this;
  reg->name = name.data();
  reg->addr = addr;
  reg->size = size;
  return reg;
}

Register *Group::add_reg(std::unique_ptr<Register> &&reg) {
  if (not reg or not detail::is_valid_register(registers, *reg))
    return nullptr;
  auto *r = registers
                .insert(std::find_if(registers.begin(), registers.end(),
                                     [addr = reg->addr](
                                         const std::unique_ptr<Register> &r) {
                                       return addr < r->addr;
                                     }),
                        std::move(reg))
                ->get();
  r->group = this;
  return r;
}

void Group::remove_reg(std::string_view name) {
  auto it = std::find_if(registers.begin(), registers.end(),
                         [name](const auto &reg) { return reg->name == name; });
  if (it == registers.end())
    return;
  registers.erase(it);
}

void Group::remove_reg(const Register *reg) {
  if (not reg or reg->group != this)
    return;
  auto it = std::find_if(registers.begin(), registers.end(),
                         [reg](const auto &r) { return reg == r.get(); });
  if (it == registers.end())
    return;
  registers.erase(it);
}

bool Group::has_default_value(std::string_view name) const {
  return defaults.contains({name.data(), name.size()});
}

bool Group::has_default_value(const std::string &name) const {
  return defaults.contains(name);
}

bool Group::has_default_value(const char *name) const {
  return defaults.contains(name);
}

bool Group::add_default_value(std::string_view name, const Value &value) {
  auto [_, inserted] = defaults.insert({{name.data(), name.size()}, value});
  return inserted;
}

bool Group::add_default_value(const std::string &name, const Value &value) {
  auto [_, inserted] = defaults.insert({name, value});
  return inserted;
}

bool Group::add_default_value(const char *name, const Value &value) {
  auto [_, inserted] = defaults.insert({name, value});
  return inserted;
}

void Group::set_default_value(std::string_view name, const Value &value) {
  defaults.insert_or_assign({name.data(), name.size()}, value);
}

void Group::set_default_value(const std::string &name, const Value &value) {
  defaults.insert_or_assign(name, value);
}

void Group::set_default_value(const char *name, const Value &value) {
  defaults.insert_or_assign(name, value);
}

Value &Group::default_value(std::string_view name) {
  return defaults.at({name.data(), name.size()});
}

Value &Group::default_value(const std::string &name) {
  return defaults.at(name);
}

Value &Group::default_value(const char *name) { return defaults.at(name); }

const Value &Group::default_value(std::string_view name) const {
  return defaults.at({name.data(), name.size()});
}

const Value &Group::default_value(const std::string &name) const {
  return defaults.at(name);
}

const Value &Group::default_value(const char *name) const {
  return defaults.at(name);
}
bool Group::has_data(std::string_view name) const {
  return extra_data.contains({name.data(), name.size()});
}

bool Group::has_data(const std::string &name) const {
  return extra_data.contains(name);
}

bool Group::has_data(const char *name) const {
  return extra_data.contains(name);
}

bool Group::add_data(std::string_view name, const Value &value) {
  auto [_, inserted] = extra_data.insert({{name.data(), name.size()}, value});
  return inserted;
}

bool Group::add_data(const std::string &name, const Value &value) {
  auto [_, inserted] = extra_data.insert({name, value});
  return inserted;
}

bool Group::add_data(const char *name, const Value &value) {
  auto [_, inserted] = extra_data.insert({name, value});
  return inserted;
}

void Group::set_data(std::string_view name, const Value &value) {
  extra_data.insert_or_assign({name.data(), name.size()}, value);
}

void Group::set_data(const std::string &name, const Value &value) {
  extra_data.insert_or_assign(name, value);
}

void Group::set_data(const char *name, const Value &value) {
  extra_data.insert_or_assign(name, value);
}

Value &Group::data(std::string_view name) {
  return extra_data.at({name.data(), name.size()});
}

Value &Group::data(const std::string &name) { return extra_data.at(name); }

Value &Group::data(const char *name) { return extra_data.at(name); }

const Value &Group::data(std::string_view name) const {
  return extra_data.at({name.data(), name.size()});
}

const Value &Group::data(const std::string &name) const {
  return extra_data.at(name);
}

const Value &Group::data(const char *name) const { return extra_data.at(name); }

void Group::reset() {
  for (auto &reg : registers)
    reg->reset();
}

Group::iterator Group::begin() { return registers.begin(); }

Group::const_iterator Group::begin() const { return registers.begin(); }

Group::iterator Group::end() { return registers.end(); }

Group::const_iterator Group::end() const { return registers.end(); }
} // namespace rds
