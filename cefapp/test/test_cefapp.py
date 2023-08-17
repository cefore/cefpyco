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
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

import sys, os

sys.path.append(os.pardir)
sys.path.append(os.getcwd())
import pytest
import mock
from cefpyco.core import CcnPacketInfo
from cefapp import *


def test_learn_iteratable_mock():
    def effect_test():
        for i in range(3):
            yield 1 + i
        while True:
            yield mock.DEFAULT

    m = mock.MagicMock(return_value=0, side_effect=effect_test())
    assert m() == 1
    assert m() == 2
    assert m() == 3
    assert m() == 0
    assert m() == 0


def create_test_info(res, echunk=-1, retcode=0):
    is_succeeded = res[0]
    version = res[1]
    type = res[2]
    returncode = retcode
    actual_data_len = res[3]
    name = res[4]
    name_len = res[5]
    chunk_num = res[6]
    end_chunk_num = echunk
    flags = -1
    payload = res[7]
    payload_len = res[8]
    hdr_org = ""
    hdr_org_len = 0
    msg_org = ""
    msg_org_len = 0
    return CcnPacketInfo(
        (
            is_succeeded,
            version,
            type,
            returncode,
            actual_data_len,
            name,
            name_len,
            chunk_num,
            end_chunk_num,
            flags,
            payload.encode(),
            payload_len,
            hdr_org,
            hdr_org_len,
            msg_org,
            msg_org_len,
        )
    )


def create_data_mock(name, data_list, meta=False):
    def _mock_effect(name, data_list, meta):
        cob_num = 0
        name_len = len(name)
        empty = create_test_info((0, 0, 0, 0, "", 0, 0, "", 0), echunk=0)
        if meta:
            meta_name = "%s/meta" % name
            data_num = "%d" % len(data_list)
            yield create_test_info(
                (1, 0, 0, 1, meta_name, len(meta_name), 0, data_num, len(data_num))
            )
        for d in data_list:
            if d is None:
                yield empty
            else:
                yield create_test_info((1, 0, 0, 1, name, name_len, cob_num, d, len(d)))
                cob_num += 1
        while True:
            yield empty

    m = mock.MagicMock()
    m.receive = mock.MagicMock(side_effect=_mock_effect(name, data_list, meta))
    return m


def test_running_consumer_for_hello():
    m = create_data_mock("ccnx:/test", ["hello"])
    app = CefAppConsumer(m)
    app.run("ccnx:/test", 1)
    assert app.data == "hello"


def test_running_consumer_for_hw():
    m = create_data_mock("ccnx:/test", ["hello", "world"])
    app = CefAppConsumer(m)
    app.run("ccnx:/test", 2)
    assert app.data == "helloworld"


def test_running_consumer_for_hw_with_error():
    m = create_data_mock("ccnx:/test", ["hello", None, "world"])
    app = CefAppConsumer(m)
    app.run("ccnx:/test", 2)
    assert app.data == "helloworld"


def test_running_consumer_random_order():
    m = mock.MagicMock()
    m.receive = mock.MagicMock(
        side_effect=[
            create_test_info((1, 0, 0, 1, "ccnx:/test", 9, 0, "aaa", 5)),
            create_test_info((1, 0, 0, 1, "ccnx:/test", 9, 2, "ccc", 5)),
            create_test_info((0, 0, 0, 0, "", 0, 0, "", 0)),
            create_test_info((1, 0, 0, 1, "ccnx:/test", 9, 0, "aaa", 5)),
            create_test_info((1, 0, 0, 1, "ccnx:/test", 9, 2, "ccc", 5)),
            create_test_info((1, 0, 0, 1, "ccnx:/test", 9, 1, "bbb", 5)),
        ]
    )
    app = CefAppConsumer(m)
    app.run("ccnx:/test", 3)
    assert app.data == "aaabbbccc"


