#include "rds/device.hpp"
#include "catch2/catch_test_macros.hpp"
#include <catch2/catch_all.hpp>
#include <utility>

#define DEFINE_DEVICE(dev)                                                     \
  rds::Device dev;                                                             \
  {                                                                            \
    dev.name = "dev";                                                          \
    dev.version = 2;                                                           \
    dev.register_width = 32;                                                   \
    dev.num_pages = 2;                                                         \
    dev.registers_per_page = 5;                                                \
    dev.extra_data.insert({"v1", 1.0});                                        \
    auto *group = dev.add_group("g1", 0, 1);                                   \
    auto *reg = group->add_reg("r1", 0, 1);                                    \
    reg->description = "description";                                          \
    reg->backup = "backup";                                                    \
    reg->unit = "unit";                                                        \
    reg->zero_code_value = 5;                                                  \
    reg->step = 2;                                                             \
    reg->initial = 1;                                                          \
    reg->value = 3;                                                            \
    reg->zeros_mask = 0x100;                                                   \
    reg->ones_mask = 1;                                                        \
    reg->x_mask = 0x200;                                                       \
    /* TODO: access*/                                                          \
    reg->is_signed = true;                                                     \
    reg->add_value(rds::EnumeratedValue{"rv1", "desc", 1});                    \
    reg->add_value(rds::EnumeratedValue{"rv2", "desc", 3});                    \
    group = dev.add_group("g2", 1, 2);                                         \
    reg = group->add_reg("r2", 0, 1);                                          \
    reg->description = "description2";                                         \
    reg->backup = "backup2";                                                   \
    reg->unit = "unit2";                                                       \
    reg->zero_code_value = 4;                                                  \
    reg->step = 3;                                                             \
    reg->initial = 3;                                                          \
    reg->value = 1;                                                            \
    reg->zeros_mask = 0x100;                                                   \
    reg->ones_mask = 1;                                                        \
    reg->x_mask = 0x200;                                                       \
    /* TODO: access*/                                                          \
    reg->is_signed = true;                                                     \
    reg->add_value(rds::EnumeratedValue{"rv1", "desc", 1});                    \
    reg->add_value(rds::EnumeratedValue{"rv2", "desc", 3});                    \
  }

#define CHECK_DEV_ARRAY(dev)                                                   \
  {                                                                            \
    REQUIRE(dev.name == "dev");                                                \
    REQUIRE(dev.version == 2);                                                 \
    REQUIRE(dev.register_width == 32);                                         \
    REQUIRE(dev.num_pages == 2);                                               \
    REQUIRE(dev.registers_per_page == 5);                                      \
    REQUIRE(dev.extra_data.size() == 1);                                       \
    REQUIRE(dev.extra_data.contains("v1"));                                    \
    REQUIRE(std::get<double>(dev.extra_data.at("v1")) == 1.0);                 \
    auto *group = dev.group("g1");                                             \
    REQUIRE(group != nullptr);                                                 \
    auto *reg = group->reg("r1");                                              \
    REQUIRE(reg != nullptr);                                                   \
    REQUIRE(reg->description == "description");                                \
    REQUIRE(reg->backup == "backup");                                          \
    REQUIRE(reg->unit == "unit");                                              \
    REQUIRE(reg->zero_code_value == 5);                                        \
    REQUIRE(reg->step == 2);                                                   \
    REQUIRE(reg->initial == 1);                                                \
    REQUIRE(reg->value == 3);                                                  \
    REQUIRE(reg->zeros_mask == 0x100);                                         \
    REQUIRE(reg->ones_mask == 1);                                              \
    REQUIRE(reg->x_mask == 0x200);                                             \
    REQUIRE(reg->is_signed == true);                                           \
    REQUIRE(reg->values.size() == 2);                                          \
    REQUIRE(reg->values[0].description == "desc");                             \
    REQUIRE(reg->values[0].value == 1);                                        \
    REQUIRE(reg->values[0].name == "rv1");                                     \
    REQUIRE(reg->values[0].description == "desc");                             \
    REQUIRE(reg->values[0].value == 1);                                        \
    REQUIRE(reg->values[1].name == "rv2");                                     \
    REQUIRE(reg->values[1].description == "desc");                             \
    REQUIRE(reg->values[1].value == 3);                                        \
    group = dev.group("g2");                                                   \
    REQUIRE(group != nullptr);                                                 \
    reg = group->reg("r2");                                                    \
    REQUIRE(reg != nullptr);                                                   \
    REQUIRE(reg->description == "description2");                               \
    REQUIRE(reg->backup == "backup2");                                         \
    REQUIRE(reg->unit == "unit2");                                             \
    REQUIRE(reg->zero_code_value == 4);                                        \
    REQUIRE(reg->step == 3);                                                   \
    REQUIRE(reg->initial == 3);                                                \
    REQUIRE(reg->value == 1);                                                  \
    REQUIRE(reg->zeros_mask == 0x100);                                         \
    REQUIRE(reg->ones_mask == 1);                                              \
    REQUIRE(reg->x_mask == 0x200);                                             \
    REQUIRE(reg->is_signed == true);                                           \
    REQUIRE(reg->values.size() == 2);                                          \
    REQUIRE(reg->values[0].description == "desc");                             \
    REQUIRE(reg->values[0].value == 1);                                        \
    REQUIRE(reg->values[0].name == "rv1");                                     \
    REQUIRE(reg->values[0].description == "desc");                             \
    REQUIRE(reg->values[0].value == 1);                                        \
    REQUIRE(reg->values[1].name == "rv2");                                     \
    REQUIRE(reg->values[1].description == "desc");                             \
    REQUIRE(reg->values[1].value == 3);                                        \
    for (const auto &group : dev.groups) {                                     \
      REQUIRE(group->device == &dev);                                          \
      for (const auto &r : group->registers) {                                 \
        REQUIRE(r->group == group.get());                                      \
        for (const auto &f : r->fields)                                        \
          REQUIRE(f->reg == r.get());                                          \
      }                                                                        \
    }                                                                          \
  }

