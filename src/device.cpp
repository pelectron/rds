#include "rds/device.hpp"
#include "rds/detail/rds.hpp"

#include <memory>

namespace rds {

Device::Device(const Device &other)
    : name(other.name), version(other.version),
      register_width(other.register_width), num_pages(other.num_pages),
      registers_per_page(other.registers_per_page), endian(other.endian),
      extra_data(other.extra_data) {
  for (const auto &group : other)
    groups.push_back(std::make_unique<Group>(group));
  for (auto &g : groups)
    g->device = this;
}

Device::Device(Device &&other)
    : name(std::move(other.name)), version(other.version),
      register_width(other.register_width), num_pages(other.num_pages),
      registers_per_page(other.registers_per_page), endian(other.endian),
      groups(std::move(other.groups)), extra_data(std::move(other.extra_data)) {
  for (auto &g : groups)
    g->device = this;
}

Device &Device::operator=(const Device &other) {
  std::vector<std::unique_ptr<Group>> gs;
  for (const auto &group : other)
    gs.push_back(std::make_unique<Group>(group));
  for (auto &g : gs)
    g->device = this;
  name = other.name;
  version = other.version;
  register_width = other.register_width;
  num_pages = other.num_pages;
  registers_per_page = other.registers_per_page;
  endian = other.endian;
  extra_data = other.extra_data;
  groups = std::move(gs);
  return *this;
}

Device &Device::operator=(Device &&other) {
  name = std::move(other.name);
  version = other.version;
  register_width = other.register_width;
  num_pages = other.num_pages;
  registers_per_page = other.registers_per_page;
  endian = other.endian;
  extra_data = other.extra_data;
  groups = std::move(other.groups);
  for (auto &g : groups)
    g->device = this;
  return *this;
}

bool Device::has_group(std::string_view name) const {
  return std::find_if(groups.begin(), groups.end(),
                      [name](const std::unique_ptr<Group> &group) {
                        return group->name == name;
                      }) != groups.end();
}

Group *Device::group(std::string_view name) {
  auto it = std::find_if(groups.begin(), groups.end(),
                         [name](const std::unique_ptr<Group> &group) {
                           return group->name == name;
                         });
  if (it == groups.end())
    return nullptr;
  else
    return it->get();
}

const Group *Device::group(std::string_view name) const {
  auto it = std::find_if(groups.begin(), groups.end(),
                         [name](const std::unique_ptr<Group> &group) {
                           return group->name == name;
                         });
  if (it == groups.end())
    return nullptr;
  else
    return it->get();
}

Group *Device::group(uint64_t base_addr) {
  auto it = std::find_if(groups.begin(), groups.end(),
                         [base_addr](const std::unique_ptr<Group> &group) {
                           return group->base_addr == base_addr;
                         });
  if (it == groups.end())
    return nullptr;
  else
    return it->get();
}

const Group *Device::group(uint64_t base_addr) const {
  auto it = std::find_if(groups.begin(), groups.end(),
                         [base_addr](const std::unique_ptr<Group> &group) {
                           return group->base_addr == base_addr;
                         });
  if (it == groups.end())
    return nullptr;
  else
    return it->get();
}

Group &Device::operator[](std::string_view name) { return *group(name); }

const Group &Device::operator[](std::string_view name) const {
  return *group(name);
}

Group *Device::add_group(std::string_view name, uint64_t base_addr,
                         uint64_t size) {
  if (not detail::is_valid_group(
          groups, Group{detail::to_string(name), base_addr, size}))
    return nullptr;

  auto *group =
      groups
          .insert(std::find_if(groups.begin(), groups.end(),
                               [base_addr](const std::unique_ptr<Group> &g) {
                                 return base_addr < g->base_addr;
                               }),
                  std::make_unique<Group>())
          ->get();
  group->device = this;
  group->name = std::string(name.data(), name.size());
  group->base_addr = base_addr;
  group->size = size;
  return group;
}

Group *Device::add_group(std::unique_ptr<Group> &&group) {
  if (not group or not detail::is_valid_group(groups, *group))
    return nullptr;

  auto *g = groups
                .insert(std::find_if(groups.begin(), groups.end(),
                                     [base_addr = group->base_addr](
                                         const std::unique_ptr<Group> &g) {
                                       return base_addr < g->base_addr;
                                     }),
                        std::make_unique<Group>())
                ->get();
  g->device = this;
  return g;
}

void Device::remove_group(std::string_view name) {
  if (name.empty())
    return;

  auto it = std::find_if(
      groups.begin(), groups.end(),
      [name](const std::unique_ptr<Group> &g) { return name == g->name; });

  if (it != groups.end())
    groups.erase(it);
}

void Device::remove_group(const Group *group) {
  if (not group or group->device != this)
    return;
  auto it = std::find_if(
      groups.begin(), groups.end(),
      [group](const std::unique_ptr<Group> &g) { return group == g.get(); });
  if (it != groups.end())
    groups.erase(it);
}

Contents Device::content() const {
  Contents c;
  for (const auto &group : groups) {
    Groupdata gd;
    gd.base_addr = group->base_addr;
    for (const auto &reg : group->registers) {
      RegisterData r;
      r.address = reg->addr;
      r.value = reg->value;
      gd.registers.push_back(r);
    }
    c.groups.push_back(gd);
  }
  return c;
}

bool Device::has_data(std::string_view name) const {
  return extra_data.contains({name.data(), name.size()});
}

bool Device::has_data(const std::string &name) const {
  return extra_data.contains(name);
}

bool Device::has_data(const char *name) const {
  return extra_data.contains(name);
}

bool Device::add_data(std::string_view name, const Value &value) {
  auto [_, inserted] = extra_data.insert({{name.data(), name.size()}, value});
  return inserted;
}

bool Device::add_data(const std::string &name, const Value &value) {
  auto [_, inserted] = extra_data.insert({name, value});
  return inserted;
}

bool Device::add_data(const char *name, const Value &value) {
  auto [_, inserted] = extra_data.insert({name, value});
  return inserted;
}

void Device::set_data(std::string_view name, const Value &value) {
  extra_data.insert_or_assign({name.data(), name.size()}, value);
}

void Device::set_data(const std::string &name, const Value &value) {
  extra_data.insert_or_assign(name, value);
}

void Device::set_data(const char *name, const Value &value) {
  extra_data.insert_or_assign(name, value);
}

Value &Device::data(std::string_view name) {
  return extra_data.at({name.data(), name.size()});
}

Value &Device::data(const std::string &name) { return extra_data.at(name); }

Value &Device::data(const char *name) { return extra_data.at(name); }

const Value &Device::data(std::string_view name) const {
  return extra_data.at({name.data(), name.size()});
}

const Value &Device::data(const std::string &name) const {
  return extra_data.at(name);
}

const Value &Device::data(const char *name) const {
  return extra_data.at(name);
}

void Device::reset() {
  for (auto &group : groups)
    group->reset();
}

std::unique_ptr<Device> Device::clone() const {
  return std::make_unique<Device>(*this);
}

Device::iterator Device::begin() { return groups.begin(); }

Device::const_iterator Device::begin() const { return groups.begin(); }

Device::iterator Device::end() { return groups.end(); }

Device::const_iterator Device::end() const { return groups.end(); }
} // namespace rds
