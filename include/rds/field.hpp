#ifndef RDS_FIELD_HPP
#define RDS_FIELD_HPP

#include "rds/common.hpp"
#include "rds/value.hpp"
#include "rds/write_constraints.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace rds {

// clang-format off
/**
 * @class Field
 * A Field is a continous region of bits in a register.
 *
 * The field occupies the bits from lsb to and including msb in its parent
 * register.
 *
 * All members of Field have public access. However, the members extra_data,
 * value, and values should not be modified through the member directly, but 
 * through the Fields public interface:
 *  - set_value(val): sets the Fields value and returns wether the value could be successfully set.
 *  - reset(): reset the Fields value to its initial value.
 *  - add_value(value): adds an enumerated value to the Field.
 *  - add_values(values): adds a list of enumerated values to the Field.
 *  - has_data(name): check if extra_data contains a value with the specified name
 *  - add_data(name, value): adds new value to extra data, but does not override existing values.
 *  - set_data(name, value): adds or overrides existing extra data with the specified name.
 *  - data(name): access the extra data value by reference. has_data(name) must be true.
 *
 * To get the fields value in units, use the unit_value() method.
 *
 * @ingroup rds
 */
struct Field {
  Register *reg = nullptr;    //< ponter to the register that contains this field.
  std::string name{};         //< the name, for example 'IE'
  std::string display_name{}; //< the pretty name, for example 'Interrupt Enable'
  std::string description{};  //< the description as a string
  std::string backup{};       //< the backup method as a string
  std::string unit{};         //< the unit as a string, for example 'm'
  double zero_code_value = 0; //< unit conversion offset.
  double step = 1;            //< unit conversion factor.
  std::uint64_t initial{0};   //< the initial/reset value
  std::uint64_t value{0};     //< the current value
  uint64_t msb{0};            //< the most significant bit the field occupies
  uint64_t lsb{0};            //< the least significant bit the field occupies
  Access access{};            //< the access rights of the field
  Protection protection;
  WriteType write_type;
  ReadType read_type;
  WriteContraints contraints;
  bool is_signed{false};      //< true if the value is signed
  std::vector<EnumeratedValue> values{};//< a list of enumerated values.
  // std::vector<std::unique_ptr<Field>> variants;
  std::map<std::string, Value> extra_data; //< any extra data in the form of key value pairs
  // clang-format on

  Field() = default;
  Field(std::string_view name, uint64_t msb, uint64_t lsb,
        Register *reg = nullptr);
  Field(const Field &other);
  Field(Field &&other);
  Field &operator=(const Field &other);
  Field &operator=(Field &&other);

  /**
   * sets the fields value, also changing the value of its register, if
   * it has one. Returns true if the value could be set, else false.
   *
   * @param val the value
   * @return true if successful, else false.
   */
  bool set_value(uint64_t val);

  /**
   * returns the value of the register in units
   * @returns value * step + zero_code_value
   */
  double unit_value() const;

  /**
   * adds enumerated values to the register.
   * Returns true if the value could be added, else false.
   * The value cannot be added if another value with the same name or value as
   * val has beeen added before.
   * @param val the value to be added
   */
  bool add_value(const EnumeratedValue &val);

  /**
   * adds enumerated values to the register.
   * Returns true if the values could be added, else false.
   * The value cannot be added if another value with the same name or value as
   * val has beeen added before.
   * @param vals the values to be added
   */
  bool add_values(const std::vector<EnumeratedValue> &vals);

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

  /**
   * resets the fields value to its initial value
   */
  void reset();

  /**
   * returns a copy of this wrapped in a unique_ptr.
   */
  std::unique_ptr<Field> clone() const;
};

} // namespace rds

#endif
