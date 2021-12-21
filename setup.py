from distutils.core import Extension, setup

setup(
    author_email='RekGRpth@gmail.com',
    author='RekGRpth',
    ext_modules=[Extension(
        libraries=["handlebars"],
        name="_pyhandlebars",
        sources=["pyhandlebars.i", "pyhandlebars.c"],
    )],
    license='MIT',
    name="pyhandlebars",
    py_modules=["pyhandlebars"],
    url="https://github.com/RekGRpth/pyhandlebars",
    version='1.0.0',
)
