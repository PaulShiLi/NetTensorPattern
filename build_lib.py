import os
from distutils import sysconfig
import setuptools
from setuptools import Extension, setup, find_packages
from setuptools.command.build_ext import build_ext
from typing import Literal

moduleName = "Tensor_Main"
cRelativeDir = f"src{os.sep}c"

sourceFiles = [
    file for file in os.listdir(os.path.join(os.path.dirname(__file__), cRelativeDir)) if file.endswith(".c")
]

# Autofind header files in ./c
headerFiles = [
    file for file in os.listdir(os.path.join(os.path.dirname(__file__), cRelativeDir)) if file.endswith(".h")
]

dependencies = [
    "Cython"
]

def setPaths(files: list[str], relativeFolder: str, exclude: list[str] = [], pathType: Literal["absolute", "relative"] = "absolute"):
    return [os.path.join(os.path.dirname(__file__), relativeFolder, file) for file in files if file not in exclude] if pathType == "absolute" else [os.path.join(cRelativeDir, file) for file in sourceFiles if file not in ["Tensor_Main.c"]]

modules = [
    Extension(
        name="Tensor_Python",
        sources=setPaths(sourceFiles, cRelativeDir, ["Tensor_Main.c"], "relative"),
        # headers=sourceAbsFiles(headerFiles, cRelativeDir),
    )
    
]

class NoSuffixBuilder(build_ext):    
    def get_ext_filename(self, ext_name):
        filename = super().get_ext_filename(ext_name)
        suffix = sysconfig.get_config_var('EXT_SUFFIX')
        ext = os.path.splitext(filename)[1]
        return filename.replace(suffix, '') + ext

setup(
    name=moduleName,
    version="1.0",
    description="...",
    ext_modules=modules,
    package_dir={"": f"src/nettensorpat"},
    cmdclass={'build_ext': NoSuffixBuilder}
)