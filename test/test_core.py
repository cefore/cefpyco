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

import pytest
import mock

import cefpyco
import os
from time import sleep

sleeptime = 0.2


def test_launch_app():
    with cefpyco.create_handle(enable_log=False) as h:
        pass


def test_launch_two_app():
    with cefpyco.create_handle(enable_log=False) as h_prod:
        sleep(sleeptime)
        with cefpyco.create_handle(enable_log=False) as h_cons:
            pass


def test_request_and_satisfy():
    with cefpyco.create_handle(enable_log=False) as h_prod:
        sleep(sleeptime)
        with cefpyco.create_handle(enable_log=False) as h_cons:
            h_prod.register("ccnx:/core/a")
            h_cons.send_interest("ccnx:/core/a", 0)
            info = h_prod.receive(True)
            assert info.is_succeeded
            assert info.is_interest
            assert info.name == "ccnx:/core/a"
            h_prod.send_data("ccnx:/core/a", "hello", chunk_num=0)
            info = h_cons.receive(True)
            assert info.is_succeeded
            assert info.is_data
            assert info.name == "ccnx:/core/a"
            assert info.payload_s == "hello"


def test_request_and_satisfy_with_binary_data():
    with cefpyco.create_handle(enable_log=False) as h_prod:
        sleep(sleeptime)
        with cefpyco.create_handle(enable_log=False) as h_cons:
            h_prod.register("ccnx:/core/c")
            h_prod.register("ccnx:/core/d")
            h_cons.send_interest("ccnx:/core/c", 0)
            info = h_prod.receive(True)
            assert info.is_succeeded
            assert info.is_interest
            assert info.name == "ccnx:/core/c"
            multibytes = "こんにちは"
            h_prod.send_data("ccnx:/core/c", multibytes, chunk_num=0)
            info = h_cons.receive(True)
            assert info.is_succeeded
            assert info.is_data
            assert info.name == "ccnx:/core/c"
            assert info.payload_s == multibytes
            h_cons.send_interest("ccnx:/core/d", 0)
            info = h_prod.receive(True)
            assert info.is_succeeded
            assert info.is_interest
            assert info.name == "ccnx:/core/d"
            h_prod.send_data("ccnx:/core/d", "\x01\x00\x80\xff\x02", chunk_num=0)
            info = h_cons.receive(True)
            assert info.is_succeeded
            assert info.is_data
            assert info.name == "ccnx:/core/d"
            assert info.payload == b"\x01\x00\x80\xff\x02"


def test_smi_request_and_satisfy():
    with cefpyco.create_handle(enable_log=False) as h_prod:
        sleep(sleeptime)
        with cefpyco.create_handle(enable_log=False) as h_cons:
            h_cons.send_symbolic_interest("ccnx:/core/b")
            sleep(0.2)
            h_prod.send_data("ccnx:/core/b", "hello", 0, end_chunk_num=1)
            h_prod.send_data("ccnx:/core/b", "world", 1)
            info = h_cons.receive(True)
            assert info.is_succeeded
            assert info.is_data
            assert info.name == "ccnx:/core/b"
            assert info.chunk_num == 0
            assert info.end_chunk_num == 1
            assert info.payload_s == "hello"
            info = h_cons.receive(True)
            assert info.is_succeeded
            assert info.is_data
            assert info.name == "ccnx:/core/b"
            assert info.chunk_num == 1
            assert info.end_chunk_num is None
            assert info.payload_s == "world"


# def test_smi_request_and_satisfy_with_nochunk():
#     with cefpyco.create_handle(enable_log=False) as h_prod:
#         sleep(sleeptime)
#         with cefpyco.create_handle(enable_log=False) as h_cons:
#             h_cons.send_symbolic_interest("ccnx:/core/d")
#             sleep(0.2)
#             h_prod.send_data("ccnx:/core/d", b"no\x00\x80chunk")
#             info = h_cons.receive(True)
#             assert info.is_succeeded
#             assert info.is_data
#             assert info.name == "ccnx:/core/d"
#             # assert info.chunk_num is None
#             assert info.chunk_num == 0
#             assert info.end_chunk_num is None
#             assert info.payload == b"no\x00\x80chunk"
