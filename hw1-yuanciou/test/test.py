#!/usr/bin/python3

import argparse
import colorama
import subprocess
import sys
from dataclasses import dataclass
from enum import Enum, auto
from pathlib import Path
from typing import Dict, List

DIR = Path(__file__).resolve().parent


class CaseType(Enum):
    OPEN = auto()
    HIDDEN = auto()


class TestStatus(Enum):
    PASS = auto()
    FAIL = auto()
    SKIP = auto()


@dataclass
class TestCase:
    type: CaseType
    score: float
    name: str


class Grader:
    """
    case_id: TestCase(case_type, score, case_name)
        case_id     Used by the "--case_id" flag to run only one test case
        case_type   The diff of CaseType.HIDDEN is not shown
        score       The max score of the test case
        case_name   The name of the file in "test_cases" and "sample_solutions"
    """
    CASES: Dict[str, TestCase] = {
        "1": TestCase(CaseType.OPEN, 2.5, "01_keywords_1"),
        "2": TestCase(CaseType.OPEN, 2.5, "02_keywords_2"),
        "3": TestCase(CaseType.OPEN, 2.5, "03_delimiters"),
        "4": TestCase(CaseType.OPEN, 2.5, "04_arithmetic"),
        "5": TestCase(CaseType.OPEN, 2.5, "05_logical"),
        "6": TestCase(CaseType.OPEN, 2.5, "06_relational"),
        "7": TestCase(CaseType.OPEN, 2.5, "07_identifier"),
        "8": TestCase(CaseType.OPEN, 2.5, "08_constant"),
        "9": TestCase(CaseType.OPEN, 2.5, "09_scientific_string"),
        "10": TestCase(CaseType.OPEN, 2.5, "10_comment"),
        "11": TestCase(CaseType.OPEN, 2.5, "11_pseudocomment"),
        "12": TestCase(CaseType.OPEN, 1.5, "12_advance_pseudo_s_1"),
        "13": TestCase(CaseType.OPEN, 1.5, "13_advance_pseudo_s_2"),
        "14": TestCase(CaseType.OPEN, 1.5, "14_advance_pseudo_t_1"),
        "15": TestCase(CaseType.OPEN, 1.5, "15_advance_pseudo_t_2"),
        "16": TestCase(CaseType.OPEN, 1.5, "16_advance_comment_1"),
        "17": TestCase(CaseType.OPEN, 1.5, "17_advance_comment_2"),
        "18": TestCase(CaseType.OPEN, 1.0, "18_advance_operator"),
        "19": TestCase(CaseType.OPEN, 1.0, "19_advance_zero"),
        "20": TestCase(CaseType.OPEN, 1.0, "20_advance_float"),
        "21": TestCase(CaseType.OPEN, 1.0, "21_advance_scientific"),
        "22": TestCase(CaseType.OPEN, 1.5, "22_advance_string_1"),
        "23": TestCase(CaseType.OPEN, 1.5, "23_advance_string_2"),
        "24": TestCase(CaseType.OPEN, 1.5, "24_advance_string_3"),
        "25": TestCase(CaseType.OPEN, 1.0, "25_advance_error_1"),
        "26": TestCase(CaseType.OPEN, 1.0, "26_advance_error_2"),
        "27": TestCase(CaseType.OPEN, 1.0, "27_advance_error_3"),
        "28": TestCase(CaseType.OPEN, 1.0, "28_advance_error_4"),
        "29": TestCase(CaseType.OPEN, 1.0, "29_advance_error_5"),
        "h1": TestCase(CaseType.HIDDEN, 2.5, "h01_keywords_1"),
        "h2": TestCase(CaseType.HIDDEN, 2.5, "h02_keywords_2"),
        "h3": TestCase(CaseType.HIDDEN, 2.5, "h03_delimiters"),
        "h4": TestCase(CaseType.HIDDEN, 2.5, "h04_arithmetic"),
        "h5": TestCase(CaseType.HIDDEN, 2.5, "h05_logical"),
        "h6": TestCase(CaseType.HIDDEN, 2.5, "h06_relational"),
        "h7": TestCase(CaseType.HIDDEN, 2.5, "h07_identifier"),
        "h8": TestCase(CaseType.HIDDEN, 2.5, "h08_constant"),
        "h9": TestCase(CaseType.HIDDEN, 2.5, "h09_scientific_string"),
        "h10": TestCase(CaseType.HIDDEN, 2.5, "h10_comment"),
        "h11": TestCase(CaseType.HIDDEN, 2.5, "h11_pseudocomment"),
        "h12": TestCase(CaseType.HIDDEN, 1.5, "h12_advance_pseudo_s_1"),
        "h13": TestCase(CaseType.HIDDEN, 1.5, "h13_advance_pseudo_s_2"),
        "h14": TestCase(CaseType.HIDDEN, 1.5, "h14_advance_pseudo_t_1"),
        "h15": TestCase(CaseType.HIDDEN, 1.5, "h15_advance_pseudo_t_2"),
        "h16": TestCase(CaseType.HIDDEN, 1.5, "h16_advance_comment_1"),
        "h17": TestCase(CaseType.HIDDEN, 1.5, "h17_advance_comment_2"),
        "h18": TestCase(CaseType.HIDDEN, 1.0, "h18_advance_operator"),
        "h19": TestCase(CaseType.HIDDEN, 1.0, "h19_advance_zero"),
        "h20": TestCase(CaseType.HIDDEN, 1.0, "h20_advance_float"),
        "h21": TestCase(CaseType.HIDDEN, 1.0, "h21_advance_scientific"),
        "h22": TestCase(CaseType.HIDDEN, 1.5, "h22_advance_string_1"),
        "h23": TestCase(CaseType.HIDDEN, 1.5, "h23_advance_string_2"),
        "h24": TestCase(CaseType.HIDDEN, 1.5, "h24_advance_string_3"),
        "h25": TestCase(CaseType.HIDDEN, 1.0, "h25_advance_error_1"),
        "h26": TestCase(CaseType.HIDDEN, 1.0, "h26_advance_error_2"),
        "h27": TestCase(CaseType.HIDDEN, 1.0, "h27_advance_error_3"),
        "h28": TestCase(CaseType.HIDDEN, 1.0, "h28_advance_error_4"),
        "h29": TestCase(CaseType.HIDDEN, 1.0, "h29_advance_error_5"),
        # Uncomment next line to add a new test case:
        # "my1": TestCase(CaseType.OPEN, 0.0, "my_test_case_1"),
    }

    def __init__(self, executable: Path) -> None:
        self.executable: Path = executable
        self.cases_to_run: list[TestCase] = list(self.CASES.values())
        self.diff_result: str = ""
        self.case_dir: Path = DIR / "test_cases"
        self.solution_dir: Path = DIR / "sample_solutions"
        self.output_dir: Path = DIR / "result"
        if not self.output_dir.exists():
            self.output_dir.mkdir()

    def find_index_of_last_digit_sequence(self, string: str) -> int:
        """Return `-1` if no digit sequence is found."""
        index: int = len(string)
        for c in reversed(string):
            if not c.isdigit():
                break
            index -= 1
        return index if index < len(string) else -1

    def set_case_id_to_run(self, input_case_id: str) -> None:
        case_id: str = input_case_id
        if case_id not in self.CASES:
            # fuzzy search the test case such as "01", "h01"; remove the prefix "0" of the digit sequence.
            number_at: int = self.find_index_of_last_digit_sequence(input_case_id)
            if number_at != -1:
                prefix: str = input_case_id[:number_at]
                number: int = int(input_case_id[number_at:])
                case_id = f"{prefix}{number}"
        if case_id in self.CASES:
            self.cases_to_run = [self.CASES[case_id]]
        else:
            print(f"ERROR: Invalid case ID {input_case_id}")
            exit(1)

    def execute_process(self, command: List[str]) -> tuple[int, bytes, bytes]:
        """Returns the exit code, stdout, and stderr of the process."""
        try:
            process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except Exception as e:
            print(f"Call of '{' '.join(command)}' failed: {e}")
            return 1, b"", b""

        exit_code: int = process.wait()
        assert process.stdout is not None and process.stderr is not None
        stdout: bytes = process.stdout.read()
        stderr: bytes = process.stderr.read()
        return exit_code, stdout, stderr

    def run_test_case(self, case: TestCase) -> TestStatus:
        """Runs the test case and outputs the diff between the result and the solution."""
        case_path: Path = self.case_dir / f"{case.name}.p"
        solution_path: Path = self.solution_dir / f"{case.name}"
        output_path: Path = self.output_dir / f"{case.name}"

        if not case_path.exists():
            return TestStatus.SKIP

        command: List[str] = [str(self.executable), str(case_path)]
        stdout: bytes
        stderr: bytes
        _, stdout, stderr = self.execute_process(command)
        with output_path.open("wb") as file:
            file.write(stdout)
            file.write(stderr)

        diff_command: List[str] = ["diff", "-u", str(output_path), str(solution_path), f"--label=your output:({output_path})", f"--label=answer:({solution_path})"]
        diff_exit_code: int
        diff_stdout: bytes
        diff_exit_code, diff_stdout, _ = self.execute_process(diff_command)
        diff_result: str = diff_stdout.decode("utf-8", errors="replace")
        if diff_exit_code != 0:
            # The header part.
            self.diff_result += f"{case.name}\n"
            # The diff part.
            if not solution_path.exists():
                self.diff_result += "// Solution file not found.\n"
            elif case.type == CaseType.HIDDEN:
                self.diff_result += "// Diff of hidden cases is not shown.\n"
            else:
                self.diff_result += f"{diff_result}\n"
        return TestStatus.PASS if diff_exit_code == 0 else TestStatus.FAIL

    def run(self) -> int:
        total_score: float = 0
        max_score: float = 0
        had_passed_all_visible_cases: bool = True

        print("---\tCase\t\tPoints")
        for case in self.cases_to_run:
            print(f"+++ TESTING {case.type.name.lower()} case {case.name}:")
            status: TestStatus = self.run_test_case(case)
            score: float = 0
            if status is TestStatus.PASS:
                score = case.score
            elif status is TestStatus.FAIL:
                had_passed_all_visible_cases = False

            self.set_text_color(status)
            if status is TestStatus.SKIP:
                print(f"---\t{case.name}\tSKIPPED\t0/{case.score}")
            else:
                print(f"---\t{case.name}\t{score}/{case.score}")
            self.reset_text_color()
            total_score += score
            max_score += case.score

        self.set_text_color(TestStatus.PASS if had_passed_all_visible_cases else TestStatus.FAIL)
        print(f"---\tTOTAL\t\t{total_score}/{max_score}")
        self.reset_text_color()

        with (self.output_dir / "score.txt").open("w") as score_file:
            score_file.write(f"---\tTOTAL\t\t{total_score}/{max_score}")
        with (self.output_dir / "diff.txt").open("w") as diff_file:
            diff_file.write(self.diff_result)

        # NOTE: Return 1 on test failure to support GitHub CI; otherwise, such CI never fails.
        return 0 if had_passed_all_visible_cases else 1

    @staticmethod
    def set_text_color(test_status: TestStatus) -> None:
        """Sets the color based on whether the test has passed or not."""
        if test_status is TestStatus.PASS:
            color = colorama.Fore.GREEN
        elif test_status is TestStatus.FAIL:
            color = colorama.Fore.RED
        else:
            color = colorama.Fore.YELLOW
        print(color, end='')

    @staticmethod
    def reset_text_color() -> None:
        print(colorama.Style.RESET_ALL, end='')


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", help="executable to grade", type=Path, default=DIR.parent / "src" / "scanner")
    parser.add_argument("--case_id", help="test case's ID", type=str)
    args = parser.parse_args()

    grader = Grader(args.executable)
    if args.case_id is not None:
        grader.set_case_id_to_run(args.case_id)
    return grader.run()


if __name__ == "__main__":
    sys.exit(main())
