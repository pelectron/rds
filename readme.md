# rds

<!--toc:start-->

- [rds](#rds)
  - [The Problem](#the-problem)
  - [The Solution](#the-solution)
  - [How To Build](#how-to-build)
    - [Building with meson](#building-with-meson)
    - [Custom Build System](#custom-build-system)
      - [Dependencies](#dependencies)
      - [Header Files](#header-files)
      - [Source Files](#source-files)
  - [Register Description Format](#register-description-format)
    - [Device](#device)
      - [Device Properties](#device-properties)
      - [Device TOML Example](#device-toml-example)
    - [Group](#group)
      - [Group Properties](#group-properties)
      - [Group TOML Example](#group-toml-example)
    - [Registers](#registers)
      - [Register Properties](#register-properties)
      - [Register TOML Example](#register-toml-example)
    - [Fields](#fields)
      - [Field Properties](#field-properties)
      - [Field TOML Example](#field-toml-example)
    - [Complete (minimal) TOML Example](#complete-minimal-toml-example)
  - [rds library](#rds-library)
    - [Reading in a file](#reading-in-a-file)
    - [Saving to file](#saving-to-file)
    - [Modifying The Device](#modifying-the-device)
  - [qrds library](#qrds-library) - [qrds Quickstart](#qrds-quickstart)
  <!--toc:end-->

This is a C++ library to read, write and manipulate register description files.

## The Problem

You have developed some kind of of embedded hardware. This hardware may contain
several ICs with a digital interface. The digital interface is usually
implemented in terms of a registers. Writing and/or reading these registers
configures and operates the IC.

The core problem is this: If you don't have a system to describe these
registers, you will reimplement the register map multiple times (of course
depending on the scale of your project and exact use). What I find useful is:

- having one source of truth for the memory/register layout, both structured and
  readable by humans and computers alike. A PDF datasheet itself is not enough.
- using that source of truth to generate code and constants for:
  - C/C++ embedded device drivers, e.g. CMSIS SVD
  - communication interfaces, e.g. for setting and retrieving parameters of your
    embedded system
  - GUI interaction for easy configuration, testing, and board bring up

## The Solution

The solution is **rds**, a register description system.

**rds** provides:

- a human readable file format to store memory layout information (toml, json)
- `rds`: a simple C++ library to read and write rds files
- `qrds`: a Qt based C++ library to read/save rds files and have GUI user
  interaction
- `rds-gui`: a very basic GUI to read, edit, convert and save rds files (`rds-gui`)
- `rds-cli`: a very basic cli application to convert from/to json and toml (`rds-cli`)

## How To Build

To build **rds**, either use meson, or integrate it into your build system.

### Building with meson

There are two project options that can be set:

- _build_tests_: if true, a test executable will also be built(requires Catch2
  V3). Default is false.
- _use_qt_: if true, _qrds_ will also be built. This
  requires a Qt 6 installation. The default is false.

To build `rds-cli` and the `rds` library, simply setup and build the project
like any meson project, i.e.

```{bash}
meson setup build --buildtype=release && meson compile -C build
```

To also build `rds-gui` and the `qrds` library, _use_qt_ has to be set to true.

```{bash}
meson setup build --buildtype=release -Duse_qt=true && meson compile -C build
```

To use **rds** as a subproject, either add **rds** directly into your
subproject folder or use a meson wrap file. You can then use `rds` just like
any normal dependency/subproject, i.e. in your meson.build file, add the
following line

```{meson}
rds_dep = dependency('rds', default_options:['build_tests=false', 'use_qt=false'])
```

Depending on the value of _use_qt_, _rds_dep_ will now either provide the
static library `rds` (_use_qt_ = false) or `qrds` (_use_qt_ = true).

`rds` can also be found as a subproject variable with the name `rds_dep`.
`qrds` can also be found as a subproject variable with the name `qrds_dep`.

### Custom Build System

Using **rds** with a new build system is relatively simple.
The easiest way, if you are familiar with meson files, is to simply translate
this project's `meson.build` file to your favorite build system. If that is not
the case, what follows are all details needed to build `rds` and `qrds`.

#### Dependencies

There are two dependencies that need to be resolved:

- [toml++](https://marzer.github.io/tomlplusplus/)
- [nlohmann/json](https://github.com/nlohmann/json)

Both of these dependencies build with either meson, cmake, or some other common
build system, or are often available through a system package manager. You
should be able to get these dependencies with ease.

To build `qrds`, a [Qt 6](https://www.qt.io/product/qt6) installation is also
required.

#### Header Files

All header files are located in the `include` directory. This is the only
"include directory" you need to make available to your compiler/build system.

To use `rds` as a library, the following headers have to be available:

- `rds.hpp`
- `rds/common.hpp`
- `rds/device.hpp`
- `rds/field.hpp`
- `rds/group.hpp`
- `rds/register.hpp`

`rds/util.hpp` is a private header an should not be used by a consumer of the library.

The subdirectory `rds/qt` is not used by `rds`.

To use `qrds`, all the headers in `rds/qt` also have to be available in
addition to `rds`'s headers. The headers in `rds/qt` also have to be run
through Qt's moc.

#### Source Files

The source files for `rds` are located in the `src` directory. To build the
`rds` library, compile all files located directly in `src` except for
`src/main.cpp` (i.e. ignore `src/qt`) and link them into a static
library (together with _toml++_ and _nlohmann/json_).

To build `rds-cli`, `src/main.cpp` also has to be compiled and linked against
`rds`.

To compile `qrds`, the files in `src/qt` also have to be compiled (except for
`src/qt/main.cpp`) and linked into a static library, along with `rds`.

To build `rds-gui`, `srd/qt/main.cpp` also has to be compiled and linked
against `qrds`.

## Register Description Format

rds divides an IC up into multiple parts:

- [device](#device): the IC which contains the memory/registers to be described
  itself. A device is made up of one or more groups.
- [groups](#group): a device has one or more memory groups.
- [registers](#registers): each group contains one or more registers.
- [fields](#fields): a register can be made up of zero or more fields.

This whole construction can be seen as a tree, with the device as the root item.
Individual items in the tree are called **nodes**.
What follows is the description of this tree. For nomenclature and clarification:

- **properties** are named children of a node. In toml, this would be
  a key value pair in a table, for json that would be a name value pair in an
  object
- **virtual properties** of a node are special properties that do not exist as
  children in a node, but are implied some other way. This applies to:
  - the **name** property. It is not present in any group, register, or field,
    but implied by the key/name it is tied to.
  - virtual maps: Again there is no actual child with the name of the map
    present. The elements of the map are present as direct children of the node
    that contains the map.

### Device

The device is the top level/root node. It contains the basic information about
the device, as well as a virtual map of [groups](#group).

#### Device Properties

- **name**\[_string_\]: the name of the device
- **version**\[_integer_\]: the rds version, must be 1
- **register_width**\[_integer_\]: this is the width in bits of one addressable
  unit. For most devices, this should be set to 8, i.e. when each individual
  address corresponds to one byte. When one address represents another amount of
  bits, set **register_width** accordingly, i.e. an I2C slave device which returns
  16 bits of information for each address should have this set to 16.
- **num_pages**\[_integer_\]: the number of pages in the device memory.
- **registers_per_page**\[_integer_\]: the number of registers per page.
- **endian**\[_Endian_|_string_\]: the memory endian, either "little" or "big"
- **groups**\[_Group..._\]: a virtual map of one or more [groups](#group) that
  make up the memory. Note: **groups** is a "virtual"" map, i.e. there is no
  actual "groups" property (which in toml language would be an array of tables),
  the groups are simply members of its devices table with the name of the group
  used as the name of the group table.
- **any extra data**\[_bool_|_int_|_double_|_string_\]: any extra key value
  pairs can be present that act as either application extra information or default
  values for all groups, registers, and fields to inherit.

The total size of the memory in bits is calculated as follows:

size$_{bits}$ = **register_width** _**num_pages**_ **registers_per_page**

#### Device TOML Example

```{.toml}
version = 1
name = 'MAX17320'
register_width = 16 # 16 bit registers
num_pages = 32 # this chip has 32 pages
registers_per_page = 16 # with each page having 16 registers each
# [Groups]..
```

### Group

A group represents a continuous region of memory.

#### Group Properties

- **name**\[_string_\]: the name of the group
- **base_addr** \[_integer_\]: the base address of the group, i.e. the start address.
- **size** \[_integer_\]: the size of the group in terms of register_width
- **defaults** \[_map of string to \_bool_|_int_|_double_|\_string\_\_\]: a map
  of default properties, which registers and fields can inherit. Example: if
  \*\*defaults\*\* contains an item with the key "size" and a value of 4, all
  registers within that group will have a default size of 4 unless a register
  explicitly overrides this property.
- **registers** \[_Register..._\]: a virtual map of one or more
  [registers](#registers) that make up the groups memory region. Note:
  **registers** is a "virtual"" map, i.e. there is no actual "registers" property
  (which in toml language would be an array of tables), the registers are simply
  members of its groups table with the name of the register used as the name of
  the register table.
- **any extra data**\[_bool_|_int_|_double_|_string_\]: any extra key value
  pairs can be present that act as either application extra information or default
  values for all registers and fields to inherit.

#### Group TOML Example

Below is an example group called "Registers" with a base_addr of 0 and size of 512

```{.toml}
[Registers]
base_addr = 0
size = 512
# [Registers]...
```

### Registers

Registers are continuous regions of memory of one or more **register_width**s
in size.

#### Register Properties

- **name**\[_string_\]: the name of the register
- **display_name**\[_string_\]: an optional, more elaborate version of **name**.
  This can be set by explicitly adding a property with the key "name" in the
  register.
- **description**\[_string_\]: an optional description of the register
- **backup**\[_string_\]: an optional string to describe how the register is
  backed up
- **unit**\[_string_\]: an optional string to describe the unit this register
  represents
- **zero_code_value**\[_floating point_\]: the value in units that corresponds
  to a register value of 0. Default is 0.
- **step**\[_floating point_\]: the step in units for one lsb. Default is 1.
- **initial**\[_integer_\]: the inital/reset value of the register.
- **value**\[_integer_\]: the value of the register. Default is the value of
  **initial**.
- **addr**\[_integer_\]: the address of the register relative to its group
  base_addr.
- **size**\[_integer_\]: the size of the register in terms of register_width.
  Defaults is 1.
- **zeros_mask**\[_integer_\]: a mask where the ones mark locations in the
  register that have to be kept at 0. Default is 0.
- **ones_mask**\[_integer_\]: a mask where the ones mark locations in the
  register that have to be kept at 1. Default is 0.
- **x_mask**\[_integer_\]: a mask where the ones mark locations in the register
  that are ignored ("don't care"). Default is 0.
- **access**\[_Access_\]: the access permissions of this register.
- **signed**\[_bool_\]: If true, then the register value is interpreted as
  signed. Default is false.
- **values** \[_map of Value_\]: an optional map of enumerated values
  (**Value**).
- **fields** \[_Field..._\]: a virtual map of one or more fields.
- **any extra data**\[_bool_|_int_|_double_|_string_\]: any extra key value
  pairs can be present that act as either application extra information or default
  values for all registers and fields to inherit.

The value of the properties **initial** and **value** can be specified in two
ways:

1. just set the **initial** and optionally the **value** property in the
   register. This way, the **initial** and **value** properties can be left out
   of the registers' fields as they are implied.
2. leave the **initial** and **value** properties out of the register. In this
   case, every field of the register must have an **initial** property (and
   optionally a **value** as well).

#### Register TOML Example

Below is an example register called "Status" in the group "Registers"

```{.toml}
[Registers.Status]
addr = 0x000
initial = 0x0002
x_mask = 0x839
description = '...'
# [Fields]...
```

### Fields

Fields are a continuous region of bits in a register.

#### Field Properties

- **name**\[_string_\]: the name of the field
- **display_name**\[_string_\]: an optional, more elaborate version of **name**.
  This can be set by explicitly adding a property with the key "name" in the
  field.
- **description**\[_string_\]: an optional description of the field
- **backup**\[_string_\]: an optional string to describe how the field is backed
  up
- **unit**\[_string_\]: an optional string to describe the unit this field
  represents
- **zero_code_value**\[_floating point_\]: the value in units that corresponds
  to a field value of 0. Default is 0.
- **step**\[_floating point_\]: the step in units for one lsb. Default is 1.
- **initial**\[_integer_\]: the inital/reset value of the field.
- **value**\[_integer_\]: the value of the register. Default is **initial**.
- **msb**\[_integer_\]: the most significant bit of the field. This is 0 based,
  i.e. the first bit has position 0.
- **lsb**\[_integer_\]: the least significant bit of the field. This is 0 based,
  i.e. the first bit has position 0.
- **position**\[_integer_\]: for single bit fields, the **position** property
  can be set to the fields position in the register, instead of specifying **lsb**
  and **msb** properties.
- **access**\[_Access_\]: the access permissions of this field.
- **signed**\[_bool_\]: If true, then the register value is interpreted as
  signed. Default is false.
- **values** \[_map of Value_\]: an optional map of enumerated values
  (**Value**).
- **any extra data**\[_bool_|_int_|_double_|_string_\]: any extra key value
  pairs can be present that act as either application extra information or default
  values for all registers and fields to inherit.

The **initial** and **value** properties only have to be specified if they are
not present in the parent register. In case they are specified in the field

#### Field TOML Example

Below is an example field called "PA" in register "Status" in the group
"Registers".

```{.toml}
[Registers.Status.PA]
position = 15
name = 'Protection Alert'
description = '...'
```

### Complete (minimal) TOML Example

```{.toml}
version = 1
name = 'MAX17320'
register_width = 16 # 16 bit registers
num_pages = 32 # this chip has 32 pages
registers_per_page = 16 # with each page having 16 registers each

[Registers]
  base_addr = 0
  size = 512

  [Registers.Status]
    addr = 0x000
    initial = 0x0002
    x_mask = 0x839
    description = '...'

    [Registers.Status.PA]
      position = 15
      name = 'Protection Alert'
      description = '...'
```

## rds library

The C++ API consists of two main functions, along with a couple of types.
Everything is contained in the namespace `rds`.

Note: because C++ is statically typed, the virtual maps described
[above](#register-description-format) are actually `std::map` or
`std::vector<std::unique_ptr<...>>` data members.

The types the rds library provides are:

- `rds::Device`
- `rds::Group`
- `rds::Register`
- `rds::Field`
- `rds::Value`

### Reading in a file

To load a device file, use

```{cpp}
std::unique_ptr<rds::Device> rds::device_from_file(std::string_view file_path);
```

This will return a unique_ptr to a `rds::Device`, or throw a `std::runtime_error`
if the call fails.

### Saving to file

To save a `rds::Device` to file, use

```{cpp}
void rds::device_to_file(const Device &device, std::string_view file_path)
```

This will throw a `std::runtime_error` is case of failure.

### Modifying The Device

To modify the device, i.e. add and remove groups, registers, and fields, use the
corresponding methods on the device, group, or register. The methods will do
checks and link up the tree correctly.

If you want to leave out the checks because of your particular use case, feel free
to do so. Just keep in mind to set the right parent device, group, or register
for each group, register, and field that is added.

## qrds library

`qrds` is a Qt wrapper around `rds`. It adds display capabilities and ties in
`rds` with Qt's property system and Model/View architecture.

`qrds` provides:

- `rds::Widget`: this is the main QWidget to be used to display an
  `rds::Device`. It manages everything and can be used as a starting point for
  creating your own rds viewer.
- `rds::QModel`: a Qt Model for an `rds::QDevice`
- `rds::QDevice`: the Qt version of `rds::Device`
- `rds::QGroup`: the Qt version of `rds::Group`
- `rds::QRegister`: the Qt version of `rds::Register`
- `rds::QField`: the Qt version of `rds::Field`
- `rds::Column`: describes how a property is displayed in an `rds::Widget`
- `rds::IO`: an extension point for `Widget` to add IO capabilities.

### qrds Quickstart

```{cpp}
#include "rds/qt/widget.hpp"

#include <QApplication>

using namespace rds;

int main(int argc, char **argv) {
  QApplication app(argc, argv);

  rds::Widget rds_viewer{rds::Options::All};

  rds_viewer.show();

  return app.exec();
}
```
