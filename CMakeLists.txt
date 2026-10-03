cmake_minimum_required(VERSION 3.25)
project(pong-cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# raylib is resolved from a plain store prefix rather than a pkg-config module
# or a CMake config package, because neither is shipped by nixpkgs. The prefix
# comes from -DRAYLIB_ROOT in `nix build`, or from the environment inside the
# dev shell; if it is set neither way, the default search paths are used.
set(RAYLIB_ROOT
    "$ENV{RAYLIB_ROOT}"
    CACHE PATH "Path to the raylib install prefix")

find_path(
  RAYLIB_INCLUDE_DIR
  NAMES raylib.h
  HINTS "${RAYLIB_ROOT}/include" REQUIRED)

# Link the resolved absolute path, not a bare `raylib`: nix's linker wrapper
# contributes -L but never -rpath, and an unresolvable relative -l leaves the
# binary failing at load time with libraylib.so.6. CMake only auto-records an
# RPATH entry for a library it located itself outside of
# CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES, which /nix/store is not part of.
find_library(
  RAYLIB_LIBRARY
  NAMES raylib
  HINTS "${RAYLIB_ROOT}/lib" REQUIRED)

add_executable(run src/main.cpp)

# nix's cc-wrapper injects every buildInputs include dir into each compile via
# NIX_CFLAGS_COMPILE, so the compiler reports them as implicit and CMake stores
# them in CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES. CMake then treats the matching
# target_include_directories entries as redundant and omits them from the
# compile rule, leaving CXX_INCLUDES empty. The build still works (the wrapper
# re-adds the flags from the environment), but compile_commands.json no longer
# records them, and clangd/clang-tidy read only that file -- so "raylib.h" is
# not found and every raylib symbol and constant is reported as undeclared.
# Re-emit the nix include dirs as explicit flags so they land in the compilation
# database. The paths are read from the compiler itself rather than hardcoded,
# so they follow the flake when it updates. SHELL: is required: without it CMake
# collapses the repeated -isystem flags into one flag followed by bare paths,
# which clang would then treat as input files.
set(RAYLIB_NIX_SYSTEM_INCLUDES "")
foreach(RAYLIB_NIX_INCLUDE_DIR IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
  if(RAYLIB_NIX_INCLUDE_DIR MATCHES "^/nix/store/")
    list(APPEND RAYLIB_NIX_SYSTEM_INCLUDES
         "SHELL:-isystem ${RAYLIB_NIX_INCLUDE_DIR}")
  endif()
endforeach()
target_compile_options(run PRIVATE ${RAYLIB_NIX_SYSTEM_INCLUDES})

target_include_directories(run PRIVATE "${RAYLIB_INCLUDE_DIR}")
target_link_libraries(run PRIVATE "${RAYLIB_LIBRARY}")

# The Ninja generator only emits an `install` target when the project actually
# declares install rules, and nixpkgs' install phase hardcodes `ninja install`.
# Without this the build compiles and links, then dies in the install phase.
install(TARGETS run RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
