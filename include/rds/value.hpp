#ifndef RDS_VALUE_HPP
#define RDS_VALUE_HPP

#include <cassert>
#include <cstdint>
#include <format>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace rds {

/**
 * A Value is a variant of std::string, uint64_t, int64_t, double, bool, vector
 * of Value, and map of std::string to Value.
 *
 * Is is used as an intermediate type for converting between an rds::Device and
 * the different file formats.
 *
 * The methods called to_XXX return an engaged optional if the type fits
 * exactly, else std::nullopt.
 *
 * The methods as_XXX return a reference to the
 * underlying value. is_XXX must return true to call as_XXX.
 *
 * The method convert_to<T> performs actual conversion. For example,
 * convert_to<std::string> on a value that contains an integer with value 1 will
 * return "1". This will return an engaged optional if the conversion succeeds,
 * else std::nullopt.
 *
 * Because Value inherits from std::variant, the usual behaviour and interaction
 * with std::visit, std::get, std::get_if is as expected.
 * @ingroup rds
 */
class Value
    : public std::variant<std::map<std::string, Value>, std::vector<Value>,
                          std::string, int64_t, uint64_t, double, bool> {
public:
  enum Type {
    map,
    list,
    string,
    integer,
    unsigned_integer,
    floating_point,
    boolean
  };

  using base = std::variant<std::map<std::string, Value>, std::vector<Value>,
                            std::string, int64_t, uint64_t, double, bool>;

  using map_type = std::map<std::string, Value>;
  using iterator = map_type::iterator;
  using const_iterator = map_type::const_iterator;
  using difference_type = map_type::difference_type;
  using value_type = map_type::value_type;
  using reference = map_type::reference;
  using const_reference = map_type::const_reference;

  using base::base;
  using base::operator=;

  constexpr Type type() const { return static_cast<Type>(index()); }

  std::optional<std::string> to_string() const {
    if (index() == Type::string)
      return std::get<Type::string>(*this);
    return std::nullopt;
  }

  std::optional<double> to_double() const {
    if (index() == Type::floating_point)
      return std::get<Type::floating_point>(*this);
    return std::nullopt;
  }

  std::optional<int64_t> to_i64() const {
    if (index() == Type::integer)
      return std::get<Type::integer>(*this);
    return std::nullopt;
  }

  std::optional<uint64_t> to_u64() const {
    if (index() == Type::unsigned_integer)
      return std::get<Type::unsigned_integer>(*this);
    return std::nullopt;
  }

  std::optional<bool> to_bool() const {
    if (index() == Type::boolean)
      return std::get<Type::boolean>(*this);
    return std::nullopt;
  }

  std::optional<std::map<std::string, Value>> to_map() const {
    if (index() == Type::map)
      return std::get<Type::map>(*this);
    return std::nullopt;
  }

  std::optional<std::vector<Value>> to_list() const {
    if (index() == Type::list)
      return std::get<Type::list>(*this);
    return std::nullopt;
  }

  std::string &as_string() {
    assert(index() == Type::string);
    return std::get<Type::string>(*this);
  }

  const std::string &as_string() const {
    assert(index() == Type::string);
    return std::get<Type::string>(*this);
  }

  double &as_double() {
    assert(index() == Type::floating_point);
    return std::get<Type::floating_point>(*this);
  }

  const double &as_double() const {
    assert(index() == Type::floating_point);
    return std::get<Type::floating_point>(*this);
  }

  int64_t &as_i64() {
    assert(index() == Type::integer);
    return std::get<Type::integer>(*this);
  }

  const int64_t &as_i64() const {
    assert(index() == Type::integer);
    return std::get<Type::integer>(*this);
  }

  uint64_t &as_u64() {
    assert(index() == Type::unsigned_integer);
    return std::get<Type::unsigned_integer>(*this);
  }

  const uint64_t &as_u64() const {
    assert(index() == Type::unsigned_integer);
    return std::get<Type::unsigned_integer>(*this);
  }

  bool &as_bool() {
    assert(index() == Type::boolean);
    return std::get<Type::boolean>(*this);
  }

  const bool &as_bool() const {
    assert(index() == Type::boolean);
    return std::get<Type::boolean>(*this);
  }

  std::map<std::string, Value> &as_map() {
    assert(index() == Type::map);
    return std::get<Type::map>(*this);
  }

  const std::map<std::string, Value> &as_map() const {
    assert(index() == Type::map);
    return std::get<Type::map>(*this);
  }

  std::vector<Value> &as_list() {
    assert(index() == Type::list);
    return std::get<Type::list>(*this);
  }

  const std::vector<Value> &as_list() const {
    assert(index() == Type::list);
    return std::get<Type::list>(*this);
  }

  template <typename T> std::optional<T> to() const {
    if (auto p = std::get_if<T>(this))
      return *p;
    return std::nullopt;
  }

  template <typename T> T &as() {
    auto p = std::get_if<T>(this);
    assert(p);
    return *p;
  }

  template <typename T> constexpr bool is() const {
    return std::get_if<T>(this) != nullptr;
  }

  template <typename T> const T &as() const {
    auto p = std::get_if<T>(this);
    assert(p);
    return *p;
  }

  bool is_map() const { return index() == Type::map; }
  bool is_list() const { return index() == Type::list; }
  bool is_string() const { return index() == Type::string; }
  bool is_i64() const { return index() == Type::integer; }
  bool is_u64() const { return index() == Type::unsigned_integer; }
  bool is_double() const { return index() == Type::floating_point; }
  bool is_bool() const { return index() == Type::boolean; }

  std::string *string_ptr() { return std::get_if<Type::string>(this); }

  const std::string *string_ptr() const {
    return std::get_if<Type::string>(this);
  }

  double *double_ptr() { return std::get_if<Type::floating_point>(this); }

  const double *double_ptr() const {
    return std::get_if<Type::floating_point>(this);
  }

  int64_t *i64_ptr() { return std::get_if<Type::integer>(this); }

  const int64_t *i64_ptr() const { return std::get_if<Type::integer>(this); }

  uint64_t *u64_ptr() { return std::get_if<Type::unsigned_integer>(this); }

  const uint64_t *u64_ptr() const {
    return std::get_if<Type::unsigned_integer>(this);
  }

  bool *bool_ptr() { return std::get_if<Type::boolean>(this); }
  const bool *bool_ptr() const { return std::get_if<Type::boolean>(this); }

  std::map<std::string, Value> *map_ptr() {
    return std::get_if<Type::map>(this);
  }

  const std::map<std::string, Value> *map_ptr() const {
    return std::get_if<Type::map>(this);
  }

  std::vector<Value> *list_ptr() { return std::get_if<Type::list>(this); }

  const std::vector<Value> *list_ptr() const {
    return std::get_if<Type::list>(this);
  }

  iterator begin() {
    if (index() == Type::map)
      return std::get<Type::map>(*this).begin();
    return {};
  }

  const_iterator begin() const {
    if (index() == Type::map)
      return std::get<Type::map>(*this).begin();
    return {};
  }

  iterator end() {
    if (index() == Type::map)
      return std::get<Type::map>(*this).end();
    return {};
  }

  const_iterator end() const {
    if (index() == Type::map)
      return std::get<Type::map>(*this).end();
    return {};
  }

  // template <typename Str> Value &at(Str &&key) {
  //   if (not is_map()) {
  //     throw std::runtime_error(std::format(
  //         "Invalid operation. Must be a map to use this function, but is {}",
  //         rds::to_string(this->type()).value()));
  //   }
  //   if (not as_map().contains(key))
  //     throw std::runtime_error("key {} not found", key);
  //   return as_map().at(std::forward<Str>(key));
  // }
  //
  // template <typename Str> const Value &at(Str &&key) const {
  //   return as_map().at(std::forward<Str>(key));
  // }
  //
  // template <typename Str> Value &operator[](Str &&key) {
  //   return as_map()[std::forward<Str>(key)];
  // }
  //
  // template <typename Str> const Value &operator[](Str &&key) const {
  //   return as_map()[std::forward<Str>(key)];
  // }

  template <typename T> T &get() { return std::get<T>(*this); }

  template <typename T> const T &get() const { return std::get<T>(*this); }

  template <std::size_t I> auto &get() { return std::get<I>(*this); }

  template <std::size_t I> const auto &get() const {
    return std::get<I>(*this);
  }

  template <typename T> std::optional<T> convert_to() const;
};

