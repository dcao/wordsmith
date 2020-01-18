# https://mesonbuild.com/IndepthTutorial.html

project('wordsmith', 'c', version : '0.1.0')

hs = dependency('libhs')
tcc = dependency('libtcc')

inc = include_directories('include')

lib_src = ['src/prose.c', 'src/ext.c', 'src/lint.c', 'src/sink.c', 'src/rule.c']
lib = static_library('ws', lib_src, dependencies : tcc, include_directories : inc, install : true)

contrib_src = ['src/sink.c', 'src/lint/regex.c']
contrib = static_library('ws_contrib',
                         contrib_src,
                         dependencies : hs,
                         include_directories : inc,
                         install : true)

install_headers('include/ws.h')
install_headers('include/ws_contrib.h')

exe = executable('ws',
                 'src/main.c',
                 include_directories : inc,
                 dependencies : hs,
                 link_with : [lib, contrib])

test_bin = executable('ws-test',
                      'test/test.c',
                      include_directories : inc,
                      link_with : [lib, contrib])

test('tests', test_bin)
