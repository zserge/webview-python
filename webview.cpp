#include <Python.h>
#include "structmember.h"
#include "webview/webview.h"

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

typedef struct {
  PyObject_HEAD
  webview_t w;
  PyObject *bindings;
} WebView;

static void WebView_dealloc(WebView *self) {
  webview_destroy(self->w);
  PyDict_Clear(self->bindings);  // TODO: What about refcounts for keys and values?
  Py_DECREF(self->bindings);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *WebView_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyObject *bindings = PyDict_New();  // TODO: Should we call Py_INCREF(bindings)?
  if (bindings == NULL) return NULL;

  WebView *self = (WebView *)type->tp_alloc(type, 0);
  if (self == NULL) return NULL;

  self->bindings = bindings;

  return (PyObject *)self;
}

static int WebView_init(WebView *self, PyObject *args, PyObject *kwds) {
  int width = 0;
  int height = 0;
  int resizable = 1;
  int debug = 0;
  char *title = NULL;
  char *kwlist[] = {"width", "height", "resizable", "debug", "title", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iiiis:WebView", kwlist, &width, &height, &resizable, &debug, &title)) {
    return -1;
  }

  // printf("size=%dx%d, resizable=%d, debug=%d, title=\"%s\"\n", width, height, resizable, debug, title);

  self->w = webview_create(debug, NULL);

  if (title) {
    webview_set_title(self->w, title);
  }

  if ((width && height) || !resizable) {
    webview_set_size(self->w, width, height, resizable ? WEBVIEW_HINT_NONE: WEBVIEW_HINT_FIXED);
  }

  return 0;
}

static PyObject *WebView_run(WebView *self) {
  Py_BEGIN_ALLOW_THREADS
  webview_run(self->w);
  Py_END_ALLOW_THREADS
  Py_RETURN_NONE;
}

static PyObject *WebView_terminate(WebView *self) {
  webview_terminate(self->w);
  Py_RETURN_NONE;
}

static PyObject *WebView_set_title(WebView *self, PyObject *args) {
  char *title = NULL;
  if (!PyArg_ParseTuple(args, "s:set_title", &title)) {
    return NULL;
  }
  webview_set_title(self->w, title);
  Py_RETURN_NONE;
}

static PyObject *WebView_set_size(WebView *self, PyObject *args) {
  int width = 0;
  int height = 0;
  int hint = WEBVIEW_HINT_NONE;

  if (!PyArg_ParseTuple(args, "iii:set_size", &width, &height, &hint)) {
    return NULL;
  }

  webview_set_size(self->w, width, height, hint);

  Py_RETURN_NONE;
}

static PyObject *WebView_navigate(WebView *self, PyObject *args) {
  const char *url;

  if (!PyArg_ParseTuple(args, "s:navigate", &url)) {
    return NULL;
  }

  webview_navigate(self->w, url);

  Py_RETURN_NONE;
}

static PyObject *WebView_init_js(WebView *self, PyObject *args) {
  const char *js = NULL;

  if (!PyArg_ParseTuple(args, "s:init", &js)) {
    return NULL;
  }

  webview_init(self->w, js);

  Py_RETURN_NONE;
}

static PyObject *WebView_eval(WebView *self, PyObject *args) {
  const char *js = NULL;

  if (!PyArg_ParseTuple(args, "s:eval", &js)) {
    return NULL;
  }

  webview_eval(self->w, js);

  Py_RETURN_NONE;
}

static void webview_bind_cb(const char *seq, const char *req, void *binding) {
  WebView *self = (WebView *)PyTuple_GetItem((PyObject *)binding, 0);
  PyObject *name = PyTuple_GetItem((PyObject *)binding, 1);
  PyObject *func = PyTuple_GetItem((PyObject *)binding, 2);

  // printf("Calling %s(%s)\n", PyUnicode_AsUTF8(name), req);

  PyObject *ret = PyObject_CallFunction(func, "Os", self, req);
  if (!ret) {
    webview_return(self->w, seq, 1, NULL);  // TODO: Return error JSON
    PyErr_Print();
    return;
  }

  PyObject *str = PyObject_Str(ret);
  Py_DECREF(ret);
  if (!str) {
    webview_return(self->w, seq, 1, NULL);  // TODO: Return error JSON
    PyErr_Print();
    return;
  }

  // printf("Returning %s\n", PyUnicode_AsUTF8(str));
  webview_return(self->w, seq, 0, PyUnicode_AsUTF8(str));

  Py_DECREF(str);
}

static PyObject *WebView_bind(WebView *self, PyObject *args) {
  PyObject *name, *func, *tuple;

  if (!PyArg_ParseTuple(args, "OO:bind", &name, &func)) { // TODO: What about ref-counts?
    return NULL;
  }

  // printf("Got name=%s func=%s\n", PyUnicode_AsUTF8(PyObject_ASCII(name)), PyUnicode_AsUTF8(PyObject_ASCII(func)));

  if (!PyUnicode_Check(name)) {
    PyErr_SetString(PyExc_TypeError, "name must be unicode");
    return NULL;
  }

  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "func must be callable");
    return NULL;
  }

  if (PyDict_Contains(self->bindings, name)) {
    PyErr_SetString(PyExc_ValueError, "name already exists");
    return NULL;
  }

  tuple = PyTuple_Pack(3, self, name, func);
  if (tuple == NULL) {
    PyErr_SetString(PyExc_MemoryError, "Could not allocate binding tuple");
    return NULL;
  }

  PyDict_SetItem(self->bindings, name, tuple);

  // printf("Bindings: %s\n", PyUnicode_AsUTF8(PyObject_ASCII(self->bindings)));

  webview_bind(self->w, PyUnicode_AsUTF8(name), webview_bind_cb, tuple);

  Py_RETURN_NONE;
}