def test_running_consumer_random_order_on_error():
    m = mock.MagicMock()
    m.receive = mock.MagicMock(
        side_effect=[
            create_test_info((1, 0, 0, 1, "ccnx:/test", 9, 0, "aaa", 5)),
            create_test_info((1, 0, 0, 1, "ccnx:/test", 9, 2, "ccc", 5)),
            create_test_info((0, 0, 0, 0, "", 0, 0, "", 0)),
            create_test_info((0, 0, 0, 0, "", 0, 0, "", 0)),
            create_test_info((1, 0, 0, 1, "ccnx:/test", 9, 1, "bbb", 5)),
        ]
    )
    app = CefAppConsumer(m, timeout_limit=2)
    app.run("ccnx:/test", 3)
    assert app.data == "aaaccc"


def test_running_consumer_only_check():
    m = create_data_mock("ccnx:/test", ["hello", "world"])
    app = CefAppConsumer(m, data_store=False)
    app.run("ccnx:/test", 2)
    assert app.data is None


def test_running_consumer_without_count():
    m = create_data_mock("ccnx:/test", ["hello", "world"], meta=True)
    app = CefAppConsumer(m)
    app.run("ccnx:/test")
    assert app.data == "helloworld"


def test_running_consumer_without_count():
    m = create_data_mock("ccnx:/test", ["hello"] * 100, meta=True)
    app = CefAppConsumer(m)
    app.run("ccnx:/test")
    assert app.data == ("hello" * 100)


def create_interest_mock(name, chunk_num_list, meta=False):
    def _mock_effect(name, chunk_num_list, meta):
        cob_num = 0
        name_len = len(name)
        empty = create_test_info((0, 0, 0, 0, "", 0, 0, "", 0))
        if meta:
            meta_name = "%s/meta" % name
            data_num = "%d" % len(chunk_num_list)
            yield create_test_info((1, 0, 0, 1, meta_name, len(meta_name), 0, "", 0))
        for c in chunk_num_list:
            if c is None:
                yield empty
            else:
                yield create_test_info((1, 0, 0, 1, name, name_len, c, "", 0))
        while True:
            yield empty

    m = mock.MagicMock()
    m.receive = mock.MagicMock(side_effect=_mock_effect(name, chunk_num_list, meta))
    return m


def test_running_producer():
    m = create_interest_mock("ccnx:/test", [0, 1])
    app = CefAppProducer(m, data="helloworld", cob_len=5)
    app.run("ccnx:/test")
    assert len(m.register.call_args_list) == 1
    c = m.send_data.call_args_list
    assert len(c) == 2
    assert c[0][0][0] == "ccnx:/test"
    assert c[0][0][1] == "hello"
    assert c[0][0][2] == 0
    assert c[1][0][0] == "ccnx:/test"
    assert c[1][0][1] == "world"
    assert c[1][0][2] == 1


def test_running_consumer_random_order_on_error():
    m = mock.MagicMock()
    m.receive = mock.MagicMock(
        side_effect=[
            create_test_info((1, 0, 0, 1, "ccnx:/test/meta", 14, 0, "", 0)),
            create_test_info((1, 0, 0, 1, "ccnx:/test", 9, 0, "", 0)),
            create_test_info((1, 0, 0, 1, "ccnx:/test", 9, 1, "", 0)),
            create_test_info((0, 0, 0, 0, "", 0, 0, "", 0)),
            create_test_info((0, 0, 0, 0, "", 0, 0, "", 0)),
            create_test_info((0, 0, 0, 0, "", 0, 0, "", 0)),
        ]
    )
    app = CefAppProducer(m, data="helloworld", cob_len=5)
    app.run("ccnx:/test")
    assert m.send_data.call_args_list
    c = m.send_data.call_args_list
    assert len(c) == 3
    assert c[0][0][1] == "2"
    assert c[1][0][1] == "hello"
    assert c[2][0][1] == "world"
