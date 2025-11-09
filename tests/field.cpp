#include "rds.hpp"

#include <catch2/catch_all.hpp>

TEST_CASE("Field::set_value") {
  rds::Register reg;
  reg.size = 1;
  auto *f0 = reg.add_field("f0", 0, 0);
  auto *f1 = reg.add_field("f1", 1, 1);
  auto *f2 = reg.add_field("f2", 2, 2);
  reg.set_value(0);
  f0->set_value(1);
  REQUIRE(reg.value == 0b001);
  f1->set_value(1);
  REQUIRE(reg.value == 0b011);
  f2->set_value(1);
  REQUIRE(reg.value == 0b111);
  f1->set_value(0);
  REQUIRE(reg.value == 0b101);
  REQUIRE_FALSE(f0->set_value(2));
}
