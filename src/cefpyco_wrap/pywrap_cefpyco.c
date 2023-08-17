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
#include "cefpyco_util.h"
#include "pywrap.h"

typedef int cefpyco_reg_api(CefT_Client_Handle, uint16_t, const char*);
static int  call_register(PyObject* args, PyObject* kw, cefpyco_reg_api api);

PYWRAP_DEF(begin) {
    long long    res;
    int          port_num = CefC_Unset_Port;
    int          enable_log = 1;
    char*        conf_path = "";
    static char* keywords[] = {"port_num", "conf_path", "enable_log", NULL};

    if (!PyArg_ParseTupleAndKeywords(
            args, kw, "|isi", keywords, &port_num, &conf_path, &enable_log)) {
        return NULL;
    }
    cefpyco_enable_log = enable_log;
    res = (long long)create_cef_handler(port_num, conf_path);
    PYWRAP_ERR;
    return Py_BuildValue("L", res);
}

PYWRAP_DEF(end) {
    long long    handler;
    int          res;
    static char* keywords[] = {"handler", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kw, "L", keywords, &handler)) { return NULL; }
    res = destroy_cef_handler((CefT_Client_Handle)handler);
    PYWRAP_ERR;
    Py_RETURN_NONE;
}

PYWRAP_DEF(send_interest) {
    int                res;
    long long          handler;
    CefT_CcnMsg_OptHdr opt_i;
    CefT_CcnMsg_MsgBdy params_i;
    static char* keywords[] = {"handler",   "name",     "chunk_num", "symbolic_f", "longlife_f",
                               "hop_limit", "lifetime", "hdr_org",   "msg_org",    NULL};
    /* Mandatory arguments */
    const char* uri;
    /* Optional keyword arguments */
    int       chunk_num = -1;
    int       symbolic_f = 0;
    int       longlife_f = 0;
    int       hop_limit = 32;
    long long lifetime = 4000ull; /* 4 seconds */
    // const char *     hdr_org, *msg_org;
    // const Py_ssize_t hdr_org_siz, msg_org_siz;
    const char* hdr_org = "";
    const char* msg_org = "";
    Py_ssize_t  hdr_org_siz = 0;
    Py_ssize_t  msg_org_siz = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args, kw, "Ls|iiiiLy#y#", keywords, &handler, &uri, &chunk_num, &symbolic_f,
            &longlife_f, &hop_limit, &lifetime, &hdr_org, &hdr_org_siz, &msg_org, &msg_org_siz)) {
        return NULL;
    }
    memset(&opt_i, 0, sizeof(CefT_CcnMsg_OptHdr));
    memset(&params_i, 0, sizeof(CefT_CcnMsg_MsgBdy));
    res = cef_frame_conversion_uri_to_name(uri, params_i.name);
    if (res < 0) {
        PyErr_SetString(PyExc_RuntimeError, CefpycoC_Err_Invalid_URI);
        return NULL;
    }
    params_i.name_len = res;

    params_i.hoplimit = hop_limit;
    if (lifetime >= 0) {
        opt_i.lifetime_f = 1;
        opt_i.lifetime = lifetime;
    }

    if (symbolic_f > 0 || longlife_f > 0) {
        // params_i.org.symbolic_f = symbolic_f;
        // params_i.org.longlife_f = longlife_f;
        Cef_Int_Symbolic(params_i);  // Cefore 0.8.3
    }

    if (chunk_num >= 0) {
        params_i.chunk_num_f = 1;
        params_i.chunk_num = chunk_num;
    }

    if (0 < (opt_i.org_len = hdr_org_siz)) { memcpy(opt_i.org_val, hdr_org, hdr_org_siz); }
    if (0 < (params_i.org_len = msg_org_siz)) { memcpy(params_i.org_val, msg_org, msg_org_siz); }
    res = cph_send_interest((CefT_Client_Handle)handler, &opt_i, &params_i);
    PYWRAP_ERR;
    Py_RETURN_NONE;
}

