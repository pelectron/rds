#ifndef RDS_DETAIL_SVD_HPP
#define RDS_DETAIL_SVD_HPP

#include "rds/common.hpp"
#include "rds/util.hpp"

#include <pugixml.hpp>

namespace rds::svd {

inline ValueMap to_list(const pugi::xml_node &node) {}

inline ValueMap svd_to_map(const pugi::xml_node &node) {}

inline ValueMap deserialize(const std::string &s) {
  pugi::xml_document doc;
  doc.load_string(s.data());
  return svd_to_map(doc);
}

inline std::string serialize(const ValueMap &map) {}
} // namespace rds::svd

#endif
