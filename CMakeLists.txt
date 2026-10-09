cmake_minimum_required(VERSION 3.25)
project(pong-cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RAYLIB_ROOT
    "$ENV{RAYLIB_ROOT}"
    CACHE PATH "Path to the raylib install prefix")

find_path(
  RAYLIB_INCLUDE_DIR
  NAMES raylib.h
  HINTS "${RAYLIB_ROOT}/include" REQUIRED)

find_library(
  RAYLIB_LIBRARY
  NAMES raylib
  HINTS "${RAYLIB_ROOT}/lib" REQUIRED)

add_executable(run src/main.cpp)
add_executable(server src/server/server.cpp)
add_executable(client src/client/client.cpp)
add_executable(udpserver src/server/udp-server.cpp)
add_executable(udpclient src/client/udp-client.cpp)

target_include_directories(server PUBLIC src/async-sockets/include)
target_include_directories(client PUBLIC src/async-sockets/include)
target_include_directories(udpserver PUBLIC src/async-sockets/include)
target_include_directories(udpclient PUBLIC src/async-sockets/include)

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

install(TARGETS run RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

set(RAYLIB_NIX_SYSTEM_INCLUDES "")
foreach(RAYLIB_NIX_INCLUDE_DIR IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
  if(RAYLIB_NIX_INCLUDE_DIR MATCHES "^/nix/store/")
    list(APPEND RAYLIB_NIX_SYSTEM_INCLUDES
         "SHELL:-isystem ${RAYLIB_NIX_INCLUDE_DIR}")
  endif()
endforeach()
target_compile_options(server PRIVATE ${RAYLIB_NIX_SYSTEM_INCLUDES})

target_include_directories(server PRIVATE "${RAYLIB_INCLUDE_DIR}")
target_link_libraries(server PRIVATE "${RAYLIB_LIBRARY}")

install(TARGETS server RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

set(RAYLIB_NIX_SYSTEM_INCLUDES "")
foreach(RAYLIB_NIX_INCLUDE_DIR IN LISTS CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES)
  if(RAYLIB_NIX_INCLUDE_DIR MATCHES "^/nix/store/")
    list(APPEND RAYLIB_NIX_SYSTEM_INCLUDES
         "SHELL:-isystem ${RAYLIB_NIX_INCLUDE_DIR}")
  endif()
endforeach()
target_compile_options(client PRIVATE ${RAYLIB_NIX_SYSTEM_INCLUDES})

target_include_directories(run PRIVATE "${RAYLIB_INCLUDE_DIR}")
target_link_libraries(client PRIVATE "${RAYLIB_LIBRARY}")

install(TARGETS client RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
