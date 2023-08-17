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

#include <Python.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "cefpyco.h"
#include "cefpyco_parse.h"
#include "cefpyco_util.h"

static CefT_Client_Handle fhdl = 0;
#define CefpycoC_Buffer_Size 16777216
static unsigned char buf[CefpycoC_Buffer_Size];

static char err_msg[1024];

typedef void cefore_reg_api(CefT_Client_Handle, uint16_t, const unsigned char*, uint16_t);
static int
call_register_api(CefT_Client_Handle handler, cefore_reg_api api, uint16_t func, const char* uri);
static int wait_receive(CefT_Client_Handle handler, int timeout_ms, int error_on_timeout);
static int exit_with_error_msg(CefT_Client_Handle handler, const char* msg);

int cefpyco_enable_log;

unsigned long begin_cef(int port_num, char* conf_path) {
    Py_Initialize();
    CefT_Client_Handle res;
    res = create_cef_handler(port_num, conf_path);
    if (res > 0) {
        fhdl = res;
        return 0;
    } else {
        return res;
    }
}

int end_cef() {
    destroy_cef_handler(fhdl);
    fhdl = 0;
    return 0;
}

int send_interest(const char* uri, int chunk_num) {
    int                res;
    CefT_CcnMsg_OptHdr opt_i;
    CefT_CcnMsg_MsgBdy params_i;

    res = set_basic_interest_params(&opt_i, &params_i, uri, chunk_num);
    if (res < 0) return exit_with_error_msg(fhdl, "Failed to set Interest parameters.");
    return cph_send_interest(fhdl, &opt_i, &params_i);
}

int send_data(const char* uri, int chunk_num, const char* payload, int payload_len) {
    int                res;
    CefT_CcnMsg_OptHdr opt_d;
    CefT_CcnMsg_MsgBdy params_d;

    res = set_basic_data_params(&opt_d, &params_d, uri, chunk_num, payload, payload_len);
    if (res < 0) return exit_with_error_msg(fhdl, "Failed to set Data parameters.");
    return cph_send_data(fhdl, &opt_d, &params_d);
}

int register_name(const char* uri) { return cph_register_name(fhdl, CefC_App_Reg, uri); }
int deregister_name(const char* uri) { return cph_register_name(fhdl, CefC_App_DeReg, uri); }

int receive(cefpyco_app_frame* app_frame, int error_on_timeout, int timeout_ms) {
    return cph_receive(fhdl, app_frame, timeout_ms, error_on_timeout);
}

CefT_Client_Handle create_cef_handler(int port_num, char* conf_path) {
    MILESTONE
    CefT_Client_Handle handler = (CefT_Client_Handle)NULL;
    int                res;
    cef_log_init("cefpyco", 1);
    cef_frame_init();
    res = cef_client_init(port_num, conf_path);
    if (res < 0) return exit_with_error_msg(handler, "Failed to init the client package.");
    handler = cef_client_connect();
    if (handler < 1) return exit_with_error_msg(handler, "Failed to create the client handle.");
    return handler;
}

int destroy_cef_handler(CefT_Client_Handle handler) {
    MILESTONE
    struct timespec waittime_ts = {0, 1000L * 1000L};  // Sleep 1 ms.
    nanosleep(&waittime_ts, NULL);
    if (handler > 0) cef_client_close(handler);
    return 0;
}

int cph_build_hdrorg_value(unsigned char* buff, CefT_HdrOrg_Params* org) {
    MILESTONE
    return cef_frame_build_hdrorg_value(buff, org);
}

int cph_build_msgorg_value(unsigned char* buff, CefT_MsgOrg_Params* org) {
    MILESTONE
    return cef_frame_build_msgorg_value(buff, org);
}

int cph_send_interest(
    CefT_Client_Handle  handler,  // cefore handler
    CefT_CcnMsg_OptHdr* opt_i,
    CefT_CcnMsg_MsgBdy* params_i) {
    MILESTONE
    if (handler < 1) return exit_with_error_msg(handler, "Handle must be created.");

    cef_client_interest_input(handler, opt_i, params_i);
    return 0;
}

int cph_send_data(
    CefT_Client_Handle  handler,  // cefore handler
    CefT_CcnMsg_OptHdr* opt_d,
    CefT_CcnMsg_MsgBdy* params_d) {
    MILESTONE
    if (handler < 1) return exit_with_error_msg(handler, "Handle must be created.");

    cef_client_object_input(handler, opt_d, params_d);
    return 0;
}

int cph_register_name(
    CefT_Client_Handle handler,  // cefore handler
    uint16_t           func,
    const char*        uri) {
    MILESTONE
    return call_register_api(handler, cef_client_prefix_reg, func, uri);
}

