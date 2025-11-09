#ifndef RDS_GROUP_HPP
#define RDS_GROUP_HPP

#include "rds/register.hpp"

#include <memory>

namespace rds {
// clang-format off
/**
 * @class Group
 * A Group is a continous region of memory, defined by a base address and size.
 *
 * All members of Group have public access. However, the members extra_data,
 * registers, and defaults should not be modified through the member 
 * directly, but through the Groups public interface:
 *  - has_data(name): check if extra_data contains a value with the specified name
 *  - add_data(name, value): adds new value to extra data, but does not override existing values.
 *  - set_data(name, value): adds or overrides existing extra data with the specified name.
 *  - data(name): access the extra data value by reference. has_data(name) must be true.
 *  - has_default_value(name): check if extra_data contains a value with the specified name
 *  - add_default_value(name, value): adds new default value, but does not override existing values.
 *  - set_default_value(name, value): adds or overrides a default value with the specified name.
 *  - default_value(name): access the default value by reference. has_default_value(name) must be true.
 *  - add_reg(): adds a register to the group
 *  - remove_reg(): removes a register from the group
 *
 * @ingroup rds
 */
struct Group {
  // TODO: parse and serialize description and defaults in Group
  Device *device = nullptr;
  std::string name{};
  std::string display_name{};
  std::string description;
  uint64_t base_addr{0};
  uint64_t size{0};
  std::map<std::string, Value> defaults;
  std::map<std::string, Value> extra_data;
  std::vector<std::unique_ptr<Register>> registers;
  // clang-format on
  using iterator = Iterator<Register>;
  using const_iterator = Iterator<const Register>;

  Group() = default;
  Group(std::string_view name, uint64_t base_addr, uint64_t size,
        Device *device = nullptr);
  Group(const Group &other);
  Group(Group &&other);
  Group &operator=(const Group &other);
  Group &operator=(Group &&other);

  bool has_register(std::string_view name) const;

  Register *reg(std::string_view name);
  const Register *reg(std::string_view name) const;
  Register *reg(uint64_t addr);
  const Register *reg(uint64_t addr) const;

  Register &operator[](std::string_view name);
  const Register &operator[](std::string_view name) const;

  Register *add_reg(std::string_view name, uint64_t addr, uint64_t size = 1);
  Register *add_reg(std::unique_ptr<Register> &&reg);

  void remove_reg(std::string_view name);

  void remove_reg(const Register *reg);

  /// check if defaults contains a value with the specified name
  bool has_default_value(std::string_view name) const;

  /// check if defaults contains a value with the specified name
  bool has_default_value(const std::string &name) const;

  /// check if defaults contains a value with the specified name
  bool has_default_value(const char *name) const;

  /// adds a default value with the specified name, but does not override
  /// existing defaults.
  bool add_default_value(std::string_view name, const Value &value);

  /// adds a default value with the specified name, but does not override
  /// existing defaults.
  bool add_default_value(const std::string &name, const Value &value);

  /// adds a default value with the specified name, but does not override
  /// existing defaults.
  bool add_default_value(const char *name, const Value &value);

  /// adds, or overrides existing, default value with the specified name.
  void set_default_value(std::string_view name, const Value &value);

  /// adds, or overrides existing, default value with the specified name.
  void set_default_value(const std::string &name, const Value &value);

  /// adds, or overrides existing, default value with the specified name.
  void set_default_value(const char *name, const Value &value);

  /// access the default value by reference. has_default_value(name) must be
  /// true to use this method.
  Value &default_value(std::string_view name);

  /// access the default value by reference. has_default_value(name) must be
  /// true to use this method.
  Value &default_value(const std::string &name);

  /// access the default value by reference. has_default_value(name) must be
  /// true to use this method.
  Value &default_value(const char *name);

  /// access the default value by reference. has_default_value(name) must be
  /// true to use this method.
  const Value &default_value(std::string_view name) const;

  /// access the default value by reference. has_default_value(name) must be
  /// true to use this method.
  const Value &default_value(const std::string &name) const;

  /// access the default value by reference. has_default_value(name) must be
  /// true to use this method.
  const Value &default_value(const char *name) const;

  /// check if extra_data contains a value with the specified name
  bool has_data(std::string_view name) const;

  /// check if extra_data contains a value with the specified name
  bool has_data(const std::string &name) const;

  /// check if extra_data contains a value with the specified name
  bool has_data(const char *name) const;

  /// adds new value to extra data, but does not override existing values.
  /// Returns true if the value has been added.
  bool add_data(std::string_view name, const Value &value);

  /// adds new value to extra data, but does not override existing values.
  /// Returns true if the value has been added.
  bool add_data(const std::string &name, const Value &value);

  /// adds new value to extra data, but does not override existing values.
  /// Returns true if the value has been added.
  bool add_data(const char *name, const Value &value);

  /// adds or overrides existing extra data with the specified name.
  void set_data(std::string_view name, const Value &value);

  /// adds or overrides existing extra data with the specified name.
  void set_data(const std::string &name, const Value &value);

  /// adds or overrides existing extra data with the specified name.
  void set_data(const char *name, const Value &value);

  /// access the extra data value by reference. has_data(name) must be true to
  /// call this method.
  Value &data(std::string_view name);

  /// access the extra data value by reference. has_data(name) must be true to
  /// call this method.
  Value &data(const std::string &name);

  /// access the extra data value by reference. has_data(name) must be true to
  /// call this method.
  Value &data(const char *name);

  /// access the extra data value by reference. has_data(name) must be true to
  /// call this method.
  const Value &data(std::string_view name) const;

  /// access the extra data value by reference. has_data(name) must be true to
  /// call this method.
  const Value &data(const std::string &name) const;

  /// access the extra data value by reference. has_data(name) must be true to
  /// call this method.
  const Value &data(const char *name) const;

  /// resets all registers in the group to their initial values
  void reset();

  /**
   * begin/end functions for range base for loop iteration over the groups
   * registers
   * @{
   */
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  /// @}
};
} // namespace rds
#endif // !RDS_GROUP_HPP
