#include "rds.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("Register::add_field(name, msb, lsb)") {
  rds::Register r;
  REQUIRE(r.num_bits() == 8);
  SECTION("add first field") {
    auto *field = r.add_field("name", 2, 1);
    REQUIRE(field != nullptr);
    REQUIRE(field->reg == &r);
    REQUIRE(field->name == "name");
    REQUIRE(field->msb == 2);
    REQUIRE(field->lsb == 1);
    REQUIRE(r.fields.size() == 1);
    REQUIRE(r.field("name") == field);
    REQUIRE(r.has_field("name"));
  }
  SECTION("add second field") {
    REQUIRE(r.fields.size() == 0);
    rds::Field *field1 = r.add_field("name1", 2, 2);
    REQUIRE(r.fields.size() == 1);
    REQUIRE(field1 != nullptr);
    REQUIRE(field1->reg == &r);
    REQUIRE(field1->msb == 2);
    REQUIRE(field1->lsb == 2);
    SECTION("first is greater") {
      auto *field2 = r.add_field("name2", 1, 0);
      REQUIRE(field2 != nullptr);
      REQUIRE(field2->reg == &r);
      REQUIRE(field2->msb == 1);
      REQUIRE(field2->lsb == 0);
      REQUIRE(r.fields.size() == 2);
      REQUIRE(r.fields[0].get() == field2);
      REQUIRE(r.fields[1].get() == field1);
      REQUIRE(r.field("name1") == field1);
      REQUIRE(r.field("name2") == field2);
      REQUIRE(r.has_field("name1"));
      REQUIRE(r.has_field("name2"));
    }
    SECTION("first is smaller") {
      auto *field2 = r.add_field("name2", 4, 3);
      REQUIRE(field2 != nullptr);
      REQUIRE(field2->reg == &r);
      REQUIRE(field2->msb == 4);
      REQUIRE(field2->lsb == 3);
      REQUIRE(r.fields.size() == 2);
      REQUIRE(r.fields[0].get() == field1);
      REQUIRE(r.fields[1].get() == field2);
      REQUIRE(r.field("name1") == field1);
      REQUIRE(r.field("name2") == field2);
      REQUIRE(r.has_field("name1"));
      REQUIRE(r.has_field("name2"));
    }
  }
  SECTION("add third register") {
    auto *field1 = r.add_field("name1", 2, 2);
    auto *field2 = r.add_field("name2", 4, 4);
    SECTION("add as first") {
      auto *field3 = r.add_field("name3", 1, 0);
      REQUIRE(field3->reg == &r);
      REQUIRE(field3->msb == 1);
      REQUIRE(field3->lsb == 0);
      REQUIRE(r.fields.size() == 3);
      REQUIRE(r.fields[0].get() == field3);
      REQUIRE(r.fields[1].get() == field1);
      REQUIRE(r.fields[2].get() == field2);
      REQUIRE(r.field("name1") == field1);
      REQUIRE(r.field("name2") == field2);
      REQUIRE(r.field("name3") == field3);
      REQUIRE(r.has_field("name1"));
      REQUIRE(r.has_field("name2"));
      REQUIRE(r.has_field("name3"));
    }
    SECTION("add in the middle") {
      auto *field3 = r.add_field("name3", 3, 3);
      REQUIRE(field3->reg == &r);
      REQUIRE(field3->msb == 3);
      REQUIRE(field3->lsb == 3);
      REQUIRE(r.fields.size() == 3);
      REQUIRE(r.fields[0].get() == field1);
      REQUIRE(r.fields[1].get() == field3);
      REQUIRE(r.fields[2].get() == field2);
      REQUIRE(r.field("name1") == field1);
      REQUIRE(r.field("name2") == field2);
      REQUIRE(r.field("name3") == field3);
      REQUIRE(r.has_field("name1"));
      REQUIRE(r.has_field("name2"));
      REQUIRE(r.has_field("name3"));
    }
    SECTION("add at the end") {
      auto *field3 = r.add_field("name3", 7, 5);
      REQUIRE(field3->reg == &r);
      REQUIRE(field3->msb == 7);
      REQUIRE(field3->lsb == 5);
      REQUIRE(r.fields.size() == 3);
      REQUIRE(r.fields[0].get() == field1);
      REQUIRE(r.fields[1].get() == field2);
      REQUIRE(r.fields[2].get() == field3);
      REQUIRE(r.field("name1") == field1);
      REQUIRE(r.field("name2") == field2);
      REQUIRE(r.field("name3") == field3);
      REQUIRE(r.has_field("name1"));
      REQUIRE(r.has_field("name2"));
      REQUIRE(r.has_field("name3"));
    }
  }
  SECTION("add invalid fields") {
    SECTION("invalid name") { REQUIRE(r.add_field({}, 0, 1) == nullptr); }
    SECTION("duplicate name") {
      r.add_field("name1", 1, 1);
      REQUIRE(r.add_field("name1", 0, 0) == nullptr);
    }
    SECTION("invalid msb lsb") {
      REQUIRE(r.add_field("name3", 0, 1) == nullptr);
    }
    SECTION("duplicate address") {
      r.add_field("name1", 1, 1);
      REQUIRE(r.add_field("name3", 1, 1) == nullptr);
    }
    SECTION("overlap with existing register") {
      r.add_field("name1", 1, 1);
      r.add_field("name2", 3, 2);
      REQUIRE(r.add_field("name3", 1, 0) == nullptr);
      REQUIRE(r.add_field("name3", 2, 0) == nullptr);
      REQUIRE(r.add_field("name3", 4, 3) == nullptr);
    }
  }
}

TEST_CASE("Register::set_value") {
  rds::Register reg;
  reg.size = 1;
  auto *f0 = reg.add_field("f0", 0, 0);
  auto *f1 = reg.add_field("f1", 1, 1);
  auto *f2 = reg.add_field("f2", 2, 2);
  REQUIRE(reg.set_value(1));
  REQUIRE(reg.value == 1);
  REQUIRE(f0->value == 1);
  REQUIRE(f1->value == 0);
  REQUIRE(f2->value == 0);
  REQUIRE(reg.set_value(2));
  REQUIRE(reg.value == 2);
  REQUIRE(f0->value == 0);
  REQUIRE(f1->value == 1);
  REQUIRE(f2->value == 0);
  REQUIRE(reg.set_value(3));
  REQUIRE(reg.value == 3);
  REQUIRE(f0->value == 1);
  REQUIRE(f1->value == 1);
  REQUIRE(f2->value == 0);
  REQUIRE(reg.set_value(4));
  REQUIRE(reg.value == 4);
  REQUIRE(f0->value == 0);
  REQUIRE(f1->value == 0);
  REQUIRE(f2->value == 1);
  REQUIRE_FALSE(reg.set_value(0xFFFFu));
}
