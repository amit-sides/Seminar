import logging
import socket
import multiprocessing
import ssl

CERTFILE = "Certificates/server.crt"
KEYFILE = "Certificates/server.key"
PORT = 12344
MAX_LISTENERS = 10

class TLSServer(object):
    def __init__(self):
        self.context = ssl.SSLContext(ssl.PROTOCOL_TLS)
        self.context.load_cert_chain(certfile=CERTFILE, keyfile=KEYFILE)
        self.context.options |= ssl.OP_NO_SSLv2 | ssl.OP_NO_SSLv3 | \
                                ssl.OP_NO_TLSv1 | ssl.OP_NO_TLSv1_1 | ssl.OP_NO_TLSv1_2

        self.socket = None
        self.server = None
        self.pool = multiprocessing.Pool()
        self.running = False

    def start_server(self):
        # Start TLS server
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind(("0.0.0.0", PORT))
        self.socket.listen(MAX_LISTENERS)
        self.server = self.context.wrap_socket(self.socket, server_side=True)

        # Set server to running
        self.running = True

    def serve_forever(self, client_handler, *args, **kwargs):
        while True:
            try:
                conn, addr = self.server.accept()
            except ssl.SSLError as e:
                logging.error("Failed to accept a client: " + e.reason)
            else:
                process = self.pool.Process(target=client_handler, args=(conn, addr) + args, kwargs=kwargs)
                process.start()

    def close(self):
        self.pool.close()
        self.__del__()

    def __del__(self):
        self.pool.terminate()
        self.pool.join()
