"""Behavioral regressions for the driver; real smoke tests never run XJ380 code."""

import argparse
import concurrent.futures
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import textwrap
import unittest


NAMES = (
    "xxcc", "xxcc--", "xxcc-elf", "xxcc-elf--",
    "xxccte", "xxccte--", "xxccte-elf", "xxccte-elf--",
)
ROOT = Path(__file__).resolve().parents[1]
BIN_DIR = ROOT / "release" / ("windows" if os.name == "nt" else "linux")
SUFFIX = ".exe" if os.name == "nt" else ""


class DriverCase(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="xxcc-tests-")
        self.addCleanup(self.temp.cleanup)
        self.work = Path(self.temp.name)
        self.env = dict(os.environ)
        for key in ("XXCC_SDK_ROOT", "XXCC_CC", "XXCC_CXX", "XXCC_LD"):
            self.env.pop(key, None)

    def run_driver(self, args, name="xxcc", env=None, binary=None):
        return subprocess.run(
            [str(binary or BIN_DIR / (name + SUFFIX)), *map(str, args)],
            cwd=self.work, env=env or self.env, capture_output=True,
            text=True, timeout=60,
        )

    def success(self, result):
        self.assertEqual(result.returncode, 0, result.stderr)


@unittest.skipIf(os.name == "nt", "mock executables use POSIX shebangs")
class DriverRegressions(DriverCase):
    def setUp(self):
        super().setUp()
        self.sdk = self.work / "SDK 空格"
        (self.sdk / "include").mkdir(parents=True)
        directory = self.sdk / "lib" / "x86_64"
        directory.mkdir(parents=True)
        for item in ("start.o", "constart.o", "crt0.o", "libxact.a", "libxgui.a",
                     "libxnet.a", "libxmedia.a", "liballoc.a"):
            (directory / item).touch()
        self.log = self.work / "calls.jsonl"
        self.tmpdir = self.work / "temporary files"
        self.tmpdir.mkdir()
        mock = self.work / "backend 空格.py"
        mock.write_text("#!" + sys.executable + "\n" + textwrap.dedent('''
            import json, os, pathlib, signal, sys
            args = sys.argv[1:]
            stage = "link" if "-Ttext=0x200000" in args else "compile"
            with open(os.environ["XXCC_TEST_LOG"], "a") as log:
                log.write(json.dumps({"stage": stage, "args": args}) + "\\n")
            output = pathlib.Path(args[args.index("-o") + 1]) if "-o" in args else None
            skip_output = os.environ.get("XXCC_TEST_NO_OUTPUT") == stage
            if output and not skip_output:
                output.write_text("partial-" + stage)
            if os.environ.get("XXCC_TEST_SIGNAL") == stage:
                os.kill(os.getpid(), signal.SIGTERM)
            if os.environ.get("XXCC_TEST_FAIL") == stage:
                sys.exit(23 if stage == "compile" else 29)
            if output and not skip_output:
                output.write_text("success-" + stage)
            elif not output:
                print("preprocessed text")
        '''), encoding="utf-8")
        mock.chmod(0o755)
        self.env.update(XXCC_SDK_ROOT=str(self.sdk), XXCC_CC=str(mock),
                        XXCC_CXX=str(mock), XXCC_LD=str(mock),
                        XXCC_TEST_LOG=str(self.log), TMPDIR=str(self.tmpdir))
        (self.work / "main.cpp").write_text("int main() { return 0; }\n")
        (self.work / "main.c").write_text("int main(void) { return 0; }\n")

    def calls(self):
        return [json.loads(line) for line in self.log.read_text().splitlines()] if self.log.exists() else []

    def assert_clean(self):
        self.assertEqual(list(self.tmpdir.iterdir()), [])
        self.assertEqual(list(self.work.glob(".xxcc-*")), [])

    def test_all_eight_profiles(self):
        for name in NAMES:
            with self.subTest(name=name):
                self.log.unlink(missing_ok=True)
                source = "main.c" if name.endswith("--") else "main.cpp"
                self.success(self.run_driver([source], name=name))
                compile_call, link_call = self.calls()
                self.assertIn("-std=c11" if name.endswith("--") else "-std=c++11", compile_call["args"])
                self.assertIn("-ffreestanding", compile_call["args"])
                self.assertIn("-fshort-wchar", compile_call["args"])
                runtime = self.sdk / "lib" / "x86_64"
                startup = "constart.o" if name.startswith("xxccte") else "start.o"
                self.assertIn(str(runtime / startup), link_call["args"])
                self.assertIn(str(runtime / "libxnet.a"), link_call["args"])
                self.assertIn(str(runtime / "crt0.o"), link_call["args"])
                self.assertIn(str(runtime / "libxact.a"), link_call["args"])
                self.assertIn(str(runtime / "liballoc.a"), link_call["args"])
                if name.startswith("xxccte"):
                    self.assertNotIn(str(runtime / "libxgui.a"), link_call["args"])
                else:
                    self.assertIn(str(runtime / "libxgui.a"), link_call["args"])
                self.assertTrue((self.work / ("a.elf" if "-elf" in name else "a.epf")).exists())
        self.assert_clean()

    def test_options_before_and_after_inputs(self):
        self.success(self.run_driver(["-O2", "-I", "my headers", "main.cpp", "-D", "VALUE=1", "-oapp.epf"]))
        command = self.calls()[0]["args"]
        for expected in ("-O2", "my headers", "VALUE=1"):
            self.assertIn(expected, command)
        self.assertTrue((self.work / "app.epf").exists())

    def test_spaces_unicode_and_shell_metacharacters_are_literal(self):
        source = "源 file;touch injected;.cpp"
        (self.work / source).touch()
        define = '-DVALUE=$(touch injected) a b"quote"\\tail'
        self.success(self.run_driver([source, define, "-o", "程序 output.elf"]))
        command = self.calls()[0]["args"]
        self.assertIn(source, command)
        self.assertIn(define, command)
        self.assertFalse((self.work / "injected").exists())
        self.assertTrue((self.work / "程序 output.elf").exists())

    def test_missing_operands_and_inputs_report_errors(self):
        for args in (["main.cpp", "-o"], ["-I"], ["--xxcc-sdk"], ["-O2"],
                     ["--xxcc-sdk="], ["main.cpp", "-o", "one", "-o", "two"]):
            with self.subTest(args=args):
                result = self.run_driver(args)
                self.assertGreater(result.returncode, 0, result.stderr)
                self.assertIn("xxcc: error:", result.stderr)
        self.assertEqual(self.calls(), [])

    def test_compiler_failure_stops_link_and_preserves_output(self):
        (self.work / "app.epf").write_text("old output")
        self.env["XXCC_TEST_FAIL"] = "compile"
        result = self.run_driver(["main.cpp", "-o", "app.epf"])
        self.assertEqual(result.returncode, 23, result.stderr)
        self.assertEqual([c["stage"] for c in self.calls()], ["compile"])
        self.assertEqual((self.work / "app.epf").read_text(), "old output")
        self.assert_clean()

    def test_linker_failure_preserves_output(self):
        (self.work / "app.epf").write_text("old output")
        self.env["XXCC_TEST_FAIL"] = "link"
        result = self.run_driver(["main.cpp", "-o", "app.epf"])
        self.assertEqual(result.returncode, 29, result.stderr)
        self.assertEqual((self.work / "app.epf").read_text(), "old output")
        self.assert_clean()

    def test_missing_executable_returns_127(self):
        self.env["XXCC_CXX"] = str(self.work / "missing compiler")
        result = self.run_driver(["main.cpp"])
        self.assertEqual(result.returncode, 127, result.stderr)
        self.assertEqual(self.calls(), [])
        self.assert_clean()

    def test_child_signal_returns_nonzero_and_stops_link(self):
        self.env["XXCC_TEST_SIGNAL"] = "compile"
        result = self.run_driver(["main.cpp"])
        self.assertEqual(result.returncode, 128 + signal.SIGTERM, result.stderr)
        self.assertEqual(len(self.calls()), 1)
        self.assert_clean()

    def test_same_basename_produces_distinct_objects(self):
        for directory in ("a", "b"):
            (self.work / directory).mkdir()
            (self.work / directory / "foo.cpp").touch()
        self.success(self.run_driver(["a/foo.cpp", "b/foo.cpp", "-o", "app.epf"]))
        first, second, link = self.calls()
        outputs = [call["args"][call["args"].index("-o") + 1] for call in (first, second)]
        self.assertNotEqual(*outputs)
        for output in outputs:
            self.assertIn(output, link["args"])
        self.assert_clean()

    def test_concurrent_invocations_have_distinct_work_directories(self):
        def build(index):
            return self.run_driver(["main.cpp", "-o", "app-" + str(index) + ".epf"])
        with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
            results = list(pool.map(build, range(8)))
        for result in results:
            self.success(result)
        outputs = [c["args"][c["args"].index("-o") + 1] for c in self.calls() if c["stage"] == "compile"]
        self.assertEqual(len(set(outputs)), 8)
        self.assert_clean()

    def test_large_argument_list_has_no_fixed_array_limit(self):
        definitions = ["-DVALUE_" + str(index) + "=1" for index in range(350)]
        self.success(self.run_driver([*definitions, "main.cpp"]))
        for define in definitions:
            self.assertIn(define, self.calls()[0]["args"])

    def test_sdk_paths_longer_than_old_buffers(self):
        destination = self.work.joinpath(*(["segment_" + "long" * 18] * 9), "SDK")
        destination.parent.mkdir(parents=True)
        shutil.move(str(self.sdk), destination)
        self.env["XXCC_SDK_ROOT"] = str(destination)
        self.success(self.run_driver(["main.cpp"]))
        self.assertGreater(len(str(destination)), 512)
        self.assertIn(str(destination / "include"), self.calls()[0]["args"])
        self.assert_clean()

    def test_backend_success_without_output_is_an_error(self):
        for stage in ("compile", "link"):
            with self.subTest(stage=stage):
                (self.work / "app.epf").write_text("old output")
                self.env["XXCC_TEST_NO_OUTPUT"] = stage
                result = self.run_driver(["main.cpp", "-o", "app.epf"])
                self.assertGreater(result.returncode, 0, result.stderr)
                self.assertEqual((self.work / "app.epf").read_text(), "old output")
                self.assert_clean()

    def test_explicit_sdk_overrides_environment_and_bad_sdk_fails_early(self):
        self.env["XXCC_SDK_ROOT"] = str(self.work / "missing SDK")
        self.success(self.run_driver(["--xxcc-sdk", self.sdk, "main.cpp"]))
        self.log.unlink()
        result = self.run_driver(["main.cpp"])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("SDK include directory not found", result.stderr)
        self.assertEqual(self.calls(), [])

    def test_executable_relative_sdk_lookup(self):
        binary = self.sdk / "tools/bin/xxcc"
        binary.parent.mkdir(parents=True)
        shutil.copy2(BIN_DIR / ("xxcc" + SUFFIX), binary)
        self.env.pop("XXCC_SDK_ROOT")
        self.success(self.run_driver(["main.cpp"], binary=binary))
        self.assertIn(str(self.sdk / "include"), self.calls()[0]["args"])

    def test_missing_runtime_library_fails_before_compilation(self):
        (self.sdk / "lib/x86_64/libxact.a").unlink()
        result = self.run_driver(["main.cpp"])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("libxact.a", result.stderr)
        self.assertEqual(self.calls(), [])

    def test_compile_only_does_not_require_runtime(self):
        shutil.rmtree(self.sdk / "lib")
        self.success(self.run_driver(["main.cpp", "-c", "-o", "main.o"]))
        self.assertEqual([c["stage"] for c in self.calls()], ["compile"])
        self.assertTrue((self.work / "main.o").exists())
        self.assert_clean()

    def test_failed_compile_only_preserves_old_object(self):
        (self.work / "main.o").write_text("old object")
        self.env["XXCC_TEST_FAIL"] = "compile"
        self.assertEqual(self.run_driver(["-c", "main.cpp"]).returncode, 23)
        self.assertEqual((self.work / "main.o").read_text(), "old object")
        self.assert_clean()

    def test_preprocessing_uses_stdout_without_linking(self):
        result = self.run_driver(["-E", "main.cpp", "-o", "-"])
        self.success(result)
        self.assertIn("preprocessed text", result.stdout)
        self.assertEqual(len(self.calls()), 1)
        self.assert_clean()

    def test_user_objects_libraries_and_linker_options_keep_order(self):
        (self.work / "helper.o").touch()
        (self.work / "custom.a").touch()
        self.success(self.run_driver(["main.cpp", "helper.o", "custom.a", "-L", "libs with spaces", "-lfoo", "-Wl,--gc-sections", "-Xlinker", "--no-undefined"]))
        args = self.calls()[-1]["args"]
        ordered = [str(self.work / "helper.o"), str(self.work / "custom.a"), "-L", "libs with spaces", "-lfoo", "--gc-sections", "--no-undefined", str(self.sdk / "lib/x86_64/libxnet.a")]
        self.assertEqual([args.index(item) for item in ordered], sorted(args.index(item) for item in ordered))

    def test_no_source_overwrite(self):
        result = self.run_driver(["main.cpp", "-o", "main.cpp"])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("overwrite input", result.stderr)
        self.assertEqual(self.calls(), [])

    def test_save_temps_keeps_only_owned_intermediate_directory(self):
        self.success(self.run_driver(["--xxcc-save-temps", "main.cpp"]))
        folders = list(self.tmpdir.iterdir())
        self.assertEqual(len(folders), 1)
        self.assertTrue((folders[0] / "input-0.o").exists())
        self.assertEqual(list(self.work.glob(".xxcc-*")), [])

    def test_leading_dash_filename_after_separator(self):
        (self.work / "-input.cpp").touch()
        self.success(self.run_driver(["-o", "app.epf", "--", "-input.cpp"]))
        self.assertEqual(self.calls()[0]["args"][-2:], ["--", "-input.cpp"])


