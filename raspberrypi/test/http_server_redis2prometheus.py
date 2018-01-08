import http.server
import redis

REDIS_HOST = 'localhost'
REDIS_PORT = 6379
REDIS_DB = 0

sr = redis.StrictRedis(host=REDIS_HOST, port=REDIS_PORT, db=REDIS_DB)

class IOTHandler(http.server.BaseHTTPRequestHandler):
    def _set_headers(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.end_headers()
        print('headers set')

    def do_GET(self):
        self._set_headers()
        val = sr.get('S01'); 
        self.wfile.write(bytes('connectory_light_value %f' % float(val), 'utf-8'))

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
