project('ryu', 'c', version: '2.0')

src = ['ryu/f2s.c', 'ryu/d2s.c', 'ryu/d2fixed.c', 'ryu/s2d.c', 'ryu/s2f.c']
inc = include_directories('.')
ryu_lib = static_library('ryu', sources: src, include_directories: inc)
ryu_dep = declare_dependency(link_with: ryu_lib, include_directories: inc)
meson.override_dependency('ryu', ryu_dep)
