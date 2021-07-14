#include <Python.h>

#include <handlebars/handlebars_compiler.h>
#include <handlebars/handlebars_delimiters.h>
#include <handlebars/handlebars_memory.h>
#include <handlebars/handlebars_opcode_serializer.h>
#include <handlebars/handlebars_parser.h>
#include <handlebars/handlebars_partial_loader.h>
#include <handlebars/handlebars_string.h>
#include <handlebars/handlebars_value.h>
#include <handlebars/handlebars_vm.h>
#include <talloc.h>

#ifndef _PyUnicode_AsStringAndSize
const char *PyUnicode_AsUTF8AndSize(PyObject *unicode, Py_ssize_t *psize) {
    if (!PyUnicode_Check(unicode)) { PyErr_BadArgument(); return NULL; }
    PyObject *utf8 = PyUnicode_AsUTF8String(unicode);
    if (!utf8) { PyErr_BadArgument(); return NULL; }
    const char *data = PyString_AsString(utf8);
    if (psize) *psize = PyString_GET_SIZE(utf8);
    return data;
}

const char *PyUnicode_AsUTF8(PyObject *unicode) {
    return PyUnicode_AsUTF8AndSize(unicode, NULL);
}
#endif

static bool convert_input = true;
static bool enable_partial_loader = true;
static long run_count = 1;
static struct handlebars_context *ctx = NULL;
static struct handlebars_string *partial_extension = NULL;
static struct handlebars_string *partial_path = NULL;
static TALLOC_CTX *root = NULL;
static unsigned long compiler_flags = handlebars_compiler_flag_none;

