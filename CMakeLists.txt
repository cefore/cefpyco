# Copyright (c) 2018, National Institute of Information and Communications
# Technology (NICT). All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the NICT nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
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

cmake_minimum_required(VERSION 2.8)
project (cefpyco)

if (test) # If needs libcefpyco.so, run 'cmake -Dtest=on .'
    find_package(PythonLibs REQUIRED)
    add_custom_target(find_python_includes  ALL
        COMMAND echo Python include directory is ${PYTHON_INCLUDE_DIRS})
    include_directories(${PYTHON_INCLUDE_DIRS} /usr/local/include /usr/local/lib c_src c_src/cefparse)
    add_definitions("-fPIC")

    add_subdirectory(c_src)

    add_library(cefpyco SHARED c_src/pywrap_cefpyco.c)
    target_link_libraries(cefpyco ${PYTHON_LIBRARIES} cefore crypto cefpyco_csrc cefpyco_parse)
    #add_custom_command(
    #    TARGET ${PROJECT_NAME}
    #    POST_BUILD
    #    COMMAND ${CMAKE_COMMAND} -E copy libcefpyco.so test/
    #)
endif()

if (python2) # If forces to use python2, run 'cmake -Dpython2=on .'
    install(
        CODE [[
            execute_process(
                COMMAND sudo -EH python setup.py install --record uninstall.txt
            )
        ]]
    )
else()
    if (COMMAND sudo)
        install(
            CODE [[
                execute_process(
                    COMMAND sudo -EH python3 setup.py install --record uninstall.txt
                )
            ]]
        )
    else()
        install(
            CODE [[
                execute_process(
                    COMMAND python3 setup.py install --record uninstall.txt
                )
            ]]
        )
    endif()
endif()

add_custom_target(uninstall
    COMMAND echo "[Uninstall] Checking uninstall.txt..."
    COMMAND test -f uninstall.txt
    COMMAND echo "[Uninstall] Removing following files..."
    COMMAND cat uninstall.txt | sed 's/^/\ \ \ \ + /g'
    COMMAND xargs rm < uninstall.txt
    COMMAND mv uninstall.txt uninstall.txt.bak
    COMMAND echo "[Uninstall] Completed.")
