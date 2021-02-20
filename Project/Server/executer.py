import socket
import select
import ssl
import contextlib
import wrapt_timeout_decorator


from docker_files import settings
from docker_files import messages
import docker_runner

CLIENT_TIMEOUT = 60  # Seconds

def find_free_port():
    with contextlib.closing(socket.socket(socket.AF_INET, socket.SOCK_STREAM)) as s:
        s.bind(('', 0))
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        return s.getsockname()[1]


@wrapt_timeout_decorator.timeout(CLIENT_TIMEOUT)
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
                receiver.send(message)
    except (socket.error, ssl.SSLError) as e:
        # Error encountered - Send Error Message
        error_text = e.args[0]
        error_message_dict = dict(
            type=messages.MessageType.ERROR,
            error_code=settings.ErrorCodes.COMMUNICATION_ERROR,
            error_message_size=len(error_text),
            error_message=error_text,
        )
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

    except TimeoutError as e:
        # Timeout occurred - Send Error Message
        error_text = f"The maximum execution time of {CLIENT_TIMEOUT} seconds has passed!"
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
        container.stop()
        client_socket.shutdown()
        client_socket.close()
