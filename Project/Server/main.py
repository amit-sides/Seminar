import sys
import logging

sys.path.append("docker_files")
import server
import executer
import docker_runner

def configure_logging():
    logging.basicConfig(level=logging.INFO, datefmt="%d.%m.%Y %H:%M:%S",
                        format="%(asctime)-19s [%(levelname)-8s] %(funcName)-21s | %(message)s")

def main():
    configure_logging()

    # Build docker image
    logging.info("Building Docker Image...")
    docker_image = docker_runner.build_image()
    logging.info("Docker Image built!")

    # Start the server
    logging.info(f"Starting server... port: {server.PORT}")
    tls_server = server.Server()
    tls_server.start_server()
    try:
        tls_server.serve_forever(executer.client_handler, docker_image)
    except KeyboardInterrupt:
        tls_server.close()
    return 0


if __name__ == '__main__':
    sys.exit(main())
