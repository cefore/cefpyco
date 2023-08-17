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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>

#include "cefpyco_util.h"
#include "cpcparse_type.h"
#include "cpcparse_tlv.h"
#include "cpcparse_interest.h"

static int is_not_targeted(unsigned char* buf, int offset);
static int parse_interest_header(
    cpcparse_parse_info* info, int offset, cefpyco_app_frame* app_frame);
static int parse_name_tlv_header(
    cpcparse_parse_info* info, int offset, cefpyco_app_frame* app_frame);
static int seek_chunk_number(
    cpcparse_parse_info* info, int offset, cefpyco_app_frame* app_frame);

int cpcparse_try_parse_interest(
    cpcparse_parse_info* info,
    cefpyco_app_frame* app_frame
) {
    /* 
        Outdated after Cefore 0.7.5. 
        Use cpcparse_try_parse_app_request, which is based on 
        cef_client_request_get_with_info, insteadly. 
    */
    int res = 0;
    int offset = info->offset;
MILESTONE
    
    if (is_not_targeted(info->buf, offset)) { return 0; }
    if ((info->len - offset) < 8) { return -1; } // Too short body
    
    res = parse_interest_header(info, offset, app_frame);
    if (res < 0) { return -1; }
    offset += res;
    
    res = parse_name_tlv_header(info, offset, app_frame);
    if (res < 0) { return -1; }
    offset += res;
    
    res = seek_chunk_number(info, offset, app_frame);
    if (res < 0) { return -1; }
	
    app_frame->payload = (unsigned char *)CefpycoC_Null_Msg;
    app_frame->payload_len = 0;
	return app_frame->actual_data_len;
}

static int is_not_targeted(unsigned char* buf, int offset) {
    return
        (buf[offset]     != CefC_Version) ||
        (buf[offset + 1] != CefC_PT_INTEREST);
}

static int parse_interest_header(
    cpcparse_parse_info* info,
    int offset,
    cefpyco_app_frame* app_frame
) {
    int pkt_len;
    struct fixed_hdr* fix_hdr;
    
    fix_hdr = (struct fixed_hdr*)(&info->buf[offset]);
    pkt_len = ntohs (fix_hdr->pkt_len);
    if (info->offset + pkt_len > info->len) { return -1; } // Too short body
    
    app_frame->version = CefC_Version;
    app_frame->type = CefC_PT_INTEREST;
    app_frame->actual_data_len = pkt_len;
    return fix_hdr->hdr_len + CefC_S_TLF;
}

static int parse_name_tlv_header(
    cpcparse_parse_info* info,
    int offset,
    cefpyco_app_frame* app_frame
) {
    struct tlv_hdr* thdr;
    uint16_t sub_type;
    uint16_t sub_length;

    thdr = (struct tlv_hdr*) &info->buf[offset];
    sub_type 	= ntohs (thdr->type);
    sub_length  = ntohs (thdr->length);
    if (sub_type != CefC_T_NAME) { return -1; } // Invalid Type
    
    app_frame->name = &(info->buf[offset + CefC_S_TLF]);
    app_frame->name_len = sub_length;
    return CefC_S_TLF;
}

static int seek_chunk_number(
    cpcparse_parse_info* info,
    int offset,
    cefpyco_app_frame* app_frame
) {
    struct tlv_hdr* thdr;
    uint16_t sub_type;
    uint16_t sub_length;
    int pkt_len = app_frame->actual_data_len;
    
    app_frame->chunk_num = 0;
    while ((offset - info->offset) < pkt_len) {
        thdr = (struct tlv_hdr*) &info->buf[offset];
        sub_type 	= ntohs (thdr->type);
        sub_length  = ntohs (thdr->length);
        offset += CefC_S_TLF;
        if (sub_type == CefC_T_CHUNK) {
            app_frame->chunk_num = cpcparse_tlv_read_segnum((unsigned char*)thdr);
        }
        offset += sub_length;
    }
    return 0;
}
