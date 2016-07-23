cmake_minimum_required(VERSION 3.0)
project(drunner)

file(GLOB_RECURSE sources *.cpp *.h *.hpp)

include_directories(source buildnum source/tests deps source/plugins deps/yaml-cpp/include)

# add the data to the target, so it becomes visible in some IDE
add_executable(drunner ${sources})

target_compile_options(drunner PUBLIC
 -DVERSION_STR=\"$ENV{VCOMBO}\" -DVERSION_STR_SHORT=\"$ENV{VSHORT}\" -fdiagnostics-color
 -Wall -Wno-unknown-pragmas -static-libgcc -std=c++11
 )

target_link_libraries (drunner PUBLIC
   PocoFoundation
   PocoUtil
 )
