#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2016--2023, National Institute of Information and Communications
# Technology (NICT). All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice,
# 	 this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
# 	 notice this list of conditions and the following disclaimer in the
# 	 documentation and/or other materials provided with the distribution.
# 3. Neither the name of the NICT nor the names of its contributors may be
# 	 used to endorse or promote products derived from this software
# 	 without specific prior written permission.
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

from . import libcefpyco as _cefpyco
import os
from sys import stderr, version_info
from contextlib import contextmanager

INTEREST_TYPE_REGULAR = 0x0000  # CefC_T_OPT_REGULAR
INTEREST_TYPE_SYMBOLIC = 0x0002  # CefC_T_LONGLIFE


class InterestReturnCode:
    class CodeObj:
        def __init__(self, code, name, desc):
            self._code = code
            self._name = name
            self._desc = desc

        @property
        def code(self):
            return self._code

        @property
        def name(self):
            return self._name

        @property
        def desc(self):
            return self._desc

        @property
        def description(self):
            return self._desc

        def __repr__(self):
            return "ReturnCode: 0x{0:02X} ({1})".format(self.code, self.name)

    def __init__(self):
        raise Exception("Instantiation is not allowed.")

    # c.f.
    # * https://datatracker.ietf.org/doc/html/rfc8569#section-10.2
    # * https://datatracker.ietf.org/doc/html/rfc8609#section-4.2
    Codes = [
        CodeObj(0x00, "Reserved", "Reserved"),
        CodeObj(
            0x01,
            "No Route",
            "The returning forwarder has no route to the Interest name.",
        ),
        CodeObj(
            0x02,
            "HopLimit Exceeded",
            "The HopLimit has decremented to 0 and needs to forward the packet.",
        ),
        CodeObj(
            0x03,
            "Interest MTU too large",
            "The Interest's MTU does not conform to the required minimum and would require fragmentation.",
        ),
        CodeObj(
            0x04,
            "No Resources",
            "The node does not have the resources to process the Interest.",
        ),
        CodeObj(
            0x05,
            "Path error",
            "There was a transmission error when forwarding the Interest along a route (a transient error).",
        ),
        CodeObj(
            0x06,
            "Prohibited",
            "An administrative setting prohibits processing this Interest.",
        ),
        CodeObj(
            0x07,
            "Congestion",
            "The Interest was dropped due to congestion (a transient error).",
        ),
        CodeObj(
            0x08,
            "Unsupported ContentObjectHash Restriction",
            "The Interest was dropped because it requested a Content Object Hash restriction using a hash algorithm that cannot be computed.",
        ),
        CodeObj(
            0x09,
            "Malformed Interest",
            "The Interest was dropped because the parse or validation were failed.",
        ),
    ]

    @classmethod
    def get(cls, returncode):
        if not isinstance(returncode, int):
            return cls.CodeObj(
                0xFF,
                "Invalid ReturnCode: '{0}'".format(returncode),
                "ReturnCode is invalid (maybe None).",
            )
        if 0x00 <= returncode and returncode <= 0x09:
            return cls.Codes[returncode]
        return cls.CodeObj(
            returncode, "Undefined", "Undefined ReturnCode is specified."
        )


