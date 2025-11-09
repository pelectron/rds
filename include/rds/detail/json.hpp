#ifndef RDS_DETAIL_JSON_HPP
#define RDS_DETAIL_JSON_HPP

#include "rds/detail/rds.hpp"
#include "rds/value.hpp"

#include <cstdint>
#include <nlohmann/json.hpp>
#include <variant>
#include <vector>

namespace rds::json {

// converts a json object to a ValueMap
inline ValueMap to_map(const nlohmann::json &json);

// converts a ValueMap to a json object
inline nlohmann::json to_json(const ValueMap &map);

// converts a json array to a ValueList
inline std::vector<Value> to_list(const nlohmann::json::array_t &array);

// converts a ValueList to a json array
inline nlohmann::json::array_t to_json(const ValueList &list);

inline std::vector<Value> to_list(const nlohmann::json::array_t &array) {
  std::vector<Value> vals;
  for (const nlohmann::json &elem : array) {
    if (elem.is_object()) {
      vals.push_back(to_map(elem));
    } else if (elem.is_string()) {
      vals.push_back(elem.get<std::string>());
    } else if (elem.is_number_float()) {
      vals.push_back(elem.get<double>());
    } else if (elem.is_number_unsigned()) {
      vals.push_back(elem.get<uint64_t>());
    } else if (elem.is_number_integer()) {
      vals.push_back(elem.get<int64_t>());
    } else if (elem.is_boolean()) {
      vals.push_back(elem.get<bool>());
    } else if (elem.is_array()) {
      vals.push_back(to_list(elem));
    }
  }
  return vals;
}

inline ValueMap to_map(const nlohmann::json &json) {
  ValueMap map;
  for (const auto &[name, value] : json.items()) {
    if (value.is_object()) {
      map.insert({name, rds::json::to_map(value)});
    } else if (value.is_string()) {
      map.insert({name, value.get<std::string>()});
    } else if (value.is_number_float()) {
      map.insert({name, value.get<double>()});
    } else if (value.is_number_unsigned()) {
      map.insert({name, value.get<uint64_t>()});
    } else if (value.is_number_integer()) {
      map.insert({name, value.get<int64_t>()});
    } else if (value.is_boolean()) {
      map.insert({name, value.get<bool>()});
    } else if (value.is_array()) {
      map.insert({name, to_list(value)});
    }
  }
  return map;
}

inline nlohmann::json::array_t to_json(const ValueList &list) {
  nlohmann::json::array_t array;
  for (const auto &value : list) {
    std::visit(
        rds::detail::overload{
            [&array](const ValueMap &map) { array.push_back(to_json(map)); },
            [&array](const ValueList &list) { array.push_back(to_json(list)); },
            [&array](const auto &v) { array.push_back(v); }},
        value);
  }
  return array;
}

inline nlohmann::json to_json(const ValueMap &map) {
  nlohmann::json json;
  for (const auto &[name, value] : map) {
    std::visit(
        rds::detail::overload{
            [&json, &name](const ValueMap &map) { json[name] = to_json(map); },
            [&json, &name](const ValueList &list) {
              json[name] = to_json(list);
            },
            [&json, &name](const auto &v) { json[name] = v; }},
        value);
  }
  return json;
}

inline ValueMap deserialize(const std::string &s) {
  return to_map(nlohmann::json::parse(s));
}

inline std::string serialize(const ValueMap &map) {
  return to_json(map).dump(2);
}

} // namespace rds::json

#endif
