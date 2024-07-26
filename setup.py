import setuptools
from setuptools import sandbox
import distutils
import textwrap
import os


cPythonExtDir = os.path.join(os.path.dirname(__file__), 'src', 'nettensorpat')

class Desc:
    comp = "Compiles the C Python extension module"

class Pnt_Cmds:
    
    def compile():
        fileList = '\t ' + '\n\t '.join([file for file in sorted(os.listdir(os.path.join(os.path.dirname(__file__), 'src', 'c'))) if file.endswith(".c") or file.endswith(".h")])
        
        print(textwrap.dedent(
            f"""
            ⚙️  Starting compilation of: 
            \t{os.path.join(os.path.dirname(__file__), 'src', 'c')}
            📝 Files scanned:
            __FILE_LIST__
            📝 Command:
            \tsh compile.sh
            """
        ).replace("__FILE_LIST__", fileList)[1:-1])
        
        cwd = os.getcwd()
        os.chdir(os.path.dirname(__file__))
        
        print(f"📂 {'Changed directory to `' + os.getcwd() + '` from `' + cwd + '`.' if cwd != os.getcwd() else 'Directory remains unchanged.'}")
        
        print("📝 Running compilation...")
        print("----------------------------------------\n")
        sandbox.run_setup("build_lib.py", ["build_ext", "--inplace"])
        print("\n----------------------------------------")
        # Check if compiled .so file exists
        if (os.path.exists(os.path.join(cPythonExtDir, 'Tensor_Python.so'))):
            print(f"🟢 Successfully compiled C Python extension module located in `{cPythonExtDir}`.")
        else:
            print(f"🔴 Failed to compile C Python extension module located in `{cPythonExtDir}`.")
            raise RuntimeError("Failed to compile C Python extension module.")        


class pnt(setuptools.Command):
    description = "Runs pattern net tensor commands"
    
    user_options = [
        ('compile', None, Desc.comp)
    ]
    
    fnMap = {
        "compile": Pnt_Cmds.compile
    }
    
    def initialize_options(self) -> None:
        self.subcommands = []
        self.compile = 0  # type: pathlib.Path

    def finalize_options(self) -> None:
        if (self.compile == 1):
            self.fnMap["compile"]()
    
    def run(self):
        print("🏁 Finished")

moduleName = "nettensorpat"

setuptools.setup(
    name=moduleName,
    version="1.0",
    description=textwrap.dedent("""
                       A package for finding the local clustering patterns in a given series of networks utilizing either local or global
                       """)[1:-1],
    package_dir={"": f"src"},
    packages=["src"],
    include_package_data=True,
    scripts=["compile.sh"],
    cmdclass={
        "pnt": pnt
    }
)