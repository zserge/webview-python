#!/usr/bin/python3
import os
import platform
import string
import sys
import sysconfig
from distutils.util import get_host_platform

sep = os.path.sep.replace('\\', '\\\\')
webview_platform = {'Linux': 'GTK', 'Darwin': 'COCOA', 'Windows': 'WINAPI'}.get(platform.system(), 'GTK')
python_include_dir = sysconfig.get_path('include').replace('\\', '\\\\')
py_major_version = sys.version_info[0]
built_module_dir = sep.join([
    '${workspaceFolder}',
    'build',
    'lib.{}-{}'.format(
        get_host_platform(),
        sysconfig.get_config_var('py_version_short')
    )
])

with open('settings.json.template') as f:
    settings = string.Template(f.read()).safe_substitute(locals())

with open('settings.json', 'w') as f:
    f.write(settings)
