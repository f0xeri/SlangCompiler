cmake_minimum_required(VERSION 3.20)
project(slangParser)

set(CMAKE_CXX_STANDARD 20)

find_package(LLVM REQUIRED CONFIG)
message("Found LLVM ${LLVM_PACKAGE_VERSION}, build type ${LLVM_BUILD_TYPE}")
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})

add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs Core support)

add_executable(slangParser main.cpp Lexer.hpp Lexer.cpp TokenType.hpp Logger.hpp Parser.cpp Parser.hpp AST.cpp AST.hpp Scope.hpp)
target_link_libraries(slangParser PRIVATE ${llvm_libs})