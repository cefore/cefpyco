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

#ifndef __CEFPYCO_DEF_HEADER__
#define __CEFPYCO_DEF_HEADER__

#include <cefore/cef_client.h>
#include <cefore/cef_define.h>
#include <cefore/cef_frame.h>
#include <cefore/cef_log.h>

// #define CEFPYCO_QUIET
// #define CEFPYCO_DEBUG
// #define CEFPYCO_DUMP

#define CefpycoC_Default_Timeout (4000)
#define CefpycoC_Null_Msg        ("(empty)")
#define CefpycoC_Name_Len        (2048)
#define CefpycoC_APIData_Len     (65536)
#define CefpycoC_Payload_Len     (CefpycoC_APIData_Len - CefpycoC_Name_Len)

#define CefpycoC_Err_Invalid_URI "Invalid URI is specified."

enum cefpycoC_packet_flags {
    REGULAR = (0x00000000ul),
    ONLY_SYMBOLIC = (0x00000001ul),
    ONLY_LONGLIFE = (0x00000002ul),
    SYMBOLIC = (0x00000003ul),
};

typedef struct {
    uint32_t       version;
    uint32_t       type;
    uint32_t       returncode;
    uint32_t       flags;
    uint64_t       actual_data_len;
    unsigned char* name;
    uint16_t       name_len;
    uint32_t       chunk_num;
    int64_t        end_chunk_num;
    unsigned char* payload;
    uint16_t       payload_len;
    unsigned char* hdr_org_val;
    uint16_t       hdr_org_len;
    unsigned char* msg_org_val;
    uint16_t       msg_org_len;
} cefpyco_app_frame;

#endif
