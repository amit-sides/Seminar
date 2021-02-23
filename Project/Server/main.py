import sys
import logging

sys.path.append("docker_files")
import server
import executer
import docker_runner

def main():
    # Build docker image
    logging.info("Building Docker Image...")
    docker_image = docker_runner.build_image()
    logging.info("Docker Image built!")

    # Start the server
    logging.info("Starting server...")
    tls_server = server.TLSServer()
    tls_server.start_server()
    tls_server.serve_forever(executer.client_handler, docker_image)
    return 0


if __name__ == '__main__':
    sys.exit(main())
