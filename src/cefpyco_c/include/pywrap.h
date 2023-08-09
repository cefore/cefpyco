/*
 * Copyright (c) 2016, National Institute of Information and Communications
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

#ifndef __PYWRAP_HEADER__
#define __PYWRAP_HEADER__

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct module_state {
    PyObject *error;
};

#define PYWRAP_DEF(name) static PyObject* pywrap_ ## name (PyObject* self, PyObject* args, PyObject *kw)
#define PYWRAP_ERR if (res < 0) { PyErr_SetString(PyExc_RuntimeError, get_error_msg()); return NULL; }
#define PYWRAP_METH (METH_VARARGS | METH_KEYWORDS)

#if PY_MAJOR_VERSION >= 3
/* Settings for Python 3.6 */
#define GETSTATE(m)  ((struct module_state*)PyModule_GetState(m))
#define PYWRAP_INIT                                                       \
static int libcefpyco_traverse(PyObject *m, visitproc visit, void *arg) { \
    Py_VISIT(GETSTATE(m)->error);                                         \
    return 0;                                                             \
}                                                                         \
static int libcefpyco_clear(PyObject *m) {                                \
    Py_CLEAR(GETSTATE(m)->error);                                         \
    return 0;                                                             \
}                                                                         \
static struct PyModuleDef moduledef = {                                   \
        PyModuleDef_HEAD_INIT,                                            \
        "libcefpyco",                                                     \
        NULL,                                                             \
        sizeof(struct module_state),                                      \
        libcefpyco_methods,                                               \
        NULL,                                                             \
        libcefpyco_traverse,                                              \
        libcefpyco_clear,                                                 \
        NULL                                                              \
};
#define INITERROR return NULL
#define INITFUNC_DEF(name) PyMODINIT_FUNC PyInit_ ## name(void)
#define INITFUNC_CREATE_MOD(varname) PyObject *varname = PyModule_Create(&moduledef);
#define INITFUNC_RETURN_MOD(varname) (varname)
#define BUILD_BYTES_ID "y"

#else
/* Settings for Python 2.7 */
#define GETSTATE(m) (&_state)
#define PYWRAP_INIT static struct module_state _state;
#define INITERROR return
#define INITFUNC_DEF(name) void init ## name(void)
#define INITFUNC_CREATE_MOD(varname) PyObject *varname = Py_InitModule("libcefpyco", libcefpyco_methods);
#define INITFUNC_RETURN_MOD(varname)
#define BUILD_BYTES_ID "s"

#endif

#else
#endif