static int call_register_api(
    CefT_Client_Handle handler,  // cefore handler
    cefore_reg_api     api,
    uint16_t           func,
    const char*        uri) {
    MILESTONE
    int           res;
    unsigned char name[CefpycoC_Name_Len];
    res = cef_frame_conversion_uri_to_name(uri, name);
    if (res < 0) return exit_with_error_msg(handler, CefpycoC_Err_Invalid_URI);
    api(handler, func, name, (uint16_t)res);
    return res;
}

int cph_receive(
    CefT_Client_Handle handler,  // cefore handler
    cefpyco_app_frame* app_frame,
    int                timeout_ms,
    int                error_on_timeout) {
    MILESTONE
    int res, rec_len;

    cpc_set_null_name_info(app_frame);
    if (handler < 1) return exit_with_error_msg(handler, "Handle must be created.");
    rec_len = wait_receive(handler, timeout_ms, error_on_timeout);

#ifdef CEFPYCO_DEBUG
    fprintf(stderr, "\nrec_len: %d\n", rec_len);
    cpc_force_print(buf, rec_len);
#endif

    MILESTONE
    if (rec_len < 0) return exit_with_error_msg(handler, "Failed to read cefnetd buf.");
    // if (rec_len == 0) return 0;
    res = cpc_parse_info(buf, rec_len, app_frame);
    if (rec_len > 0) {
        if (res < 0) return exit_with_error_msg(handler, "Failed to parse cefnetd buf.");
        if (app_frame->actual_data_len == 0)
            return exit_with_error_msg(handler, "Retrieved data is empty.");
    }

    MILESTONE
#ifdef CEFPYCO_DEBUG
    fprintf(stderr, "\nparse_len: %d\n", res);
    show_app_frame(app_frame);
#endif

    MILESTONE
    return 0;
}

int set_basic_interest_params(
    CefT_CcnMsg_OptHdr* opt_i, CefT_CcnMsg_MsgBdy* params_i, const char* uri, int chunk_num) {
    int res;

    memset(opt_i, 0, sizeof(CefT_CcnMsg_OptHdr));
    memset(params_i, 0, sizeof(CefT_CcnMsg_MsgBdy));
    res = cef_frame_conversion_uri_to_name(uri, params_i->name);
    if (res < 0) return exit_with_error_msg(fhdl, CefpycoC_Err_Invalid_URI);
    params_i->name_len = res;

    params_i->hoplimit = 32;
    opt_i->lifetime_f = 1;
    opt_i->lifetime = 4000ull; /* 4 seconds */

    params_i->chunk_num_f = 1;
    params_i->chunk_num = chunk_num;

    return 0;
}

int set_basic_data_params(
    CefT_CcnMsg_OptHdr* opt_d,
    CefT_CcnMsg_MsgBdy* params_d,
    const char*         uri,
    int                 chunk_num,
    const char*         payload,
    int                 payload_len) {
    struct timeval now_t;
    uint64_t       now_ms;
    int            res;

    memset(opt_d, 0, sizeof(CefT_CcnMsg_OptHdr));
    memset(params_d, 0, sizeof(CefT_CcnMsg_MsgBdy));
    res = cef_frame_conversion_uri_to_name(uri, params_d->name);
    if (res < 0) return exit_with_error_msg(fhdl, CefpycoC_Err_Invalid_URI);
    params_d->name_len = res;

    memcpy(params_d->payload, payload, sizeof(char) * payload_len);
    params_d->payload_len = payload_len;

    params_d->chunk_num_f = 1;
    params_d->chunk_num = chunk_num;
    gettimeofday(&now_t, NULL);
    now_ms = now_t.tv_sec * 1000ull + now_t.tv_usec / 1000ull;
    params_d->expiry = now_ms + 36000ull * 1000ull; /* 36000 seconds (10 hours) */

    opt_d->cachetime_f = 1;
    opt_d->cachetime = now_ms + 36000ull * 1000ull; /* 36000 seconds (10 hours) */

    return 0;
}

int convert_nametlv_to_readable_str(unsigned char* tlv, int name_len, char* output) {
    return cef_frame_conversion_name_to_uri_without_chunknum(tlv, name_len, output);
}

