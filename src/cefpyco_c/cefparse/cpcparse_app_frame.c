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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "cefpyco_util.h"
#include "cpcparse_app_frame.h"
#include "cpcparse_tlv.h"
#include "cpcparse_type.h"

static int is_not_targeted_header(struct cef_app_frame* wrk_frame);
static int is_not_targeted_footer(cpcparse_parse_info* info, struct cef_app_frame* wrk_frame);

int cpcparse_try_parse_app_frame(cpcparse_parse_info* info, cefpyco_app_frame* app_frame) {
    /*
        Outdated after Cefore 0.7.5.
        Use cpcparse_try_parse_app_frame_7_5, which is based on
        cef_client_payload_get_with_info, insteadly.
    */
    struct cef_app_frame* wrk_frame;
    // uint32_t magic_no = CefC_App_Magic_No;
    int acutal_len;
    MILESTONE

    wrk_frame = (struct cef_app_frame*)&info->buf[info->offset];
    if (is_not_targeted_header(wrk_frame)) { return 0; }

    acutal_len = wrk_frame->actual_data_len;
    if (info->offset + acutal_len > info->len) { return -1; }  // Too short body

    if (is_not_targeted_footer(info, wrk_frame)) { return 0; }

    app_frame->version = wrk_frame->version;
    app_frame->type = wrk_frame->type;
    app_frame->flags = 0x00000000ul;
    app_frame->actual_data_len = acutal_len;
    app_frame->name = &(wrk_frame->data_entity[0]);
    app_frame->name_len = wrk_frame->name_len;
    app_frame->chunk_num = wrk_frame->chunk_num;
    app_frame->payload = &(wrk_frame->data_entity[wrk_frame->name_len]);
    app_frame->payload_len = wrk_frame->payload_len;
    return acutal_len;
}

static int is_not_targeted_header(struct cef_app_frame* wrk_frame) {
    return (wrk_frame->version != CefC_App_Version) || (wrk_frame->type != CefC_App_Type_Internal);
}

static int is_not_targeted_footer(cpcparse_parse_info* info, struct cef_app_frame* wrk_frame) {
    uint32_t magic_no = CefC_App_Magic_No;
    int      magic_no_offset = info->offset + wrk_frame->actual_data_len - sizeof(magic_no);
    int      cmp_res = memcmp(
        (const void*)&magic_no, (const void*)&(info->buf[magic_no_offset]), sizeof(magic_no));
    return cmp_res != 0;
}
