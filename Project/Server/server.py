import logging
import socket
import multiprocessing
import ssl

CERTFILE = "Certificates/server.crt"
KEYFILE = "Certificates/server.key"
PORT = 12344
MAX_LISTENERS = 10

class Server(object):
    def __init__(self):
        self.pool = multiprocessing.Pool()
        self.server = None
        self.running = False

    def start_server(self):
        # Start server
        self.server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server.bind(("0.0.0.0", PORT))
        self.server.listen(MAX_LISTENERS)

        # Set server to running
        self.running = True

    def serve_forever(self, client_handler, *args, **kwargs):
        while True:
            try:
                conn, addr = self.server.accept()
            except (OSError, ConnectionError) as e:
                logging.error("Failed to accept a client: {}".format(e.args))
            else:
                try:
                    process = self.pool.Process(target=client_handler, args=(conn, addr) + args, kwargs=kwargs)
                except TypeError:  # In python 3.8+ The Process function requires additional parameter called `ctx`
                    ctx = multiprocessing.get_context("fork")
                    process = self.pool.Process(ctx, target=client_handler, args=(conn, addr) + args, kwargs=kwargs)
                process.start()

    def close(self):
        self.pool.close()
        self.__del__()

    def __del__(self):
        self.pool.terminate()
        self.pool.join()