using ValueMap = std::map<std::string, Value>;

using ValueList = std::vector<Value>;

namespace detail {
std::string_view to_string(Value::Type t);
template <typename T> constexpr std::string_view type_name() {
  if constexpr (std::is_same_v<T, ValueMap>) {
    return "map";
  } else if constexpr (std::is_same_v<T, ValueList>) {
    return "list";
  } else if constexpr (std::is_same_v<T, std::string>) {
    return "string";
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return "integer";
  } else if constexpr (std::is_same_v<T, uint64_t>) {
    return "unsigned integer";
  } else if constexpr (std::is_same_v<T, double>) {
    return "floating point";
  } else if constexpr (std::is_same_v<T, bool>) {
    return "boolean";
  }
}

std::string to_string_impl(const Value &v, size_t indent, size_t num_indents);
std::string to_string(const Value &v, size_t indent = 2);
} // namespace detail
} // namespace rds

namespace std {

/**
 * formatter specialization for rds::Value
 */
template <> struct formatter<rds::Value> : formatter<std::string_view> {

  auto format(const rds::Value &v, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{}", ::rds::detail::to_string(v, 2));
  }
};

} // namespace std

template <typename T> std::optional<T> rds::Value::convert_to() const {
  constexpr bool is_bool = std::is_same_v<T, bool>;
  switch (this->type()) {
  case map:
    if constexpr (std::is_same_v<T, ValueMap>) {
      return as_map();
    } else if constexpr (std::is_same_v<T, std::string>) {
      return rds::detail::to_string(*this);
    } else {
      return std::nullopt;
    }
    break;
  case list:
    if constexpr (std::is_same_v<T, ValueList>) {
      return as_list();
    } else if constexpr (std::is_same_v<T, std::string>) {
      return rds::detail::to_string(*this);
    } else {
      return std::nullopt;
    }
    break;
  case string:
    if constexpr (std::is_same_v<T, std::string>) {
      return as_string();
    } else {
      return std::nullopt;
    }
    break;
  case integer: {
    const int64_t &i = std::get<integer>(*this);
    if constexpr (is_bool) {
      return i != 0;
    } else if constexpr (std::is_integral_v<T>) {
      if (i > std::numeric_limits<T>::max() or
          i < std::numeric_limits<T>::min())
        return std::nullopt;
      else
        return static_cast<T>(i);
    } else if constexpr (std::is_floating_point_v<T>) {
      constexpr auto n_digits = std::numeric_limits<T>::digits;
      if (i >= (int64_t{1} << n_digits) or i < -(int64_t{1} << n_digits))
        return std::nullopt;
      else
        return static_cast<T>(i);
    } else if constexpr (std::is_same_v<T, std::string>) {
      return rds::detail::to_string(*this);
    } else {
      return std::nullopt;
    }
  } break;
  case unsigned_integer: {
    const uint64_t &i = as_u64();
    if constexpr (is_bool) {
      return i != 0;
    } else if constexpr (std::is_integral_v<T>) {
      if (i > std::numeric_limits<T>::max())
        return std::nullopt;
      else
        return static_cast<T>(i);
    } else if constexpr (std::is_floating_point_v<T>) {
      if (i >= (uint64_t{1} << std::numeric_limits<T>::digits))
        return std::nullopt;
      return static_cast<T>(i);
    } else if constexpr (std::is_same_v<T, std::string>) {
      return rds::detail::to_string(*this);
    } else if constexpr (std::is_constructible_v<T, uint64_t>) {
      return T{i};
    } else {
      return std::nullopt;
    }
  } break;
  case floating_point: {
    auto &f = as_double();
    if constexpr (not is_bool and
                  (std::is_integral_v<T> or std::is_floating_point_v<T>)) {
      if (f > std::numeric_limits<T>::max() or
          f < std::numeric_limits<T>::min())
        return std::nullopt;
      else
        return static_cast<T>(f);
    } else if constexpr (std::is_same_v<T, std::string>) {
      return rds::detail::to_string(*this);
    } else if constexpr (not is_bool and std::is_constructible_v<T, double>) {
      return static_cast<T>(f);
    } else {
      return std::nullopt;
    }
  } break;
  case boolean: {
    auto &b = as_bool();
    if constexpr (std::is_same_v<T, std::string>) {
      return b ? "true" : "false";
    } else if constexpr (std::is_constructible_v<T, bool>) {
      return static_cast<T>(b);
    } else {
      return std::nullopt;
    }
  } break;
  }
  return std::nullopt;
}
#endif