class CcnPacketInfo:
    PacketTypeInterest = 0x00
    PacketTypeData = 0x01
    PacketTypeInterestReturn = 0x02
    PacketFlagSymbolic = 0x01
    PacketFlagLonglife = 0x02

    def __init__(self, res):
        self.is_succeeded = res[0] >= 0 and res[4] > 0
        self.is_failed = not self.is_succeeded
        self.version = res[1]
        self.type = res[2]
        self.returncode = res[3] if self.is_return else None
        self.actual_data_len = res[4]
        self.name = res[5]
        self.name_len = res[6]
        self.chunk_num = res[7] if res[7] >= 0 else None
        self.end_chunk_num = res[8] if res[8] >= 0 else None
        self.flags = res[9]
        self.payload = res[10]
        self.payload_len = res[11]
        self.hdr_org = res[12]
        self.hdr_org_len = res[13]
        self.msg_org = res[14]
        self.msg_org_len = res[15]

    @property
    def is_interest(self):
        return self.is_succeeded and (self.type == self.PacketTypeInterest)

    @property
    def is_regular_interest(self):
        return (
            self.is_succeeded
            and (self.type == self.PacketTypeInterest)
            and not self.is_symbolic
        )

    @property
    def is_symbolic_interest(self):
        return (
            self.is_succeeded
            and (self.type == self.PacketTypeInterest)
            and self.is_symbolic
        )

    @property
    def is_longlife_interest(self):
        return (
            self.is_succeeded
            and (self.type == self.PacketTypeInterest)
            and self.is_longlife
        )

    @property
    def is_data(self):
        return self.is_succeeded and (self.type == self.PacketTypeData)

    @property
    def is_interest_return(self):
        return self.is_succeeded and (self.type == self.PacketTypeInterestReturn)

    @property
    def is_return(self):
        return self.is_interest_return

    @property
    def is_symbolic(self):
        return ((self.flags & self.PacketFlagSymbolic) != 0) and (
            (self.flags & self.PacketFlagLonglife) != 0
        )

    @property
    def is_longlife(self):
        return (self.flags & self.PacketFlagLonglife) != 0

    @property
    def packet_type(self):
        if self.is_succeeded:
            if self.is_symbolic_interest:
                return "Symbolic-interest"
            elif self.is_longlife_interest:
                return "Longlife-interest"
            elif self.is_interest:
                return "Interest"
            elif self.is_data:
                return "Data"
            elif self.is_return:
                return "Interest-Return"
        return "Unknown (id: %08x)" % self.type

    @property
    def payload_s(self):
        if self.payload:
            return self.payload.decode("raw_unicode_escape", errors="replace")
        else:
            return ""

    @property
    def hdr_org_s(self):
        if self.hdr_org:
            return self.hdr_org.decode("raw_unicode_escape", errors="replace")
        else:
            return ""

    @property
    def msg_org_s(self):
        if self.msg_org:
            return self.msg_org.decode("raw_unicode_escape", errors="replace")
        else:
            return ""

    def __repr__(self):
        if self.is_failed:
            return "Info: Failed to receive"
        ret = ("Info: Succeeded in receiving {0} packet with " "name '{1}'").format(
            self.packet_type, self.name
        )
        if self.chunk_num is not None:
            ret += " (#chunk: {0})".format(self.chunk_num)
        if self.payload_len > 0:
            ret += " and payload '{0}' ({1} Bytes)".format(
                self.payload, self.payload_len
            )
        if self.is_interest_return:
            ret += " and {0}".format(InterestReturnCode.get(self.returncode))
        return ret


