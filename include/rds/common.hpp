/**
 * @file rds/common.hpp
 * @brief contains types, enums, and functions used by the rest of rds.
 */

#ifndef RDS_COMMON_HPP
#define RDS_COMMON_HPP

#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace rds {

/**
 * Describes how a register or field can be accessed.
 *
 * Below is the mapping from Access to and from strings.
 * All strings given can pe parsed to an Access value. A given access value is
 * transfomred into a string according to the first string given in the list.
 *
 * Value            | string representation
 * ---------------------------------------------------
 *  read_only       | 'read-only' or 'r'
 *  write_only      | 'write-only' or 'w'
 *  read_write      | 'read-write', 'rw' or 'wr'
 *  write_once      | 'write-once' or 'wo'
 *  read_write_once | 'read-write-once', 'rwo' or 'wro'
 * @ingroup rds
 * @ingroup access-protection
 */
enum class Access {
  // clang-format off
  read_only,      ///< Read access is permitted. Write operations have an undefined result.
  write_only,     ///< Read operations have an undefined result. Write access is permitted.
  read_write,     ///< Read and write accesses are permitted. Writes affect the state of the register and reads return the register value.
  write_once,     ///< Read operations have an undefined results. Only the first write after reset has an effect.
  read_write_once ///< Read access is always permitted. Only the first write access after a reset will have an effect on the content. Other write operations have an undefined result.
};
// clang-format on

/**
 * Describes the security privilege needed to access an address region.
 * @ingroup rds
 * @ingroup access-protection
 */
enum class Protection {
  // clang-format off
  non_secure, ///< non-secure or secure permission required for access
  secure,     ///< secure permission required for access
  privileged, ///< privileged permission required for access
};
// clang-format on

/**
 * Describes how a register or field is affected by a write
 * @ingroup rds
 * @ingroup access-protection
 */
enum class WriteType {
  // clang-format off
  modify,         ///< after a write operation all bit in the field may be modified.
  one_to_clear,   ///< write data bits of one shall clear (set to zero) the corresponding bit in the register.
  one_to_set,     ///< write data bits of one shall set (set to one) the corresponding bit in the register.
  one_to_toggle,  ///< write data bits of one shall toggle (invert) the corresponding bit in the register.
  zero_to_clear,  ///< write data bits of zero shall clear (set to zero) the corresponding bit in the register.
  zero_to_set,    ///< write data bits of zero shall set (set to one) the corresponding bit in the register.
  zero_to_toggle, ///< write data bits of zero shall toggle (invert) the corresponding bit in the register.
  clear,          ///< after a write operation all bits in the field are cleared (set to zero).
  set,            ///< after a write operation all bits in the field are set (set to one).
};
// clang-format on

/**
 * Describes how a register or field is affected by a read operation
 * @ingroup rds
 * @ingroup access-protection
 */
enum class ReadType {
  // clang-format off
  none,           ///< The register is unchanged after a read
  clear,          ///< The register is cleared (set to zero) following a read operation.
  set,            ///< The register is set (set to ones) following a read operation.
  modify,         ///< The register is modified in some way after a read operation.
  modify_external ///< One or more dependent resources other than the current register are immediately affected by a read operation (it is recommended that the register description specifies these dependencies).
};
// clang-format on

/**
 * This enumerates the supported endian types
 * @ingroup rds
 */
enum class Endian {
  little, ///< little endian
  big     ///< big endian
};

/**
 *
 * @ingroup rds
 */
using SimpleValue = std::variant<std::string, double, uint64_t, int64_t, bool>;

/**
 * @class RegisterData
 * @brief
 * @ingroup rds
 *
 */
struct RegisterData {
  uint64_t address;
  uint64_t value;
};

/**
 * @class Groupdata
 * @brief
 * @ingroup rds
 *
 */
struct Groupdata {
  uint64_t base_addr;
  std::vector<RegisterData> registers;
};

/**
 * @class Contents
 * @ingroup rds
 */
struct Contents {
  std::vector<Groupdata> groups;
};

/**
 * @class EnumeratedValue
 * @ingroup rds
 */
struct EnumeratedValue {
  std::string name{};
  std::string description{};
  uint64_t value{0};
};

struct Field;
struct Register;
struct Group;
struct Device;

