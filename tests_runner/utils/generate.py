import os

from tests_runner.utils.command import run_command
from tests_runner.utils.dir import list_test_files, dump_string_to_file

from tests_runner.utils.config import COMPILER_EXEC_PATH


def generate_test_cases(parent_dir: str, type_dir: str, flag: str, ext: str) -> None:
    for file in list_test_files():
        timedout, output, exit_code = run_command([COMPILER_EXEC_PATH, file, flag])

        if timedout or exit_code != 0:
            continue

        dump_string_to_file(os.path.join(parent_dir, type_dir, file+ext), output)