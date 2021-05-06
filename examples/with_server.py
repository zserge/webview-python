import json
import random
import threading
import time
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import quote
from itertools import count

import webview


HTML = '''
This is a test<br><button onclick="invoke('print_hello');">Click me</button>
'''


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
        self.send_html(HTML)


def run_server():
    while 1:
        port = random.randint(20000, 30000)
        try:
            server = HTTPServer(('localhost', port), MyHTTPRequestHandler)
        except Exception:
            continue
        break
    threading.Thread(target=server.serve_forever, daemon=True).start()
    return server


def callback(w, req):
    # print(f'callback was called with {req!r}')
    arg = json.loads(req)[0]
    print(f'callback was called with {arg!r}')


def threadfunc():
    for n in count():
        print(n)
        time.sleep(0.1)


def main():
    s = run_server()
    url = f'http://127.0.0.1:{s.server_port}'
    print(url)
    w = webview.WebView(width=320, height=240, title="My App", resizable=True, debug=True)
    w.bind('invoke', callback)
    w.navigate(url)
    w.run()
    s.shutdown()


if __name__ == "__main__":
    main()
