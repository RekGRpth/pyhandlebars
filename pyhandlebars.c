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

static unsigned long compiler_flags = handlebars_compiler_flag_alternate_decorators|handlebars_compiler_flag_compat|handlebars_compiler_flag_explicit_partial_context|handlebars_compiler_flag_ignore_standalone|handlebars_compiler_flag_known_helpers_only|handlebars_compiler_flag_mustache_style_lambdas|handlebars_compiler_flag_prevent_indent|handlebars_compiler_flag_use_data|handlebars_compiler_flag_use_depths;

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

static PyObject *pyhandlebars_internal(PyObject *json, PyObject *template, PyObject *file) {
    const char *json_data;
    const char *template_data;
    jmp_buf jmp;
    Py_ssize_t json_len;
    Py_ssize_t template_len;
    struct handlebars_ast_node *ast;
    struct handlebars_compiler *compiler;
    struct handlebars_context *ctx;
    struct handlebars_module *module;
    struct handlebars_parser *parser;
    struct handlebars_program *program;
    struct handlebars_string *buffer;
    struct handlebars_string *tmpl;
    struct handlebars_value *input;
    struct handlebars_value *partials;
    struct handlebars_vm *vm;
    if (!PyUnicode_Check(json)) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_Check"); Py_RETURN_NONE; }
    if (!PyUnicode_Check(template)) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_Check"); Py_RETURN_NONE; }
    if (!(json_data = PyUnicode_AsUTF8AndSize(json, &json_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); Py_RETURN_NONE; }
    if (!(template_data = PyUnicode_AsUTF8AndSize(template, &template_len))) { PyErr_SetString(PyExc_TypeError, "!PyUnicode_AsUTF8AndSize"); Py_RETURN_NONE; }
    ctx = handlebars_context_ctor();
    if (handlebars_setjmp_ex(ctx, &jmp)) {
        PyErr_SetString(PyExc_TypeError, handlebars_error_message(ctx));
        handlebars_context_dtor(ctx);
        Py_RETURN_NONE;
    }
    compiler = handlebars_compiler_ctor(ctx);
    handlebars_compiler_set_flags(compiler, compiler_flags);
    parser = handlebars_parser_ctor(ctx);
    tmpl = handlebars_string_ctor(ctx, template_data, template_len);
    if (compiler_flags & handlebars_compiler_flag_compat) tmpl = handlebars_preprocess_delimiters(ctx, tmpl, NULL, NULL);
    ast = handlebars_parse_ex(parser, tmpl, compiler_flags);
    program = handlebars_compiler_compile_ex(compiler, ast);
    module = handlebars_program_serialize(ctx, program);
    input = handlebars_value_ctor(ctx);
    buffer = handlebars_string_ctor(ctx, json_data, json_len);
    handlebars_value_init_json_string(ctx, input, hbs_str_val(buffer));
    handlebars_value_convert(input);
    partials = handlebars_value_partial_loader_init(ctx, handlebars_string_ctor(ctx, ".", sizeof(".") - 1), handlebars_string_ctor(ctx, ".hbs", sizeof(".hbs") - 1), handlebars_value_ctor(ctx));
    vm = handlebars_vm_ctor(ctx);
    handlebars_vm_set_flags(vm, compiler_flags);
    handlebars_vm_set_partials(vm, partials);
    buffer = talloc_steal(ctx, handlebars_vm_execute(vm, module, input));
    handlebars_vm_dtor(vm);
    handlebars_value_dtor(input);
    handlebars_value_dtor(partials);
    if (file) {
        if (!buffer) {
            handlebars_context_dtor(ctx);
            Py_RETURN_FALSE;
        } else {
            const char *name;
            FILE *out;
            if (!PyUnicode_Check(file)) handlebars_throw(ctx, HANDLEBARS_ERROR, "!PyUnicode_Check");
            if (!(name = PyUnicode_AsUTF8(file))) handlebars_throw(ctx, HANDLEBARS_ERROR, "!PyUnicode_AsUTF8");
            if (!(out = fopen(name, "wb"))) handlebars_throw(ctx, HANDLEBARS_ERROR, "!fopen");
            fwrite(hbs_str_val(buffer), sizeof(char), hbs_str_len(buffer), out);
            fclose(out);
            handlebars_context_dtor(ctx);
            Py_RETURN_TRUE;
        }
    } else {
        if (!buffer) {
            handlebars_context_dtor(ctx);
            Py_RETURN_NONE;
        } else {
            PyObject *output = PyUnicode_FromStringAndSize(hbs_str_val(buffer), hbs_str_len(buffer));
            handlebars_context_dtor(ctx);
            return output;
        }
    }
}

PyObject *pyhandlebars(PyObject *json, PyObject *template) { return pyhandlebars_internal(json, template, NULL); }
PyObject *pyhandlebars_file(PyObject *json, PyObject *template, PyObject *file) { return pyhandlebars_internal(json, template, file); }
