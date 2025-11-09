#include "rds.hpp"
#include "rds/common.hpp"
#include "rds/detail/rds.hpp"
#include <memory>

namespace rds {

void device_to_file(const Device &device, const std::string &file_path) {
  device_to_file(device, std::string_view{file_path});
}

void device_to_file(const Device &device, std::string_view file_path) {
  using namespace ::rds::detail;
  seralize_to_file(file_path, to_map(device));
}

std::unique_ptr<Device> device_from_file(const std::string &path) {
  return device_from_file(std::string_view{path});
}

std::unique_ptr<Device> device_from_file(std::string_view path) {
  using namespace ::rds::detail;
  return validate(sort(to_device(derive(deseralize_from_file(path)))));
}

std::string to_string(const Device &device) {
  return detail::to_string(Value{detail::to_map(device)});
}
} // namespace rds
