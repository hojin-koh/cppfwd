cmake_minimum_required(VERSION 3.14.5)
project (cppfwdv0 LANGUAGES CXX C)

file(GLOB_RECURSE hdrs LIST_DIRECTORIES true CONFIGURE_DEPENDS src/fwdv0/*.h)
file(GLOB_RECURSE srcs LIST_DIRECTORIES true CONFIGURE_DEPENDS src/fwdv0/*.cc)
add_library(${PROJECT_NAME} STATIC)
target_sources(${PROJECT_NAME} INTERFACE ${hdrs} ${srcs})
target_include_directories(${PROJECT_NAME} INTERFACE src)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

## Link sds
include(FetchContent)
FetchContent_Declare(sds GIT_TAG v2.2 GIT_REPOSITORY https://github.com/jcorporation/sds)
FetchContent_MakeAvailable(sds)
FetchContent_GetProperties(sds)
target_sources(${PROJECT_NAME} PRIVATE ${sds_SOURCE_DIR}/sds.c)
target_include_directories(${PROJECT_NAME} PRIVATE ${sds_SOURCE_DIR})
