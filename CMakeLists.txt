cmake_minimum_required(VERSION 3.17)
project(SlangcRefactored)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR})

#DEBUG
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message("Building in DEBUG mode.")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(LLVM-debug_DIR "D:\\Program Files (x86)\\LLVM-debug\\lib\\cmake\\llvm")
    find_package(LLVM-debug REQUIRED CONFIG)
    message(${LLVM_INCLUDE_DIRS})
endif()
#RELEASE
if(CMAKE_BUILD_TYPE MATCHES "Release")
    message("Building in RELEASE mode.")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_BUILD_TYPE Release)
    set(LLVM_DIR "D:\\Program Files (x86)\\LLVM\\lib\\cmake\\llvm")
    find_package(LLVM 13 REQUIRED CONFIG)
    message(${LLVM_INCLUDE_DIRS})
    #add_definitions(-DDEBUG=0)
endif()

message("Found LLVM ${LLVM_PACKAGE_VERSION}, build type ${LLVM_BUILD_TYPE}")
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})

add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
llvm_map_components_to_libnames(llvm_libs support core irreader TransformUtils Analysis Passes x86asmparser x86codegen x86desc x86info linker)

add_executable(SlangcRefactored main.cpp common.hpp source/SourceBuffer.cpp source/SourceBuffer.hpp CompilerOptions.hpp lexer/Lexer.cpp parser/Parser.cpp parser/Parser.hpp parser/ASTBox.hpp parser/AST.hpp codegen/CodeGenContext.hpp codegen/CodeGen.cpp codegen/CodeGen.hpp parser/Scope.hpp)
target_link_libraries(SlangcRefactored PRIVATE ${llvm_libs})

#DEBUG
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(SlangcRefactored PRIVATE DEBUG=0)
endif()
#RELEASE
if(CMAKE_BUILD_TYPE MATCHES "Release")
    target_compile_definitions(SlangcRefactored PRIVATE DEBUG=0)
endif()