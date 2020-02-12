#!/usr/bin/python3
import os
import platform
import string
import sys
import sysconfig

webview_platform = {'Linux': 'GTK', 'Darwin': 'COCOA', 'Windows': 'WINAPI'}.get(platform.system(), 'GTK')
python_include_dir = sysconfig.get_path('include').replace('\\', '\\\\')
py_major_version = sys.version_info[0]

with open('settings.json.template') as f:
    settings = string.Template(f.read()).safe_substitute(locals())

with open('settings.json', 'w') as f:
    f.write(settings)
