#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2016--2023, National Institute of Information and Communications
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
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINEPSS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

import mock

from cefpyco import libcefpyco
import os
from struct import pack
from time import sleep

sleeptime = 0.1


def test_launch_app():
    cefdir = ""
    portnum = 0
    handler = libcefpyco.begin(portnum, cefdir, 1)
    libcefpyco.end(handler)
    assert True


def test_launch_two_app():
    cefdir = ""
    portnum = 0
    handler1 = libcefpyco.begin(portnum, cefdir, 1)
    sleep(sleeptime)
    handler2 = libcefpyco.begin(portnum, cefdir, 1)
    sleep(sleeptime)
    libcefpyco.end(handler1)
    sleep(sleeptime)
    libcefpyco.end(handler2)
    sleep(sleeptime)
    assert True


def test_request_and_satisfy():
    handler_p = libcefpyco.begin(0, "", 1)
    sleep(sleeptime)
    handler_c = libcefpyco.begin(0, "", 1)
    sleep(sleeptime)
    libcefpyco.register(handler_p, "ccnx:/lib/d")
    libcefpyco.send_interest(handler_c, "ccnx:/lib/d", 0)
    res = libcefpyco.receive(handler_p, 1)
    assert res[0] >= 0
    print(res)
    assert res[6] == 11  # name_len
    assert res[5] == "ccnx:/lib/d"
    libcefpyco.send_data(handler_p, "ccnx:/lib/d", "hello", 5, 0)
    res = libcefpyco.receive(handler_c, 1)
    assert res[0] >= 0
    assert res[5] == "ccnx:/lib/d"
    assert res[7] == 0  # chunk_num
    assert res[11] == 5  # payload_len
    assert res[10] == b"hello"
    libcefpyco.end(handler_p)
    sleep(sleeptime)
    libcefpyco.end(handler_c)
    sleep(sleeptime)


def test_request_and_satisfy_with_binary_data():
    handler_p = libcefpyco.begin(0, "", 1)
    sleep(sleeptime)
    handler_c = libcefpyco.begin(0, "", 1)
    sleep(sleeptime)
    libcefpyco.register(handler_p, "ccnx:/lib/f")
    libcefpyco.send_interest(handler_c, "ccnx:/lib/f", 0)
    res = libcefpyco.receive(handler_p, 1)
    assert res[0] >= 0
    print(res)
    assert res[6] == 11  # name_len
    assert res[5] == "ccnx:/lib/f"
    data = pack("B" * 256, *range(256))
    libcefpyco.send_data(handler_p, "ccnx:/lib/f", data, 256, 0)
    res = libcefpyco.receive(handler_c, 1)
    assert res[0] >= 0
    assert res[5] == "ccnx:/lib/f"
    assert res[7] == 0  # chunk_num
    assert res[11] == 256  # payload_len
    assert res[10] == data
    libcefpyco.end(handler_p)
    sleep(sleeptime)
    libcefpyco.end(handler_c)
    sleep(sleeptime)


def test_request_with_smi_and_satisfy():
    handler_p = libcefpyco.begin(0, "", 1)
    sleep(sleeptime)
    handler_c = libcefpyco.begin(0, "", 1)
    sleep(sleeptime)
    libcefpyco.send_interest(handler_c, "ccnx:/lib/e", longlife_f=1)  # CefC_T_LONGLIFE
    sleep(0.2)
    libcefpyco.send_data(handler_p, "ccnx:/lib/e", "hello", 5, 0)
    libcefpyco.send_data(handler_p, "ccnx:/lib/e", "world", 5, 1, end_chunk_num=10)
    sleep(sleeptime)
    res = libcefpyco.receive(handler_c, 1)
    assert res[0] >= 0
    assert res[5] == "ccnx:/lib/e"
    assert res[7] == 0  # chunk_num
    assert res[11] == 5  # payload_len
    assert res[10] == b"hello"
    sleep(sleeptime)
    res = libcefpyco.receive(handler_c, 1)
    assert res[0] >= 0
    assert res[5] == "ccnx:/lib/e"
    assert res[7] == 1  # chunk_num
    assert res[8] == 10  # end_chunk_num
    assert res[11] == 5  # payload_len
    assert res[10] == b"world"
    libcefpyco.end(handler_p)
    sleep(sleeptime)
    libcefpyco.end(handler_c)
    sleep(sleeptime)
