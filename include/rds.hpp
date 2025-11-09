/**
 * @file rds.hpp
 *
 * @defgroup rds rds
 * rds (register description system) is a simple library for reading and
 * writing memory map meta data files.
 *
 * Embedded devices have memory. This memory has a word width and a size. It is
 * segmented into individual groups. These groups are made up of registers.
 * Registers can optionally be made up of fields. THis library allows to write
 * a descriptive toml file of your embedded device memory map. This can be used
 * to generate embedded device driver constants or can be incorporated into an
 * editor.
 *
 *
 * Two main funtions are provided:
 * - @ref rds::device_from_file() "rds::device_from_file(file_path)": returns an
 * rds::Device from an rds file.
 * - @ref rds::device_to_file() "rds::device_to_file(device, file_path)":
 * serializes an rds::Device to a file.
 */

#ifndef RDS_HPP
#define RDS_HPP

#include "rds/device.hpp"

#include <memory>

namespace rds {

/**
 * parses a Device from a toml description file.
 *
 * This function can fail in several ways:
 *   - file does not exist
 *   - invalid file type
 *   - invalid structure
 *   - missing required entries
 *   - invalid types of values, e.g. string instead of int etc.
 *   - logically invalid values
 *      - overlapping fields, registers, or groups
 *      - mask overlap in registers
 *      - initial or value does not fit into field/register or overlaps with a
 *        mask.
 * @param file_path path to the register file
 * @param type the kind of files that are allowed
 * @throws std::runtime_error in case of failure
 * @ingroup rds
 * @{
 */
std::unique_ptr<Device> device_from_file(std::string_view file_path);

std::unique_ptr<Device> device_from_file(const std::string &file_path);
/// @}

/**
 * @brief serializes a device to a toml description file
 *
 * @param device the device to serialize
 * @param file_path the path to the file to create
 * @throws std::runtime_error in case of failure
 * @ingroup rds
 * @{
 */
void device_to_file(const Device &device, std::string_view file_path);

void device_to_file(const Device &device, const std::string &file_path);
/// @}

std::string to_string(const Device &device);
} // namespace rds
#endif
