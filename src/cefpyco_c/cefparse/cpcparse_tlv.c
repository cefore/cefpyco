/*
 * Copyright (c) 2016--2023, National Institute of Information and Communications
 * Technology (NICT). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source cod+
 * +e must retain the above copyright notice,
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
#include "cpcparse_tlv.h"

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

static unsigned char work_buff[CefpycoC_AppBuff_Size];

uint32_t cpcparse_tlv_read_length(unsigned char* tlv) {
    return ((uint32_t)tlv[2] << 8ull) + (uint32_t)tlv[3];
}

// unsigned int cpcparse_tlv_read_segnum(unsigned char* tlv) {
//    return ((uint64_t)tlv[4] << 24ull) +
//           ((uint64_t)tlv[5] << 16ull) +
//           ((uint64_t)tlv[6] <<  8ull) +
//           ((uint64_t)tlv[7] <<  0ull);
// }

/*--------------------------------------------------------------------------------------
        Obtains one Interest message from the buffer
    (Copy of cefore-0.8.2.2 - cef_client.c - cef_client_request_get_with_info)
----------------------------------------------------------------------------------------*/
int /* remaining length of buffer 			*/
cpc_client_request_get_with_info(
    unsigned char* buff, int buff_len, struct cef_app_request* app_request) {
    int                i = 0;
    struct fixed_hdr*  fix_hdr;
    uint16_t           pkt_len;
    uint8_t            hdr_len;
    CefT_CcnMsg_MsgBdy pm;
    CefT_CcnMsg_OptHdr poh;
    int                res;
    int                new_len = 0;

    /* Searches the top of the message */
    if ((buff[i] != CefC_Version) || (buff[i + 1] > CefC_PT_MAX)) {

        while (i < buff_len) {
            if ((buff[i] != CefC_Version) || (buff[i + 1] != CefC_PT_INTEREST)) {
                i += 2;
            } else {
                break;
            }
        }
        if (i >= buff_len) { return (-1); }
    }
    if ((buff_len - i) < 8) { return (-1); }

    /* Parses the message */
    fix_hdr = (struct fixed_hdr*)(&buff[i]);
    pkt_len = ntohs(fix_hdr->pkt_len);
    hdr_len = fix_hdr->hdr_len;

    if (pkt_len > (buff_len - i)) { return (-1); }

    new_len = buff_len - pkt_len;

    res = cef_frame_message_parse(&buff[i], pkt_len, hdr_len, &poh, &pm, CefC_PT_INTEREST);
    if (res < 0) {
        memcpy(&work_buff[0], &buff[buff_len - new_len], new_len);
        memcpy(&buff[0], &work_buff[0], new_len);
        return (new_len);
    }

    /* [Restriction]
     */
    /* For renovation in FY 2018, only Regular/NWProc are allowed,	*/
    /* ignoring everything else. */
    // if (pm.org.longlife_f || poh.piggyback_f || poh.bitmap_f ||
    // 	poh.number_f || poh.symbolic_code_f || poh.app_reg_f) {
    // 	memcpy (&work_buff[0], &buff[buff_len-new_len], new_len);
    // 	memcpy (&buff[0], &work_buff[0], new_len);
    // 	return(new_len);
    // }

    app_request->version = CefC_App_Version;
    app_request->type = CefC_App_Type_Internal;
    if (pm.chunk_num_f) {
        app_request->chunk_num = pm.chunk_num;
    } else {
        app_request->chunk_num = -1;
    }
    app_request->symbolic_f = 0;
    if (pm.org.symbolic_f) app_request->symbolic_f |= 1;
    if (pm.org.longlife_f) app_request->symbolic_f |= 2;

    app_request->name_len = pm.name_len;
    app_request->total_segs_len = cef_frame_get_len_total_namesegments(pm.name, pm.name_len);

    memcpy(&(app_request->data_entity[0]), pm.name, pm.name_len);
    app_request->name = &(app_request->data_entity[0]);

    app_request->hdr_org_len = poh.org_len;
    if (0 < app_request->hdr_org_len) {
        memcpy(app_request->hdr_org_val, poh.org_val, poh.org_len);
    } else {
        app_request->hdr_org_val[0] = 0x00;
    }

    app_request->msg_org_len = pm.org_len;
    if (0 < app_request->msg_org_len) {
        memcpy(app_request->msg_org_val, pm.org_val, pm.org_len);
    } else {
        app_request->msg_org_val[0] = 0x00;
    }

    if (new_len != buff_len) {
        memcpy(&work_buff[0], &buff[buff_len - new_len], new_len);
        memcpy(&buff[0], &work_buff[0], new_len);
    }
    return (new_len);
}