#include <Python.h>

#include <handlebars/handlebars_compiler.h>
#include <handlebars/handlebars_delimiters.h>
#include <handlebars/handlebars_json.h>
#include <handlebars/handlebars_memory.h>
#include <handlebars/handlebars_opcode_serializer.h>
#include <handlebars/handlebars_parser.h>
#include <handlebars/handlebars_partial_loader.h>
#include <handlebars/handlebars_string.h>
#include <handlebars/handlebars_value.h>
#include <handlebars/handlebars_vm.h>
#include <json-c/json.h>
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
    if (!PyUnicode_Check(extension)) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_Check"); Py_RETURN_NONE; }
    if (!(data = PyUnicode_AsUTF8AndSize(extension, &len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); Py_RETURN_NONE; }
    if (!root) root = talloc_new(NULL);
    if (!ctx) ctx = handlebars_context_ctor_ex(root);
    if (handlebars_setjmp_ex(ctx, &jmp)) {
        PyErr_SetString(PyExc_TypeError, handlebars_error_message(ctx));
        pyhandlebars_clean();
        Py_RETURN_NONE;
    }
    partial_extension = handlebars_string_ctor(ctx, data, len);
    Py_RETURN_NONE;
}

PyObject *pyhandlebars_partial_path(PyObject *path) {
    const char *data;
    jmp_buf jmp;
    Py_ssize_t len;
    if (!PyUnicode_Check(path)) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_Check"); Py_RETURN_NONE; }
    if (!(data = PyUnicode_AsUTF8AndSize(path, &len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); Py_RETURN_NONE; }
    if (!root) root = talloc_new(NULL);
    if (!ctx) ctx = handlebars_context_ctor_ex(root);
    if (handlebars_setjmp_ex(ctx, &jmp)) {
        PyErr_SetString(PyExc_TypeError, handlebars_error_message(ctx));
        pyhandlebars_clean();
        Py_RETURN_NONE;
    }
    partial_path = handlebars_string_ctor(ctx, data, len);
    Py_RETURN_NONE;
}

static void handlebars_value_init_json_string_length(struct handlebars_context *ctx, struct handlebars_value *value, const char *json, size_t length) {
    enum json_tokener_error error;
    struct json_object *root;
    struct json_tokener *tok;
    if (!(tok = json_tokener_new())) handlebars_throw(ctx, HANDLEBARS_ERROR, "!json_tokener_new");
    do root = json_tokener_parse_ex(tok, json, length); while ((error = json_tokener_get_error(tok)) == json_tokener_continue);
    if (error != json_tokener_success) handlebars_throw(ctx, HANDLEBARS_ERROR, "!json_tokener_parse_ex and %s", json_tokener_error_desc(error));
    if (json_tokener_get_parse_end(tok) < length) handlebars_throw(ctx, HANDLEBARS_ERROR, "json_tokener_get_parse_end < %li", length);
    json_tokener_free(tok);
    handlebars_value_init_json_object(ctx, value, root);
    json_object_put(root);
}

static PyObject *pyhandlebars_internal(PyObject *json, PyObject *template, PyObject *file) {
    jmp_buf jmp;
    struct handlebars_ast_node *ast;
    struct handlebars_compiler *compiler;
    struct handlebars_module *module;
    struct handlebars_parser *parser;
    struct handlebars_program *program;
    struct handlebars_string *buffer = NULL;
    struct handlebars_string *tmpl;
    struct handlebars_value *input;
    struct handlebars_value *partials;
    const char *json_data;
    const char *template_data;
    Py_ssize_t json_len;
    Py_ssize_t template_len;
    if (!PyUnicode_Check(json)) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_Check"); Py_RETURN_NONE; }
    if (!PyUnicode_Check(template)) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_Check"); Py_RETURN_NONE; }
    if (!(json_data = PyUnicode_AsUTF8AndSize(json, &json_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); Py_RETURN_NONE; }
    if (!(template_data = PyUnicode_AsUTF8AndSize(template, &template_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); Py_RETURN_NONE; }
    if (!root) root = talloc_new(NULL);
    if (!ctx) ctx = handlebars_context_ctor_ex(root);
    if (handlebars_setjmp_ex(ctx, &jmp)) {
        PyErr_SetString(PyExc_TypeError, handlebars_error_message(ctx));
        pyhandlebars_clean();
        Py_RETURN_NONE;
    }
    compiler = handlebars_compiler_ctor(ctx);
    handlebars_compiler_set_flags(compiler, compiler_flags);
    parser = handlebars_parser_ctor(ctx);
    tmpl = handlebars_string_ctor(HBSCTX(parser), template_data, template_len);
    if (compiler_flags & handlebars_compiler_flag_compat) tmpl = handlebars_preprocess_delimiters(ctx, tmpl, NULL, NULL);
    ast = handlebars_parse_ex(parser, tmpl, compiler_flags);
    program = handlebars_compiler_compile_ex(compiler, ast);
    module = handlebars_program_serialize(ctx, program);
    input = handlebars_value_ctor(ctx);
    handlebars_value_init_json_string_length(ctx, input, json_data, json_len);
    if (convert_input) handlebars_value_convert(input);
    if (enable_partial_loader) partials = handlebars_value_partial_loader_init(ctx, partial_path ? partial_path : handlebars_string_ctor(ctx, ".", sizeof(".") - 1), partial_extension ? partial_extension : handlebars_string_ctor(ctx, ".hbs", sizeof(".hbs") - 1), handlebars_value_ctor(ctx));
    do {
        struct handlebars_vm *vm = handlebars_vm_ctor(ctx);
        handlebars_vm_set_flags(vm, compiler_flags);
        if (enable_partial_loader) handlebars_vm_set_partials(vm, partials);
        if (buffer) handlebars_talloc_free(buffer);
        buffer = handlebars_vm_execute(vm, module, input);
        buffer = talloc_steal(ctx, buffer);
        handlebars_vm_dtor(vm);
    } while(--run_count > 0);
    handlebars_value_dtor(input);
    handlebars_value_dtor(partials);
    if (file) {
        if (!buffer) {
            pyhandlebars_clean();
            Py_RETURN_FALSE;
        } else {
            const char *name;
            FILE *out;
            if (!PyUnicode_Check(file)) handlebars_throw(ctx, HANDLEBARS_ERROR, "!PyUnicode_Check");
            if (!(name = PyUnicode_AsUTF8(file))) handlebars_throw(ctx, HANDLEBARS_ERROR, "!PyUnicode_AsUTF8");
            if (!(out = fopen(name, "wb"))) handlebars_throw(ctx, HANDLEBARS_ERROR, "!fopen");
            fwrite(hbs_str_val(buffer), sizeof(char), hbs_str_len(buffer), out);
            fclose(out);
            pyhandlebars_clean();
            Py_RETURN_TRUE;
        }
    } else {
        if (!buffer) {
            pyhandlebars_clean();
            Py_RETURN_NONE;
        } else {
            PyObject *output = PyUnicode_FromStringAndSize(hbs_str_val(buffer), hbs_str_len(buffer));
            pyhandlebars_clean();
            return output;
        }
    }
}

PyObject *pyhandlebars(PyObject *json, PyObject *template) { return pyhandlebars_internal(json, template, NULL); }
PyObject *pyhandlebars_file(PyObject *json, PyObject *template, PyObject *file) { return pyhandlebars_internal(json, template, file); }
