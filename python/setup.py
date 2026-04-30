import os, sys, shutil, sysconfig
import pybind11
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

class FastestBuildExt(build_ext):
    user_options = build_ext.user_options + [
        ("user-lib=",     None, "path to user's compiled .a"),
        ("fastest-home=", None, "path to fastest source/build root"),
        ("module-name=",  None, "python module name (default: fastest)"),
    ]

    def initialize_options(self):
        super().initialize_options()
        self.user_lib     = None
        self.fastest_home = None
        self.module_name  = None

    def finalize_options(self):
        super().finalize_options()

        if self.fastest_home is None:
            self.fastest_home = os.environ.get("FASTEST_HOME")
        if self.fastest_home is None:
            raise RuntimeError("provide --fastest-home=... or set FASTEST_HOME")

        if self.user_lib is None:
            self.user_lib = os.environ.get("FASTEST_USER_LIB")
        if self.user_lib is None:
            raise RuntimeError("provide --user-lib=... or set FASTEST_USER_LIB")

        if self.module_name is None:
            self.module_name = os.environ.get("FASTEST_MODULE_NAME", "fastest")

        self.fastest_home = os.path.abspath(os.path.expanduser(self.fastest_home))
        self.user_lib     = os.path.abspath(os.path.expanduser(self.user_lib))

        pybind_src = os.path.join(self.fastest_home, "bindings", "pybind.cpp")
        with open(pybind_src) as f:
            content = f.read()

        content = content.replace("PYBIND11_MODULE(fastest,",
                                  f"PYBIND11_MODULE({self.module_name},")

        os.makedirs(self.build_temp, exist_ok=True)
        patched = os.path.join(self.build_temp, "pybind_patched.cpp")
        with open(patched, "w") as f:
            f.write(content)

        fastest_lib = os.path.join(self.fastest_home, "build", "libfastest.a")
        include_dir = os.path.join(self.fastest_home, "include")

        for ext in self.extensions:
            ext.name         = self.module_name
            ext.sources      = [patched]
            ext.include_dirs = [include_dir, pybind11.get_include()]
            if sys.platform == "darwin":
                ext.extra_link_args = ["-Wl,-all_load", fastest_lib, self.user_lib]
            else:
                ext.extra_link_args = ["-Wl,--whole-archive", fastest_lib, self.user_lib,
                                       "-Wl,--no-whole-archive"]

    def run(self):
        super().run()
        self._generate_stub()

    def build_extensions(self):
        super().build_extensions()

    def _generate_stub(self):
        import importlib.util

        search_dirs = [".", self.build_lib]
        so = None
        for d in search_dirs:
            for p in os.listdir(d):
                if p.startswith(self.module_name) and p.endswith(".so"):
                    so = os.path.join(d, p)
                    break
            if so:
                break

        if so is None:
            raise RuntimeError(f"could not find {self.module_name}*.so")

        spec = importlib.util.spec_from_file_location(self.module_name, so)
        mod  = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)

        tests_attrs = [k for k in dir(mod.tests) if not k.startswith("_")]

        stub_lines = [
            "from typing import Any",
            "",
            "def get_tests() -> list[dict[str, Any]]: ...",
            "def get_test(name: str) -> dict[str, Any]: ...",
            "def get_subtests(prefix: str) -> list[dict[str, Any]]: ...",
            "def run_test(name: str) -> None: ...",
            "",
            "class tests:",
        ] + [f"    {k}: str" for k in tests_attrs]

        stub_path = f"{self.module_name}.pyi"
        with open(stub_path, "w") as f:
            f.write("\n".join(stub_lines) + "\n")

        site_packages = sysconfig.get_path("purelib")
        shutil.copy(stub_path, os.path.join(site_packages, stub_path))

        print(f"generated {stub_path} -> {site_packages}")

ext = Extension(name="fastest", sources=[])

setup(
    name="fastest-tests",
    ext_modules=[ext],
    cmdclass={"build_ext": FastestBuildExt},
)
