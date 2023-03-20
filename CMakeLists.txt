cmake_minimum_required(VERSION 3.17)
project(slangc)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(CMAKE_CXX_STANDARD 20)
#DEBUG
message("Building in DEBUG mode.")
set(LLVM-debug_DIR "D:\\Program Files (x86)\\LLVM-debug\\lib\\cmake\\llvm")
find_package(LLVM-debug REQUIRED CONFIG)
#RELEASE
#message("Building in RELEASE mode.")
#set(CMAKE_BUILD_TYPE Release)
#set(LLVM_DIR "D:\\Program Files (x86)\\LLVM\\lib\\cmake\\llvm")
#find_package(LLVM REQUIRED CONFIG)
#add_definitions(-DDEBUG=0)

message("Found LLVM ${LLVM_PACKAGE_VERSION}, build type ${LLVM_BUILD_TYPE}")
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})

add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs support core irreader TransformUtils)

add_executable(slangc main.cpp Lexer.hpp Lexer.cpp TokenType.hpp Logger.hpp Parser.cpp Parser.hpp AST.cpp AST.hpp Scope.hpp)
target_link_libraries(slangc PRIVATE ${llvm_libs})

#DEBUG
#target_compile_definitions(slangc PRIVATE DEBUG=1)
#RELEASE
target_compile_definitions(slangc PRIVATE DEBUG=0)