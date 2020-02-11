import random
import threading
import time
from http.server import BaseHTTPRequestHandler, HTTPServer, HTTPStatus
from urllib.parse import quote

import webview


class MyHTTPRequestHandler(BaseHTTPRequestHandler):
    def send_data(self, data, status=HTTPStatus.OK, mimetype='text/plain'):
        if isinstance(data, str):
            data = data.encode('utf8')
        self.send_response(status)
        self.send_header('Content-Type', mimetype)
        self.send_header('Content-Length', len(data))
        self.end_headers()
        self.wfile.write(data)

    def send_html(self, text):
        self.send_data(text.encode('utf8'), mimetype='text/html')

    def do_GET(self):
        self.send_html('This is a test<br><button onclick="window.external.invoke(\'print_hello\');">Click me</button>')


class CooperativeHTTPServer(HTTPServer):
    timeout = 0.001


def run_server():
    while 1:
        port = random.randint(20000, 30000)
        try:
            server = CooperativeHTTPServer(('localhost', port), MyHTTPRequestHandler)
        except Exception:
            continue
        break
    threading.Thread(target=server.serve_forever, daemon=True).start()
    return server


def callback(arg):
    print('callback was called with {!r}'.format(arg))


HTML = '''
This is a test<br><button onclick="window.external.invoke(\'print_hello\');">Click me</button>
'''


def main():
    # s = run_server()
    # url = "http://127.0.0.1:{}".format(s.server_port)
    url = 'data:text/html,' + quote(HTML)
    print('url =', url)
    w = webview.WebView(width=320, height=240, title="My App", url=url, resizable=True, debug=True)
    w.callback = callback
    while w.loop(True):
        pass
    # s.shutdown()


if __name__ == "__main__":
    main()
