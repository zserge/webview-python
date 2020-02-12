import distutils.util
import os
import runpy
import subprocess
import sys
import sysconfig


def main():
    base_dir = os.path.abspath(os.path.dirname(__file__))

    # Build the package (if necessary)
    subprocess.check_call(
        [sys.executable, 'setup.py', 'build'],
        cwd=base_dir,
    )

    # Make sure we import the built C extension (and not a properly installed one, if it exists)
    module_dir = os.path.join(
        base_dir,
        'build',
        'lib.{}-{}'.format(
            distutils.util.get_host_platform(),
            sysconfig.get_config_var('py_version_short'),
        )
    )
    sys.path.insert(0, module_dir)

    # Run examples/minimal.py
    runpy.run_path(
        os.path.join(
            base_dir,
            'examples',
            'minimal.py',
        ),
        run_name='__main__',
    )


if __name__ == "__main__":
    main()
