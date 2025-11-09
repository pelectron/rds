#include "rds/parse.hpp"

#include <format>
#include <iostream>

int main() {
  try {
    auto val = rds::parse::parse(
        "{id: 'hello', i: -1, u: 243, d: 5.4, sd: -5.4, array:[ 'a value', "
        "2.5, 4, 2, -5,-12.1,3e5,5e-6,-7.8e-5]}");
    std::cout << std::format("val:\n{}\n\n", val);
  } catch (const std::exception &e) {
    std::cout << "Exception 1:\n" << e.what() << std::endl;
  }

  try {
    auto val = rds::parse::parse(
        "{id: 'hello', i: -1, u: 243, d: 5.4, sd: -5.4, array:[ 'a value' "
        "2.5, 4, 2, -5,-12.1,3e5,5e-6,-7.8e-5]}");
    std::cout << std::format("val:\n{}", val);
  } catch (const std::exception &e) {
    std::cout << "Exception 2:\n" << e.what() << std::endl;
  }

  try {
    auto val = rds::parse::parse(
        "{id: 'hello', i: -1, u: 243, d: 5.4, sd: -5.4, array:[ 'a value', "
        "2.5, 4, 2, -5,-12.1,3e5,5e-6,-7.8e-5], 'rguub'}");
    std::cout << std::format("val:\n{}\n\n", val);
  } catch (const std::exception &e) {
    std::cout << "Exception 3:\n" << e.what() << std::endl;
  }
}
