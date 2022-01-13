import os

from tests_runner.framework import batch_run
from tests_runner.framework import remove_files
from tests_runner.framework import TestResult
from tests_runner.framework import LLC_BIN
from tests_runner.framework import compile_phase, command_on_compile_success_output_assert


def run_single(file_name: str) -> TestResult:
    llc_file_name = file_name + ".llc"

    return command_on_compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-llvm",
            compiler_output_dump_file_path=llc_file_name,
            create_executable=False,
            skip_on_compile_error=True
        ),
        expected_on_compile_result_fail=None,
        command_on_compile_result_pass=[LLC_BIN, llc_file_name],
        expected_command_output=None,
    )


def run() -> None:
    os.chdir("tests/compiler")

    remove_files(".llc")

    batch_run("llc", run_single)

    remove_files(".o")
    remove_files(".llc")
    remove_files(".llc.s")

    os.chdir("../..")
