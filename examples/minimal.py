from urllib.parse import quote

import webview


HTML = '''
This is a test<br><button onclick="window.external.invoke(\'print_hello\');">Click me</button>
'''


def callback(arg):
    print('callback was called with {!r}'.format(arg))


def main():
    url = 'data:text/html,' + quote(HTML)
    print('url =', url)
    w = webview.WebView(width=320, height=240, title="My App", url=url, resizable=True, debug=True)
    w.callback = callback
    while w.loop(True):
        pass


if __name__ == "__main__":
    main()
