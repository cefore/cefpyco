/*
 * Copyright (c) 2016--2023, National Institute of Information and Communications
 * Technology (NICT). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the NICT nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NICT AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE NICT OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "cefpyco.h"
// #include <Python.h>
#include <assert.h>
#include <time.h>

static const struct timespec slee_time_ns = {0, 10 * 1000 * 1000};

#define TEST_CEF_PORT (CefC_Unset_Port)
#define TEST_CEF_CONF ("")

CefT_Client_Handle cphp, cphc;

static void setup_cefore_handler() {
    cphp = create_cef_handler(TEST_CEF_PORT, TEST_CEF_CONF);
    assert(cphp > 0);
    nanosleep(&slee_time_ns, NULL);
    cphc = create_cef_handler(TEST_CEF_PORT, TEST_CEF_CONF);
    assert(cphc > 0);
    nanosleep(&slee_time_ns, NULL);
}

static void teardown_cefore_handler() {
    int res;
    nanosleep(&slee_time_ns, NULL);
    res = destroy_cef_handler(cphp);
    assert(res == 0);
    res = destroy_cef_handler(cphc);
    assert(res == 0);
}

static void test_begin_end_cefore_simple() {
    int res;
    res = begin_cef(TEST_CEF_PORT, TEST_CEF_CONF);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    res = end_cef();
    assert(res == 0);
}

static void test_send_interest_simple() {
    int res;

    res = begin_cef(TEST_CEF_PORT, TEST_CEF_CONF);
    assert(res == 0);
    res = send_interest("ccnx:/csrc/b", 0);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    res = end_cef();
    assert(res == 0);
}

static void test_begin_end_cefore() {
    CefT_Client_Handle handler;
    int                res;
    handler = create_cef_handler(TEST_CEF_PORT, TEST_CEF_CONF);
    assert(handler > 0);
    nanosleep(&slee_time_ns, NULL);
    res = destroy_cef_handler(handler);
    assert(res == 0);
}

static void test_launch_two_cefore() {
    setup_cefore_handler();
    teardown_cefore_handler();
}

static void test_send_and_receive_interest() {
    CefT_CcnMsg_OptHdr opt_i, opt_d;
    CefT_CcnMsg_MsgBdy params_i;
    CefT_CcnMsg_MsgBdy params_d;
    cefpyco_app_frame  frame;
    char               uri_name[256];
    int                res;

    setup_cefore_handler();
    res = cph_register_name(cphp, CefC_App_Reg, "ccnx:/csrc/a");
    assert(res >= 0);
    nanosleep(&slee_time_ns, NULL);

    set_basic_interest_params(&opt_i, &params_i, "ccnx:/csrc/a", 0);
    res = cph_send_interest(cphc, &opt_i, &params_i);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    res = cph_receive(cphp, &frame, 4000, 1);
    assert(res == 0);
    cef_frame_conversion_name_to_uri(frame.name, frame.name_len, uri_name);
    assert(strncmp(uri_name, "ccnx:/csrc/a", sizeof(char) * 11) == 0);
    assert(frame.chunk_num == 0);
    nanosleep(&slee_time_ns, NULL);

    set_basic_interest_params(&opt_i, &params_i, "ccnx:/csrc/a", 1);
    res = cph_send_interest(cphc, &opt_i, &params_i);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    res = cph_receive(cphp, &frame, 4000, 1);
    assert(res == 0);
    cef_frame_conversion_name_to_uri(frame.name, frame.name_len, uri_name);
    assert(strncmp(uri_name, "ccnx:/csrc/a", sizeof(char) * 11) == 0);
    assert(frame.chunk_num == 1);
    nanosleep(&slee_time_ns, NULL);

    teardown_cefore_handler();
}

static void test_request_and_satisfy() {
    CefT_CcnMsg_OptHdr opt_i, opt_d;
    CefT_CcnMsg_MsgBdy params_i;
    CefT_CcnMsg_MsgBdy params_d;
    cefpyco_app_frame  frame;
    char               uri_name[256];
    int                res;

    setup_cefore_handler();
    res = cph_register_name(cphp, CefC_App_Reg, "ccnx:/csrc/c");
    assert(res >= 0);
    nanosleep(&slee_time_ns, NULL);
    set_basic_interest_params(&opt_i, &params_i, "ccnx:/csrc/c", 0);
    res = cph_send_interest(cphc, &opt_i, &params_i);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    res = cph_receive(cphp, &frame, 4000, 0);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    set_basic_data_params(&opt_d, &params_d, "ccnx:/csrc/c", 0, "test.", 5);
    res = cph_send_data(cphp, &opt_d, &params_d);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    res = cph_receive(cphc, &frame, 4000, 0);
    assert(res == 0);
    cef_frame_conversion_name_to_uri(frame.name, frame.name_len, uri_name);
    assert(strncmp(uri_name, "ccnx:/csrc/c", sizeof(char) * 11) == 0);
    assert(strncmp((const char*)frame.payload, "test.", sizeof(char) * 5) == 0);
    teardown_cefore_handler();
}

static void test_request_and_satisfy_with_cast() {
    CefT_CcnMsg_OptHdr opt_i, opt_d;
    CefT_CcnMsg_MsgBdy params_i;
    CefT_CcnMsg_MsgBdy params_d;
    long long          handler_p, handler_c;
    cefpyco_app_frame  frame;
    char               uri_name[256];
    int                res;

    setup_cefore_handler();
    handler_p = (long long)cphp;
    handler_c = (long long)cphc;
    res = cph_register_name((CefT_Client_Handle)handler_p, CefC_App_Reg, "ccnx:/csrc/d");
    assert(res >= 0);
    nanosleep(&slee_time_ns, NULL);
    set_basic_interest_params(&opt_i, &params_i, "ccnx:/csrc/d", 0);
    res = cph_send_interest((CefT_Client_Handle)handler_c, &opt_i, &params_i);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    res = cph_receive((CefT_Client_Handle)handler_p, &frame, 4000, 0);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    set_basic_data_params(&opt_d, &params_d, "ccnx:/csrc/d", 0, "test.", 5);
    res = cph_send_data((CefT_Client_Handle)handler_p, &opt_d, &params_d);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    res = cph_receive((CefT_Client_Handle)handler_c, &frame, 4000, 0);
    assert(res == 0);
    cef_frame_conversion_name_to_uri(frame.name, frame.name_len, uri_name);
    assert(strncmp(uri_name, "ccnx:/csrc/d", sizeof(char) * 11) == 0);
    assert(strncmp((const char*)frame.payload, "test.", sizeof(char) * 5) == 0);
    teardown_cefore_handler();
}

static void test_send_and_receive_smi() {
    CefT_CcnMsg_OptHdr opt_i, opt_d;
    CefT_CcnMsg_MsgBdy params_i;
    CefT_CcnMsg_MsgBdy params_d;
    cefpyco_app_frame  frame;
    char               uri_name[256];
    int                res;

    setup_cefore_handler();
    res = cph_register_name(cphp, CefC_App_Reg, "ccnx:/csrc/d");
    assert(res >= 0);
    nanosleep(&slee_time_ns, NULL);

    set_basic_interest_params(&opt_i, &params_i, "ccnx:/csrc/d", 0);
    res = cph_send_interest(cphc, &opt_i, &params_i);
    assert(res == 0);
    nanosleep(&slee_time_ns, NULL);
    res = cph_receive(cphp, &frame, 4000, 1);
    assert(res == 0);
    cef_frame_conversion_name_to_uri(frame.name, frame.name_len, uri_name);
    assert(strncmp(uri_name, "ccnx:/csrc/d", sizeof(char) * 11) == 0);
    assert(frame.chunk_num == 0);
    nanosleep(&slee_time_ns, NULL);

    teardown_cefore_handler();
}

int main(int argc, char const* argv[]) {
    int res;
    printf("test_begin_end_cefore\n");
    test_begin_end_cefore();
    printf("test_begin_end_cefore_simple\n");
    test_begin_end_cefore_simple();
    printf("test_launch_two_cefore\n");
    test_launch_two_cefore();
    printf("test_send_and_receive_interest\n");
    test_send_and_receive_interest();
    printf("test_request_and_satisfy\n");
    test_request_and_satisfy();
    printf("test_request_and_satisfy_with_cast\n");
    test_request_and_satisfy_with_cast();
    return 0;
}
