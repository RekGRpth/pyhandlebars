%module pyhandlebars
%{
#include <stdbool.h>

extern PyObject *pyhandlebars_compiler_flag_all(void);
extern PyObject *pyhandlebars_compiler_flag_alternate_decorators(void);
extern PyObject *pyhandlebars_compiler_flag_assume_objects(void);
extern PyObject *pyhandlebars_compiler_flag_compat(void);
extern PyObject *pyhandlebars_compiler_flag_explicit_partial_context(void);
extern PyObject *pyhandlebars_compiler_flag_ignore_standalone(void);
extern PyObject *pyhandlebars_compiler_flag_known_helpers_only(void);
extern PyObject *pyhandlebars_compiler_flag_mustache_style_lambdas(void);
extern PyObject *pyhandlebars_compiler_flag_no_escape(void);
extern PyObject *pyhandlebars_compiler_flag_none(void);
extern PyObject *pyhandlebars_compiler_flag_prevent_indent(void);
extern PyObject *pyhandlebars_compiler_flag_strict(void);
extern PyObject *pyhandlebars_compiler_flag_string_params(void);
extern PyObject *pyhandlebars_compiler_flag_track_ids(void);
extern PyObject *pyhandlebars_compiler_flag_use_data(void);
extern PyObject *pyhandlebars_compiler_flag_use_depths(void);

extern PyObject *pyhandlebars(PyObject *json, PyObject *template);
extern PyObject *pyhandlebars_file(PyObject *json, PyObject *template, PyObject *file);
%}

extern PyObject *pyhandlebars_compiler_flag_all(void);
extern PyObject *pyhandlebars_compiler_flag_alternate_decorators(void);
extern PyObject *pyhandlebars_compiler_flag_assume_objects(void);
extern PyObject *pyhandlebars_compiler_flag_compat(void);
extern PyObject *pyhandlebars_compiler_flag_explicit_partial_context(void);
extern PyObject *pyhandlebars_compiler_flag_ignore_standalone(void);
extern PyObject *pyhandlebars_compiler_flag_known_helpers_only(void);
extern PyObject *pyhandlebars_compiler_flag_mustache_style_lambdas(void);
extern PyObject *pyhandlebars_compiler_flag_no_escape(void);
extern PyObject *pyhandlebars_compiler_flag_none(void);
extern PyObject *pyhandlebars_compiler_flag_prevent_indent(void);
extern PyObject *pyhandlebars_compiler_flag_strict(void);
extern PyObject *pyhandlebars_compiler_flag_string_params(void);
extern PyObject *pyhandlebars_compiler_flag_track_ids(void);
extern PyObject *pyhandlebars_compiler_flag_use_data(void);
extern PyObject *pyhandlebars_compiler_flag_use_depths(void);

extern PyObject *pyhandlebars(PyObject *json, PyObject *template);
extern PyObject *pyhandlebars_file(PyObject *json, PyObject *template, PyObject *file);
