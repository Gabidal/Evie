# -----------------------------------------------------------------------------
# evie Project Build Configuration
# -----------------------------------------------------------------------------
# This is the Meson build configuration for the evie project. It defines the
# project settings, source files, compilation flags, and platform-specific 
# configurations. The script supports different build types via Meson's
# standard `buildtype` option (set at meson setup time).
# -----------------------------------------------------------------------------

project(
  'evie', 'cpp',
  version : '0.1',
  default_options : [
    'warning_level=3',
    'cpp_std=c++17'
    # , 'b_sanitize=address,undefined',     # Enable this to check if there are any UB
  ]
)

# Compiler handle
cppc = meson.get_compiler('cpp')
is_cross = meson.is_cross_build()   # This is to stop infinite recursion, since cross-platform will start meson but with other host OS, which could trigger second cross-platform

# Source files for the project
lib_sources = [
  # Core utilities
  '../src/docker/docker.cpp',
  '../src/lexer/lexer.cpp',
  '../src/parser/parser.cpp',
]

# Use Meson's buildtype instead of a custom TYPE variable
build_type = get_option('buildtype')

# Common C++ warning flags (filtered per compiler)
base_warn_flags = [
  '-Wno-narrowing',              # Disable narrowing warnings
  '-Wshadow',
  '-Wdangling-reference',        # Not supported by some MinGW toolchains
  '-Wformat=2',
  '-Wnon-virtual-dtor',
  '-Wcast-align',
  '-Wunused',
  '-Wmisleading-indentation',
  '-Wduplicated-cond',
  '-Wduplicated-branches',
  '-Wlogical-op',
  '-Wnull-dereference',
  '-Wuseless-cast',
  '-Wdouble-promotion',
  '-Wimplicit-fallthrough',
]

# cross-compilers don't always have full argument coverage, so lets disable those that the cross compiler doesn't have.
cpp_args = []
foreach f : base_warn_flags
  if cppc.has_argument(f)
    cpp_args += [f]
  endif
endforeach

# Linker arguments (collected project-wide)
link_args = []

# Add custom flags based on Meson's buildtype
if build_type == 'profile'
  cpp_args += ['-Devie_DEBUG', '-Og']
  link_args += ['-static']  # Link all libc and other libraries statically for maximum inline and true profiling.

elif build_type == 'debug'
  cpp_args += ['-Devie_DEBUG']  # no flto nor -Og since debugging becomes an hassle with weird line skipping.

  if host_machine.system() == 'windows'
    link_args += ['-lDbgHelp', '-Wl,--export-all-symbols']    # Link with DbgHelp library and export all symbols for stacktrace
  elif host_machine.system() == 'linux'
    cpp_args += ['-rdynamic']   # Export all symbols for stacktrace
  endif

elif build_type == 'release'
  cpp_args += ['-Devie_RELEASE']

  # NOTE: -static doesn't react similar to how it behaves in linux.
  link_args += ['-static'] # Link all libc and other libraries statically for maximum inline.

  # Avoid -march=native for cross builds (e.g., MinGW)
  if not is_cross and cppc.has_argument('-march=native')
    cpp_args += ['-march=native']
  endif
  if cppc.has_argument('-flto')
    cpp_args += ['-flto']
    link_args += ['-flto']
  endif
  if cppc.has_argument('-fwhole-program')
    cpp_args += ['-fwhole-program']
    link_args += ['-fwhole-program']
  endif
endif

# Apply computed flags project-wide so individual targets don't need to repeat them
if cpp_args.length() > 0
  add_project_arguments(cpp_args, language: 'cpp')
endif
if link_args.length() > 0
  add_project_link_arguments(link_args, language: 'cpp')
endif

# Path to include directory
ggui_inc = include_directories('../external/ggui')

# Discover the prebuilt static library from the repo root
ggui_lib_dir = join_paths(meson.current_source_dir(), '..', 'external', 'ggui')
ggui_lib = cppc.find_library('ggui', dirs: [ggui_lib_dir], required: true, static: true)
ggui_dep = declare_dependency(
  include_directories: ggui_inc,
  dependencies: [ggui_lib]
)

# Build the core static library
evie_core = static_library(
  'evie_core',
  lib_sources,
  dependencies: ggui_dep,
  install: false
)

# Build the main demo executable linked with the core library
executable(
  'evie',
  '../src/main.cpp',
  link_with: evie_core,
  install: true
)

# Test target: link with exported lib and include exported header
tester = executable(
  'tester',
  [
    '../test/tester.cpp'
  ],
  dependencies: ggui_dep,
  link_with: evie_core,
  install: false
)

# Run the tester to see if there are any problems
test('evie unit tests', tester)