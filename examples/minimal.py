import json
import time
from urllib.parse import quote

import webview


HTML = '''
This is a test<br/>
<div id="timestr">Time should tick here</div><br/>
<button onclick="quit();">Quit</button>
<script type="text/javascript">
    timediv = document.getElementById('timestr');
    setInterval(function() {
        get_time('%Y-%m-%d %H:%M:%S').then((s) => {
            timediv.innerHTML = s;
        });
    }, 1000);
</script>
'''


def get_time(w, req):
    fmt = json.loads(req)[0]
    return json.dumps(time.strftime(fmt))

def quit(w, req):
    w.terminate()


def main():
    w = webview.WebView(width=320, height=240, resizable=True, title="My App", debug=True)
    w.bind('get_time', get_time)
    w.bind('quit', quit)
    w.navigate('data:text/html,' + quote(HTML))
    w.run()


if __name__ == "__main__":
    main()
