from distutils.core import setup, Extension
from distutils.command.build_py import build_py as build_py_orig
import sys, os

class build_ext_first(build_py_orig):
    def run(self):
        self.run_command("build_ext")
        return build_py_orig.run(self)

if sys.platform.startswith("linux"):
    module = Extension(
        "_pyhandlebars",
        ["pyhandlebars.i", "pyhandlebars.c"],
        libraries=["handlebars"],
    )

setup(
    name="pyhandlebars",
    url="https://github.com/RekGRpth/pyhandlebars",
    cmdclass={"build_py": build_ext_first},
    ext_modules=[module],
    py_modules=["pyhandlebars"],
)
