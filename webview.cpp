#include <Python.h>
#include "structmember.h"
#include "webview.h"

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

typedef struct {
  PyObject_HEAD
  webview_t w;
  PyObject *callback;
} WebView;

// static void webview_python_cb(webview_t w, const char *arg) {
//   WebView *self = w->userdata;
//   if (self->callback) {
//     PyObject_CallFunction(self->callback, "Os", self, arg);
//     PyErr_Print();
//   }
// }

static void WebView_dealloc(WebView *self) {
  webview_destroy(self->w);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static int WebView_init(WebView *self, PyObject *args, PyObject *kwds) {
  int width = 480, height = 320;
  char resizable = 0;
  char debug = 0;
  char *url = NULL;
  char *title = NULL;
  const char *kwlist[] = {"width", "height", "resizable", "debug", "url", "title", NULL};

  if (!PyArg_ParseTupleAndKeywords(
          args, kwds, "ii|iiss", kwlist, &width, &height,
          &resizable, &debug, &url, &title)) {
    return -1;
  }

  self->w = webview_create(debug, NULL);
  webview_set_title(self->w, title);
  webview_set_size(self->w, width, height, resizable ? WEBVIEW_HINT_NONE: WEBVIEW_HINT_FIXED);
  webview_navigate(self->w, url);

  // TODO: Maybe deal with bind/return ?
}

static PyObject *WebView_run(WebView *self) {
  webview_run(self->w);
  Py_RETURN_NONE;
}

static PyObject *WebView_terminate(WebView *self) {
  webview_terminate(self->w);
  Py_RETURN_NONE;
}

static PyObject *WebView_set_title(WebView *self, PyObject *args) {
  char *title = NULL;
  if (!PyArg_ParseTuple(args, "s", &title)) {
    return NULL;
  }
  webview_set_title(self->w, title);
  Py_RETURN_NONE;
}

static PyObject *WebView_eval(WebView *self, PyObject *args) {
  const char *js = NULL;
  if (!PyArg_ParseTuple(args, "s", &js)) {
    return NULL;
  }
  webview_eval(self->w, js);
  Py_RETURN_NONE;
}

static void webview_dispatch_cb(webview_t w, void *arg) {
  PyObject *cb = (PyObject *)arg;
  /* TODO */
  PyObject_CallObject(cb, NULL);
  Py_XINCREF(cb);
}

static PyObject *WebView_dispatch(WebView *self, PyObject *args) {
  PyObject *tmp;
  if (!PyArg_ParseTuple(args, "O:dispatch", &tmp)) {
    return NULL;
  }
  if (!PyCallable_Check(tmp)) {
    PyErr_SetString(PyExc_TypeError, "parameter must be callable");
    return NULL;
  }
  Py_XINCREF(tmp);
  webview_dispatch(self->w, webview_dispatch_cb, tmp);
  Py_RETURN_NONE;
}

static PyObject *WebView_bind(WebView *self) {
  /* TODO, very complex implementation */
  Py_RETURN_NONE;
}

static PyMemberDef WebView_members[] = {
    {"callback", T_OBJECT, offsetof(WebView, callback), 0, "Sould be a callabale that accepts (WebView, str) or None"},
    {NULL} /* Sentinel */
};
static PyMethodDef WebView_methods[] = {
    {"run", (PyCFunction)WebView_run, METH_NOARGS, "WebView.run() -> None"},
    {"terminate", (PyCFunction)WebView_terminate, METH_NOARGS, "WebView.terminate() -> None"},
    {"dispatch", (PyCFunction)WebView_dispatch, METH_VARARGS, "WebView.dispatch(callback: Callable) -> None"},
    {"eval", (PyCFunction)WebView_eval, METH_VARARGS, "WebView.eval(js: str) -> None"},
    {"set_title", (PyCFunction)WebView_set_title, METH_VARARGS, "WebView.set_title(title: str) -> None"},
    {"bind", (PyCFunction)WebView_bind, METH_VARARGS, "WebView.bind() -> None"},
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
    "webview.WebView(width: int, height: int, "
    "resizable: bool = False, debug: bool = False, "
    "url: str = '', title: str = '')",                /* tp_doc */
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
    0,                                                /* tp_new */
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
#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}
