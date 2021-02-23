import socket
import select
import ssl
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
    docker_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    docker_socket.connect(("localhost", docker_port))

    # Transmit data between client and docker
    try:
        while True:
            r, _, _ = select.select([docker_socket, client], [], [])
            for sender in r:
                receiver = docker_socket if sender is client else client
                message = sender.recv(settings.MESSAGE_SIZE)
                logging.warning(message[:40])
                logging.warning(f"Moving message from {sender} to {receiver}")
                receiver.send(message)
    except (socket.error, ssl.SSLError) as e:
        # Error encountered - Send Error Message
        error_text = bytes(e.args[1], "ascii")
        error_message_dict = dict(
            type=messages.MessageType.ERROR,
            error_code=settings.ErrorCodes.COMMUNICATION_ERROR,
            error_message_size=len(error_text),
            error_message=error_text,
        )
        logging.warning(f"Client Error: {error_text}")
        message = messages.ERROR_MESSAGE.build(error_message_dict)
        client.send(message)

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
        import ipdb; ipdb.set_trace()
        client_socket.send(message)
    finally:
        # Clean all resources
        client_socket.close()
        container.stop()
