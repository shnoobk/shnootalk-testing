import os

from tests_runner.utils.command import run_command
from tests_runner.utils.result import TestResult, ResultPrinter

from tests_runner.utils.config import COMPILER_EXEC_PATH


USAGE_HELP = '''USAGE: shtkc FILE OPTION

Available options:
    -c         Compile program
    -ast       Print parse tree
    -json-ast  Print parse tree in JSON
    -ir        intermediate code representation
    -json-ir   Print intermediate code representation completely in JSON
    -llvm      Print llvm ir

Use shtkc -version for compiler version
'''

FILE_IO_ERROR = "File I/O error\n"


def run_version_test() -> TestResult:
    cmd = [COMPILER_EXEC_PATH, "-version"]

    timedout, output, exit_code = run_command(cmd)

    if exit_code != 0 or timedout:
        return TestResult.failed(output)

    if "SNAPSHOT" not in output and "RELEASE" not in output:
        return TestResult.failed(output)

    return TestResult.passed(output)


def run_invalid_args(empty: bool) -> TestResult:
    cmd = \
        [COMPILER_EXEC_PATH] if empty else [COMPILER_EXEC_PATH, "TestModules/Math.shtk", "-invalid"]

    timedout, output, exit_code = run_command(cmd)

    if exit_code == 0 or timedout:
        return TestResult.failed(output)

    if output != USAGE_HELP:
        return TestResult.failed(output, USAGE_HELP)

    return TestResult.passed(output)


def run_file_no_exists() -> TestResult:
    timedout, output, exit_code = run_command([COMPILER_EXEC_PATH, "NoExist.shtk", "-c"])

    if exit_code == 0 or timedout:
        return TestResult.failed(output)

    if output != FILE_IO_ERROR:
        return TestResult.failed(output, FILE_IO_ERROR)

    return TestResult.passed(output)


def run() -> None:
    os.chdir("tests/compiler")

    printer = ResultPrinter('cli args')

    printer.print_result('No args', run_invalid_args(empty=True))
    printer.print_result('Invalid args', run_invalid_args(empty=False))
    printer.print_result('File not found', run_file_no_exists())
    printer.print_result('-version', run_version_test())

    os.chdir("../..")