PyObject *pyhandlebars_compiler_flag_all(void) { compiler_flags |= handlebars_compiler_flag_all; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_alternate_decorators(void) { compiler_flags |= handlebars_compiler_flag_alternate_decorators; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_assume_objects(void) { compiler_flags |= handlebars_compiler_flag_assume_objects; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_compat(void) { compiler_flags |= handlebars_compiler_flag_compat; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_explicit_partial_context(void) { compiler_flags |= handlebars_compiler_flag_explicit_partial_context; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_ignore_standalone(void) { compiler_flags |= handlebars_compiler_flag_ignore_standalone; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_known_helpers_only(void) { compiler_flags |= handlebars_compiler_flag_known_helpers_only; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_mustache_style_lambdas(void) { compiler_flags |= handlebars_compiler_flag_mustache_style_lambdas; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_no_escape(void) { compiler_flags |= handlebars_compiler_flag_no_escape; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_none(void) { compiler_flags = handlebars_compiler_flag_none; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_prevent_indent(void) { compiler_flags |= handlebars_compiler_flag_prevent_indent; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_strict(void) { compiler_flags |= handlebars_compiler_flag_strict; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_string_params(void) { compiler_flags |= handlebars_compiler_flag_string_params; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_track_ids(void) { compiler_flags |= handlebars_compiler_flag_track_ids; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_use_data(void) { compiler_flags |= handlebars_compiler_flag_use_data; Py_RETURN_NONE; }
PyObject *pyhandlebars_compiler_flag_use_depths(void) { compiler_flags |= handlebars_compiler_flag_use_depths; Py_RETURN_NONE; }

PyObject *pyhandlebars_convert_input(bool convert) { convert_input = convert; Py_RETURN_NONE; }
PyObject *pyhandlebars_enable_partial_loader(bool partial) { enable_partial_loader = partial; Py_RETURN_NONE; }
PyObject *pyhandlebars_run_count(long run) { run_count = run; Py_RETURN_NONE; }

static void pyhandlebars_clean(void) {
    handlebars_context_dtor(ctx);
    ctx = NULL;
    talloc_free(root);
    root = NULL;
    partial_extension = NULL;
    partial_path = NULL;
}

PyObject *pyhandlebars_partial_extension(PyObject *extension) {
    const char *data;
    jmp_buf jmp;
    Py_ssize_t len;
    if (!PyUnicode_Check(extension)) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_Check"); goto ret; }
    if (!(data = PyUnicode_AsUTF8AndSize(extension, &len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); goto ret; }
    if (!root) root = talloc_new(NULL);
    if (!ctx) ctx = handlebars_context_ctor_ex(root);
    if (handlebars_setjmp_ex(ctx, &jmp)) {
        PyErr_SetString(PyExc_TypeError, handlebars_error_message(ctx));
        pyhandlebars_clean();
        goto ret;
    }
    partial_extension = handlebars_string_ctor(ctx, data, len);
ret:
    Py_RETURN_NONE;
}

/*PyObject *pyhandlebars_partial_path(PyObject *path) {
    jmp_buf jmp;
    text *path;
    if (PG_ARGISNULL(0)) E("path is null!");
    path = DatumGetTextP(PG_GETARG_DATUM(0));
    if (!root) root = talloc_new(NULL);
    if (!ctx) ctx = handlebars_context_ctor_ex(root);
    if (handlebars_setjmp_ex(ctx, &jmp)) {
        const char *error = pstrdup(handlebars_error_message(ctx));
        pyhandlebars_clean();
        E(error);
    }
    partial_path = handlebars_string_ctor(ctx, VARDATA_ANY(path), VARSIZE_ANY_EXHDR(path));
    PG_RETURN_NULL();
}*/

static PyObject *pyhandlebars_internal(PyObject *json, PyObject *template, PyObject *file) {
/*    char *output_data;
    const char *json_data;
    const char *template_data;
    FILE *out;
    Py_ssize_t json_len;
    Py_ssize_t output_len;
    Py_ssize_t template_len;
    if (!PyUnicode_Check(json)) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_Check"); goto ret; }
    if (!PyUnicode_Check(template)) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_Check"); goto ret; }
    if (!(json_data = PyUnicode_AsUTF8AndSize(json, &json_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); goto ret; }
    if (!(template_data = PyUnicode_AsUTF8AndSize(template, &template_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); goto ret; }
    if (file) {
        const char *file_data;
        if (!PyUnicode_Check(file)) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_Check"); goto ret; }
        if (!(file_data = PyUnicode_AsUTF8(file))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8"); goto ret; }
        if (!(out = fopen(file_data, "wb"))) { PyErr_SetString(PyExc_TypeError, "!fopen"); goto ret; }
    } else {
        if (!(out = open_memstream(&output_data, (size_t *)&output_len))) { PyErr_SetString(PyExc_TypeError, "!open_memstream"); goto ret; }
    }
    switch (pymustach_process(template_data, template_len, json_data, json_len, out)) {
        case MUSTACH_OK: break;
        case MUSTACH_ERROR_SYSTEM: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_SYSTEM"); goto free;
        case MUSTACH_ERROR_UNEXPECTED_END: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_UNEXPECTED_END"); goto free;
        case MUSTACH_ERROR_EMPTY_TAG: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_EMPTY_TAG"); goto free;
        case MUSTACH_ERROR_TAG_TOO_LONG: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_TAG_TOO_LONG"); goto free;
        case MUSTACH_ERROR_BAD_SEPARATORS: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_BAD_SEPARATORS"); goto free;
        case MUSTACH_ERROR_TOO_DEEP: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_TOO_DEEP"); goto free;
        case MUSTACH_ERROR_CLOSING: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_CLOSING"); goto free;
        case MUSTACH_ERROR_BAD_UNESCAPE_TAG: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_BAD_UNESCAPE_TAG"); goto free;
        case MUSTACH_ERROR_INVALID_ITF: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_INVALID_ITF"); goto free;
        case MUSTACH_ERROR_ITEM_NOT_FOUND: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_ITEM_NOT_FOUND"); goto free;
        case MUSTACH_ERROR_PARTIAL_NOT_FOUND: PyErr_SetString(PyExc_TypeError, "MUSTACH_ERROR_PARTIAL_NOT_FOUND"); goto free;
        default: PyErr_SetString(PyExc_TypeError, "ngx_http_mustach_process"); goto free;
    }
    if (file) Py_RETURN_TRUE; else {
        fclose(out);
        PyObject *unicode = PyUnicode_FromStringAndSize(output_data, output_len);
        free(output_data);
        return unicode;
    }
free:
    if (!file) free(output_data);
ret:*/
    Py_RETURN_NONE;
}

PyObject *pyhandlebars(PyObject *json, PyObject *template) { return pyhandlebars_internal(json, template, NULL); }
PyObject *pyhandlebars_file(PyObject *json, PyObject *template, PyObject *file) { return pyhandlebars_internal(json, template, file); }
