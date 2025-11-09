#include "rds/detail/json.hpp"
#include "rds/detail/rds.hpp"
#include "rds/detail/toml.hpp"
#include "rds/device.hpp"

namespace rds::detail {

std::map<std::string, Serializer> &serializers() {
  static std::map<std::string, Serializer> sers{
      {".json", &rds::json::serialize},
      {".toml", &rds::toml::serialize},
      {".rds", &rds::detail::rds_serialize},
      //{".svd", &svd_serialize}
  };
  return sers;
}

std::map<std::string, Deserializer> &deserializers() {
  static std::map<std::string, Deserializer> desers{
      {".json", &rds::json::deserialize},
      {".toml", &rds::toml::deserialize},
      {".rds", &rds::detail::rds_deserialize},
      // {".svd", &svd_deserialize}
  };
  return desers;
}

bool register_deserializer(const std::string &file_ext, Deserializer deser) {
  if (not deser)
    throw std::runtime_error("cannot register empty deserializer");
  return deserializers().insert({file_ext, std::move(deser)}).second;
}

bool register_serializer(const std::string &file_ext, Serializer ser) {
  if (not ser)
    throw std::runtime_error("cannot register empty serializer");
  return serializers().insert({file_ext, std::move(ser)}).second;
}

bool register_file_type(const std::string &file_ext, Serializer ser,
                        Deserializer deser) {
  if (not register_serializer(file_ext, std::move(ser)))
    return false;
  try {
    if (not register_deserializer(file_ext, std::move(deser))) {
      serializers().erase(file_ext);
      return false;
    }
  } catch (...) {
    serializers().erase(file_ext);
    return false;
  }
  return true;
}

ValueMap deseralize_from_file(std::string_view file_path) {
  std::filesystem::path path(file_path);
  const auto ext = path.extension().string();

  auto &deserializers_ = deserializers();
  if (not deserializers_.contains(ext))
    throw std::runtime_error(
        std::format("No deserializer for the extension {} registered", ext));

  std::ifstream infile{path};
  std::string file_contents{std::istreambuf_iterator<char>(infile),
                            std::istreambuf_iterator<char>()};

  return std::invoke(deserializers_.at(ext), file_contents);
}

void seralize_to_file(std::string_view file_path, const ValueMap &map) {
  std::filesystem::path path(file_path);
  const auto ext = path.extension().string();

  auto &serializers_ = serializers();
  if (not serializers_.contains(ext))
    throw std::runtime_error(
        std::format("No serializer for the extension {} registered", ext));

  const auto contents = std::invoke(serializers_.at(ext), map);
  std::ofstream outfile{path};
  outfile.write(contents.data(), contents.size());
  outfile.close();
}

ValueList to_list(const std::vector<EnumeratedValue> &vals) {
  // TODO: implement to_list(const std::vector<EnumeratedValue>&vals)
  return {};
}

Value to_value(const WriteContraints &w) {
  if (w.use_enumerated_values()) {
    return "use_enumerated_values";
  } else if (w.write_as_read()) {
    return "write_as_read";
  } else {
    auto r = w.range().value();
    return ValueMap{{"maximum", r.maximum}, {"minimum", r.minimum}};
  }
}

ValueMap to_map(const Field &f) {
  ValueMap field;
  if (not f.display_name.empty())
    field["name"] = f.display_name;
  if (not f.description.empty())
    field["description"] = f.description;
  if (not f.backup.empty())
    field["backup"] = f.backup;
  if (not f.unit.empty())
    field["unit"] = f.unit;
  field["zero_code_value"] = f.zero_code_value;
  field["step"] = f.step;
  field["initial"] = f.initial;
  field["value"] = f.value;
  if (f.msb == f.lsb)
    field["position"] = f.msb;
  else {
    field["msb"] = f.msb;
    field["lsb"] = f.lsb;
  }
  field["access"] = to_string(f.access);
  field["protection"] = to_string(f.protection);
  field["write_type"] = to_string(f.write_type);
  field["read_type"] = to_string(f.read_type);
  field["constraints"] = to_value(f.contraints);
  field["signed"] = f.is_signed;
  if (not f.values.empty())
    field["values"] = to_list(f.values);
  field.insert(f.extra_data.begin(), f.extra_data.end());
  return field;
}

ValueMap to_map(const Register &r, const ValueMap &group) {
  ValueMap reg;
  if (not r.display_name.empty())
    reg["name"] = r.display_name;
  if (not r.description.empty())
    reg["description"] = r.description;
  if (not r.backup.empty())
    reg["backup"] = r.backup;
  if (not r.unit.empty())
    reg["unit"] = r.unit;
  reg["zero_code_value"] = r.zero_code_value;
  reg["step"] = r.step;
  reg["initial"] = r.initial;
  reg["value"] = r.value;
  reg["addr"] = r.addr;
  reg["size"] = r.size;
  reg["zeros_mask"] = r.zeros_mask;
  reg["ones_mask"] = r.ones_mask;
  reg["x_mask"] = r.x_mask;
  reg["access"] = to_string(r.access);
  reg["protection"] = to_string(r.protection);
  reg["write_type"] = to_string(r.write_type);
  reg["read_type"] = to_string(r.read_type);
  reg["constraints"] = to_value(r.contraints);
  reg["signed"] = r.is_signed;
  if (not r.values.empty())
    reg["values"] = to_list(r.values);
  reg.insert(r.extra_data.begin(), r.extra_data.end());
  for (const auto &f : r)
    reg[f.name] = to_map(f);
  return reg;
}

ValueMap to_map(const Group &g) {
  ValueMap group;
  if (not g.display_name.empty())
    group["name"] = g.display_name;
  group["description"] = g.description;
  group["base_addr"] = g.base_addr;
  group["size"] = g.size;
  group["defaults"] = g.defaults;
  group.insert(g.extra_data.begin(), g.extra_data.end());
  for (const auto &r : g) {
    group[r.name] = to_map(r, group);
  }
  return group;
}

ValueMap to_map(const Device &d) {
  ValueMap dev;
  dev["name"] = d.name;
  dev["version"] = d.version;
  dev["license"] = d.license;
  dev["description"] = d.description;
  dev["register_width"] = d.register_width;
  dev["word_width"] = d.word_width;
  dev["num_pages"] = d.num_pages;
  dev["registers_per_page"] = d.registers_per_page;
  dev["endian"] = to_string(d.endian);

  for (const auto &g : d) {
    ValueMap group;
    if (not g.display_name.empty())
      group["name"] = g.display_name;
    group["description"] = g.description;
    group["base_addr"] = g.base_addr;
    group["size"] = g.size;
    group["defaults"] = g.defaults;
    group.insert(g.extra_data.begin(), g.extra_data.end());

    for (const auto &r : g) {
      ValueMap reg;
      if (not r.display_name.empty())
        reg["name"] = r.display_name;
      if (not r.description.empty())
        reg["description"] = r.description;
      if (not r.backup.empty())
        reg["backup"] = r.backup;
      if (not r.unit.empty())
        reg["unit"] = r.unit;
      reg["zero_code_value"] = r.zero_code_value;
      reg["step"] = r.step;
      reg["initial"] = r.initial;
      reg["value"] = r.value;
      reg["addr"] = r.addr;
      reg["size"] = r.size;
      reg["zeros_mask"] = r.zeros_mask;
      reg["ones_mask"] = r.ones_mask;
      reg["x_mask"] = r.x_mask;
      reg["access"] = to_string(r.access);
      reg["protection"] = to_string(r.protection);
      reg["write_type"] = to_string(r.write_type);
      reg["read_type"] = to_string(r.read_type);
      reg["constraints"] = to_value(r.contraints);
      reg["signed"] = r.is_signed;
      if (not r.values.empty())
        reg["values"] = to_list(r.values);
      reg.insert(r.extra_data.begin(), r.extra_data.end());

      for (const auto &f : r) {
        ValueMap field;
        if (not f.display_name.empty())
          field["name"] = f.display_name;
        if (not f.description.empty())
          field["description"] = f.description;
        if (not f.backup.empty())
          field["backup"] = f.backup;
        if (not f.unit.empty())
          field["unit"] = f.unit;
        field["zero_code_value"] = f.zero_code_value;
        field["step"] = f.step;
        field["initial"] = f.initial;
        field["value"] = f.value;
        if (f.msb == f.lsb)
          field["position"] = f.msb;
        else {
          field["msb"] = f.msb;
          field["lsb"] = f.lsb;
        }
        field["access"] = to_string(f.access);
        field["protection"] = to_string(f.protection);
        field["write_type"] = to_string(f.write_type);
        field["read_type"] = to_string(f.read_type);
        field["constraints"] = to_value(f.contraints);
        field["signed"] = f.is_signed;
        if (not f.values.empty())
          field["values"] = to_list(f.values);
        field.insert(f.extra_data.begin(), f.extra_data.end());
        reg[f.name] = field;
      }
      group[r.name] = reg;
    }
    dev[g.name] = group;
  }
  dev.insert(d.extra_data.begin(), d.extra_data.end());
  return dev;
}

template <typename T>
void extract_property(const std::string &name, T &property,
                      const ValueMap &device_table) {
  if (not device_table.contains(name))
    throw std::runtime_error(std::format("no property '{}' found", name));
  const auto &v = device_table.at(name);
  auto opt = v.convert_to<T>();
  if (not opt)
    throw std::runtime_error(
        std::format("Type mismatch for property '{}': expected {}, got {}",
                    name, type_name<T>(), to_string(v.type())));

  property = std::move(opt).value();
}

template <typename T>
std::optional<T> get_property(const ValueMap &map, const std::string &name) {
  if (map.contains(name)) {
    return map.at(name).convert_to<T>();
  }
  return std::nullopt;
}

template <typename T>
std::optional<T>
get_inherited_property(const Device &device, const Group &group,
                       const std::string &name, const std::string &reg_name,
                       const ValueMap &reg, bool ignore_errors) {
  if (reg.contains(name)) {
    return reg.at(name).convert_to<T>();
  } else if (group.defaults.contains(name)) {
    return group.defaults.at(name).convert_to<T>();
  } else if (group.has_data(name)) {
    return group.extra_data.at(name).convert_to<T>();
  } else if (device.has_data(name)) {
    return device.extra_data.at(name).convert_to<T>();
  }
  return std::nullopt;
}

template <typename T>
  requires std::is_enum_v<T>
std::optional<T>
get_inherited_property(const Device &device, const Group &group,
                       const std::string &name, const std::string &reg_name,
                       const ValueMap &reg, bool ignore_errors) {
  std::optional<std::string> str;
  try {
    str = get_inherited_property<std::string>(device, group, name, reg_name,
                                              reg, false);
  } catch (...) {
    if (ignore_errors)
      return std::nullopt;
    else
      throw;
  }

  if (not str) {
    if (ignore_errors)
      return std::nullopt;
    else
      throw std::runtime_error(
          std::format("invalid value for property '{}' in register "
                      "'{}.{}': got {}",
                      name, group.name, reg_name, *str));
  }
  // str either contains a string, or get_inherited_property throws
  auto ret = from_string<T>(str.value());
  if (not ret) {
    if (ignore_errors)
      return std::nullopt;
    else
      throw std::runtime_error(
          std::format("invalid value for property '{}' in register "
                      "'{}.{}': got {}",
                      name, group.name, reg_name, *str));
  }
  return ret;
}

template <typename T>
std::optional<T>
get_inherited_property(const Device &device, const Group &group,
                       const std::string &name, const std::string &reg_name,
                       const ValueMap &reg, const std::string &field_name,
                       const ValueMap &field, bool ignore_errors) {
  if (field.contains(name)) {
    return field.at(name).convert_to<T>();
  } else if (reg.contains(name)) {
    return reg.at(name).convert_to<T>();
  } else if (group.defaults.contains(name)) {
    return group.defaults.at(name).convert_to<T>();
  } else if (group.has_data(name)) {
    return group.extra_data.at(name).convert_to<T>();
  } else if (device.has_data(name)) {
    return device.extra_data.at(name).convert_to<T>();
  }
  return std::nullopt;
}

template <typename T>
  requires std::is_enum_v<T>
std::optional<T>
get_inherited_property(const Device &device, const Group &group,
                       const std::string &name, const std::string &reg_name,
                       const ValueMap &reg, const std::string &field_name,
                       const ValueMap &field, bool ignore_errors) {
  std::optional<std::string> str;
  try {
    str = get_inherited_property<std::string>(device, group, name, reg_name,
                                              reg, field_name, field, false);
  } catch (...) {
    if (ignore_errors)
      return std::nullopt;
    else
      throw;
  }
  if (not str) {
    if (ignore_errors)
      return std::nullopt;
    else
      throw std::runtime_error(
          std::format("invalid value for property '{}' in field "
                      "'{}.{}.{}': got {}",
                      name, group.name, field_name, reg_name, *str));
  }
  // str either contains a string, or get_inherited_property throws
  auto ret = from_string<T>(str.value());
  if (not ret and not ignore_errors)
    throw std::runtime_error(
        std::format("invalid value for property '{}' in field "
                    "'{}.{}.{}': got {}",
                    name, group.name, field_name, reg_name, *str));
  return ret;
}

std::optional<WriteContraints> to_write_constraints(const Value &v) {
  switch (v.type()) {
  case Value::map: {
    const auto &map = v.as_map();
    if (not map.contains("maximum") or not map.contains("minimum")) {
      return std::nullopt;
    }
    auto min = map.at("minimum").convert_to<uint64_t>();
    auto max = map.at("maximum").convert_to<uint64_t>();

    if (not min or not max)
      return std::nullopt;
    return Range{.minimum = min.value(), .maximum = max.value()};
  }
  case Value::string:
    if (v.as_string() == "use_enumerated_values") {
      return use_enumerated_values;
    } else if (v.as_string() == "write_as_read") {
      return write_as_read;
    } else {
      return std::nullopt;
    }
  default:
    return std::nullopt;
  }
}

WriteContraints get_write_contraints(const Device &device, const Group &group,
                                     const std::string &reg_name,
                                     const ValueMap &reg) {
  const Value *v = nullptr;
  if (auto it = reg.find("constraints"); it != reg.end()) {
    v = &it->second;
  } else if (auto it = group.defaults.find("constraints");
             it != group.defaults.end()) {
    if (auto s = std::get_if<std::string>(&it->second)) {
      if (*s == "use_enumerated_values")
        return use_enumerated_values;
      else if (*s == "write_as_read") {
        return write_as_read;
      } else {
        throw std::runtime_error(std::format(
            "invalid value for property 'write_constraints' in register "
            "'{}.{}': exptected object/table with properties 'minimum' and "
            "'maximum', or a string with hte value 'use_enumerated_values' or "
            "'write_as_read', "
            "got '{}'",
            group.name, reg_name, *s));
      }
    }
  } else if (auto it = group.extra_data.find("constraints");
             it != group.extra_data.end()) {
    v = &it->second;
  } else if (auto it = device.extra_data.find("constraints");
             it != device.extra_data.end()) {
    v = &it->second;
  }

  if (v) {
    auto wc = to_write_constraints(*v);
    if (wc)
      return wc.value();
    throw std::runtime_error(std::format(
        "invalid value for property 'write_constraints' in register "
        "'{}.{}': exptected object/table with properties 'minimum' and "
        "'maximum', or a string with hte value 'use_enumerated_values' or "
        "'write_as_read', "
        "got '{}'",
        group.name, reg_name, *v));
  }
  return {};
}

WriteContraints get_write_contraints(const Device &device, const Group &group,
                                     const std::string &reg_name,
                                     const ValueMap &reg,
                                     const std::string &field_name,
                                     const ValueMap &field) {
  if (auto it = field.find("constraints"); it != field.end()) {
    auto &v = it->second;
    auto wc = to_write_constraints(v);
    if (wc)
      return wc.value();
    throw std::runtime_error(std::format(
        "invalid value for property 'write_constraints' in field "
        "'{}.{}.{}': exptected object/table with properties 'minimum' and "
        "'maximum', or a string with hte value 'use_enumerated_values' or "
        "'write_as_read', "
        "got '{}'",
        group.name, reg_name, field_name, v));
  }
  return get_write_contraints(device, group, reg_name, reg);
}

std::vector<EnumeratedValue> get_values(const Device &device,
                                        const Group &group,
                                        const std::string &reg_name,
                                        const ValueMap &reg) {
  // TODO: implement get_values
  return {};
}

std::unique_ptr<Field> to_field(const Device &device, const Group &group,
                                const std::string &reg_name,
                                const ValueMap &reg,
                                const std::string &field_name,
                                const ValueMap &field) {
  std::optional<std::string> display_name{};
  std::optional<std::string> description{};
  std::optional<std::string> backup{};
  std::optional<std::string> unit{};
  std::optional<double> zero_code_value;
  std::optional<double> step;
  std::optional<std::uint64_t> initial;
  std::optional<std::uint64_t> value;
  std::optional<std::uint64_t> position;
  std::optional<std::uint64_t> msb;
  std::optional<std::uint64_t> lsb;
  std::optional<Access> access;
  std::optional<Protection> protection;
  std::optional<WriteType> write_type;
  std::optional<ReadType> read_type;
  std::optional<bool> is_signed;
  std::vector<EnumeratedValue> values{};
  std::map<std::string, Value> extra_data{};

  msb = get_inherited_property<uint64_t>(device, group, "msb", reg_name, reg,
                                         field_name, field, true);
  if (not msb) {
    position = get_inherited_property<uint64_t>(
        device, group, "position", reg_name, reg, field_name, field, true);
    if (not position)
      return {};
    else {
      msb = position;
      lsb = position;
    }
  } else {
    lsb = get_inherited_property<uint64_t>(device, group, "lsb", reg_name, reg,
                                           field_name, field, true);
    if (not lsb)
      return {};
  }

  display_name = get_property<std::string>(field, "name");
  description = get_property<std::string>(field, "description");
  backup = get_property<std::string>(field, "backup");
  unit = get_property<std::string>(field, "unit");
  zero_code_value =
      get_inherited_property<double>(device, group, "zero_code_value", reg_name,
                                     reg, field_name, field, false);
  step = get_inherited_property<double>(device, group, "step", reg_name, reg,
                                        field_name, field, false);
  initial = get_inherited_property<uint64_t>(device, group, "initial", reg_name,
                                             reg, field_name, field, true);
  value = get_inherited_property<uint64_t>(device, group, "value", reg_name,
                                           reg, field_name, field, true);
  access = get_inherited_property<Access>(device, group, "access", reg_name,
                                          reg, field_name, field, true);
  protection = get_inherited_property<Protection>(
      device, group, "protection", reg_name, reg, field_name, field, true);
  write_type = get_inherited_property<WriteType>(
      device, group, "write_type", reg_name, reg, field_name, field, true);
  read_type = get_inherited_property<ReadType>(
      device, group, "read_type", reg_name, reg, field_name, field, true);
  is_signed = get_inherited_property<bool>(device, group, "signed", reg_name,
                                           reg, field_name, field, true);
  values = get_values(device, group, field_name, field);
  std::unique_ptr<Field> f = std::make_unique<Field>();
  f->name = field_name;
  f->display_name = display_name.value_or({});
  f->description = description.value_or({});
  f->backup = backup.value_or({});
  f->unit = unit.value_or({});
  f->zero_code_value = zero_code_value.value_or(0.0);
  f->step = step.value_or(1.0);
  f->value = value.value_or(0);
  f->initial = initial.value_or(0);
  f->msb = msb.value();
  f->lsb = lsb.value();
  f->access = access.value_or(Access::read_only);
  f->protection = protection.value_or(Protection::non_secure);
  f->write_type = write_type.value_or(WriteType::modify);
  f->read_type = read_type.value_or(ReadType::modify);
  f->contraints =
      get_write_contraints(device, group, reg_name, reg, field_name, field);
  f->is_signed = is_signed.value_or(false);
  f->values = std::move(values);
  f->extra_data = std::move(extra_data);
  return f;
}

std::unique_ptr<Register> to_register(const Device &device, const Group &group,
                                      const std::string &reg_name,
                                      const ValueMap &reg) {
  std::optional<std::string> display_name{};
  std::optional<std::string> description{};
  std::optional<std::string> backup{};
  std::optional<std::string> unit{};
  std::optional<double> zero_code_value;
  std::optional<double> step;
  std::optional<std::uint64_t> initial;
  std::optional<std::uint64_t> value;
  std::optional<std::uint64_t> addr;
  std::optional<std::uint64_t> size;
  std::optional<std::uint64_t> zeros_mask;
  std::optional<std::uint64_t> ones_mask;
  std::optional<std::uint64_t> x_mask;
  std::optional<Access> access;
  std::optional<Protection> protection;
  std::optional<WriteType> write_type;
  std::optional<ReadType> read_type;
  std::optional<bool> is_signed;
  std::vector<EnumeratedValue> values{};
  std::map<std::string, Value> extra_data{};
  using namespace std::string_view_literals;
  // first check if addr is defined, which it has to be in all cases.
  addr = get_inherited_property<uint64_t>(device, group, "addr", reg_name, reg,
                                          true);
  if (not addr)
    return {};
  // reg has the properties addr and size -> it is assumed that this is
  // actually should be a register.
  size = get_inherited_property<uint64_t>(device, group, "size", reg_name, reg,
                                          false);
  std::unique_ptr<Register> r = std::make_unique<Register>();
  r->name = reg_name;

  display_name = get_property<std::string>(reg, "name");
  description = get_property<std::string>(reg, "description");
  backup = get_property<std::string>(reg, "backup");
  unit = get_property<std::string>(reg, "unit");
  zero_code_value = get_inherited_property<double>(
      device, group, "zero_code_value", reg_name, reg, false);
  step = get_inherited_property<double>(device, group, "step", reg_name, reg,
                                        false);
  initial = get_inherited_property<uint64_t>(device, group, "initial", reg_name,
                                             reg, true);
  value = get_inherited_property<uint64_t>(device, group, "value", reg_name,
                                           reg, true);
  zeros_mask = get_inherited_property<uint64_t>(device, group, "zeros_mask",
                                                reg_name, reg, true);
  ones_mask = get_inherited_property<uint64_t>(device, group, "ones_mask",
                                               reg_name, reg, true);
  x_mask = get_inherited_property<uint64_t>(device, group, "x_mask", reg_name,
                                            reg, true);
  access = get_inherited_property<Access>(device, group, "access", reg_name,
                                          reg, true);
  protection = get_inherited_property<Protection>(device, group, "protection",
                                                  reg_name, reg, true);
  write_type = get_inherited_property<WriteType>(device, group, "write_type",
                                                 reg_name, reg, true);
  read_type = get_inherited_property<ReadType>(device, group, "read_type",
                                               reg_name, reg, true);
  is_signed = get_inherited_property<bool>(device, group, "signed", reg_name,
                                           reg, true);
  values = get_values(device, group, reg_name, reg);
  bool fields_have_value = true;
  bool fields_have_initial = true;
  for (const auto &[name, value] : reg) {
    bool already_visited = false;
    for (const auto &n : {"display_name", "description",     "backup",
                          "unit",         "zero_code_value", "step",
                          "initial",      "value",           "addr",
                          "size",         "zeros_mask",      "ones_mask",
                          "x_mask",       "access",          "protection",
                          "write_type",   "read_type",       "constraints",
                          "is_signed",    "values"}) {
      if (n == name) {
        already_visited = true;
        break;
      }
    }

    if (already_visited)
      continue;

    switch (value.type()) {
    case Value::map:
      if (auto field =
              to_field(device, group, reg_name, reg, name, value.as_map())) {
        field->reg = r.get();
        r->fields.push_back(std::move(field));
        fields_have_value &= value.as_map().contains("value");
        fields_have_initial &= value.as_map().contains("initial");
      } else {
        // the map was not a field -> insert in in extra_data.
        extra_data.insert({name, value});
      }
      break;
    default:
      extra_data.insert({name, value});
    }
  }

  r->display_name = display_name.value_or({});
  r->description = description.value_or({});
  r->backup = backup.value_or({});
  r->unit = unit.value_or({});
  r->zero_code_value = zero_code_value.value_or(0.0);
  r->step = step.value_or(1.0);
  r->value = value.value_or(0);
  r->initial = initial.value_or(0);
  r->addr = addr.value();
  r->size = size.value_or(1);
  r->zeros_mask = zeros_mask.value_or(0);
  r->ones_mask = ones_mask.value_or(0);
  r->x_mask = x_mask.value_or(0);
  r->access = access.value_or(Access::read_only);
  r->protection = protection.value_or(Protection::non_secure);
  r->write_type = write_type.value_or(WriteType::modify);
  r->read_type = read_type.value_or(ReadType::modify);
  r->contraints = get_write_contraints(device, group, reg_name, reg);
  r->is_signed = is_signed.value_or(false);
  r->values = std::move(values);
  r->extra_data = std::move(extra_data);

  if (initial) {
    if (not fields_have_initial)
      for (auto &f : r->fields)
        f->initial = make_mask(f->msb - f->lsb, 0) & (r->initial >> f->lsb);
    else {
      // check initial of fields and register
      uint64_t initial = 0;
      uint64_t f_mask = 0;
      for (const auto &f : r->fields) {
        initial |= f->initial << f->lsb;
        f_mask |= make_mask(f->msb, f->lsb);
      }
      if (initial != (f_mask & r->initial))
        throw std::runtime_error(
            "field initial value and register initial value do not match");
    }
  } else {
    if (not fields_have_initial) {
      throw std::runtime_error("Expected either register to specify an "
                               "initial value, or the fields "
                               "to make up an initial value");
    }
    for (const auto &f : r->fields)
      r->initial |= f->initial << f->lsb;
  }

  if (value) {
    if (not fields_have_value) {
      for (auto &f : r->fields)
        f->value = make_mask(f->msb - f->lsb, 0) & (*value >> f->lsb);
    } else {
      // check initial of fields and register
      uint64_t value = 0;
      uint64_t f_mask = 0;
      for (const auto &f : r->fields) {
        value |= f->value << f->lsb;
        f_mask |= make_mask(f->msb, f->lsb);
      }
      if (value != (f_mask & r->value))
        throw std::runtime_error("field value and register value do not match");
    }
  } else {
    if (not fields_have_value) {
      r->value = r->initial;
      for (auto &f : r->fields)
        f->value = make_mask(f->msb - f->lsb, 0) & (r->value >> f->lsb);
    } else {
      for (const auto &f : r->fields)
        r->value |= f->value << f->lsb;
    }
  }
  return r;
}

std::unique_ptr<Group> to_group(const Device &device,
                                const std::string &group_name,
                                const ValueMap &group) {
  if (not group.contains("base_addr")) {
    return {};
  }

  auto base_addr = group.at("base_addr").convert_to<uint64_t>();
  if (not base_addr) {
    throw std::runtime_error(
        std::format("Type mismatch for property 'base_addr' in group '{}': "
                    "expected unsigned integer, got {}",
                    group_name, to_string(group.at("base_adddr").type())));
  }

  if (not group.contains("size")) {
    return {};
  }

  auto size = group.at("size").convert_to<uint64_t>();
  if (not size) {
    throw std::runtime_error(
        std::format("Type mismatch for property 'size' in group '{}': expected "
                    "unsigned integer, got {}",
                    group_name, to_string(group.at("size").type())));
  }

  std::unique_ptr<Group> g = std::make_unique<Group>();
  g->name = group_name;
  g->base_addr = *base_addr;
  g->size = *size;

  if (auto it = group.find("defaults");
      it != group.end() and it->second.is_map()) {
    g->defaults = it->second.as_map();
  }

  for (const auto &[name, value] : group) {
    if (name == "base_adddr" or name == "size" or name == "defaults")
      continue;
    else if (value.is_map()) {
      if (auto reg = to_register(device, *g, name, value.as_map())) {
        reg->group = g.get();
        g->registers.push_back(std::move(reg));
      } else {
        g->extra_data.insert_or_assign(name, value);
      }
    } else {
      g->extra_data.insert_or_assign(name, value);
    }
  }
  return g;
}

std::unique_ptr<Device> to_device(const ValueMap &dev) {
  auto device = std::make_unique<Device>();
  extract_property("name", device->name, dev);
  extract_property("version", device->version, dev);
  extract_property("register_width", device->register_width, dev);
  extract_property("word_width", device->word_width, dev);
  extract_property("num_pages", device->num_pages, dev);
  extract_property("registers_per_page", device->registers_per_page, dev);
  if (not dev.contains("endian")) {
    // TODO:
    throw std::runtime_error("expected key 'endian' in device");
  }
  auto &endian = dev.at("endian");
  if (not endian.is_string()) {
    // TODO:
    throw std::runtime_error(std::format("Type mismatch for property 'endian' "
                                         "in device: expected string, got {}",
                                         to_string(endian.type())));
  }
  if (auto e = from_string<Endian>(endian.as_string()))
    device->endian = *e;
  else
    throw std::runtime_error(
        std::format("invalid value for property 'endian' in device: expected "
                    "'little' or 'big', got '{}'",
                    endian));

  if (auto it = dev.find("defaults"); it != dev.end()) {
    if (not it->second.is_map()) {
      throw std::runtime_error(std::format(
          "Type mismatch for property 'defaults': expected map, got {}",
          to_string(it->second.type())));
    }
    device->extra_data = it->second.as_map();
  }

  for (const auto &[name, value] : dev) {
    if (name == "defaults" or name == "version" or name == "register_width" or
        name == "word_width" or name == "endian" or name == "num_pages" or
        name == "registers_per_page")
      continue;
    switch (value.type()) {
    case Value::map:
      if (auto group = to_group(*device, name, value.as_map())) {
        group->device = device.get();
        device->groups.push_back(std::move(group));
      } else {
        device->extra_data.insert({name, value});
      }
      break;
    case Value::list:
      [[fallthrough]];
    case Value::string:
      [[fallthrough]];
    case Value::integer:
      [[fallthrough]];
    case Value::unsigned_integer:
      [[fallthrough]];
    case Value::floating_point:
      [[fallthrough]];
    case Value::boolean:
      device->extra_data.insert({name, value});
    }
  }

  return device;
}

std::vector<std::string> split_at(const auto &str, char delimiter) {
  std::vector<std::string> list;
  std::string elem;
  for (char c : str) {
    if (c == delimiter) {
      list.push_back(elem);
      elem.clear();
    } else {
      elem.push_back(c);
    }
  }
  list.push_back(std::move(elem));
  return list;
}

void do_derive(ValueMap &map, std::vector<ValueMap *> parents,
               std::string_view name) {

  if (not map.contains("derived_from")) {
    parents.push_back(&map);
    for (auto &[name, value] : map) {
      map.insert({name, value});
      if (value.is_map())
        do_derive(map.at(name).as_map(), parents, name);
    }
    return;
  }

  if (parents.empty())
    throw std::runtime_error(
        "It is invalid to have a 'derived_from' entry on the top level!");

  const auto &d = map.at("derived_from");
  if (not d.is_string())
    throw std::runtime_error("Invalid type for proerty 'derived_from' in ");

  const auto &derived_from = d.as_string();
  const auto fragments = split_at(derived_from, '.');
  if (fragments.size() > parents.size())
    throw std::runtime_error("invalid 'derived_from' entry on line ");

  std::vector<ValueMap *> derived_from_parents(
      parents.begin(),
      std::next(parents.begin(), parents.size() - fragments.size() + 1));

  for (const auto &fragment : fragments) {
    auto &parent = derived_from_parents.back();
    auto &next = parent->at(fragment).as_map();
    derived_from_parents.push_back(&next);
  }

  auto *base = derived_from_parents.back();
  parents.push_back(&map);

  for (auto &[name, value] : *base) {
    map.insert({name, value});
    if (value.is_map())
      do_derive(map.at(name).as_map(), parents, name);
  }
  map.erase("derived_from");
}

ValueMap derive(ValueMap &&map) {
  do_derive(map, {}, {});
  return std::move(map);
}

std::unique_ptr<Device> sort(std::unique_ptr<Device> &&device) {
  // sort groups in ascending order of base_addr
  std::sort(device->groups.begin(), device->groups.end(),
            [](const std::unique_ptr<Group> &group1,
               const std::unique_ptr<Group> &group2) {
              return group1->base_addr < group2->base_addr;
            });

  for (auto &group : device->groups) {
    // sort registers in ascending order of addr
    std::sort(group->registers.begin(), group->registers.end(),
              [](const std::unique_ptr<Register> &r1,
                 const std::unique_ptr<Register> &r2) {
                return r1->addr < r2->addr;
              });
    // sort the fields in ascending order of lsb
    for (auto &reg : group->registers) {
      std::sort(
          reg->fields.begin(), reg->fields.end(),
          [](const std::unique_ptr<Field> &f1,
             const std::unique_ptr<Field> &f2) { return f1->lsb < f2->lsb; });
    }
  }
  return std::move(device);
}

std::unique_ptr<Device> validate(std::unique_ptr<Device> &&d) {
  if (d->version != 1)
    throw std::runtime_error("invalid version, expected 1, got " +
                             std::to_string(d->version));
  if (d->register_width > 64)
    throw std::runtime_error(
        "register_width is too large, must be 64 or smaller");

  if (d->register_width < 1)
    throw std::runtime_error(
        "register_width is too small, must be 1 or larger");

  if (d->registers_per_page < 1)
    throw std::runtime_error(
        "registers_per_page is too small, must be 1 or larger");

  if (d->num_pages < 1)
    throw std::runtime_error("num_pages is too small, must be 1 or larger");

  if (d->groups.size() == 1 and d->groups[0]->size == 0) {
    d->groups[0]->size =
        d->register_width * d->registers_per_page * d->num_pages -
        d->groups[0]->base_addr;
  } else {
    bool first = true;
    for (const auto &group : d->groups) {
      if (group->size == 0)
        throw std::runtime_error("Invalid group size value of 0 in group '" +
                                 group->name + "'");
      if (not first) {
        const auto &prev_group = *(std::addressof(group) - 1);
        if ((prev_group->base_addr + prev_group->size) > group->base_addr) {
          throw std::runtime_error("group ranges overlap for group '" +
                                   prev_group->name + "' and '" + group->name +
                                   "'");
        }
      }
      first = false;
    }
  }

  for (const auto &group : d->groups) {
    // check validity of memory and set initial field value
    for (auto &reg : group->registers) {
      const uint64_t size_mask = make_mask(reg->num_bits() - 1, 0);
      // check that masks dont overlap
      if (reg->x_mask & reg->ones_mask & reg->ones_mask) {
        throw std::runtime_error(
            "overlapping x_mask, ones_mask and zeros_mask in register '" +
            group->name + "." + reg->name + "'");
      }
      const auto reg_mask = reg->x_mask | reg->ones_mask | reg->zeros_mask;
      // check that masks dont go over the register size
      if (reg_mask & ~size_mask) {
        throw std::runtime_error(
            "one of x_mask, ones_mask and zeros_mask in register '" +
            group->name + "." + reg->name + "' don't fit inside the register");
      }

      uint64_t field_mask = 0;
      for (const auto &field : reg->fields) {
        const auto mask = make_mask(field->msb, field->lsb);
        if (field_mask & mask) {
          throw std::runtime_error("field " + group->name + "." + reg->name +
                                   "." + field->name +
                                   "' overlaps with previous field");
        }
        if (field_mask & ~size_mask) {
          throw std::runtime_error(
              "field position outside valid range in field '" + group->name +
              "." + reg->name + "." + field->name + "'");
        }
        if (field_mask & reg_mask) {
          throw std::runtime_error(
              "overlap with the mask defined "
              "by x_mask, zeros_mask, and ones_mask in field'" +
              group->name + "." + reg->name + "." + field->name + "'");
        }

        field_mask |= mask;
      }
    }
  }
  if (d->groups.back()->size + d->groups.back()->base_addr >
      (d->register_width * d->registers_per_page * d->num_pages -
       d->groups.back()->base_addr))
    throw std::runtime_error("group is too big to fit into memory: '" +
                             d->groups.back()->name + "'");
  return std::move(d);
}
} // namespace rds::detail
