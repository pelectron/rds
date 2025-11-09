#include "rds.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>

int main(int argc, const char **argv) {
  if (argc < 2) {
    std::cerr << std::format(
        "Error: no input supplied\nUsage: {} input [output]\n", argv[0]);
    return -1;
  }

  const bool use_std_out_as_output = argc < 3;
  const auto input_path = std::filesystem::path(argv[1]);
  const auto output_path = not use_std_out_as_output
                               ? std::filesystem::path(argv[2])
                               : std::filesystem::path{};

  if (std::filesystem::is_directory(input_path)) {
    std::cerr << std::format("Error: input '{}' is a not a file\n",
                             input_path.string());
    return -1;
  }

  if (not std::filesystem::exists(input_path)) {
    std::cerr << std::format("Error: input file '{}' does not exist\n",
                             input_path.string());
    return -1;
  }

  std::unique_ptr<rds::Device> dev;
  try {
    dev = rds::device_from_file(input_path.string());
  } catch (const std::out_of_range &o) {
    std::cerr << std::format("implementation error: {}\n", o.what());
    return -1;
  } catch (const std::exception &e) {
    std::cerr << std::format("Error in input file: {}\n", e.what());
    return -1;
  }
  if (not dev) {
    std::cerr << "Error in implementation: dev is null\n";
    return -1;
  }

  try {
    const auto output_ext = output_path.extension();

    if (use_std_out_as_output) {
      std::cout << rds::to_string(*dev) << std::endl;
    } else {
      try {
        rds::device_to_file(*dev, output_path.string());
      } catch (...) {
        std::cerr << std::format("Error: invalid output file type, must be "
                                 "toml or json\nUsage: {} input [output]\n",
                                 argv[0]);
        return -1;
      }
    }
  } catch (const std::exception &e) {
    std::cerr << std::format("Error in output file: {}\n", e.what());
  }

  return 0;
}
