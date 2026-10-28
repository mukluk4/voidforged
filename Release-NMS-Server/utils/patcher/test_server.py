"""Serve a publish folder for testing VoidPatcher, with HTTP Range support like a real CDN.

    python test_server.py C:\\NMS\\VoidPatch\\publish [--port 8088] [--drop 0.2]

--drop cuts that share of object downloads partway through, to test that the patcher resumes them.
Point the patcher at it with a VoidPatcher.ini next to the exe:  url=http://127.0.0.1:8088/
"""
import argparse
import http.server
import os
import random
import re
import socketserver
import urllib.parse

CHUNK = 1 << 20


class Handler(http.server.BaseHTTPRequestHandler):
    root = '.'
    drop = 0.0
    protocol_version = 'HTTP/1.1'

    def do_GET(self):
        rel = urllib.parse.unquote(urllib.parse.urlsplit(self.path).path).lstrip('/')
        full = os.path.realpath(os.path.join(self.root, rel))
        if not full.startswith(os.path.realpath(self.root) + os.sep) or not os.path.isfile(full):
            self.send_error(404)
            return

        size = os.path.getsize(full)
        start, end = 0, size - 1
        m = re.fullmatch(r'bytes=(\d+)-(\d*)', self.headers.get('Range', ''))
        if m:
            start = int(m.group(1))
            end = min(int(m.group(2)), size - 1) if m.group(2) else size - 1
            if start >= size:
                self.send_response(416)
                self.send_header('Content-Range', f'bytes */{size}')
                self.send_header('Content-Length', '0')
                self.end_headers()
                return
            self.send_response(206)
            self.send_header('Content-Range', f'bytes {start}-{end}/{size}')
        else:
            self.send_response(200)
        length = end - start + 1
        self.send_header('Content-Length', str(length))
        self.send_header('Accept-Ranges', 'bytes')
        self.send_header('Content-Type', 'application/octet-stream')
        self.end_headers()

        cut = length // 2 if rel.startswith('objects/') and length > CHUNK and random.random() < self.drop else None
        with open(full, 'rb') as f:
            f.seek(start)
            sent = 0
            while sent < length:
                n = min(CHUNK, length - sent)
                if cut is not None and sent + n > cut:
                    self.wfile.write(f.read(cut - sent))
                    self.close_connection = True
                    return
                self.wfile.write(f.read(n))
                sent += n

    def log_message(self, fmt, *args):
        pass


class Server(socketserver.ThreadingMixIn, http.server.HTTPServer):
    daemon_threads = True
    request_queue_size = 64


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('root')
    ap.add_argument('--port', type=int, default=8088)
    ap.add_argument('--drop', type=float, default=0.0)
    args = ap.parse_args()
    Handler.root = os.path.abspath(args.root)
    Handler.drop = args.drop
    print(f'serving {Handler.root} on http://127.0.0.1:{args.port}/ (drop {args.drop})', flush=True)
    Server(('127.0.0.1', args.port), Handler).serve_forever()


if __name__ == '__main__':
    main()
