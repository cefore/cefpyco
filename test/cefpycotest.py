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

import libcefpyco as cefpyco
import click
import os


def get_portnum():
    return 0


def get_cefdir():
    # return "%s/cefore" % (os.environ.get("CEFORE_DIR") or  "/usr/local")
    return ""


def show_res(res):
    result, version, type, len, name, namelen, chunknum, payload, payloadlen = res
    print(
        "--------------------\n"
        "     result: {0}\n"
        "    version: {1:08x}\n"
        "       type: {2:08x}\n"
        "        len: {3}\n"
        "       name: {4}\n"
        "   name_len: {5}\n"
        "  chunk_num: {6}\n"
        "    payload: {7}\n"
        "payload_len: {8}\n"
        "--------------------\n".format(*res)
    )


def send_i():
    print("1")
    cefpyco.begin(get_portnum(), get_cefdir())
    print("2")
    cefpyco.send_interest("ccnx:/a", 0)
    print("3")
    cefpyco.send_interest("ccnx:/a", 1)
    print("4")
    cefpyco.end()


def send_d():
    cefpyco.begin(get_portnum(), get_cefdir())
    cefpyco.send_data("ccnx:/a", "hello", 5, 0)
    cefpyco.send_data("ccnx:/a", "world", 5, 1)
    cefpyco.end()


def recv_i():
    cefpyco.begin(get_portnum(), get_cefdir())
    cefpyco.register("ccnx:/a")
    res = cefpyco.receive()
    show_res(res)
    res = cefpyco.receive()
    show_res(res)
    cefpyco.end()


def recv_d():
    cefpyco.begin(get_portnum(), get_cefdir())
    cefpyco.send_interest("ccnx:/a", 0)
    cefpyco.send_interest("ccnx:/a", 1)
    res = cefpyco.receive()
    show_res(res)
    res = cefpyco.receive()
    show_res(res)
    cefpyco.end()


def response():
    cefpyco.begin(get_portnum(), get_cefdir())
    cefpyco.register("ccnx:/a")
    res = cefpyco.receive()
    show_res(res)
    cefpyco.send_data("ccnx:/a", "hello", 5, 0)
    res = cefpyco.receive()
    show_res(res)
    cefpyco.send_data("ccnx:/a", "world", 5, 1)
    cefpyco.end()


@click.command()
@click.option("--id", "-i", default=1)
def main(id):
    if id == 1:
        print("[MODE] Send Interest")
        send_i()
    elif id == 2:
        print("[MODE] Send Data")
        send_d()
    elif id == 3:
        print("[MODE] Receive Interest")
        recv_i()
    elif id == 4:
        print("[MODE] Receive Data")
        recv_d()
    elif id == 5:
        print("[MODE] Response")
        response()
    else:
        print("[ERROR] Please specify mode with option '-i'")


if __name__ == "__main__":
    main()
