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

#include "cefpyco_parse.h"
#include "cefpyco_util.h"
#include "cpcparse_tlv.h"
#include "cpcparse_type.h"

static cpcparse_parse_info info = {NULL, 0, 0};

int cpc_buf_remains() {
    if (info.offset >= info.len) {
        info.len = 0;
        info.offset = 0;
    }
    if (info.len > 0) return info.len;
    return 0;
}

int cpc_parse_info(unsigned char* _buf, int _len, cefpyco_app_frame* app_frame) {
    int                     parsed_offset, res;
    static const try_parse* pi = NULL;

    info.buf = _buf;
    info.len = _len;

    MILESTONE
    /* Seek the head of message 		*/
    res = 0;
    memset(app_frame, 0, sizeof(cefpyco_app_frame));
    while (res == 0 && info.offset < info.len) {
        for (pi = try_parser_list; *pi != NULL; pi++) {
            res = (*pi)(&info, app_frame);
            if (res != 0) break;
        }
    }
    MILESTONE
    if (res > 0) {
        /* Complete */
        parsed_offset = info.offset;
        info.offset += res;
        return parsed_offset;
    } else {
        /* Failed to seek */
        info.len = 0;
        info.offset = 0;
        cpc_set_null_name_info(app_frame);
        return -1;
    }
}

/* The name tlv is a sequence of the following 3 segments:
 *   - TYPE (2bytes)
 *   - LENGTH (2bytes)
 *   - VALUE (LENGTH bytes).
 * In cefore app, a name always has a segment for
 * chunk number of 4 byte length, that is,
 * name_len is 8 bytes larger than
 * the length for only name segments.
 *
 * For example, a name 'ccnx:/a' is 13 byte length as follows:
 *   - Name segument 'a' (5B): T(2B) + L(2B) + 'a'(1B)
 *   - Segment number   (8B): T(2B) + L(2B) + 0x0000(4B)
 */
int cpc_convert_nametlv_to_readable_str(unsigned char* tlv, int name_len, unsigned char* output) {
    /*
        Outdated after Cefore 0.7.5.
        Use cef_frame_conversion_name_to_uri_without_chunknum insteadly.
    */
    int            offset;
    unsigned int   len, rnlen;
    unsigned char* p = output + 5;

    // Need a segment of 'Segment number' (8 bytes).
    if (name_len < 8) return -1;

    memcpy(output, "ccnx:/", 5);
    offset = 0;
    rnlen = 5;
    while (offset < name_len - 8) {
        len = cpcparse_tlv_read_length(tlv + offset);
        if (len <= 0) { return -1; }
        memcpy(p, tlv + offset + 4, len);
        p += len;
        *p = '/';
        p++;
        offset += 4 + len;
        rnlen += len;
    }
    *(p - 1) = '\0';

    return rnlen;
}

#ifdef CEFPYCO_DEBUG
void cpc_parse_show_current_state() {
    printf(
        "\n"
        "   len: %d\n"
        "offset: %d\n",
        info.len, info.offset);
    cpc_force_print(info.buf + info.offset, info.len - info.offset);
}
#endif
