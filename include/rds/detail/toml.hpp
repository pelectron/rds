#ifndef RDS_DETAIL_TOML_HPP
#define RDS_DETAIL_TOML_HPP

#include "rds/detail/rds.hpp"
#include "rds/value.hpp"

#include <cstdint>
#include <sstream>
#include <toml++/toml.hpp>

namespace rds::toml {

/// converts a toml table into a ValueMap
inline ValueMap from_toml(const ::toml::table &table);

/// converts a toml array into a ValueList
inline ValueList from_toml(const ::toml::array &array);

/// converts a ValueMap into a toml table
inline ::toml::table to_toml(const ValueMap &map);

/// converts a ValueList into a toml array
inline ::toml::array to_toml(const ValueList &list);

inline ValueMap from_toml(const ::toml::table &table) {
  ValueMap map;
  table.for_each(detail::overload{
      [&map](const ::toml::key &name, const ::toml::table &t) {
        map.insert({name.data(), from_toml(t)});
      },
      [&map](const ::toml::key &name, const ::toml::array &a) {
        map.insert({name.data(), from_toml(a)});
      },
      [&map](const ::toml::key &name, const ::toml::value<std::string> &s) {
        map.insert({name.data(), s.get()});
      },
      [&map](const ::toml::key &name, const ::toml::value<int64_t> &i) {
        map.insert({name.data(), i.get()});
      },
      [&map](const ::toml::key &name, const ::toml::value<double> &d) {
        map.insert({name.data(), d.get()});
      },
      [&map](const ::toml::key &name, const ::toml::value<bool> &b) {
        map.insert({name.data(), b.get()});
      }});
  return map;
}

inline ValueList from_toml(const ::toml::array &array) {
  ValueList list;
  for (const auto &value : array) {
    if (auto t = value.as_table()) {
      list.push_back(from_toml(*t));
    } else if (auto a = value.as_array()) {
      list.push_back(from_toml(*a));
    } else if (auto s = value.as_string()) {
      list.push_back(s->get());
    } else if (auto i = value.as_integer()) {
      list.push_back(i->get());
    } else if (auto d = value.as_floating_point()) {
      list.push_back(d->get());
    } else if (auto b = value.as_boolean()) {
      list.push_back(b->get());
    }
  }
  return list;
}

inline ::toml::table to_toml(const ValueMap &map) {
  ::toml::table t;
  for (const auto &[name, value] : map) {
    std::visit(
        rds::detail::overload{
            [&t, &name](const ValueMap &map) { t.insert(name, to_toml(map)); },
            [&t, &name](const ValueList &list) {
              t.insert(name, to_toml(list));
            },
            [&t, &name]<typename T>(const T &v) {
              if constexpr (std::is_same_v<T, uint64_t>)
                t.insert(name, static_cast<int64_t>(v));
              else
                t.insert(name, v);
            }},
        value);
  }
  return t;
}

inline ::toml::array to_toml(const ValueList &list) {
  ::toml::array array;
  for (const auto &value : list) {
    std::visit(
        rds::detail::overload{
            [&array](const ValueMap &map) { array.push_back(to_toml(map)); },
            [&array](const ValueList &list) { array.push_back(to_toml(list)); },
            [&array]<typename T>(const T &v) {
              if constexpr (std::is_same_v<T, uint64_t>)
                array.push_back(static_cast<int64_t>(v));
              else
                array.push_back(v);
            }},
        value);
  }
  return array;
}

inline ValueMap deserialize(const std::string &s) {
  return from_toml(::toml::parse(s));
}

inline std::string serialize(const ValueMap &map) {
  std::stringstream s;
  s << to_toml(map);
  return s.str();
}

} // namespace rds::toml

#endif
