import sys

from tests_runner.framework import tester

from tests_runner import parser_tests, ir_tests, compiler_tests, llc_tests, cli_tests


def main() -> int:
    tester.generator("tests/parser", "expected/json", "json", "-json-ast", False)
    tester.generator("tests/parser", "expected/pretty", "txt", "-ast", False)
    tester.generator("tests/ir", "expected/json", "json", "-json-ir", False)
    tester.generator("tests/compiler", "expected/json", "json", "-json-icode-all", False)
    tester.generator("tests/compiler", "expected/pretty", "txt", "-icode-all", False)
    tester.register([parser_tests, ir_tests, compiler_tests, llc_tests, cli_tests])

    return tester.run()


if __name__ == "__main__":
    sys.exit(main())