@unittest.skipUnless(shutil.which("clang") and shutil.which("clang++") and shutil.which("ld") and os.name != "nt",
                     "real smoke tests need Linux Clang and GNU ld")
class RealToolchain(DriverCase):
    def test_all_eight_entry_points_preserve_elf_abi(self):
        body = '''
            #if __STDC_HOSTED__ != 0
            #error freestanding flag missing
            #endif
            #ifndef __XJ380_OS__
            #error OS macro missing
            #endif
            #ifdef __cplusplus
            static_assert(__cplusplus == 201103L, "C++11 default");
            static_assert(sizeof(void*) == 8 && sizeof(wchar_t) == 2, "ABI");
            #else
            _Static_assert(__STDC_VERSION__ == 201112L, "C11 default");
            _Static_assert(sizeof(void*) == 8 && sizeof(__WCHAR_TYPE__) == 2, "ABI");
            #endif
            int main(int argc, char **argv, char **envp) { return 0; }
        '''
        for name in NAMES:
            with self.subTest(name=name):
                source = "hello world.c" if name.endswith("--") else "hello world.cpp"
                (self.work / source).write_text(textwrap.dedent(body))
                output = self.work / (name + (".elf" if "-elf" in name else ".epf"))
                self.success(self.run_driver(["-O2", source, "-o", output], name=name))
                self.assertEqual(output.read_bytes()[:4], b"\x7fELF")
                if shutil.which("readelf"):
                    # Keep readelf output parseable regardless of the host locale.
                    readelf_env = dict(os.environ, LC_ALL="C", LANG="C")
                    header = subprocess.check_output(["readelf", "-h", output], text=True, env=readelf_env)
                    sections = subprocess.check_output(["readelf", "-SW", output], text=True, env=readelf_env)
                    symbols = subprocess.check_output(["readelf", "-sW", output], text=True, env=readelf_env)
                    segments = subprocess.check_output(["readelf", "-lW", output], text=True, env=readelf_env)
                    self.assertIn("ELF64", header)
                    self.assertIn("Advanced Micro Devices X86-64", header)
                    self.assertRegex(sections, r"\]\s+\.text\s+PROGBITS\s+0*200000\b")
                    entry = int(re.search(r"Entry point address:\s*(0x[0-9a-f]+)", header).group(1), 16)
                    start = next(int(line.split()[1], 16) for line in symbols.splitlines() if line.split() and line.split()[-1] == "_start")
                    self.assertEqual(entry, start)
                    self.assertNotIn("INTERP", segments)

    def test_real_multiple_sources_options_and_dependencies(self):
        for directory in ("a", "b", "my headers"):
            (self.work / directory).mkdir()
        (self.work / "my headers/config.h").write_text("#define EXPECTED 7\n")
        (self.work / "a/foo.cpp").write_text('#include "config.h"\nint helper() { return EXPECTED; }\n')
        (self.work / "b/foo.cpp").write_text("int helper();\nint main() { return helper() - VALUE; }\n")
        self.success(self.run_driver(["a/foo.cpp", "-I", "my headers", "b/foo.cpp", "-DVALUE=7", "-O2", "-o", "app.epf"]))
        self.assertTrue((self.work / "app.epf").exists())
        self.success(self.run_driver(["-c", "a/foo.cpp", "-I", "my headers", "-MMD", "-o", "helper.o"]))
        dependency = (self.work / "helper.d").read_text()
        self.assertIn("helper.o:", dependency)
        self.assertIn("config.h", dependency)
        self.assertNotIn(".xxcc-", dependency)

    def test_real_compile_and_link_errors_preserve_existing_output(self):
        output = self.work / "app.epf"
        output.write_bytes(b"old working program")
        (self.work / "bad.cpp").write_text("this is not valid C++\n")
        result = self.run_driver(["bad.cpp", "-o", output])
        self.assertGreater(result.returncode, 0)
        self.assertIn("compilation failed", result.stderr)
        self.assertEqual(output.read_bytes(), b"old working program")
        (self.work / "bad.cpp").write_text("extern int missing(); int main() { return missing(); }\n")
        result = self.run_driver(["bad.cpp", "-o", output])
        self.assertGreater(result.returncode, 0)
        self.assertIn("linking failed", result.stderr)
        self.assertEqual(output.read_bytes(), b"old working program")
        self.assertEqual(list(self.work.glob(".xxcc-*")), [])

    def test_existing_example_compiles_and_links(self):
        self.success(self.run_driver([ROOT / "example.cpp", "-o", "example.epf"]))
        self.assertTrue((self.work / "example.epf").exists())

    @unittest.skipUnless(shutil.which("ar"), "archive smoke test needs ar")
    def test_real_user_static_library_and_object_inputs(self):
        (self.work / "helper.c").write_text("int helper(void) { return 7; }\n")
        (self.work / "main.cpp").write_text('extern "C" int helper(void);\nint main() { return helper() - 7; }\n')
        self.success(self.run_driver(["-c", "helper.c", "-o", "helper.o"], name="xxcc--"))
        subprocess.run(["ar", "rcs", "libhelper.a", "helper.o"], cwd=self.work, check=True, capture_output=True)
        self.success(self.run_driver(["main.cpp", "libhelper.a", "-o", "archive.epf"]))
        self.success(self.run_driver(["main.cpp", "helper.o", "-o", "object.epf"]))

    def test_real_assembly_and_preprocessor_modes(self):
        (self.work / "main.c").write_text("int main(void) { return NUMBER; }\n")
        self.success(self.run_driver(["-S", "main.c", "-DNUMBER=17", "-o", "main.s"], name="xxcc--"))
        self.assertIn("main", (self.work / "main.s").read_text())
        result = self.run_driver(["-E", "main.c", "-DNUMBER=17"], name="xxcc--")
        self.success(result)
        self.assertIn("return 17", result.stdout)
        self.success(self.run_driver(["-E", "main.c", "-DNUMBER=17", "-o", "main.i"], name="xxcc--"))
        self.assertIn("return 17", (self.work / "main.i").read_text())


if __name__ == "__main__":
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--bin-dir", type=Path, default=BIN_DIR)
    parser.add_argument("--suffix", default=SUFFIX)
    options, rest = parser.parse_known_args()
    BIN_DIR = options.bin_dir.resolve()
    SUFFIX = options.suffix
    unittest.main(argv=[sys.argv[0], *rest])