PYWRAP_DEF(send_data) {
    int                res;
    long long          handler;
    CefT_CcnMsg_OptHdr opt_d;
    CefT_CcnMsg_MsgBdy params_d;
    struct timeval     now_t;
    uint64_t           now_ms;
    static char*       keywords[] = {"handler",    "name",          "payload",   "payload_len",
                                     "chunk_num",  "end_chunk_num", "hop_limit", "expiry",
                                     "cache_time", "hdr_org",       "msg_org",   NULL};

    /* Mandatory arguments */
    const char* uri;
    const char* payload;
    Py_ssize_t  tmp;
    int         payload_len;
    /* Optional keyword arguments */
    int       chunk_num = -1;
    int       end_chunk_num = -1;
    int       hop_limit = 32;
    long long expiry = 36000ull * 1000ull; /* 36000 seconds (10 hours) */
    long long cache_time = -1;
    // const char *     hdr_org, *msg_org;
    // const Py_ssize_t hdr_org_siz, msg_org_siz;
    const char* hdr_org = "";
    const char* msg_org = "";
    Py_ssize_t  hdr_org_siz = 0;
    Py_ssize_t  msg_org_siz = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args, kw, "Lss#i|iiiLLz#z#", keywords, &handler, &uri, &payload, &tmp, &payload_len,
            &chunk_num, &end_chunk_num, &hop_limit, &expiry, &cache_time, &hdr_org, &hdr_org_siz,
            &msg_org, &msg_org_siz)) {
        return NULL;
    }
    memset(&opt_d, 0, sizeof(CefT_CcnMsg_OptHdr));
    memset(&params_d, 0, sizeof(CefT_CcnMsg_MsgBdy));
    res = cef_frame_conversion_uri_to_name(uri, params_d.name);
    if (res < 0) {
        PyErr_SetString(PyExc_RuntimeError, CefpycoC_Err_Invalid_URI);
        return NULL;
    }
    params_d.name_len = res;

    memcpy(params_d.payload, payload, sizeof(char) * payload_len);
    params_d.payload_len = payload_len;

    if (chunk_num >= 0) {
        params_d.chunk_num_f = 1;
        params_d.chunk_num = chunk_num;
    }
    gettimeofday(&now_t, NULL);
    now_ms = now_t.tv_sec * 1000ull + now_t.tv_usec / 1000ull;
    params_d.expiry = now_ms + expiry;

    if (cache_time >= 0) {
        opt_d.cachetime_f = 1;
        opt_d.cachetime = now_ms + cache_time;
    }
    if (end_chunk_num >= 0) {
        params_d.end_chunk_num_f = 1;
        params_d.end_chunk_num = end_chunk_num;
    }
    if (0 < (opt_d.org_len = hdr_org_siz)) {  //
        memcpy(opt_d.org_val, hdr_org, hdr_org_siz);
    }
    if (0 < (params_d.org_len = msg_org_siz)) {  //
        memcpy(params_d.org_val, msg_org, msg_org_siz);
    }
    res = cph_send_data((CefT_Client_Handle)handler, &opt_d, &params_d);
    PYWRAP_ERR;
    Py_RETURN_NONE;
}

PYWRAP_DEF(build_hdrorg_value) {
    unsigned char      buff[4096];
    Py_ssize_t         siz;
    CefT_HdrOrg_Params org;
    static char*       keywords[] = {
        "t_hw_flags_f",
        "t_hw_flags_symbolic_f",
        "t_hw_flags_enablecache_f",
        "t_hw_timestamp_f",
        "t_hw_timestamp_long_f",
        "t_hw_timestamp_in",
        "t_hw_timestamp_out",
        "tp_variant",
        "tp_val",
        NULL};
    /* Optional keyword arguments */
    int              t_hw_flags_f = 0;
    int              t_hw_flags_symbolic_f = 0;
    int              t_hw_flags_enablecache_f = 0;
    int              t_hw_timestamp_f = 0;
    int              t_hw_timestamp_long_f = 0;
    int              t_hw_timestamp_in = 0;
    int              t_hw_timestamp_out = 0;
    int              tp_variant = 0;
    const char*      tp_val;
    const Py_ssize_t tp_siz = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args, kw, "iiiiiiiiz#", keywords, &t_hw_flags_f, &t_hw_flags_symbolic_f,
            &t_hw_flags_enablecache_f, &t_hw_timestamp_f, &t_hw_timestamp_long_f,
            &t_hw_timestamp_in, &t_hw_timestamp_out, &tp_variant, &tp_val, &tp_siz)) {
        return NULL;
    }
    memset(&org, 0x00, sizeof(org));
    org.t_hw_flags_f = t_hw_flags_f;
    org.t_hw_flags_symbolic_f = t_hw_flags_symbolic_f;
    org.t_hw_flags_enablecache_f = t_hw_flags_enablecache_f;
    org.t_hw_timestamp_f = t_hw_timestamp_f;
    org.t_hw_timestamp_long_f = t_hw_timestamp_long_f;
    org.t_hw_timestamp_in = t_hw_timestamp_in;
    org.t_hw_timestamp_out = t_hw_timestamp_out;

    if (tp_variant) {
        org.tp_variant = tp_variant;
        org.tp_len = tp_siz;
        memcpy(org.tp_val, tp_val, tp_siz);
    }

    memset(buff, 0x00, sizeof(buff));
    siz = cph_build_hdrorg_value(buff, &org);
    if (0 < siz) { return Py_BuildValue(BUILD_BYTES_ID "#", buff, siz); }
    Py_RETURN_NONE;
}