class CefpycoHandle(object):
    def __init__(self, enable_log=True):
        self.handler = None
        self.enable_log = enable_log
        self.sequence_of_chars_type = str
        self.sequence_of_bytes_type = bytes

    def log(self, msg, force=False):
        if self.enable_log or force:
            stderr.write("[cefpyco] %s\n" % msg)

    def begin(self, ceforedir=None, portnum=0):
        if self.handler is not None:
            raise Exception("This handler has been already used.")
        if ceforedir is None:
            self.cefdir = "%s/cefore" % (os.environ.get("CEFORE_DIR") or "/usr/local")
        else:
            self.cefdir = ceforedir
        self.portnum = portnum
        self.log("Configure directory is %s" % self.cefdir)
        self.handler = _cefpyco.begin(
            self.portnum, self.cefdir, 1 if self.enable_log else 0
        )

    def end(self):
        if self.handler is not None:
            _cefpyco.end(self.handler)
            self.handler = None

    def send_interest(
        self,
        name,
        # chunk_num=-1,
        chunk_num=0,
        symbolic_f=0,
        longlife_f=0,
        hop_limit=32,
        lifetime=4000,
        hdr_org=b"",
        msg_org=b"",
    ):
        _cefpyco.send_interest(
            self.handler,
            name,
            chunk_num=chunk_num,
            symbolic_f=symbolic_f,
            longlife_f=longlife_f,
            hop_limit=hop_limit,
            lifetime=lifetime,
            hdr_org=hdr_org,
            msg_org=msg_org,
        )

    def send_symbolic_interest(
        self, name, hop_limit=32, lifetime=10000, hdr_org=b"", msg_org=b""
    ):
        _cefpyco.send_interest(
            self.handler,
            name,
            chunk_num=-1,
            symbolic_f=1,
            longlife_f=1,
            hop_limit=hop_limit,
            lifetime=lifetime,
            hdr_org=hdr_org,
            msg_org=msg_org,
        )

    def send_longlife_interest(
        self, name, chunk_num=0, hop_limit=32, lifetime=4000, hdr_org=b"", msg_org=b""
    ):
        _cefpyco.send_interest(
            self.handler,
            name,
            chunk_num=chunk_num,
            symbolic_f=0,
            longlife_f=1,
            hop_limit=hop_limit,
            lifetime=lifetime,
            hdr_org=hdr_org,
            msg_org=msg_org,
        )

    def build_hdrorg(
        self,
        t_hw_flags_f=0,
        t_hw_timestamp_f=0,
        t_hw_timestamp_long_f=0,
        t_hw_timestamp_in=0,
        t_hw_timestamp_out=0,
        t_hw_flags_symbolic_f=0,
        t_hw_flags_enablecache_f=0,
        tp_variant=0,
        tp_val=b"",
    ):
        return _cefpyco.build_hdrorg(
            t_hw_flags_f=t_hw_flags_f,
            t_hw_timestamp_f=t_hw_timestamp_f,
            t_hw_timestamp_long_f=t_hw_timestamp_long_f,
            t_hw_timestamp_in=t_hw_timestamp_in,
            t_hw_timestamp_out=t_hw_timestamp_out,
            t_hw_flags_symbolic_f=t_hw_flags_symbolic_f,
            t_hw_flags_enablecache_f=t_hw_flags_enablecache_f,
            tp_variant=tp_variant,
            tp_val=tp_val,
        )

    def build_msgorg(
        self,
        symbolic_f=0,
        longlife_f=0,
        selective_f=0,
        req_chunk=0,
        first_chunk=0,
        last_chunk=-1,
        version_val=b"",
        from_pub_f=0,
    ):
        return _cefpyco.build_msgorg(
            symbolic_f=symbolic_f,
            longlife_f=longlife_f,
            selective_f=selective_f,
            req_chunk=req_chunk,
            first_chunk=first_chunk,
            last_chunk=last_chunk,
            version_val=version_val,
            from_pub_f=from_pub_f,
        )

    def _convert_to_bytes_like_object(self, payload):
        payloadtype = type(payload)
        if payloadtype is self.sequence_of_chars_type:
            return payload.encode("raw_unicode_escape")
        elif payloadtype is self.sequence_of_bytes_type:
            return payload
        else:
            return str(payload).encode("raw_unicode_escape")

    def send_data(
        self,
        name,
        payload,
        chunk_num=-1,
        end_chunk_num=-1,
        hop_limit=32,
        expiry=36000000,
        cache_time=-1,
        hdr_org=b"",
        msg_org=b"",
    ):
        payload = self._convert_to_bytes_like_object(payload)
        _cefpyco.send_data(
            self.handler,
            name,
            payload,
            len(payload),
            chunk_num=chunk_num,
            end_chunk_num=end_chunk_num,
            hop_limit=hop_limit,
            expiry=expiry,
            cache_time=cache_time,
            hdr_org=hdr_org,
            msg_org=msg_org,
        )

    def register(self, name):
        _cefpyco.register(self.handler, name, is_reg=1)

    def deregister(self, name):
        _cefpyco.register(self.handler, name, is_reg=0)

    def register_pit(self, name):
        _cefpyco.register_for_pit(self.handler, name, is_reg=1)

    def deregister_pit(self, name):
        _cefpyco.register_for_pit(self.handler, name, is_reg=0)

    def receive(self, error_on_timeout=False, timeout_ms=4000):
        i = 1 if error_on_timeout else 0
        res = _cefpyco.receive(self.handler, i, timeout_ms)
        return CcnPacketInfo(res)


@contextmanager
def create_handle(ceforedir=None, portnum=0, enable_log=True):
    h = None
    try:
        h = CefpycoHandle(enable_log)
        h.begin(ceforedir, portnum)
        yield h
    except Exception as e:
        # print(e)
        raise e
    finally:
        if h:
            h.end()
