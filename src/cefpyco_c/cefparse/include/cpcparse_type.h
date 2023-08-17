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

#ifndef __CEFPYCO_PARSE_TYPE_HEADER__
#define __CEFPYCO_PARSE_TYPE_HEADER__

#include "cefpyco_def.h"
#include <cefore/cef_client.h>
#include <cefore/cef_define.h>
#include <cefore/cef_frame.h>
#include <cefore/cef_log.h>

#define CPC_CCNX_VERSION     (0x01)
#define CPC_CCNX_PT_INTEREST (0x00)
#define CPC_CCNX_PT_CONTENT  (0x01)
#define CPC_CCNX_PT_RETURN   (0x02)

typedef struct {
    unsigned char*         buf;
    int                    len;
    int                    offset;
    struct cef_app_frame   wrk_frame;
    struct cef_app_request wrk_request;
} cpcparse_parse_info;

typedef int (*try_parse)(  //
    cpcparse_parse_info* info,
    cefpyco_app_frame*   app_frame);

int cpcparse_stop_parse_and_next_byte(  //
    cpcparse_parse_info* info,
    cefpyco_app_frame*   app_frame);

extern const try_parse try_parser_list[];

#endif
