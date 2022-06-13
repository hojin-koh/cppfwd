cmake_minimum_required(VERSION 3.14.5)
project(cppfwd-test LANGUAGES CXX)

## Enforce out-of-tree build
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
  message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

## Sanitizers on debug mode
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fanitize=leak")
  set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fanitize=leak")
endif()

## Enforce c++17 usage
set(CMAKE_CXX_STANDARD 17 CACHE INTERNAL "CMAKE_CXX_STANDARD")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE INTERNAL "CMAKE_CXX_STANDARD_REQUIRED") # c++17 is required
set(CMAKE_CXX_EXTENSIONS OFF CACHE INTERNAL "CMAKE_CXX_EXTENSIONS") # use -std=c++* instead of -std=gnu++*

## Force colored output in c compilers
## https://medium.com/@alasher/colored-c-compiler-output-with-ninja-clang-gcc-10bfe7f2b949
if(NOT ALREADY_FORCED_COLORED_OUTPUT)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always" CACHE INTERNAL "CMAKE_CXX_FLAGS")
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics" CACHE INTERNAL "CMAKE_CXX_FLAGS")
  endif()
endif()
set(ALREADY_FORCED_COLORED_OUTPUT TRUE CACHE INTERNAL "ALREADY_FORCED_COLORED_OUTPUT")

## Source file
file(GLOB srcs LIST_DIRECTORIES true CONFIGURE_DEPENDS *.h *.cc */*.cc */*.h)
add_executable(test-program ${srcs} )

## Link the original library
add_subdirectory(.. orig)
target_link_libraries(test-program PRIVATE cppfwdv0)

## Link doctest
include(FetchContent)
FetchContent_Declare(doctest
  URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.8.tar.gz
  )
FetchContent_MakeAvailable(doctest)
target_compile_features(test-program PRIVATE cxx_std_17) # Needed by doctest but actually duplicated with orig
target_compile_definitions(test-program PRIVATE DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING)
target_compile_definitions(test-program PRIVATE DOCTEST_CONFIG_SUPER_FAST_ASSERTS)
target_compile_definitions(test-program PRIVATE DOCTEST_CONFIG_ASSERTION_PARAMETERS_BY_VALUE)
target_compile_definitions(test-program PRIVATE DOCTEST_CONFIG_NO_WINDOWS_SEH)
target_compile_definitions(test-program PRIVATE DOCTEST_CONFIG_NO_POSIX_SIGNALS)
target_compile_definitions(test-program PRIVATE DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS)
target_link_libraries(test-program PRIVATE doctest)

## Link rapidcheck
FetchContent_Declare(rapidcheck
  URL https://github.com/emil-e/rapidcheck/tarball/8fafda42e732164db58003e542196e94a28481f9
  PATCH_COMMAND sed -i "s/CMAKE_CXX_STANDARD 11/CMAKE_CXX_STANDARD 17/" <SOURCE_DIR>/CMakeLists.txt
  )
FetchContent_MakeAvailable(rapidcheck)
target_link_libraries(test-program PRIVATE rapidcheck)

add_custom_target(run COMMAND ${CMAKE_COMMAND} -E env RC_PARAMS="max_size=1001 max_success=333" $<TARGET_FILE:test-program> -fc DEPENDS test-program)


