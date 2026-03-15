project(
  'orbit',
  'cpp',
  default_options: [
    'cpp_std=c++23',
  ],
)

#########################################################################
# Library dependency
#########################################################################
orbit_dep = declare_dependency(
    include_directories: include_directories('include')
)

#########################################################################
# Benchmarks and testing. Requires cloning the other benchmarked repos next to this one.
# Could be done with meson wrap but I don't really expect many people to use this.
#########################################################################
if not meson.is_subproject()

add_project_arguments('-O2', language: 'cpp')
add_project_arguments('-march=native', language: 'cpp')
add_project_arguments('-Werror', language: 'cpp')
add_project_arguments('-Wall', language: 'cpp')
add_project_arguments('-Wno-sign-compare', language: 'cpp')

timer = declare_dependency(include_directories: 'benchmarks/include')

boost_dep = dependency('boost')
atomic_queue_dep = declare_dependency(include_directories: '../atomic_queue/include')
xenium_dep = declare_dependency(include_directories: '../xenium')
moodycamel_dep = declare_dependency(include_directories: '../concurrentqueue')
moodycamel_spsc_dep = declare_dependency(include_directories: '../readerwriterqueue')

external_deps = [boost_dep, atomic_queue_dep, xenium_dep, moodycamel_dep, moodycamel_spsc_dep]

include_dirs = ['include', 'benchmarks/include']

executable(
  'test_correctness',
  'tests/test_correctness.cpp',
  dependencies: [
    timer,
    orbit_dep,
  ],
)

executable(
  'test_throughput',
  'benchmarks/test_throughput.cpp',
  dependencies: [
    timer,
    orbit_dep,
  ],
)

executable(
  'test_latency',
  'benchmarks/test_latency.cpp',
  dependencies: [
    timer,
    orbit_dep,
  ],
)

executable(
  'compare_throughput',
  'benchmarks/compare_throughput.cpp',
  dependencies: external_deps
  + [
    timer,
    orbit_dep,
  ],
)

executable(
  'compare_throughput2',
  'benchmarks/compare_throughput2.cpp',
  dependencies: external_deps
  + [
    timer,
    orbit_dep,
  ],
)

executable(
  'compare_latency',
  'benchmarks/compare_latency.cpp',
  dependencies: external_deps
  + [
    timer,
    orbit_dep,
  ],
)

endif