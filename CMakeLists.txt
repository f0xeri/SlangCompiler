cmake_minimum_required(VERSION 3.17)
project(slangc)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_SOURCE_DIR})

#DEBUG
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    message("Building in DEBUG mode.")
    #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set(LLVM_DIR "D:\\llvm17debug\\lib\\cmake\\llvm")
    #set(LLVM_DIR "C:\\llvm-18-debug\\lib\\cmake\\llvm")
    find_package(LLVM REQUIRED CONFIG)
    message(${LLVM_INCLUDE_DIRS})
endif()
#RELEASE
if(CMAKE_BUILD_TYPE MATCHES "Release")
    message("Building in RELEASE mode.")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(CMAKE_BUILD_TYPE Release)
    set(LLVM_DIR "D:\\Program Files (x86)\\LLVM\\lib\\cmake\\llvm")
    #set(LLVM_DIR "C:\\llvm-18-release\\lib\\cmake\\llvm")
    find_package(LLVM REQUIRED CONFIG)
    message(${LLVM_INCLUDE_DIRS})
    #add_definitions(-DDEBUG=0)
endif()

message("Found LLVM ${LLVM_PACKAGE_VERSION}, build type ${LLVM_BUILD_TYPE}")
list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})

add_definitions(${LLVM_DEFINITIONS})
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

foreach(target ${LLVM_TARGETS_TO_BUILD})
    if (TARGET "LLVM${target}CodeGen")
        list(APPEND targets "${target}CodeGen")
    endif()
    if (TARGET "LLVM${target}AsmParser")
        list(APPEND targets "${target}AsmParser")
    endif()
    if (TARGET "LLVM${target}Desc")
        list(APPEND targets "${target}Desc")
    endif()
    if (TARGET "LLVM${target}Info")
        list(APPEND targets "${target}Info")
    endif()
endforeach()

# only x86
#list(APPEND targets "X86CodeGen" "X86AsmParser" "X86Desc" "X86Info")
llvm_map_components_to_libnames(llvm_libs support core irreader TransformUtils Analysis Passes ${targets} linker)

add_executable(slangc main.cpp common.hpp source/SourceBuffer.cpp source/SourceBuffer.hpp CompilerOptions.hpp lexer/Lexer.cpp parser/Parser.cpp parser/Parser.hpp parser/AST.hpp codegen/CodeGenContext.hpp codegen/CodeGen.cpp codegen/CodeGen.hpp parser/Scope.hpp parser/ParseExpr.cpp parser/ParseDecl.cpp parser/ParseStmt.cpp parser/AST.cpp check/Context.cpp check/Context.hpp parser/ASTFwdDecl.hpp
        check/Check.cpp
        check/Check.hpp
        check/CheckStmt.cpp
        check/CheckExpr.cpp
        check/CheckDecl.cpp
        check/SymbolTable.hpp
        codegen/CodeGenHelpers.cpp
        driver/Driver.cpp
        driver/Driver.hpp
        log/Logger.hpp
        opt/Optimizer.cpp
        opt/Optimizer.hpp
        codegen/DebugInfoBuilder.cpp
        codegen/DebugInfoBuilder.hpp
)
target_link_libraries(slangc PRIVATE ${llvm_libs})

#DEBUG
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(slangc PRIVATE DEBUG=0)
endif()
#RELEASE
if(CMAKE_BUILD_TYPE MATCHES "Release")
    target_compile_definitions(slangc PRIVATE DEBUG=0)
endif()