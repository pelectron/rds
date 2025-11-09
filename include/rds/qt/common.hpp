/**
 * @file rds/qt/common.hpp
 * @brief This file contains enums, forward declarations, and structs used by
 * differernt parts of qrds.
 */

#ifndef RDS_QT_COMMON_HPP
#define RDS_QT_COMMON_HPP

#include "qtmetamacros.h"
#include "qvariant.h"
#include "rds/common.hpp"
#include <QString>
#include <QtCore/QtGlobal>
#include <charconv>

namespace rds {
Q_NAMESPACE

class QDevice;
class QGroup;
class QRegister;
class QField;
class QModel;
class Widget;
class QColumn;

/**
 *
 * @addtogroup qrds
 */
enum class Fmt { integer, hex, binary };

Q_ENUM_NS(Fmt);

/**
 *
 * @addtogroup qrds
 */
enum class Options : uint32_t {
  None = 0,
  LoadButton = 1 << 0,  //< provide a load from file button
  SaveButton = 1 << 1,  //< provide a save to file button
  WriteButton = 1 << 2, //< provide a write button
  ReadButton = 1 << 3,  //< provide a read button
  ProgessBar = 1 << 4,  //< provide a QStatusBar
  StatusBar = 1 << 5,   //<
  FilterSort = 1 << 6,
  All = LoadButton | SaveButton | WriteButton | ReadButton | ProgessBar |
        StatusBar | FilterSort
};

Q_ENUM_NS(Options);

/**
 *
 * @addtogroup qrds
 */
enum class Error : uint32_t {
  None = 0,
  CannotLoadFile,
  CannotSaveFile,
  IoUnavailable,
  NotConnected,
  InvalidAddress,
  ConnectionAborted,
  ProtocolError,
  InvalidOperation,
};

Q_ENUM_NS(Error);

/**
 * combines two sets of options together
 * @param o1 the first option set
 * @param o2 the second opion set
 * @return the combination of the first and second options
 * @addtogroup qrds
 */
constexpr Options operator|(Options o1, Options o2) noexcept {
  return static_cast<Options>(static_cast<uint32_t>(o1) |
                              static_cast<uint32_t>(o2));
}

/**
 * forms the insersection of two option sets
 * @param o1 the first option set
 * @param o2 the second opion set
 * @return common subset of o1 and o2, or Options::None.
 * @addtogroup qrds
 */
constexpr Options operator&(Options o1, Options o2) noexcept {
  return static_cast<Options>(static_cast<uint32_t>(o1) &
                              static_cast<uint32_t>(o2));
}

/**
 * checks if o contains every option in options.
 * @addtogroup qrds
 */
constexpr bool contains(Options o, Options options) {
  return (options & o) == options;
}

/**
 * checks if o contains any option in options
 * @addtogroup qrds
 */
constexpr bool contains_any_of(Options o, Options options) {
  return (options & o) != Options::None;
}

/// returns o with options removed
/// @addtogroup qrds
constexpr Options without(Options o, Options options) {
  return static_cast<Options>(static_cast<uint32_t>(o) &
                              ~static_cast<uint32_t>(options));
}

template <typename T> QString to_string(T val, Fmt format) {
  int base = 10;
  std::string res(std::size_t{64}, char{0});
  QString prefix{};
  switch (format) {
  case Fmt::integer:
    break;
  case Fmt::hex:
    base = 16;
    prefix = "0x";
    break;
  case Fmt::binary:
    base = 2;
    prefix = "0b";
    break;
  default:
    return {};
  }

  auto [ptr, ec] =
      std::to_chars(res.data(), res.data() + res.size(), val, base);
  if (ec != std::errc{})
    return {};

  return prefix + QString(res.data()).toUpper();
}

template <typename T> std::optional<T> from_string(const QString &val) {
  if (val.isEmpty())
    return 0;
  const auto upper = val.toUpper().toStdString();
  T value = 0;
  int base = 10;
  qulonglong offset = 0;
  if (upper.starts_with("0X")) {
    if (upper.size() == 2)
      return 0;
    base = 16;
    offset = 2;
  } else if (upper.starts_with("0B")) {
    if (upper.size() == 2)
      return 0;
    base = 2;
    offset = 2;
  }
  auto [ptr, ec] = std::from_chars(upper.data() + offset,
                                   upper.data() + upper.size(), value, base);
  if (ec != std::errc{} or ptr != upper.data() + upper.size())
    return std::nullopt;

  return value;
}

QVariant to_qvariant(const SimpleValue &v);

std::optional<SimpleValue> to_simple_value(const QVariant &v);
} // namespace rds

#endif