PYWRAP_DEF(build_msgorg_value) {
    unsigned char      buff[4096];
    Py_ssize_t         siz;
    CefT_MsgOrg_Params org;
    static char*       keywords[] = {"symbolic_f",  "longlife_f",  "selective_f",
                                     "req_chunk",   "first_chunk", "last_chunk",
                                     "version_val", "from_pub_f",  NULL};
    /* Optional keyword arguments */
    int              symbolic_f = 0;
    int              longlife_f = 0;
    int              selective_f = 0;
    int              req_chunk = 0;
    int              first_chunk = 0;
    int              last_chunk = -1;
    const char*      version_val;
    const Py_ssize_t version_siz;
    int              from_pub_f = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args, kw, "iiiiiiz#i", keywords, &symbolic_f, &longlife_f, &selective_f, &req_chunk,
            &first_chunk, &last_chunk, &version_val, &version_siz, &from_pub_f)) {
        return NULL;
    }
    memset(&org, 0x00, sizeof(org));
    org.symbolic_f = symbolic_f;
    org.longlife_f = longlife_f;
    org.selective_f = selective_f;
    if (selective_f) {
        org.req_chunk = req_chunk;
        org.first_chunk = first_chunk;
        if (first_chunk <= last_chunk) {
            org.last_chunk_f = 1;
            org.last_chunk = last_chunk;
        }
    }
    if (version_siz) {
        org.version_f = 1;
        org.version_len = version_siz;
        memcpy(org.version_val, version_val, version_siz);
    }

    memset(buff, 0x00, sizeof(buff));
    siz = cph_build_msgorg_value(buff, &org);
    if (0 < siz) { return Py_BuildValue(BUILD_BYTES_ID "#", buff, siz); }
    Py_RETURN_NONE;
}

PYWRAP_DEF(register) {
    int res;
    res = call_register(args, kw, cph_register_name);
    if (res < -1) return NULL;
    PYWRAP_ERR;
    Py_RETURN_NONE;
}

static int call_register(PyObject* args, PyObject* kw, cefpyco_reg_api api) {
    int          res;
    long long    handler;
    int          is_reg = 1;  // Reg
    const char*  uri;
    static char* keywords[] = {"handler", "name", "is_reg", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kw, "Ls|i", keywords, &handler, &uri, &is_reg)) {
        return -2;
    }
    res = api((CefT_Client_Handle)handler, CefC_App_DeReg, uri);
    if (res < 0) return -1;
    if (is_reg) res = api((CefT_Client_Handle)handler, CefC_App_Reg, uri);
    if (res < 0) return -1;
    return 0;
}

