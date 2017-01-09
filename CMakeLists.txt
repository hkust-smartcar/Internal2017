cmake_minimum_required(VERSION 3.6)
project(Internal2017)

include(CMakeForceCompiler)

set(COMPILE_MACHINE Laptop)
set(SCCC_CONFIG 2016_INNO)

if(${COMPILE_MACHINE} STREQUAL Desktop)
    set(LIBSCCC_PATH E:/David/Dropbox/ENGG4950S/libsccc)
else(${COMPILE_MACHINE} STREQUAL Desktop)
    set(LIBSCCC_PATH C:/Users/david/Dropbox/ENGG4950S/libsccc)
endif(${COMPILE_MACHINE} STREQUAL Desktop)

# specify the cross compiler
set(TOOLCHAIN_PREFIX arm-none-eabi-)
set(CC ${TOOLCHAIN_PREFIX}gcc)
set(CXX ${TOOLCHAIN_PREFIX}g++)
set(AR ${TOOLCHAIN_PREFIX}ar)
set(CMAKE_C_COMPILER ${CC})
SET(CMAKE_CXX_COMPILER ${CXX})
CMAKE_FORCE_C_COMPILER(${CC} GNU)
CMAKE_FORCE_CXX_COMPILER(${CXX} GNU)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

include_directories(inc)
include_directories(${LIBSCCC_PATH}/inc)

if(${CMAKE_BUILD_TYPE} STREQUAL Release)
    set(MAKE_BUILD_TYPE RELEASE)
elseif(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set(MAKE_BUILD_TYPE DEBUG)
else(${CMAKE_BUILD_TYPE} STREQUAL Release)
    message(WARNING "No build type configured. Defaulting to Debug")
    set(MAKE_BUILD_TYPE DEBUG)
endif(${CMAKE_BUILD_TYPE} STREQUAL Release)

if(${COMPILE_MACHINE} STREQUAL Desktop)
    set(MAKE_PATH "D:/Program Files (x86)/GnuWin32/bin/make.exe")
else(${COMPILE_MACHINE} STREQUAL Desktop)
    set(MAKE_PATH "C:/Program Files (x86)/GnuWin32/bin/make.exe")
endif(${COMPILE_MACHINE} STREQUAL Desktop)
set(MAKE_ARGS -f ${Internal2017_SOURCE_DIR}/Makefile -k -j4)
set(MAKE_VARS SCCC_BUILD=${MAKE_BUILD_TYPE} SCCC_CONFIG=${SCCC_CONFIG} PWD=${Internal2017_SOURCE_DIR} CWD=${PWD}
        LIBSCCC_BIN_PATH=${LIBSCCC_PATH}/lib
        LIBSCCC_INC_PATH=${LIBSCCC_PATH}/inc)

add_custom_target(make_once
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} all
        WORKING_DIRECTORY ${Internal2017_SOURCE_DIR})
add_custom_target(clean_only
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} clean
        WORKING_DIRECTORY ${Internal2017_SOURCE_DIR})

file(GLOB_RECURSE USER_SOURCES "src/*.cpp")

# for generation of symbols and declarations
add_executable(DoNotBuild ${USER_SOURCES})
