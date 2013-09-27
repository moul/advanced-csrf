"""Advanced CSRF.

For fun and profit.

"""

__version__ = "0.1"

__all__ = [ "Acsrf", "main" ]

import urllib
import BaseHTTPServer

class AcsrfRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
    pass

class Acsrf:
    def __init__(self, port = 8000, background = False):
        self.port = port
        self.background = background

    def run(self):
        handler = SimpleHTTPRequestHandler
        handler.protocol_version = 'HTTP/1.0'
        httpd = BaseHTTPServer.HTTPServer(server_address, handler)
        BaseHTTPServer.test

def main():
    acsrf = Acsrf()
    acsrf.run()

if __name__ == "__main__":
    main()
