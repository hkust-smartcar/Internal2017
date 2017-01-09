cmake_minimum_required(VERSION 3.6)
project(Internal2017)

include(CMakeForceCompiler)

# specific what we're working with
set(COMPILE_MACHINE Desktop)

# specify the cross compiler
set(TOOLCHAIN_PREFIX arm-none-eabi-)
set(CC ${TOOLCHAIN_PREFIX}gcc)
set(CXX ${TOOLCHAIN_PREFIX}g++)
set(AR ${TOOLCHAIN_PREFIX}ar)
set(CMAKE_C_COMPILER ${CC})
SET(CMAKE_CXX_COMPILER ${CXX})
CMAKE_FORCE_C_COMPILER(${CC} GNU)
CMAKE_FORCE_CXX_COMPILER(${CXX} GNU)

# find libsccc
if(${COMPILE_MACHINE} STREQUAL Desktop)
    set(LIBSCCC_PATH E:/David/Dropbox/ENGG4950S/libsccc)
else(${COMPILE_MACHINE} STREQUAL Desktop)
    set(LIBSCCC_PATH C:/Users/david/Dropbox/ENGG4950S/libsccc)
endif(${COMPILE_MACHINE} STREQUAL Desktop)

# define the c++ standard and includes to resolve *.h
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
include_directories(inc)
include_directories(${LIBSCCC_PATH}/inc)

# set build type according to what we have in CLion
if(${CMAKE_BUILD_TYPE} STREQUAL Release)
    set(MAKE_BUILD_TYPE RELEASE)
elseif(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    set(MAKE_BUILD_TYPE DEBUG)
else(${CMAKE_BUILD_TYPE} STREQUAL Release)
    message(WARNING "No build type configured. Defaulting to Debug")
    set(MAKE_BUILD_TYPE DEBUG)
endif(${CMAKE_BUILD_TYPE} STREQUAL Release)

# set make parameters
if(${COMPILE_MACHINE} STREQUAL Desktop)
    set(MAKE_PATH "D:/Program Files (x86)/GnuWin32/bin/make.exe")
else(${COMPILE_MACHINE} STREQUAL Desktop)
    set(MAKE_PATH "C:/Program Files (x86)/GnuWin32/bin/make.exe")
endif(${COMPILE_MACHINE} STREQUAL Desktop)
set(MAKE_ARGS -f ${Internal2017_SOURCE_DIR}/Makefile -k -j4)
set(MAKE_VARS SCCC_BUILD=${MAKE_BUILD_TYPE} PWD=${Internal2017_SOURCE_DIR} CWD=${PWD}
        LIBSCCC_BIN_PATH=${LIBSCCC_PATH}/lib
        LIBSCCC_INC_PATH=${LIBSCCC_PATH}/inc)

# buiders
add_custom_target(make_once
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} SCCC_CONFIG=${SCCC_CONFIG} all
        WORKING_DIRECTORY ${Internal2017_SOURCE_DIR})
add_custom_target(2016_INNO_make
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} SCCC_CONFIG=2016_INNO all
        WORKING_DIRECTORY ${Internal2017_SOURCE_DIR})
add_custom_target(VCAN_FX15DEV_make
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} SCCC_CONFIG=VCAN_FX15DEV all
        WORKING_DIRECTORY ${Internal2017_SOURCE_DIR})

# clean-then-builders
add_custom_target(clean_and_make
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} SCCC_CONFIG=${SCCC_CONFIG} clean
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} SCCC_CONFIG=${SCCC_CONFIG} all
        WORKING_DIRECTORY ${Internal2017_SOURCE_DIR})
add_custom_target(2016_INNO_clean_and_make
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} SCCC_CONFIG=2016_INNO clean
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} SCCC_CONFIG=2016_INNO all
        WORKING_DIRECTORY ${Internal2017_SOURCE_DIR})
add_custom_target(VCAN_FX15DEV_clean_and_make
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} SCCC_CONFIG=VCAN_FX15DEV clean
        COMMAND ${MAKE_PATH} ${MAKE_ARGS} ${MAKE_VARS} SCCC_CONFIG=VCAN_FX15DEV all
        WORKING_DIRECTORY ${Internal2017_SOURCE_DIR})

# for generation of symbols and declarations
file(GLOB_RECURSE USER_SOURCES "src/*.cpp")
add_executable(DoNotBuild ${USER_SOURCES})
