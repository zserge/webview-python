import platform
import subprocess

from setuptools import Extension, setup


OSNAME = platform.system()

if OSNAME == 'Linux':
    def pkgconfig(flags):
        return subprocess.check_output(
            'pkg-config {} gtk+-3.0 webkit2gtk-4.0'.format(flags),
            shell=True,
            stderr=subprocess.STDOUT).decode()

    define_macros = [("WEBVIEW_GTK", '1')]
    extra_cflags = pkgconfig("--cflags").split()
    extra_ldflags = pkgconfig("--libs").split()

elif OSNAME == 'Darwin':
    define_macros = [('WEBVIEW_COCOA', '1')]
    extra_cflags = ['-std=c++11']
    extra_ldflags = ['-framework', 'WebKit']

elif OSNAME == 'Windows':
    define_macros = [('WEBVIEW_WINAPI', '1')]
    extra_cflags = ['/std:c++17', '/Iwebview', '/Iwebview\\script']
    extra_ldflags = [R'webview\script\microsoft.web.webview2.1.0.664.37\build\native\x64\WebView2Loader.dll.lib']


webview = Extension(
    'webview',
    sources=['webview.cpp'],
    define_macros=define_macros,
    extra_compile_args=extra_cflags,
    extra_link_args=extra_ldflags,
)

setup(
    name='webview',
    version='0.1.5',
    description='Python WebView bindings',
    author='Serge Zaitsev',
    author_email='zaitsev.serge@gmail.com',
    url='https://github.com/zserge/webview',
    keywords=[],
    license='MIT',
    classifiers=[
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: C',
        'Programming Language :: Python',
        'Topic :: Desktop Environment',
        'Topic :: Software Development :: Libraries',
        'Topic :: Software Development :: User Interfaces',
    ],
    ext_modules=[webview],
    py_modules=['pywebview'],
    entry_points=dict(
        gui_scripts=[
            'pywv = pywebview:main',
        ],
    ),
    extras_require=dict(
        dev=[
            'pip',
            'setuptools',
            'wheel',
        ],
    ),
)
