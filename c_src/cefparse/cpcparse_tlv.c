/*
 * Copyright (c) 2016, National Institute of Information and Communications
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>

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
 * For example, a name 'ccn:/a' is 13 byte length as follows:
 *   - Name segument 'a' (5B): T(2B) + L(2B) + 'a'(1B)
 *   - Segment number   (8B): T(2B) + L(2B) + 0x0000(4B)
 */

uint32_t cpcparse_tlv_read_length(unsigned char* tlv) {
    return ((uint32_t)tlv[2] << 8ull) + (uint32_t)tlv[3];
}

// unsigned int cpcparse_tlv_read_segnum(unsigned char* tlv) {
//    return ((uint64_t)tlv[4] << 24ull) + 
//           ((uint64_t)tlv[5] << 16ull) +
//           ((uint64_t)tlv[6] <<  8ull) + 
//           ((uint64_t)tlv[7] <<  0ull);
// }
