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

import numpy as np
import cefpyco
from sys import stderr

class CefAppRunningInfo(object):
    def __init__(self, name, count):
        self.name = name
        self.metaname = "%s/meta" % name
        self.count = count
        self.n_finished = 0
        self.finished_flag = np.zeros(count)
        self.timeout_count = 0

class MetaInfoNotResolvedError(Exception):
    pass

class CefApp(object):
    def __init__(self, cef_handle, target_name, action_name, timeout_limit, enable_log):
        self.cef_handle = cef_handle
        self.target_name = target_name
        self.action_name = action_name
        self.timeout_limit = timeout_limit
        self.enable_log = enable_log
    
    def log(self, msg, force=False):
        if self.enable_log or force: stderr.write("[cefapp] %s\n" % msg)
    
    def run(self, name, count=0):
        if count <= 0:
            count = self.resolve_count(name)
            if not count:
                errmsg = "{0}/meta is not resolved.".format(name)
                self.log(errmsg)
                raise MetaInfoNotResolvedError(errmsg)

        info = CefAppRunningInfo(name, count)
        self.on_start(info)
        while info.timeout_count < self.timeout_limit and self.continues_to_run(info):
            packet = self.cef_handle.receive()
            if packet.is_failed:
                info.timeout_count += 1
                self.log("Wait for {0}...({1}/{2})".format(
                    self.target_name, info.timeout_count, self.timeout_limit))
                self.on_rcv_failed(info)
            elif packet.name == info.name:
                # self.log("Scceed to {0} ({1} #{2})".format(
                #     self.action_name, packet.name, packet.chunk_num))
                self.on_rcv_succeeded(info, packet)
            elif packet.name == info.metaname:
                self.on_rcv_meta(info, packet)
            # else:
            #     self.log("{0} is expected but received {1}.".format(
            #         info.name, packet.name))
        if info.n_finished == info.count:
            self.show_result_on_success(info)
        else:
            self.show_result_on_failure(info)
    
    def resolve_count(self, name):
        raise NotImplementedError()
    
    def on_start(self, info):
        pass
    
    def on_rcv_failed(self, info):
        pass
    
    def on_rcv_succeeded(self, info, packet):
        pass
    
    def on_rcv_meta(self, info, packet):
        pass
    
    def continues_to_run(self, info):
        raise NotImplementedError()
    
    def show_result_on_success(self, info):
        self.log("Succeed to {0}.".format(self.action_name))
        
    def show_result_on_failure(self, info):
        buf = ""
        last = -2
        seq = False
        miss_count = 0
        for i in range(info.count):
            if info.finished_flag[i]:
                continue
            miss_count += 1
            if last == i - 1:
                if not seq:
                    buf += "--"
                seq = True
            else:
                if seq:
                    buf += "#{0}, #{1}".format(last, i)
                else:
                    sep = ", " if last >= 0 else ""
                    buf += "{0}#{1}".format(sep, i)
                seq = False
            last = i
        if seq:
            buf += "#{0}".format(last)
        self.log("Failed to {3} following chunks [{0}/{1}]: {2}".format(
            miss_count, info.count, buf, self.action_name))
    
class CefAppConsumer(CefApp):
    def __init__(self, cef_handle, 
        pipeline=1000, timeout_limit=2, data_store=True, enable_log=True):
        self.pipeline = pipeline
        self.data_store = data_store
        super(CefAppConsumer, self).__init__(
            cef_handle, "Data", "receive", timeout_limit, enable_log)
    
    @property
    def data(self):
        return "".join(self.cob_list) if self.data_store else None
    
    def resolve_count(self, name):
        meta_name = "{0}/meta".format(name)
        for i in range(self.timeout_limit):
            self.cef_handle.send_interest(meta_name, 0)
            packet = self.cef_handle.receive()
            if packet.is_failed: continue
            if packet.is_interest_return:
                self.log(str(packet))
                continue
            if packet.name != meta_name: continue
            return int(packet.payload_s)
        return None
    
    def on_start(self, info):
        self.req_flag = np.zeros(info.count)
        if self.data_store: self.cob_list = [""] * info.count
        self.rcv_tail_index = 0
        self.req_tail_index = 0
        self.send_interests_with_pipeline(info)

    def continues_to_run(self, info):
        return info.n_finished < info.count
    
    def on_rcv_failed(self, info):
        self.reset_req_status(info)
        self.send_interests_with_pipeline(info)
        
    def on_rcv_succeeded(self, info, packet):
        c = packet.chunk_num
        if info.finished_flag[c]: return
        if self.data_store: self.cob_list[c] = packet.payload_s
        info.finished_flag[c] = 1
        info.n_finished += 1
        self.send_next_interest(info)
    
    def on_rcv_meta(self, info, packet):
        pass
    
    def reset_req_status(self, info):
        self.req_flag = np.zeros(info.count)
        self.req_tail_index = self.rcv_tail_index
        while self.req_tail_index < info.count and info.finished_flag[self.req_tail_index]:
            self.req_tail_index += 1
    
    def send_interests_with_pipeline(self, info):
        to_index = min(info.count, self.req_tail_index + self.pipeline)
        for i in range(self.req_tail_index, to_index):
            if info.finished_flag[i]: continue
            self.cef_handle.send_interest(info.name, i)
            self.req_flag[i] = 1
    
    def send_next_interest(self, info):
        while self.rcv_tail_index < info.count and info.finished_flag[self.rcv_tail_index]:
            self.rcv_tail_index += 1
        while (self.req_tail_index < info.count and 
            (info.finished_flag[self.req_tail_index] or self.req_flag[self.req_tail_index])):
            self.req_tail_index += 1
        if self.req_tail_index < info.count:
            self.cef_handle.send_interest(info.name, self.req_tail_index)
            self.req_flag[self.req_tail_index] = 1
        
class CefAppProducer(CefApp):
    def __init__(self, cef_handle, 
        data="hello", cob_len=1024, timeout_limit=2, enable_log=True):
        super(CefAppProducer, self).__init__(
            cef_handle, "Interest", "send", timeout_limit, enable_log)
        self.data = data
        self.cob_len = cob_len
        data_len = len(self.data)
        self.cob_count = ((data_len - 1) // self.cob_len) + 1
    
    def resolve_count(self, name):
        data_len = len(self.data)
        self.cob_count = ((data_len - 1) // self.cob_len) + 1
        return self.cob_count
        
    def on_start(self, info):
        self.log("Receiving Interest...")
        self.cef_handle.register(info.name)
        # self.cef_handle.register(info.metaname)

    def continues_to_run(self, info):
        return True

    def on_rcv_failed(self, info):
        pass

    def on_rcv_succeeded(self, info, packet):
        # self.cef_handle.send_data(info.name, packet.chunk_num, "hello")
        c = packet.chunk_num
        if c >= info.count: return
        offset = c * self.cob_len
        cob = self.data[offset:offset + self.cob_len]
        self.cef_handle.send_data(info.name, cob, c)
        if not info.finished_flag[c]: info.n_finished += 1
        info.finished_flag[c] = 1

    def on_rcv_meta(self, info, packet):
        self.log("Receive request for meta info")
        self.cef_handle.send_data(packet.name, str(info.count), packet.chunk_num)
