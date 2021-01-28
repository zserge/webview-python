import json
import time
from urllib.parse import quote

import webview


HTML = '''
This is a test</br>
<div id="timestr"></div></br>
<button onclick="invoke('print_hello');">Click me</button></br>
<button onclick="invoke('quit');">Quit</button>
<script type="text/javascript">
    timediv = document.getElementById('timestr');
    setInterval(function() {
        invoke('getTime');
    }, 1000);
    window.setTime = function(t) {
        timediv.innerHTML = t;
    };
</script>
'''


def callback(w, seq, req):
    print(f'callback was called with {seq!r} {req!r}')
    arg = json.loads(req)[0]
    print('callback was called with {!r}'.format(arg))
    if arg == 'getTime':
        w.eval('setTime({!r})'.format(time.time()))
    elif arg == 'quit':
        w.terminate()


def main():
    url = 'data:text/html,' + quote(HTML)
    print('url =', url)
    w = webview.WebView(width=320, height=240, title="My App", url=url, resizable=True, debug=True)
    w.callback = callback
    w.run()


if __name__ == "__main__":
    main()