int show_app_frame(cefpyco_app_frame* app_frame) {
    unsigned char* payload = (unsigned char*)CefpycoC_Null_Msg;
    char           name[CefpycoC_Name_Len];

    if (app_frame == NULL || app_frame->actual_data_len == 0) return -1;
    convert_nametlv_to_readable_str(app_frame->name, app_frame->name_len, name);
    if (app_frame->payload != 0 && app_frame->payload_len > 0) {
        payload = app_frame->payload;
        payload[app_frame->payload_len] = '\0';
    } else {
        payload = (unsigned char*)CefpycoC_Null_Msg;
    }
    cef_log_write(
        CefC_Log_Info,
        "\n[Packet info]\n"
        "  *         version: %08x\n"
        "  *            type: %08x\n"
        "  * actual_data_len: %llu\n"
        "  *            name: '%s'\n"
        "  *        name_len: %hu\n"
        "  *       chunk_num: %u\n"
        "  *   end_chunk_num: %u\n"
        "  *         payload: '%s'\n"
        "  *     payload_len: %hu\n",
        app_frame->version,          //
        app_frame->type,             //
        app_frame->actual_data_len,  //
        name,                        //
        app_frame->name_len,         //
        app_frame->chunk_num,        //
        app_frame->end_chunk_num,    //
        payload,                     //
        app_frame->payload_len       //
    );
#ifdef CEFPYCO_DUMP
    fprintf(stderr, "\nlen: %d\n", app_frame->payload_len);
    fprintf(stderr, "payload: ");
    cpc_force_print(app_frame->payload, app_frame->payload_len);
#endif
    return 0;
}

const char* get_error_msg() { return err_msg; }

static void set_timespec(struct timespec* ts, long nano_sec) {
    const long one_sec_ns = 1000L * 1000L * 1000L;
    ts->tv_sec = nano_sec / one_sec_ns;
    ts->tv_nsec = nano_sec % one_sec_ns;
}

static int wait_receive(CefT_Client_Handle handler, int timeout_ms, int error_on_timeout) {
    MILESTONE
    int res = 0;
    int elapsedtime_ms = 0;

    if (handler < 1) return exit_with_error_msg(handler, "Handle must be created.");
    res = cpc_buf_remains();
    if (res) return res;
    while (1) {
        Py_BEGIN_ALLOW_THREADS;
        res = cef_client_read(handler, buf, CefpycoC_Buffer_Size);  // It takes 1 sec at most.
        Py_END_ALLOW_THREADS;
        elapsedtime_ms += 1000;
        if (res > 0) break;
        if (elapsedtime_ms >= timeout_ms) {
            if (error_on_timeout) {
                if (cefpyco_enable_log) {
                    cef_log_write(
                        CefC_Log_Info, "\033[101m*** [CAUTION] Stop to wait. ***\033[0m\n");
                }
                return -1;
            } else {
                return 0;
            }
        }
    }
    return res;
}

// Deprecated because cef_client_read takes 1 sec from cefore-0.8.2.2
static int wait_receive_old(CefT_Client_Handle handler, int timeout_ms, int error_on_timeout) {
    MILESTONE
    const long      ms_to_ns = 1000L * 1000L;
    const long      waittime_step_ns = 500L * 1000L;  // 500 us
    int             res = 0;
    time_t          waittime_sec = 0;
    long long       elapsedtime_ns = 0L;
    long long       timeout_ns;
    long            waittime_ns;
    long            tryn = 1L;
    struct timespec waittime_ts = {0, 1000L};  // 1 us

    if (timeout_ms < 0) {
        timeout_ns = CefpycoC_Default_Timeout * ms_to_ns;
    } else {
        timeout_ns = timeout_ms * ms_to_ns;
    }

    if (handler < 1) return exit_with_error_msg(handler, "Handle must be created.");
    res = cpc_buf_remains();
    if (res) return res;
    while (1) {
        res = cef_client_read(handler, buf, CefpycoC_Buffer_Size);
        if (res > 0) break;
        if (elapsedtime_ns >= timeout_ns) {  // 18-tries take 1s, 29-tries take 4s
            if (error_on_timeout) {
                if (cefpyco_enable_log) {
                    cef_log_write(
                        CefC_Log_Info, "\033[101m*** [CAUTION] Stop to wait. ***\033[0m\n");
                }
                return -1;
            } else {
                return 0;
            }
        }

        Py_BEGIN_ALLOW_THREADS;
        nanosleep(&waittime_ts, NULL);
        Py_END_ALLOW_THREADS;

        waittime_ns = waittime_step_ns * tryn * tryn;
        elapsedtime_ns += waittime_ns;
        set_timespec(&waittime_ts, waittime_ns);
        tryn++;
    }
    return res;
}

static int exit_with_error_msg(CefT_Client_Handle handler, const char* msg) {
    fprintf(stderr, "[ERROR] %s\n", msg);
    sprintf(err_msg, "%s\n", msg);
    // destroy_cef_handler(handler);
    return -1;
}
