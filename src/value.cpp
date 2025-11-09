#include "rds/value.hpp"
#include <ryu/ryu.h>
namespace rds {

namespace detail {
std::string_view to_string(Value::Type t) {
  using enum Value::Type;
  switch (t) {
  case map:
    return "map";
  case list:
    return "list";
  case string:
    return "string";
  case integer:
    return "integer";
  case unsigned_integer:
    return "unsigned integer";
  case floating_point:
    return "floating point";
  case boolean:
    return "boolean";
  }
}

std::string to_string_impl(const Value &v, size_t indent, size_t num_indents) {
  std::string res;
  const std::string ind = std::string(indent * (num_indents + 1), ' ');
  const std::string close_ind = std::string(indent * num_indents, ' ');
  switch (v.type()) {
  case rds::Value::map: {
    res = "{";
    auto &map = v.as_map();
    size_t n = 0;
    const auto n_children = map.size();
    for (const auto &[name, value] : map) {
      ++n;
      res += std::format("\n{}{} : {}", ind, name,
                         to_string_impl(value, indent, num_indents + 1));
      if (n < n_children) {
        res += ",";
      }
    }
    res += '\n' + close_ind + "}";
    return res;
  }
  case rds::Value::list: {
    res = "[";
    size_t n = 0;
    auto &list = v.as_list();
    const auto n_children = list.size();
    for (const auto &value : list) {
      ++n;
      res += std::format("\n{}{}", ind,
                         to_string_impl(value, indent, num_indents + 1));
      if (n < n_children) {
        res += ",";
      }
    }
    res += '\n' + close_ind + "]";
    return res;
  }
  case rds::Value::integer:
    return std::format("{}", v.as_i64());
  case rds::Value::unsigned_integer:
    return std::format("{}", v.as_u64());
  case rds::Value::string:
    return std::format("'{}'", v.as_string());
  case rds::Value::floating_point: {

    std::string res(std::size_t{25}, 0);
    res.resize(
        static_cast<std::size_t>(d2s_buffered_n(v.as_double(), res.data())));
    return res;
  }
  case rds::Value::boolean:
    if (v.as_bool())
      return std::format("{}", "true");
    else
      return std::format("{}", "false");
  }
  return res;
}

std::string to_string(const Value &v, size_t indent) {
  return to_string_impl(v, indent, 0);
}
} // namespace detail
} // namespace rds