namespace detail {

// TODO: make to_string constexpr

constexpr uint64_t make_mask(uint64_t msb, uint64_t lsb) {
  if (msb > 63)
    msb = 63;
  if (lsb > 63)
    lsb = 63;

  uint64_t r{0};
  for (auto i = lsb; i <= msb; ++i) {
    r |= (1u << i);
  }
  return r;
}
template <typename Enum>
  requires std::is_enum_v<Enum>
constexpr std::optional<Enum> from_string(std::string_view sv) {
  return std::nullopt;
}

template <typename Enum>
  requires std::is_enum_v<Enum>
inline std::string to_string(Enum e) {
  return {};
}

template <> inline std::string to_string(Endian e) {
  switch (e) {
  case Endian::little:
    return "little";
  case Endian::big:
    return "big";
  }
  return "unkown";
}

template <>
constexpr std::optional<Endian> from_string<Endian>(std::string_view sv) {
  if (sv == "little")
    return Endian::little;
  if (sv == "big")
    return Endian::big;
  return std::nullopt;
}

template <>
constexpr std::optional<Access> from_string<Access>(std::string_view sv) {
  if (sv == "read-only" or sv == "r")
    return Access::read_only;
  if (sv == "write-only" or sv == "w")
    return Access::write_only;
  if (sv == "read-write" or sv == "rw" or sv == "wr")
    return Access::read_write;
  if (sv == "write-once" or sv == "w1" or sv == "writeOnce")
    return Access::write_once;
  if (sv == "read-write-once" or sv == "rw1" or sv == "wr1" or
      sv == "read-writeOnce")
    return Access::read_write_once;
  return std::nullopt;
}

template <> inline std::string to_string<Access>(Access a) {
  using enum Access;
  switch (a) {
  case read_only:
    return "read-only";
  case write_only:
    return "write-only";
  case read_write:
    return "read-write";
  case write_once:
    return "write-once";
  case read_write_once:
    return "read-write-once";
  }
  return {};
}

template <>
constexpr std::optional<Protection>
from_string<Protection>(std::string_view sv) {
  if (sv == "non-secure" or sv == "n")
    return Protection::non_secure;
  if (sv == "secure" or sv == "s")
    return Protection::secure;
  if (sv == "privileged" or sv == "p")
    return Protection::privileged;
  return std::nullopt;
}

template <> inline std::string to_string<Protection>(Protection p) {
  using enum Protection;
  switch (p) {
  case non_secure:
    return "non-secure";
  case secure:
    return "secure";
  case privileged:
    return "privileged";
  }
  return {};
}

template <>
constexpr std::optional<WriteType> from_string<WriteType>(std::string_view sv) {
  if (sv == "modify" or sv == "m")
    return WriteType::modify;
  if (sv == "one-to-clear" or sv == "oc" or sv == "oneToClear")
    return WriteType::one_to_clear;
  if (sv == "one-to-set" or sv == "os" or sv == "oneToSet")
    return WriteType::one_to_set;
  if (sv == "one-to-toggle" or sv == "ot" or sv == "oneToToggle")
    return WriteType::one_to_toggle;
  if (sv == "zero-to-clear" or sv == "zc" or sv == "zeroToClear")
    return WriteType::zero_to_clear;
  if (sv == "zero-to-set" or sv == "zs" or sv == "zeroToSet")
    return WriteType::zero_to_set;
  if (sv == "zero-to-toggle" or sv == "zt" or sv == "zeroToToggle")
    return WriteType::zero_to_toggle;
  if (sv == "clear" or sv == "c")
    return WriteType::clear;
  if (sv == "set" or sv == "s")
    return WriteType::set;
  return std::nullopt;
}

template <> inline std::string to_string<WriteType>(WriteType w) {
  switch (w) {
  case WriteType::modify:
    return "modify";
  case WriteType::one_to_clear:
    return "one-to-clear";
  case WriteType::one_to_set:
    return "one-to-set";
  case WriteType::one_to_toggle:
    return "one-to-toggle";
  case WriteType::zero_to_clear:
    return "zero-to-clear";
  case WriteType::zero_to_set:
    return "zero-to-set";
  case WriteType::zero_to_toggle:
    return "zero-to-toggle";
  case WriteType::clear:
    return "clear";
  case WriteType::set:
    return "set";
  }
  return {};
}

template <>
constexpr std::optional<ReadType> from_string<ReadType>(std::string_view sv) {
  if (sv == "clear" or sv == "c")
    return ReadType::clear;
  if (sv == "set" or sv == "s")
    return ReadType::set;
  if (sv == "modify" or sv == "m")
    return ReadType::modify;
  if (sv == "modify-external" or sv == "me" or sv == "modifyExternal")
    return ReadType::modify_external;
  return std::nullopt;
}

template <> inline std::string to_string<ReadType>(ReadType r) {
  switch (r) {
  case ReadType::none:
    return "none";
  case ReadType::clear:
    return "clear";
  case ReadType::set:
    return "set";
  case ReadType::modify:
    return "modify";
  case ReadType::modify_external:
    return "modify-external";
  }
  return {};
}
} // namespace detail
} // namespace rds

#endif
