#from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import http.server


class IOTHandler(http.server.BaseHTTPRequestHandler):
    def _set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        print('headers set')

    def do_GET(self):
        self._set_headers()
        self.wfile.write(bytes('test_stat_uno 1.0', 'utf-8'))

    def do_HEAD(self):
        self._set_headers()

def run(server_class=http.server.HTTPServer, handler_class=IOTHandler, port=80):
    server_address = ('localhost', port)
    httpd = server_class(server_address, handler_class)
    print('Starting httpd... port ', port)
    httpd.serve_forever()

if __name__ == '__main__':
    from sys import argv

    if len(argv) == 2:
        run(port=int(argv[1]))
    else:
        run()
