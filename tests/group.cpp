#include "catch2/catch_test_macros.hpp"
#include "rds.hpp"
#include <catch2/catch_all.hpp>

TEST_CASE("Group::add/get_register") {
  rds::Group g;
  SECTION("add first register") {
    auto *reg = g.add_reg("name", 0, 1);
    REQUIRE(reg != nullptr);
    REQUIRE(reg->group == &g);
    REQUIRE(g.registers.size() == 1);
    REQUIRE(g.reg("name") == reg);
    REQUIRE(g.has_register("name"));
  }
  SECTION("add second register") {
    auto *reg1 = g.add_reg("name1", 1, 1);
    REQUIRE(reg1 != nullptr);
    REQUIRE(reg1->group == &g);
    SECTION("first is greater") {
      auto *reg2 = g.add_reg("name2", 0, 1);
      REQUIRE(reg2 != nullptr);
      REQUIRE(reg2->group == &g);
      REQUIRE(g.registers.size() == 2);
      REQUIRE(g.registers[0].get() == reg2);
      REQUIRE(g.registers[1].get() == reg1);
      REQUIRE(g.reg("name1") == reg1);
      REQUIRE(g.reg("name2") == reg2);
      REQUIRE(g.has_register("name1"));
      REQUIRE(g.has_register("name2"));
    }
    SECTION("first is smaller") {
      auto *reg2 = g.add_reg("name2", 2, 1);
      REQUIRE(reg2 != nullptr);
      REQUIRE(reg2->group == &g);
      REQUIRE(g.registers.size() == 2);
      REQUIRE(g.registers[0].get() == reg1);
      REQUIRE(g.registers[1].get() == reg2);
      REQUIRE(g.reg("name1") == reg1);
      REQUIRE(g.reg("name2") == reg2);
      REQUIRE(g.has_register("name1"));
      REQUIRE(g.has_register("name2"));
    }
  }
  SECTION("add third register") {
    auto *reg1 = g.add_reg("name1", 1, 1);
    auto *reg2 = g.add_reg("name2", 3, 1);
    SECTION("add as first") {
      auto *reg3 = g.add_reg("name3", 0, 1);
      REQUIRE(reg3 != nullptr);
      REQUIRE(reg3->group == &g);
      REQUIRE(g.registers.size() == 3);
      REQUIRE(g.registers[0].get() == reg3);
      REQUIRE(g.registers[1].get() == reg1);
      REQUIRE(g.registers[2].get() == reg2);
      REQUIRE(g.reg("name1") == reg1);
      REQUIRE(g.reg("name2") == reg2);
      REQUIRE(g.reg("name3") == reg3);
      REQUIRE(g.has_register("name1"));
      REQUIRE(g.has_register("name2"));
      REQUIRE(g.has_register("name3"));
    }
    SECTION("add in the middle") {
      auto *reg3 = g.add_reg("name3", 2, 1);
      REQUIRE(reg3 != nullptr);
      REQUIRE(reg3->group == &g);
      REQUIRE(g.registers.size() == 3);
      REQUIRE(g.registers[0].get() == reg1);
      REQUIRE(g.registers[1].get() == reg3);
      REQUIRE(g.registers[2].get() == reg2);
      REQUIRE(g.reg("name1") == reg1);
      REQUIRE(g.reg("name2") == reg2);
      REQUIRE(g.reg("name3") == reg3);
      REQUIRE(g.has_register("name1"));
      REQUIRE(g.has_register("name2"));
      REQUIRE(g.has_register("name3"));
    }
    SECTION("add at the end") {
      auto *reg3 = g.add_reg("name3", 4, 1);
      REQUIRE(reg3 != nullptr);
      REQUIRE(reg3->group == &g);
      REQUIRE(g.registers.size() == 3);
      REQUIRE(g.registers[0].get() == reg1);
      REQUIRE(g.registers[1].get() == reg2);
      REQUIRE(g.registers[2].get() == reg3);
      REQUIRE(g.reg("name1") == reg1);
      REQUIRE(g.reg("name2") == reg2);
      REQUIRE(g.reg("name3") == reg3);
      REQUIRE(g.has_register("name1"));
      REQUIRE(g.has_register("name2"));
      REQUIRE(g.has_register("name3"));
    }
  }
  SECTION("add invalid register") {
    SECTION("invalid name") { REQUIRE(g.add_reg({}, 0, 1) == nullptr); }
    SECTION("duplicate name") {
      g.add_reg("name1", 1, 1);
      REQUIRE(g.add_reg("name1", 0, 1) == nullptr);
    }
    SECTION("invalid size") { REQUIRE(g.add_reg("name3", 0, 0) == nullptr); }
    SECTION("duplicate address") {
      g.add_reg("name1", 1, 1);
      REQUIRE(g.add_reg("name3", 1, 1) == nullptr);
    }
    SECTION("overlap with existing register") {
      g.add_reg("name1", 1, 1);
      g.add_reg("name2", 3, 2);
      REQUIRE(g.add_reg("name3", 0, 2) == nullptr);
      REQUIRE(g.add_reg("name3", 0, 3) == nullptr);
      REQUIRE(g.add_reg("name3", 2, 2) == nullptr);
      REQUIRE(g.add_reg("name3", 3, 1) == nullptr);
      REQUIRE(g.add_reg("name3", 4, 1) == nullptr);
      REQUIRE(g.add_reg("name3", 4, 2) == nullptr);
    }
  }
}
