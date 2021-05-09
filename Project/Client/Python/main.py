import sys
import os
import enum
import socket
import select
import ssl

sys.path.append("../../Server/")
sys.path.append("../../Server/docker_files")
import settings
import messages

# CA_CERTFILE = "../../Server/Certificates/ca.crt"

class ReturnCodes(enum.IntEnum):
    SUCCESS =       0
    UNKNOWN_ERROR = -1
    INVALID_USAGE = -2

def connect_to_server(ip, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH, cafile=CA_CERTFILE)
    context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
    context.options |= ssl.OP_NO_SSLv2 | ssl.OP_NO_SSLv3 | \
                       ssl.OP_NO_TLSv1 | ssl.OP_NO_TLSv1_1 | ssl.OP_NO_TLSv1_2

    client = context.wrap_socket(s)
    client.connect((ip, port))

    return client

def send_message(sender, message):
    if type(message) is str:
        message = bytes(message, "ascii")
    data_message_dict = dict(
        type=messages.MessageType.DATA,
        chunk_size=len(message),
        chunk=message,
    )
    message = messages.DATA_MESSAGE.build(data_message_dict)
    sender.send(message)

def transfer_file(client, python_file):
    filename = os.path.basename(python_file)
    file_size = os.stat(python_file).st_size

    # Send TRANSFER_FILE message
    file_transfer_message_dict = dict(
        type=messages.MessageType.TRANSFER_FILE,
        file_size=file_size,
        filename_size=len(filename),
        filename=bytes(filename, "ascii"),
    )
    message = messages.TRANSFER_FILE_MESSAGE.build(file_transfer_message_dict)
    client.send(message)

    # Send file data
    total_size = 0
    chunk_size = settings.MESSAGE_SIZE - 5
    with open(python_file, "rb") as file:
        while total_size < file_size:
            chunk = file.read(chunk_size)
            total_size += len(chunk)

            send_message(client, chunk)

    message = client.recv(settings.MESSAGE_SIZE)
    if len(message) != settings.MESSAGE_SIZE:
        raise ValueError(f"got message of length {len(message)}")

    generic_message = messages.GENERIC_MESSAGE.parse(message)
    if generic_message.type == messages.MessageType.ERROR:
        error_message = messages.ERROR_MESSAGE.parse(message)
        raise Exception(f"Transfer failed with {error_message.error_code}: {error_message.error_message}")
    elif int(generic_message.type) != messages.MessageType.DONE_TRANSFER:
        print(generic_message)
        return False

    message = messages.DONE_TRANSFER_MESSAGE.parse(message)
    if message.return_code != 0:
        raise ValueError(f"got return code of {message.return_code}")

    return True


def communicate_with_script(client):
    message = client.recv(settings.MESSAGE_SIZE)
    if len(message) != settings.MESSAGE_SIZE:
        raise ValueError(f"got message of length {len(message)}")

    message = messages.TRANSFER_EXECUTION_RESULTS_MESSAGE.parse(message)

    maximum_data_size = settings.MESSAGE_SIZE - 5
    while True:
        r, _, _ = select.select([client, sys.stdin], [], [])
        if sys.stdin in r:
            user_input = sys.stdin.readline(maximum_data_size)
            send_message(client, user_input)
        if client in r:
            message = client.recv(settings.MESSAGE_SIZE)
            final_message = messages.GENERIC_MESSAGE.parse(message)
            if int(final_message.type) == messages.MessageType.DONE_TRANSFER or int(final_message.type) == messages.MessageType.ERROR:
                break
            data_message = messages.DATA_MESSAGE.parse(message)
            print(data_message.chunk.decode("utf-8"), end="")

    if int(final_message.type) == messages.MessageType.ERROR:
        error_message = messages.ERROR_MESSAGE.parse(message)
        raise Exception(f"Execution failed with {hex(error_message.error_code)}: {error_message.error_message.decode('ascii')}")

    done_message = messages.DONE_TRANSFER_MESSAGE.parse(message)
    print(f"Execution finished with code {done_message.return_code}")


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <IP> <Port> <Python File>")
        return ReturnCodes.INVALID_USAGE

    ip = sys.argv[1]
    port = int(sys.argv[2])
    python_file = sys.argv[3]

    client = connect_to_server(ip, port)
    if not transfer_file(client, python_file):
        raise Exception("Transfer failed!")
    communicate_with_script(client)
    return 0


if __name__ == '__main__':
    sys.exit(main())