PYWRAP_DEF(receive) {
    int               res;
    long long         handler;
    long long         version, type, actual_data_len;
    int               name_len = 0;
    cefpyco_app_frame app_frame;
    int               timeout_ms = CefpycoC_Default_Timeout;
    int               error_on_timeout = 0;
    char              name[4096];
    Py_ssize_t        r_name_len;
    char              payload[4096];
    Py_ssize_t        r_payload_len;
    char              hdr_org[256];
    Py_ssize_t        hdr_org_len;
    char              msg_org[1024];
    Py_ssize_t        msg_org_len;
    static char*      keywords[] = {"handler", "error_on_timeout", "timeout_ms", NULL};

    if (!PyArg_ParseTupleAndKeywords(
            args, kw, "L|ii", keywords, &handler, &error_on_timeout, &timeout_ms)) {
        return NULL;
    }
    res = cph_receive((CefT_Client_Handle)handler, &app_frame, timeout_ms, error_on_timeout);
    version = app_frame.version;
    type = app_frame.type;
    actual_data_len = app_frame.actual_data_len;
    name_len = convert_nametlv_to_readable_str(app_frame.name, app_frame.name_len, name);
    if (name_len < 0) {
        name_len = 0;
        sprintf(name, "%s%c", CefpycoC_Null_Msg, '\0');
        r_name_len = sizeof(CefpycoC_Null_Msg);
    } else {
        name[name_len] = '\0';
        r_name_len = name_len;
    }
    if (app_frame.payload_len <= 0) {
        sprintf(payload, "%s%c", CefpycoC_Null_Msg, '\0');
        r_payload_len = sizeof(CefpycoC_Null_Msg);
    } else {
        memcpy(payload, app_frame.payload, app_frame.payload_len);
        payload[app_frame.payload_len] = '\0';
        r_payload_len = app_frame.payload_len;
    }

#define minimum(a, b) ((a) < (b) ? (a) : (b))

    hdr_org_len = minimum(sizeof(hdr_org) - 1, app_frame.hdr_org_len);
    if (0 < hdr_org_len) {
        memcpy(hdr_org, app_frame.hdr_org_val, hdr_org_len);
        hdr_org[hdr_org_len] = '\0';
    } else {
        sprintf(hdr_org, "%s%c", CefpycoC_Null_Msg, '\0');
        hdr_org_len = sizeof(CefpycoC_Null_Msg);
    }

    msg_org_len = minimum(sizeof(msg_org) - 1, app_frame.msg_org_len);
    if (0 < msg_org_len) {
        memcpy(msg_org, app_frame.msg_org_val, msg_org_len);
        msg_org[msg_org_len] = '\0';
    } else {
        sprintf(msg_org, "%s%c", CefpycoC_Null_Msg, '\0');
        msg_org_len = sizeof(CefpycoC_Null_Msg);
    }
    PYWRAP_ERR;

#ifdef CEFPYCO_DUMP
    MILESTONE
    fprintf(stderr, "          res:%d\n", res);
    fprintf(stderr, "      version:0x%llx\n", version);
    fprintf(stderr, "         type:%lld\n", type);
    fprintf(stderr, "   returncode:%lld\n", app_frame.returncode);
    fprintf(stderr, "        flags:%lx\n", app_frame.flags);
    fprintf(stderr, "   actual len:%llu\n", app_frame.actual_data_len);
    fprintf(stderr, "         name:[%s]\n", name);
    fprintf(stderr, "     name len:%d (%d)\n", name_len, strlen(name));
    fprintf(stderr, "    chunk num:%d\n", app_frame.chunk_num);
    fprintf(stderr, "end chunk num:%d\n", app_frame.end_chunk_num);
    fprintf(stderr, "      payload:[%s]\n", payload);
    fprintf(stderr, "  payload len:%d (%d)\n", app_frame.payload_len, strlen(payload));
    MILESTONE
#endif

    MILESTONE
    return Py_BuildValue(
        "iLLLLs#iiiL" BUILD_BYTES_ID "#i" BUILD_BYTES_ID "#i" BUILD_BYTES_ID "#i", res, version,
        type, app_frame.returncode, actual_data_len, name, r_name_len, name_len,
        (int)app_frame.chunk_num, (int)app_frame.end_chunk_num, (long long)app_frame.flags, payload,
        r_payload_len, (int)app_frame.payload_len, hdr_org, hdr_org_len, (int)app_frame.hdr_org_len,
        msg_org, msg_org_len, (int)app_frame.msg_org_len);
}

static PyMethodDef libcefpyco_methods[] = {
    {"begin", (PyCFunction)pywrap_begin, PYWRAP_METH, ""},
    {"end", (PyCFunction)pywrap_end, PYWRAP_METH, ""},
    {"send_interest", (PyCFunction)pywrap_send_interest, PYWRAP_METH, ""},
    {"send_data", (PyCFunction)pywrap_send_data, PYWRAP_METH, ""},
    {"register", (PyCFunction)pywrap_register, PYWRAP_METH, ""},
    {"receive", (PyCFunction)pywrap_receive, PYWRAP_METH, ""},
    {"build_hdrorg", (PyCFunction)pywrap_build_hdrorg_value, PYWRAP_METH, ""},
    {"build_msgorg", (PyCFunction)pywrap_build_msgorg_value, PYWRAP_METH, ""},
    {NULL, NULL, 0, NULL}};

static int libcefpyco_traverse(PyObject* m, visitproc visit, void* arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}
static int libcefpyco_clear(PyObject* m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}
static struct PyModuleDef moduledef = {  //
    PyModuleDef_HEAD_INIT,       "libcefpyco",       NULL,
    sizeof(struct module_state), libcefpyco_methods, NULL,
    libcefpyco_traverse,         libcefpyco_clear,   NULL};

INITFUNC_DEF(libcefpyco) {
    PyObject* module = PyModule_Create(&moduledef);
    if (module == NULL) { INITERROR; }
    struct module_state* st = GETSTATE(module);

    st->error = PyErr_NewException("libcefpyco.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }
    return module;
}
