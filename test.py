import os
import runpy
import subprocess
import sys


def dirname(path):
    return os.path.abspath(os.path.dirname(path))


def main():
    base_dir = dirname(__file__)

    # Build the package (if necessary)
    # subprocess.check_call([sys.executable, 'setup.py', 'build_ext', '--inplace'], cwd=base_dir)

    # Run code
    if len(sys.argv) > 1:
        fn = sys.argv[1]
    else:
        fn = os.path.join(base_dir, 'examples', 'minimal.py')

    os.chdir(base_dir)
    runpy.run_path(fn, run_name='__main__')


if __name__ == "__main__":
    main()
