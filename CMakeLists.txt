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

target_include_directories(run PRIVATE "${RAYLIB_INCLUDE_DIR}")
target_link_libraries(run PRIVATE "${RAYLIB_LIBRARY}")

# The Ninja generator only emits an `install` target when the project actually
# declares install rules, and nixpkgs' install phase hardcodes `ninja install`.
# Without this the build compiles and links, then dies in the install phase.
install(TARGETS run RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
