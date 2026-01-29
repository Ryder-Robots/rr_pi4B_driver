cmake_minimum_required(VERSION 3.8)
project(motor_tests)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

################################################################################
# Find the pigpio shared libraries
################################################################################

# Find the path to the pigpio includes
find_path(pigpio_INCLUDE_DIR 
  NAMES pigpio.h pigpiod_if.h pigpiod_if2.h
  HINTS /usr/local/include)

# Find the pigpio libraries
find_library(pigpio_LIBRARY 
  NAMES libpigpio.so pigpio
  HINTS /usr/local/lib)

find_library(pigpiod_if_LIBRARY 
  NAMES libpigpiod_if.so pigpiod_if
  HINTS /usr/local/lib)

find_library(pigpiod_if2_LIBRARY 
  NAMES libpigpiod_if2.so pigpiod_if2
  HINTS /usr/local/lib)

# Set the pigpio variables to plural form
set(pigpio_INCLUDE_DIRS ${pigpio_INCLUDE_DIR})
set(pigpio_INCLUDES     ${pigpio_INCLUDE_DIR})
set(pigpio_LIBRARIES    ${pigpio_LIBRARY} ${pigpiod_if_LIBRARY} ${pigpiod_if2_LIBRARY})

# Handle REQUIRED, QUIET, and version arguments 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pigpio 
  DEFAULT_MSG 
  pigpio_INCLUDE_DIR 
  pigpio_LIBRARY 
  pigpiod_if_LIBRARY 
  pigpiod_if2_LIBRARY)

# Print pigpio status
if(pigpio_FOUND)
  message(STATUS "Found pigpio:")
  message(STATUS "  Include dir: ${pigpio_INCLUDE_DIR}")
  message(STATUS "  Libraries: ${pigpio_LIBRARIES}")
else()
  message(FATAL_ERROR "pigpio not found! Install with:\n"
    "  cd ~ && git clone https://github.com/joan2937/pigpio.git\n"
    "  cd pigpio && make && sudo make install")
endif()

################################################################################
# Include directories
################################################################################

include_directories(${pigpio_INCLUDE_DIRS})

################################################################################
# Build tst_motor_cntl executable
################################################################################

add_executable(tst_motor_cntl
    src/tst_motor_cntl.cpp
)

target_link_libraries(tst_motor_cntl
  ${pigpio_LIBRARIES}
  pthread
  rt
)

################################################################################
# Build tst_motor_enc executable
################################################################################

add_executable(tst_motor_enc
  src/tst_motor_enc.cpp
)
target_compile_options(tst_motor_enc PRIVATE -Wimplicit-fallthrough)

target_link_libraries(tst_motor_enc
  ${pigpio_LIBRARIES}
  pthread
  rt
)

################################################################################
# Install (optional)
################################################################################

install(TARGETS tst_motor_cntl tst_motor_enc
  DESTINATION bin
)

################################################################################
# Print build info
################################################################################

message(STATUS "")
message(STATUS "Build configuration:")
message(STATUS "  Executables: tst_motor_cntl, tst_motor_enc")
message(STATUS "  Source directory: ${CMAKE_CURRENT_SOURCE_DIR}/src")
message(STATUS "  Install directory: ${CMAKE_INSTALL_PREFIX}/bin")
message(STATUS "")