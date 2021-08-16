#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

from setuptools import setup, find_packages, Extension

with open("README.md", "r") as f:
    long_description = f.read()

module_libcefpyco = Extension(    
    "libcefpyco", 
    sources=[
        "c_src/cefpyco_util.c",
        "c_src/cefparse/cpcparse_tlv.c",
        # "c_src/cefparse/cpcparse_app_frame.c",
        # "c_src/cefparse/cpcparse_interest.c",
        "c_src/cefparse/cpcparse_app_frame_7_5.c",
        "c_src/cefparse/cpcparse_app_request.c",
        "c_src/cefparse/cpcparse_intreturn.c",
        "c_src/cefparse/cpcparse_type.c",
        "c_src/cefpyco_parse.c",
        "c_src/cefpyco.c",
        "c_src/pywrap_cefpyco.c",
        ],
    libraries=[
        "cefore",
        "crypto",
        ],
    include_dirs=[
        "c_src", 
        "c_src/cefparse",
        ],
    swig_opts=["-fPIC"],
    )

setup(
    name="cefpyco",
    version="0.6.0",
    author="Atsushi Ooka",
    author_email="a-ooka@nict.go.jp",
    description=(
        "Cefore Python Compact (cefpyco) package for "
        "developing a Cefore application in Python."
        ),
    long_description=long_description,
    long_description_content_type="text/markdown",
    #url="https://github.com/pypa/sampleproject",
    packages=find_packages(exclude=["test"]),
    classifiers=[
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3",
        "Operating System :: MacOS",
        "Operating System :: POSIX :: Linux",
    ],
    ext_modules = [module_libcefpyco],
)
