%module pyhandlebars
%{
PyObject *pyhandlebars_compiler_flag_all(void);
PyObject *pyhandlebars_compiler_flag_alternate_decorators(void);
PyObject *pyhandlebars_compiler_flag_assume_objects(void);
PyObject *pyhandlebars_compiler_flag_compat(void);
PyObject *pyhandlebars_compiler_flag_explicit_partial_context(void);
PyObject *pyhandlebars_compiler_flag_ignore_standalone(void);
PyObject *pyhandlebars_compiler_flag_known_helpers_only(void);
PyObject *pyhandlebars_compiler_flag_mustache_style_lambdas(void);
PyObject *pyhandlebars_compiler_flag_no_escape(void);
PyObject *pyhandlebars_compiler_flag_none(void);
PyObject *pyhandlebars_compiler_flag_prevent_indent(void);
PyObject *pyhandlebars_compiler_flag_strict(void);
PyObject *pyhandlebars_compiler_flag_string_params(void);
PyObject *pyhandlebars_compiler_flag_track_ids(void);
PyObject *pyhandlebars_compiler_flag_use_data(void);
PyObject *pyhandlebars_compiler_flag_use_depths(void);

extern PyObject *pyhandlebars(PyObject *json, PyObject *template);
extern PyObject *pyhandlebars_file(PyObject *json, PyObject *template, PyObject *file);
%}

PyObject *pyhandlebars_compiler_flag_all(void);
PyObject *pyhandlebars_compiler_flag_alternate_decorators(void);
PyObject *pyhandlebars_compiler_flag_assume_objects(void);
PyObject *pyhandlebars_compiler_flag_compat(void);
PyObject *pyhandlebars_compiler_flag_explicit_partial_context(void);
PyObject *pyhandlebars_compiler_flag_ignore_standalone(void);
PyObject *pyhandlebars_compiler_flag_known_helpers_only(void);
PyObject *pyhandlebars_compiler_flag_mustache_style_lambdas(void);
PyObject *pyhandlebars_compiler_flag_no_escape(void);
PyObject *pyhandlebars_compiler_flag_none(void);
PyObject *pyhandlebars_compiler_flag_prevent_indent(void);
PyObject *pyhandlebars_compiler_flag_strict(void);
PyObject *pyhandlebars_compiler_flag_string_params(void);
PyObject *pyhandlebars_compiler_flag_track_ids(void);
PyObject *pyhandlebars_compiler_flag_use_data(void);
PyObject *pyhandlebars_compiler_flag_use_depths(void);

extern PyObject *pyhandlebars(PyObject *json, PyObject *template);
extern PyObject *pyhandlebars_file(PyObject *json, PyObject *template, PyObject *file);