static void webview_dispatch_cb(webview_t w, void *arg) {
  PyObject_CallObject((PyObject *)arg, NULL);
}

static PyObject *WebView_dispatch(WebView *self, PyObject *args) {
  PyObject *func;

  if (!PyArg_ParseTuple(args, "O:dispatch", &func)) {
    return NULL;
  }

  if (!PyCallable_Check(func)) {
    PyErr_SetString(PyExc_TypeError, "parameter must be callable");
    return NULL;
  }

  webview_dispatch(self->w, webview_dispatch_cb, func);

  Py_RETURN_NONE;
}

static PyMemberDef WebView_members[] = {
    {"bindings", T_OBJECT, offsetof(WebView, bindings), 0, "Debug access to internal bindings"},
    {NULL} /* Sentinel */
};
static PyMethodDef WebView_methods[] = {
    {"run",       (PyCFunction)WebView_run,       METH_NOARGS,  "WebView.run() -> None"},
    {"terminate", (PyCFunction)WebView_terminate, METH_NOARGS,  "WebView.terminate() -> None"},
    {"set_title", (PyCFunction)WebView_set_title, METH_VARARGS, "WebView.set_title(title: str) -> None"},
    {"set_size",  (PyCFunction)WebView_set_size,  METH_VARARGS, "WebView.set_size(width: Optional[int], height: Optional[int], hints: Optional[int]) -> None"},
    {"navigate",  (PyCFunction)WebView_navigate,  METH_VARARGS, "WebView.navigate(url: str) -> None"},
    {"init",      (PyCFunction)WebView_init_js,   METH_VARARGS, "WebView.init(js: str) -> None"},
    {"eval",      (PyCFunction)WebView_eval,      METH_VARARGS, "WebView.eval(js: str) -> None"},
    {"bind",      (PyCFunction)WebView_bind,      METH_VARARGS, "WebView.bind(name: str, func: Callable) -> None"},
    {"dispatch",  (PyCFunction)WebView_dispatch,  METH_VARARGS, "WebView.dispatch(func: Callable) -> None"},
    {NULL} /* Sentinel */
};

static PyTypeObject WebViewType = {
    PyVarObject_HEAD_INIT(NULL, 0) "webview.WebView", /* tp_name */
    sizeof(WebView),                                  /* tp_basicsize */
    0,                                                /* tp_itemsize */
    (destructor)WebView_dealloc,                      /* tp_dealloc */
    0,                                                /* tp_print */
    0,                                                /* tp_getattr */
    0,                                                /* tp_setattr */
    0,                                                /* tp_compare */
    0,                                                /* tp_repr */
    0,                                                /* tp_as_number */
    0,                                                /* tp_as_sequence */
    0,                                                /* tp_as_mapping */
    0,                                                /* tp_hash */
    0,                                                /* tp_call */
    0,                                                /* tp_str */
    0,                                                /* tp_getattro */
    0,                                                /* tp_setattro */
    0,                                                /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,         /* tp_flags */
    "webview.WebView("
    "width: int, height: int, "
    "resizable: bool = False, "
    "debug: bool = False, "
    "title: str = '')",                               /* tp_doc */
    0,                                                /* tp_traverse */
    0,                                                /* tp_clear */
    0,                                                /* tp_richcompare */
    0,                                                /* tp_weaklistoffset */
    0,                                                /* tp_iter */
    0,                                                /* tp_iternext */
    WebView_methods,                                  /* tp_methods */
    WebView_members,                                  /* tp_members */
    0,                                                /* tp_getset */
    0,                                                /* tp_base */
    0,                                                /* tp_dict */
    0,                                                /* tp_descr_get */
    0,                                                /* tp_descr_set */
    0,                                                /* tp_dictoffset */
    (initproc)WebView_init,                           /* tp_init */
    0,                                                /* tp_alloc */
    WebView_new,                                      /* tp_new */
};

static PyMethodDef module_methods[] = {
    {NULL} /* Sentinel */
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "webview",
        "Python bindings for the WebView C library.",
        0,
        module_methods,
        NULL,
        NULL,
        NULL,
        NULL
};
#define MODINIT_ERROR NULL
#define MODINIT_NAME PyInit_webview
#else
#define MODINIT_ERROR
#define MODINIT_NAME initwebview
#endif
PyMODINIT_FUNC MODINIT_NAME(void) {
  PyObject *m;

  if (PyType_Ready(&WebViewType) < 0) {
    return MODINIT_ERROR;
  }

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&moduledef);
#else
    m = Py_InitModule3("webview", module_methods, "Python bindings for the WebView C library.");
#endif

  if (m == NULL) {
    return MODINIT_ERROR;
  }

  Py_INCREF(&WebViewType);
  PyModule_AddObject(m, "WebView", (PyObject *)&WebViewType);

  PyModule_AddIntConstant(m, "SIZE_HINT_NONE", WEBVIEW_HINT_NONE);
  PyModule_AddIntConstant(m, "SIZE_HINT_MIN", WEBVIEW_HINT_MIN);
  PyModule_AddIntConstant(m, "SIZE_HINT_MAX", WEBVIEW_HINT_MAX);
  PyModule_AddIntConstant(m, "SIZE_HINT_FIXED", WEBVIEW_HINT_FIXED);

#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}
