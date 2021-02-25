import socket
import select
import ssl
import time
import contextlib
import logging
import timeout_decorator


from docker_files import settings
from docker_files import messages
import docker_runner

CLIENT_TIMEOUT = 60  # Seconds

def find_free_port():
    with contextlib.closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as s:
        s.bind(('', 0))
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        return s.getsockname()[1]

@timeout_decorator.timeout(CLIENT_TIMEOUT)
def handle_client(client, docker_port):
    # Connects to docker socket
    while True:
        time.sleep(1)
        docker_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            docker_socket.connect(("localhost", docker_port))
            docker_socket.send(settings.SYNC_MESSAGE)
            break
        except socket.error as e:
            pass

    # Transmit data between client and docker
    try:
        connected = True
        while connected:
            r, _, _ = select.select([client, docker_socket], [], [])
            for sender in r:
                receiver = docker_socket if sender is client else client
                message = sender.recv(settings.MESSAGE_SIZE)
                if len(message) == 0:
                    connected = False
                    break
                receiver.send(message)
    except (socket.error, ssl.SSLError) as e:
        # Error encountered - Send Error Message
        if len(e.args) > 1:
            error_text = bytes(e.args[1], "ascii")
        else:
            error_text = b""
        error_message_dict = dict(
            type=messages.MessageType.ERROR,
            error_code=settings.ErrorCodes.COMMUNICATION_ERROR,
            error_message_size=len(error_text),
            error_message=error_text,
        )
        logging.warning(f"Client Error: {error_text}")
        message = messages.ERROR_MESSAGE.build(error_message_dict)
        client.send(message)
    finally:
        client.close()
        docker_socket.close()

def client_handler(client_socket, address, docker_image):
    # Get a random available port
    docker_port = find_free_port()

    # Start the container
    container = docker_runner.run_container(docker_image, docker_port)
    container.start()

    try:
        # Run the client handler
        handle_client(client_socket, docker_port)

    except timeout_decorator.TimeoutError as e:
        # Timeout occurred - Send Error Message
        logging.warning("Timeout passed!")
        error_text = bytes(f"The maximum execution time of {CLIENT_TIMEOUT} seconds has passed!", "ascii")
        error_message_dict = dict(
            type=messages.MessageType.ERROR,
            error_code=settings.ErrorCodes.TIMED_OUT,
            error_message_size=len(error_text),
            error_message=error_text,
        )
        message = messages.ERROR_MESSAGE.build(error_message_dict)
        client_socket.send(message)
    finally:
        # Clean all resources
        client_socket.close()
        container.stop()
