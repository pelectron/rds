#ifndef RDS_REGISTER_HPP
#define RDS_REGISTER_HPP

#include "rds/common.hpp"
#include "rds/field.hpp"
#include "rds/iterator.hpp"
#include "rds/write_constraints.hpp"

#include <string>

namespace rds {

// clang-format off
/**
 * @class Register
 *
 * Registers are continuous regions of memory of one or more
 * Device.register_widths in size.
 *
 * There are three kinds of registers:
 *  - integer like: these registers don't have any enumerated 
 *    values or fields associated with them.
 * - enum like: these regisers don't have any fields, but do 
 *   have enumerated values associated wiht them.
 * - struct like: these registers have subfields, which are 
 *   like members of a struct. This kind usually doesn't have 
 *   any enumerated values.
 *
 * All members of Register have public access. However, the members extra_data,
 * value, values, and fields should not be modified through the member directly, but
 * through the Registers public interface:
 *  - modifying value:
 *    - set_value(val): sets the Registers value and returns wether the value could be successfully set.
 *    - reset(): reset the Registers value to its initial value.
 *    - unit_value(): returns the Registers value in units.
 *  - modifying (enumerated) values:
 *    - add_value(value): adds an enumerated value to the register.
 *    - add_values(values): adds a list of enumerated values to the register.
 *  - modifying and accessing extra_data
 *    - has_data(name): returns true if extra_data contains a value with the specified name
 *    - add_data(name, value): adds new value to extra data, but does not override existing values.
 *    - set_data(name, value): adds or overrides existing extra data with the specified name.
 *    - data(name): access the extra data value by reference. has_data(name) must be true.
 *  - modifying and accessing fields
 *    - add_field(...): adds a field to fields.
 *    - remove_field(...): removes a field.
 *    - field(name): get a pointer to a field, if it exists.
 *    - operator[name]: access a field by name.
 *
 * In addition to operator[] and field(), Register provides iterators for its fields. 
 * Note that this iterator is a wrapper around 
 * std::vector<std::unique_pt<Field>>::iterator so Register "feels like" a container of fields.
 *
 * Example:
 * ```
 *  rds::Register r{...};
 *  // r.add_field(...);
 *  // r.add_field(...);
 *  // ...
 *  for(const rds::Field&f: r){
 *   // do something with f
 *  }
 * ```
 * @ingroup rds
 */
struct Register {
  Group *group = nullptr;
  std::string name{};
  std::string display_name{};
  std::string description{};
  std::string backup{};
  std::string unit{};
  double zero_code_value = 0;
  double step = 1;
  std::uint64_t initial{0};
  std::uint64_t value{0};
  std::uint64_t addr{0};
  std::uint64_t size{1};
  std::uint64_t zeros_mask{0};
  std::uint64_t ones_mask{0};
  std::uint64_t x_mask{0};
  Access access{};
  Protection protection{};
  WriteType write_type{};
  ReadType read_type{};
  WriteContraints contraints{};
  bool is_signed{false};
  std::vector<EnumeratedValue> values{};
  std::vector<std::unique_ptr<Field>> fields{};
  std::map<std::string, Value> extra_data{};
  // clang-format on

  /// the non-const iterator for iterating fields
  using iterator = Iterator<Field>;
  /// the const iterator for iterating fields
  using const_iterator = Iterator<const Field>;

  /**
   * construct a Register with its name, address size, and optional parent group
   *
   * @note setting group to a non-null pointer will not add this to the groups
   * registers.
   *
   * @param name the name
   * @param addr the address
   * @param size the size
   * @param group the group
   */
  Register(std::string_view name, uint64_t addr, uint64_t size,
           Group *group = nullptr);

  Register() = default;
  Register(const Register &other);
  Register(Register &&other);

  Register &operator=(const Register &other);
  Register &operator=(Register &&other);

  /**
   * returns the value of the register in units
   * @returns value * step + zero_code_value
   */
  double unit_value() const;

  /**
   * return the size of the register in bits.
   * @note this is not the size, but size * group->device->register_width. The
   * default register width used is 8 if the register has not been added to a
   * group yet.
   */
  uint64_t num_bits() const;

  /// checks if this register contains a field with the specified name
  bool has_field(std::string_view name) const;

  /// gets the field with the specified name. Returns nullptr if such a
  /// field does not exist.
  Field *field(std::string_view name);

  /// gets the field with the specified name. Returns nullptr if such a
  /// field does not exist.
  const Field *field(std::string_view name) const;

  /// returns a reference to the field with the specified name.
  /// `has_field(name)` must be true.
  Field &operator[](std::string_view name);

  /// returns a reference to the field with the specified name.
  /// `has_field(name)` must be true.
  const Field &operator[](std::string_view name) const;

  /// adds a field to the register. This will return a pointer to the added
  /// Field, or nullptr in case of failure.
  Field *add_field(std::unique_ptr<Field> &&f);

  /**
   * add a field to the register. This will return a pointer to the added Field,
   * or nullptr in case of failure. This will fail if a field with the same name
   * or an overlapping field has already been added, or if the field is outside
   * the valid bitrange for the register.
   *
   * @param name the name
   * @param position the bit position
   * @{
   */
  Field *add_field(std::string_view name, uint64_t position);
  Field *add_field(const std::string &name, uint64_t position);
  Field *add_field(const char *name, uint64_t position);
  /// @}

  /**
   * add a field to the register. This will return a pointer to the added Field,
   * or nullptr in case of failure. This will fail if a field with the same name
   * or an overlapping field has already been added, or if the field is outside
   * the valid bitrange for the register.
   *
   * @param name the name
   * @param msb the msb position in the register (inclusive)
   * @param lsb the lsb position in the reigster (inclusive)
   * @{
   */
  Field *add_field(std::string_view name, const uint64_t msb,
                   const uint64_t lsb);
  Field *add_field(const std::string &name, const uint64_t msb,
                   const uint64_t lsb);
  Field *add_field(const char *name, const uint64_t msb, const uint64_t lsb);
  /// @}

  /**
   * adds enumerated values to the register.
   * Returns true if the value could be added, else false.
   * The value cannot be added if another value with the same name or value as
   * val has beeen added before.
   *
   * @param val the value to be added
   */
  bool add_value(const EnumeratedValue &val);

  /**
   * adds enumerated values to the register.
   * Returns true if the value could be added, else false.
   * The value cannot be added if another value with the same name or value as
   * val has beeen added before.
   *
   * @param vals the values to be added
   */
  bool add_values(const std::vector<EnumeratedValue> &vals);

  ///  sets the registers value and propogates the value forward to its fields,
  ///  if it has any. Returns true if the value could be set, else false.
  bool set_value(uint64_t val);

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

  /// resets the register value to its initial value.
  void reset();

  /**
   * begin/end functions for range base for loop iteration over the registers
   * fields
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
