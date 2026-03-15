project(
  'orbit',
  'cpp',
  default_options: [
    'cpp_std=c++23',
  ],
)

add_global_arguments('-O2', language: 'cpp')
add_global_arguments('-march=native', language: 'cpp')
#add_global_arguments('-DNDEBUG', language: 'cpp')
#add_global_arguments('-flto', language: 'cpp')
add_global_arguments('-std=c++23', language: 'cpp')
#add_global_arguments('-g', language: 'cpp')
add_global_arguments('-Werror', language: 'cpp')
add_global_arguments('-Wall', language: 'cpp')
add_global_arguments('-Wno-sign-compare', language: 'cpp')

timer_lib = static_library('timer', 'benchmarks/include/timer.h')
timer = declare_dependency(link_with: timer_lib, include_directories: 'include')

boost_dep = dependency('boost')
atomic_queue_dep = declare_dependency(include_directories: '../atomic_queue', include_directories: '../atomic_queue/include')
xenium_dep = declare_dependency(include_directories: '../xenium', include_directories: '../xenium')
moodycamel_dep = declare_dependency(include_directories: '../concurrentqueue', include_directories: '../concurrentqueue')
moodycamel_spsc_dep = declare_dependency(include_directories: '../readerwriterqueue', include_directories: '../readerwriterqueue')

external_deps = [boost_dep, atomic_queue_dep, xenium_dep, moodycamel_dep, moodycamel_spsc_dep]

include_dirs = ['include', 'benchmarks/include']

executable(
  'test_correctness',
  'tests/test_correctness.cpp',
  include_directories: include_dirs,
  dependencies: [
    timer,
  ],
)

executable(
  'test_throughput',
  'benchmarks/test_throughput.cpp',
  include_directories: include_dirs,
  dependencies: [
    timer,
  ],
)

executable(
  'test_latency',
  'benchmarks/test_latency.cpp',
  include_directories: include_dirs,
  dependencies: [
    timer,
  ],
)

executable(
  'compare_throughput',
  'benchmarks/compare_throughput.cpp',
  include_directories: include_dirs,
  dependencies: external_deps
  + [
    timer,
  ],
)

executable(
  'compare_throughput2',
  'benchmarks/compare_throughput2.cpp',
  include_directories: include_dirs,
  dependencies: external_deps
  + [
    timer,
  ],
)

executable(
  'compare_latency',
  'benchmarks/compare_latency.cpp',
  include_directories: include_dirs,
  dependencies: external_deps
  + [
    timer,
  ],
)