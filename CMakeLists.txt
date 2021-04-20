cmake_minimum_required(VERSION 3.16.0)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

function(build_libff)
  set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
  set(WITH_PROCPS Off)
  set(CMAKE_POSITION_INDEPENDENT_CODE true)
  add_compile_options(-Wno-error)
  add_subdirectory(depends/libff)
endfunction()

build_libff()

project(CryptoUtils)

# detect operating system
message(STATUS "We are on a ${CMAKE_SYSTEM_NAME} system")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
#
# check dependencies
#
find_package(PkgConfig REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(Boost COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

#TODO: upgrade to OpenSSL 1.1.1a
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# customize the flags for RELWITHDEBINFO
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb -DNDEBUG")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# compiler and linker options
add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-Wextra)
add_compile_options(-Wpedantic)

if (THREAD_SANITIZER AND ADDRESS_SANITIZER)
    message(FATAL_ERROR "Cannot use ThreadSanitizer (THREAD_SANITIZER=ON) and AddressSanitizer (ADDRESS_SANITIZER=ON) at the same time")
endif()

if (THREAD_SANITIZER)
    add_compile_options(-fsanitize=thread)
    link_libraries(-fsanitize=thread)
    message(STATUS "THREAD SANITIZER enabled")
endif()

if (ADDRESS_SANITIZER)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fno-omit-frame-pointer)
    link_libraries(-fsanitize=address)
    message(STATUS "ADDRESS SANITIZER enabled")
endif()

if (ENABLE_COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
    if (NOT TESTS)
        message(FATAL_ERROR "TESTS is not ON")
    endif()
    include(CodeCoverage)
    add_compile_options(--coverage)
    link_libraries(--coverage)
    add_custom_target(ctest COMMAND ${CMAKE_CTEST_COMMAND})
    # TODO: remove the hardcoded number in -j option
    setup_target_for_coverage(${PROJECT_NAME}_coverage ctest coverage "-j2;--output-on-failure")
endif()

# Build shared libraries by default.
option(CRYPTOUTILS_BUILD_ARCHIVE "Build static library (archive)"
       Off)
if (NOT CRYPTOUTILS_BUILD_ARCHIVE)
    message(STATUS "Building shared library ...")
    add_library(CryptoUtils SHARED)
else()
    message(STATUS "Building archive ...")
    add_library(CryptoUtils STATIC)
    set_target_properties(CryptoUtils PROPERTIES POSITION_INDEPENDENT_CODE True)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(lib/ethash)
add_subdirectory(lib/Snark)
add_subdirectory(lib/Bech32)

install(
    TARGETS CryptoUtils
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include
    DESTINATION ${CMAKE_INSTALL_PREFIX}
    USE_SOURCE_PERMISSIONS
)

# add clang-format and clang-tidy targets lastly
#if(LLVM_EXTRA_TOOLS)
#    include(LLVMExtraTools)
#endif()

include(CPack)
