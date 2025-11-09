#ifndef RDS_DEVICE_HPP
#define RDS_DEVICE_HPP

#include "rds/group.hpp"

#include <cstdint>
#include <memory>

namespace rds {

/**
 * This class is the root of the root node of the register description format.
 *
 * @ingroup rds
 */
struct Device {
  std::string name{};
  int64_t version{1};
  // TODO: parse license
  std::string license{};
  std::string description{};
  // TODO:: change register_width to better name
  int64_t register_width{8};
  // TODO: parse word_width
  uint64_t word_width;
  int64_t num_pages{0};
  int64_t registers_per_page{0};
  Endian endian = Endian::little;
  std::vector<std::unique_ptr<Group>> groups{};
  std::map<std::string, Value> extra_data{};
  using iterator = Iterator<Group>;
  using const_iterator = Iterator<const Group>;

  Device() = default;
  Device(const Device &other);
  Device(Device &&other);
  Device &operator=(const Device &other);
  Device &operator=(Device &&other);

  /// checks if the device has a group with the specified name
  bool has_group(std::string_view name) const;

  /// returns a pointer to the group with the specified name, or nullptr if such
  /// a group does not exist.
  Group *group(std::string_view name);

  /// returns a pointer to the group with the specified name, or nullptr if such
  /// a group does not exist.
  const Group *group(std::string_view name) const;

  /// returns a pointer to the group with the specified base address, or nullptr
  /// if such a group does not exist.
  Group *group(uint64_t base_addr);

  /// returns a pointer to the group with the specified base address, or nullptr
  /// if such a group does not exist.
  const Group *group(uint64_t base_addr) const;

  /// returns a reference to the group with the specified base address.
  /// has_group(name) must be true in order to call this function.
  Group &operator[](std::string_view name);

  /// returns a reference to the group with the specified base address.
  /// has_group(name) must be true in order to call this function.
  const Group &operator[](std::string_view name) const;

  /**
   * adds a group to the device. Returns a pointer to the added Group or
   * nullptr if the group could not be added.
   *
   * @note a group can't be added if another group with the same name or base
   * address already exists or if the new group would overap with existing
   * groups.
   *
   * @param name the groups name
   * @param base_addr the groups base address
   * @param size the groups size
   */
  Group *add_group(std::string_view name, uint64_t base_addr, uint64_t size);

  /**
   * adds a group to the device. Returns a pointer to the added Group or
   * nullptr if the group could not be added.
   *
   * @note a group can't be added if another group with the same name or base
   * address already exists or if the new group would overap with existing
   * groups.
   *
   * @param group the group to add
   */
  Group *add_group(std::unique_ptr<Group> &&group);

  /// removes the group with the specified name
  void remove_group(std::string_view name);

  /// remove the specified group
  void remove_group(const Group *group);

  Contents content() const;

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

  /// reset all registers to their initial values
  void reset();

  /// returns a copy of this wrapped in a unique_ptr.
  std::unique_ptr<Device> clone() const;

  /**
   * begin/end functions for range base for loop iteration over the device
   * groups
   * @{
   */
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
  /// @}
};

} // namespace rds

#endif
