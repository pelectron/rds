project(
  'rds',
  'cpp',
  'c',
  version: '1.0.0',
  default_options: ['cpp_std=c++20'],
  meson_version: '>=1.1',
)

ryu_dep = dependency('ryu')

toml_dep = dependency(
  'tomlplusplus',
  default_options: [
    'build_lib=false',
    'build_tests=false',
    'build_tt=false',
    'build_examples=false',
    'compile_library=false',
  ],
)

json_dep = dependency(
  'nlohmann_json',

)

pugixml_dep = dependency('pugixml')

install_headers('include/rds.hpp')
install_headers(
  'include/rds/common.hpp',
  'include/rds/device.hpp',
  'include/rds/field.hpp',
  'include/rds/group.hpp',
  'include/rds/iterator.hpp',
  'include/rds/register.hpp',
  'include/rds/value.hpp',
  'include/rds/write_constraints.hpp',
  subdir: 'rds',
)

inc = include_directories('include')

src = files(
  'src/detail.cpp',
  'src/device.cpp',
  'src/field.cpp',
  'src/group.cpp',
  'src/rds.cpp',
  'src/register.cpp',
  'src/value.cpp',
)

rds_lib = static_library(
  'rds',
  sources: src,
  include_directories: inc,
  dependencies: [toml_dep, json_dep, pugixml_dep, ryu_dep],
)

rds_dep = declare_dependency(
  link_whole: rds_lib,
  include_directories: inc,
  version: '1.0.0',
)

if (get_option('use_qt'))

  qt6 = import('qt6')
  qt6_dep = dependency('qt6', modules: ['Core', 'Gui', 'Widgets'], method: 'qmake')

  qt_src = [
    files(
      'src/qt/column.cpp',
      'src/qt/common.cpp',
      'src/qt/delegate.cpp',
      'src/qt/device.cpp',
      'src/qt/field.cpp',
      'src/qt/filter.cpp',
      'src/qt/group.cpp',
      'src/qt/io.cpp',
      'src/qt/model.cpp',
      'src/qt/node.cpp',
      'src/qt/register.cpp',
      'src/qt/widget.cpp',
    ),
    qt6.compile_moc(
      headers: [
        'include/rds/qt/common.hpp',
        'include/rds/qt/delegate.hpp',
        'include/rds/qt/device.hpp',
        'include/rds/qt/editors.hpp',
        'include/rds/qt/field.hpp',
        'include/rds/qt/filter.hpp',
        'include/rds/qt/group.hpp',
        'include/rds/qt/io.hpp',
        'include/rds/qt/model.hpp',
        'include/rds/qt/node.hpp',
        'include/rds/qt/register.hpp',
        'include/rds/qt/widget.hpp',
      ],
      include_directories: inc,
      dependencies: [rds_dep, qt6_dep],
    ),
  ]

  qrds_lib = static_library(
    'qrds',
    sources: qt_src,
    dependencies: [rds_dep, qt6_dep, toml_dep, json_dep, pugixml_dep],
  )

  qrds_dep = declare_dependency(
    link_with: qrds_lib,
    include_directories: inc,
    version: '1.0.0',
    dependencies: [rds_dep, qt6_dep],
  )

  install_headers(
    'include/rds/qt/common.hpp',
    'include/rds/qt/device.hpp',
    'include/rds/qt/field.hpp',
    'include/rds/qt/filter.hpp',
    'include/rds/qt/group.hpp',
    'include/rds/qt/model.hpp',
    'include/rds/qt/node.hpp',
    'include/rds/qt/register.hpp',
    'include/rds/qt/widget.hpp',
    subdir: 'rds/qt',
  )

  meson.override_dependency('rds', qrds_dep)

  executable(
    'rds-gui',
    sources: files('src/qt/main.cpp'),
    dependencies: [qrds_dep],
    include_directories: inc,
  )
else
  meson.override_dependency('rds', rds_dep)
endif

executable(
  'rds-cli',
  sources: files('src/main.cpp'),
  dependencies: [rds_dep, toml_dep, json_dep, pugixml_dep],
  include_directories: inc,
)

if (get_option('build_tests'))
  catch2_dep = dependency(
    'catch2',
    default_options: ['tests=false'],
    fallback: ['catch2', 'catch2_with_main_dep'],
  )

  test_src = [
    'tests/test.cpp',
    'tests/device.cpp',
    'tests/field.cpp',
    'tests/group.cpp',
    'tests/register.cpp',
  ]

  executable(
    'tests',
    sources: test_src,
    dependencies: [rds_dep, catch2_dep, toml_dep, json_dep, pugixml_dep],
  )
endif
