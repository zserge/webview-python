import argparse
import os

import webview


def main():
    p = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    p.add_argument('url')
    p.add_argument('-s', '--size', default='800x600', help='Window size in the form <WIDTH>x<HEIGHT>')
    p.add_argument('-f', '--fixed-size', action='store_true', help='Makes the window non-resizable')
    p.add_argument('-t', '--title', default='Webview', help='Set the window title')
    args = p.parse_args()

    try:
        width, height = map(int, args.size.split('x'))
    except Exception:
        p.error('Size must be of the form <WIDTH>x<HEIGHT>')

    if not os.path.exists(args.url) and '://' not in args.url:
        args.url = 'http://' + args.url

    w = webview.WebView(width, height, resizable=not args.fixed_size, url=args.url, title=args.title)
    w.run()


if __name__ == "__main__":
    main()
