from setuptools import setup, Extension

module = Extension(
    "_pyhandlebars",
    ["pyhandlebars.i", "pyhandlebars.c"],
    libraries=["handlebars"],
)

setup(
    ext_modules=[module],
    name="pyhandlebars",
    py_modules=["pyhandlebars"],
    url="https://github.com/RekGRpth/pyhandlebars",
    version='1.0.0',
)
