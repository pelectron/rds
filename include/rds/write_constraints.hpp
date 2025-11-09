/**
 * @file rds/write_constraints.hpp
 *
 * @defgroup write-constraints Write Constraints
 *
 * Write contraints specify additional contraints when writing a register or
 * field. This is a set of three mutually exlusive options:
 *  - write as read: only the last value that was read can be written
 *  - use enumerated values: only the enumerated values specified in the
 * feild/register can be written
 *  - range: only values inside the specified range can be written.
 */

#ifndef RDS_WRITE_CONSTRAINTS_HPP
#define RDS_WRITE_CONSTRAINTS_HPP

#include <cstdint>
#include <limits>
#include <optional>
#include <variant>

namespace rds {
/**
 * This is used to set the write constraint to "range"
 * @ingroup rds
 * @ingroup write-contraints
 * @ingroup access-protection
 */
struct Range {
  uint64_t minimum{0};
  uint64_t maximum{std::numeric_limits<uint64_t>::max()};
};

/**
 * This is used to set the write constraint to "write as read"
 * @ingroup rds
 * @ingroup write-contraints
 * @ingroup access-protection
 */
constexpr inline struct write_as_read_t {
} write_as_read;

/**
 * This is used to set the write constraint to "use enumerated values"
 * @ingroup rds
 * @ingroup write-contraints
 * @ingroup access-protection
 */
constexpr inline struct use_enumerated_values_t {
} use_enumerated_values;

/**
 * A write constraint can beconstructed from an rds::Range,
 * rds::use_enumerated_values(_t), or rds::write_as_read(_t).
 *
 * @ingroup rds
 * @ingroup write-contraints
 * @ingroup access-protection
 */
class WriteContraints {
public:
  /// constructs a range write constraint
  constexpr WriteContraints() = default;
  constexpr WriteContraints(write_as_read_t) : v_(write_as_read_t{}) {}

  constexpr WriteContraints(use_enumerated_values_t)
      : v_(use_enumerated_values_t{}) {}

  constexpr WriteContraints(const Range &r) : v_(r) {}

  constexpr WriteContraints(const WriteContraints &) = default;
  constexpr WriteContraints(WriteContraints &&) = default;
  constexpr WriteContraints &operator=(const WriteContraints &) = default;
  constexpr WriteContraints &operator=(WriteContraints &&) = default;

  constexpr WriteContraints &operator=(write_as_read_t) {
    v_ = write_as_read_t{};
    return *this;
  }

  constexpr WriteContraints &operator=(use_enumerated_values_t) {
    v_ = use_enumerated_values_t{};
    return *this;
  }

  constexpr WriteContraints &operator=(const Range &r) {
    v_ = r;
    return *this;
  }

  /// returns true if this constraint is "write as read"
  constexpr bool write_as_read() const noexcept { return v_.index() == 2; }

  /// returns true if this constraint is "use enumerated values"
  constexpr bool use_enumerated_values() const noexcept {
    return v_.index() == 1;
  }

  /// returns a Range if this constraint is "range", else std::nullopt.
  constexpr std::optional<Range> range() const noexcept {
    if (v_.index() == 0)
      return std::get<0>(v_);
    return std::nullopt;
  }

  constexpr bool operator==(const WriteContraints &) const = default;
  constexpr bool operator!=(const WriteContraints &) const = default;

private:
  std::variant<Range, use_enumerated_values_t, write_as_read_t> v_;
};

} // namespace rds

#endif
