#! /usr/bin/env python3

# This dockerfile is intended to be used by students
from datetime import datetime
from pathlib import Path
import argparse
import os
import subprocess
import sys

try:
    assert sys.version_info >= (3, 6)
except AssertionError:
    print("Requires at least python 3.6")


def get_course_name():
    SPRING_START_MONTH = 2
    FALL_START_MONTH = 8
    today = datetime.today()
    semester = "s" if today.month in range(SPRING_START_MONTH, FALL_START_MONTH) else "f"
    year = (
        today.replace(year=today.year - 1) if today.year < SPRING_START_MONTH else today
    ).strftime("%y")
    return f"compiler-{semester}{year}"


COURSE_NAME = get_course_name()

parser = argparse.ArgumentParser(
    description=f'Activate homework environment for {COURSE_NAME}')
parser.add_argument('-t', '--test-src-fld',
                    nargs=1,
                    type=Path,
                    default=[None],
                    help="Append different test folder")
parser.add_argument('-u', '--username', default="student",)
parser.add_argument('--hostname', default=f'{COURSE_NAME}')
parser.add_argument('--homedir', type=Path, default='/home/student')
parser.add_argument('--binddir', type=Path, default='/home/student/hw')
parser.add_argument('-i', '--imagename', default='compiler-s20-env')
parser.add_argument('command', default=[], nargs=argparse.REMAINDER, help="command to run in docker")

args = parser.parse_args()
test_src_fld = args.test_src_fld[0]
DOCKER_USER_NAME = args.username
DOCKER_HOST_NAME = args.hostname
DOCKER_IMG_NAME = args.imagename
binddir = args.binddir
dk_home = args.homedir

DOCKER_CMD = args.command

dirpath = Path(__file__).parent.resolve()

recursive_docker_msg = '''
    .
    .
   . ;.
    .;
     ;;.
   ;.;;
   ;;;;.
   ;;;;;
   ;;;;;
   ;;;;;
   ;;;;;    Don't activate environment twice QQ
   ;;;;;
 ..;;;;;..  You are already inside our docker environment, see?
  ':::::'
    ':`                              - SSLAB @NYCUCS
'''


def main():
    if "STATUS_DOCKER_ACTIVATED" in os.environ:
        print(recursive_docker_msg)
        sys.exit(0)

    bash_his = dirpath / '.history' / 'docker_bash_history'
    bash_his.parent.mkdir(exist_ok=True)
    bash_his.touch(exist_ok=True)

    if test_src_fld and not test_src_fld.exists():
        raise FileNotFoundError(f"Folder: `{test_src_fld}` doesn't exist.")

    docker_options = [
        'docker', 'run',
        '--rm',
        '--hostname', DOCKER_HOST_NAME,
        '--cap-add=SYS_PTRACE',
        '-u', f'{os.getuid()}:{os.getgid()}',
        '-e', f'HOME={dk_home}',
        '-e', f'USER={DOCKER_USER_NAME}',
        # Since memory leaks do not affect the program's behavior, we are disabling their
        # detection to reduce the burden on students.
        '-e', 'ASAN_OPTIONS=detect_leaks=0',
        '-v', f'{os.getcwd()}:{binddir}',
        '-w', f'{binddir}',

        # bash history file
        '-v', f'{bash_his}:/{dk_home}/.bash_history',
    ]
    if test_src_fld:
        docker_options.extend(['-v', f'{test_src_fld.resolve()}:{dk_home}/test'])
    if not DOCKER_CMD:
        docker_options.append('-it')
    else:
        docker_options.extend(['-a', 'stderr'])
        docker_options.extend(['-a', 'stdout'])

    cmd_list = docker_options + [DOCKER_IMG_NAME] + DOCKER_CMD
    return subprocess.run(cmd_list).returncode


if __name__ == "__main__":
    exit_status = main()
    sys.exit(exit_status)
