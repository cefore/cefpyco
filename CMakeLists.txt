# Copyright (c) 2016--2023, National Institute of Information and Communications
# Technology (NICT). All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
# notice this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
# 3. Neither the name of the NICT nor the names of its contributors may be
# used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE NICT AND CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE NICT OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

cmake_minimum_required(VERSION 3.16)
project(cefpyco)

find_package(Python REQUIRED COMPONENTS Interpreter Development)
find_package(OpenSSL REQUIRED)

add_custom_target(debug_cmake ALL
    COMMAND echo "          Python_FOUND: ${Python_FOUND}"
    COMMAND echo "      Python_LIBRARIES: ${Python_LIBRARIES}"
    COMMAND echo "   Python_INCLUDE_DIRS: ${Python_INCLUDE_DIRS}"
    COMMAND echo "     Python_EXECUTABLE: ${Python_EXECUTABLE}"
    # COMMAND echo "         OPENSSL_FOUND: ${OPENSSL_FOUND}"
    # COMMAND echo "   OPENSSL_SSL_LIBRARY: ${OPENSSL_SSL_LIBRARY}"
    # COMMAND echo "OPENSSL_CRYPTO_LIBRARY: ${OPENSSL_CRYPTO_LIBRARY}"
    # COMMAND echo "     OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}"
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    message("[CMakeLists.txt] Debug mode.")
    add_definitions(-DCEFPYCO_DEBUG)
endif()

add_subdirectory(src/cefpyco_c)
add_subdirectory(src/cefpyco_wrap)
add_subdirectory(src/cefpyco)
add_subdirectory(test)

################################################################################
### TESTING ####################################################################
################################################################################
enable_testing()

add_test(NAME test_with_libcefpyco COMMAND test_cefpyco
    WORKING_DIRECTORY test)

add_test(NAME test_with_pytest COMMAND ${Python_EXECUTABLE} -m pytest
    WORKING_DIRECTORY test)
set_tests_properties(test_with_pytest PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/src:$ENV{PYTHONPATH}")

add_test(NAME test_with_cefapp COMMAND ${Python_EXECUTABLE} -m pytest
    WORKING_DIRECTORY cefapp/test)
set_tests_properties(test_with_cefapp PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_BINARY_DIR}/src:${CMAKE_BINARY_DIR}/cefapp/src:$ENV{PYTHONPATH}")

################################################################################
### INSTALLATION ###############################################################
################################################################################
file(STRINGS ${CMAKE_BINARY_DIR}/setup.cfg version
    REGEX "^version = [0-9]+\\.[0-9]+\\.[0-9]+.+$")
string(REGEX REPLACE "^version = ([0-9]+\\.[0-9]+\\.[0-9]+.+)$" "\\1" version "${version}")

set(cefpyco_ver latest CACHE STRING "The version for cefpyco when not specified.")

install(CODE "
    execute_process(COMMAND ${Python_EXECUTABLE} -m build
        RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR \"Build process failed with error code: ${result}\")
    endif()
    message(\"Install '${CMAKE_BINARY_DIR}/dist/cefpyco-${version}.tar.gz'.\")
    execute_process(COMMAND ${Python_EXECUTABLE} -m pip install
        ${CMAKE_BINARY_DIR}/dist/cefpyco-${version}.tar.gz
        RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR \"Installation failed with error code: ${result}\")
    endif()
    message(\"Installation completed.\")
")

add_custom_target(uninstall
    COMMAND ${Python_EXECUTABLE} -m pip uninstall --yes cefpyco
)