TEST_CASE("Device copy ctor") {
  DEFINE_DEVICE(dev);
  rds::Device d(std::as_const(dev));
  CHECK_DEV_ARRAY(d);
}

TEST_CASE("Device move ctor") {
  DEFINE_DEVICE(dev);
  rds::Device d(std::move(dev));
  CHECK_DEV_ARRAY(d);
}

TEST_CASE("Device move assignment") {
  DEFINE_DEVICE(dev);
  rds::Device d;
  d = std::move(dev);
  CHECK_DEV_ARRAY(d);
}

TEST_CASE("Device copy assignment") {
  DEFINE_DEVICE(dev);
  rds::Device d;
  d = dev;
  CHECK_DEV_ARRAY(d);
}

TEST_CASE("Device::reset") {
  DEFINE_DEVICE(dev);
  dev.reset();
  for (const auto &group : dev.groups)
    for (const auto &reg : group->registers)
      REQUIRE(reg->value == reg->initial);

  REQUIRE(dev.group("g1")->reg("r1")->initial == 1);
  REQUIRE(dev.group("g2")->reg("r2")->initial == 3);
}

TEST_CASE("Device::content") {}

TEST_CASE("Device::set_content") {}

TEST_CASE("Device::add/group") {
  rds::Device d;
  SECTION("add first group") {
    auto *group = d.add_group("name", 0, 1);
    REQUIRE(group != nullptr);
    REQUIRE(d.groups.size() == 1);
    REQUIRE(d.group("name") == group);
  }
  SECTION("add second group") {
    auto *group1 = d.add_group("name1", 1, 1);
    REQUIRE(group1 != nullptr);
    REQUIRE(d.group("name1") == group1);
    SECTION("first is greater") {
      auto *group2 = d.add_group("name2", 0, 1);
      REQUIRE(group2 != nullptr);
      REQUIRE(d.groups.size() == 2);
      REQUIRE(d.groups[0].get() == group2);
      REQUIRE(d.groups[1].get() == group1);
      REQUIRE(d.group("name1") == group1);
      REQUIRE(d.group("name2") == group2);
    }
    SECTION("first is smaller") {
      auto *group2 = d.add_group("name2", 2, 1);
      REQUIRE(group2 != nullptr);
      REQUIRE(d.groups.size() == 2);
      REQUIRE(d.groups[0].get() == group1);
      REQUIRE(d.groups[1].get() == group2);
      REQUIRE(d.group("name1") == group1);
      REQUIRE(d.group("name2") == group2);
    }
  }
  SECTION("add third group") {
    auto *group1 = d.add_group("name1", 1, 1);
    auto *group2 = d.add_group("name2", 3, 1);
    SECTION("add as first") {
      auto *group3 = d.add_group("name3", 0, 1);
      REQUIRE(d.groups.size() == 3);
      REQUIRE(d.groups[0].get() == group3);
      REQUIRE(d.groups[1].get() == group1);
      REQUIRE(d.groups[2].get() == group2);
      REQUIRE(d.group("name1") == group1);
      REQUIRE(d.group("name2") == group2);
      REQUIRE(d.group("name3") == group3);
    }
    SECTION("add in the middle") {
      auto *group3 = d.add_group("name3", 2, 1);
      REQUIRE(d.groups.size() == 3);
      REQUIRE(d.groups[0].get() == group1);
      REQUIRE(d.groups[1].get() == group3);
      REQUIRE(d.groups[2].get() == group2);
      REQUIRE(d.group("name1") == group1);
      REQUIRE(d.group("name2") == group2);
      REQUIRE(d.group("name3") == group3);
    }
    SECTION("add at the end") {
      auto *group3 = d.add_group("name3", 4, 1);
      REQUIRE(d.groups.size() == 3);
      REQUIRE(d.groups[0].get() == group1);
      REQUIRE(d.groups[1].get() == group2);
      REQUIRE(d.groups[2].get() == group3);
      REQUIRE(d.group("name1") == group1);
      REQUIRE(d.group("name2") == group2);
      REQUIRE(d.group("name3") == group3);
    }
  }

  SECTION("add invalid group") {
    d.add_group("n", 5, 1);
    SECTION("empty name") { REQUIRE(d.add_group({}, 0, 1) == nullptr); }
    SECTION("existing name") { REQUIRE(d.add_group("n", 0, 1) == nullptr); }
    SECTION("invalid size") { REQUIRE(d.add_group("name", 0, 0) == nullptr); }
    SECTION("overlapp with existing groups") {
      REQUIRE(d.add_group("n1", 0, 6) == nullptr);
      REQUIRE(d.add_group("n2", 4, 2) == nullptr);
      REQUIRE(d.add_group("n3", 5, 1) == nullptr);
      REQUIRE(d.add_group("n4", 5, 2) == nullptr);
    }
  }
}
