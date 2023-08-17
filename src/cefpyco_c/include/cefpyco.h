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

#ifndef __CEFPYCO_HEADER__
#define __CEFPYCO_HEADER__

#include "cefpyco_def.h"

CefT_Client_Handle create_cef_handler(int port_num, char* conf_path);
int                destroy_cef_handler(CefT_Client_Handle handler);

// int cph_send_interest(CefT_Client_Handle handler, const char *uri, int chunk_num);
// int cph_send_data(CefT_Client_Handle handler, const char *uri, int chunk_num,
//      const char *payload, int payload_len);
int cph_send_interest(
    CefT_Client_Handle handler, CefT_CcnMsg_OptHdr* opt_i, CefT_CcnMsg_MsgBdy* params_i);
int cph_send_data(
    CefT_Client_Handle handler, CefT_CcnMsg_OptHdr* opt_d, CefT_CcnMsg_MsgBdy* params_d);

int cph_register_name(CefT_Client_Handle handler, uint16_t func, const char* uri);
int cph_receive(
    CefT_Client_Handle handler, cefpyco_app_frame* app_frame, int timeout_ms, int error_on_timeout);

int cph_build_hdrorg_value(unsigned char* buff, CefT_HdrOrg_Params* org);
int cph_build_msgorg_value(unsigned char* buff, CefT_MsgOrg_Params* org);

int set_basic_interest_params(
    CefT_CcnMsg_OptHdr* opt_i, CefT_CcnMsg_MsgBdy* params_i, const char* uri, int chunk_num);
int set_basic_data_params(
    CefT_CcnMsg_OptHdr* opt_d,
    CefT_CcnMsg_MsgBdy* params_d,
    const char*         uri,
    int                 chunk_num,
    const char*         payload,
    int                 payload_len);

unsigned long begin_cef(int port_num, char* conf_path);
int           end_cef(void);

int send_interest(const char* uri, int chunk_num);
int send_data(const char* uri, int chunk_num, const char* payload, int payload_len);
int register_name(const char* uri);
int deregister_name(const char* uri);
int receive(cefpyco_app_frame* app_frame, int error_on_timeout, int timeout_ms);

int convert_nametlv_to_readable_str(unsigned char* tlv, int name_len, char* output);
int show_app_frame(cefpyco_app_frame* app_frame);

const char* get_error_msg(void);
extern int  cefpyco_enable_log;

#endif
