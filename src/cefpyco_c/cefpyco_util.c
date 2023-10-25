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

#ifdef CEFPYCO_DUMP
static unsigned char outputbuf[65536];
#endif

void cpc_force_print(const unsigned char* src, int len) {
#ifdef CEFPYCO_DUMP
    outputbuf[0] = '[';
    outputbuf[1] = '\0';
    for (int i = 0; i < len; i++) {
        sprintf(outputbuf, "%s%2c[%02x]", outputbuf, isalnum(src[i]) ? src[i] : ' ', src[i]);
        if ((i % 8) == 7) sprintf(outputbuf, "%s\n", outputbuf);
    }
    fprintf(stderr, "%s\n", outputbuf);
#endif
}

void cpc_set_null_name_info(cefpyco_app_frame* app_frame) {
    app_frame->version = 0;
    app_frame->type = 0;
    app_frame->actual_data_len = 0;
    app_frame->name = (unsigned char*)CefpycoC_Null_Msg;
    app_frame->name_len = 0;
    app_frame->chunk_num = 0;
    app_frame->end_chunk_num = 0;
    app_frame->payload = (unsigned char*)CefpycoC_Null_Msg;
    app_frame->payload_len = 0;
    app_frame->hdr_org_val = NULL;
    app_frame->hdr_org_len = 0;
    app_frame->msg_org_val = NULL;
    app_frame->msg_org_len = 0;
}
