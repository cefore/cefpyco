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

#include "cefpyco.h"
#include <Python.h>

#define NAME  ("ccnx:/a")
#define NAMEA ("ccnx:/a/a")
#define NAMEB ("ccnx:/b")

static int request_state();
static int provide_state();
static int listen_state();
static int ask_state();
static int respond_state();
static int stop_on_error(int is_err, char* msg, int status);

int main(int argc, char const* argv[]) {
    int   res;
    char* mode_text;
    int (*run_on_state)();
    switch (argc) {
        case 1:
            mode_text = "REQUEST";
            run_on_state = request_state;
            break;
        case 2:
            mode_text = "PROVIDE";
            run_on_state = provide_state;
            break;
        case 3:
            mode_text = "LISTEN";
            run_on_state = listen_state;
            break;
        case 4:
            mode_text = "ASK";
            run_on_state = ask_state;
            break;
        default:
            mode_text = "RESPOND";
            run_on_state = respond_state;
            break;
    }
    printf("Cefpycotest (mode=%s)\n", mode_text);
    res = begin_cef(CefC_Unset_Port, "");
    stop_on_error(res < 0, "Error on begin_cef.", res);
    printf("Begin connection with cefore(%d)\n", res);
    run_on_state();
    usleep(1);
    res = end_cef();
    stop_on_error(res < 0, "Error on end_cef.", res);
    printf("End connection with cefore(%d)\n", res);
    return 0;
}

static int request_state() {
    int               res;
    cefpyco_app_frame app_frame;

    res = send_interest(NAME, 0);
    res = send_interest(NAME, 1);
    stop_on_error(res < 0, "Failed to send Interest.", res);
    printf("Send Interest with name %s(%d)\n", NAME, res);
    res = receive(&app_frame, -1, 1);
    stop_on_error(res < 0, "Failed to satisfy Interest.", res);
    res = receive(&app_frame, -1, 1);
    stop_on_error(res < 0, "Failed to satisfy Interest.", res);
    printf("Succeeded to receive Data with name %s(%d)\n", NAME, res);
    return 0;
}

static int provide_state() {
    int res;
    res = send_data(NAME, 0, "hello", 5);
    stop_on_error(res < 0, "Failed to send Data.", res);
    // usleep(1000000);
    res = send_data(NAME, 1, "world", 5);
    stop_on_error(res < 0, "Failed to send Data.", res);
    printf("Send Data with name %s (%d)\n", NAME, res);
    return 0;
}

static int listen_state() {
    int               res;
    cefpyco_app_frame app_frame;

    res = register_name(NAME);
    res = register_name(NAMEA);
    if (res < 0) printf("Failed to register name to receive Interest.");
    res = receive(&app_frame, -1, 1);
    printf(
        "%s to receive Interest with name %s(%d)\n", res == 0 ? "Succeeded" : "Failed", NAME, res);
    res = receive(&app_frame, -1, 1);
    printf(
        "%s to receive Interest with name %s(%d)\n", res == 0 ? "Succeeded" : "Failed", NAME, res);
    res = receive(&app_frame, -1, 0);
    printf(
        "%s to receive Interest with name %s(%d)\n", res == 0 ? "Succeeded" : "Failed", NAME, res);
    return 0;
}

static int ask_state() {
    int res;
    res = send_interest(NAME, 0);
    res = send_interest(NAME, 1);
    stop_on_error(res < 0, "Failed to send Interest.", res);
    printf("Send Interest with name %s(%d)\n", NAME, res);
    return 0;
}

static int respond_state() {
    int               res;
    cefpyco_app_frame app_frame;

    res = register_name(NAME);
    if (res < 0) printf("Failed to register name to receive Interest.");
    res = receive(&app_frame, -1, 1);
    printf(
        "%s to receive Interest with name %s(%d)\n", res == 0 ? "Succeeded" : "Failed", NAME, res);
    res = send_data(NAME, 0, "hello", 5);
    res = send_data(NAME, 1, "world", 5);
    printf("Send Data with name %s (%d)\n", NAME, res);
    return 0;
}

static int stop_on_error(int is_err, char* msg, int status) {
    if (is_err) {
        fprintf(stderr, "%s [exit_status=%d]\n", msg, status);
        exit(1);
    }
}
