import os
import importlib

class Runner:
    def __init__(self, backend=None):
        self._backend = None
        self.verbose = False
        if backend is not None:
            self.set_backend(backend)
        elif os.environ.get("FASTEST_PROJECT"):
            self._auto_detect_backend()

    def _auto_detect_backend(self):
        project = os.environ["FASTEST_PROJECT"]
        mod = importlib.import_module(project)
        self.set_backend(mod)

    def set_backend(self, module):
        required = ["get_tests", "get_test", "run_test", "tests"]
        if not all(hasattr(module, attr) for attr in required):
            raise TypeError("Module missing required fastest backend interface")
        self._backend = module

    @property
    def backend(self):
        if self._backend is None:
            raise RuntimeError("No backend set. Pass one to Runner() or set FASTEST_PROJECT.")
        return self._backend

    def get_tests(self):
        return self.backend.get_tests()

    def run_test(self, name):
        return self.backend.run_test(name)

    def run_all(self):
        for t in self.get_tests():
            if self.verbose:
                print(f"Running {t['test_name']}")
            self.run_test(t["test_name"])

    def report(self):
        for t in self.get_tests():
            print(f"{t['test_name']:30s} {t['time_ns']} ns")
