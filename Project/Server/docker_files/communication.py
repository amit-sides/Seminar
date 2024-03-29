import os
import socket
import select

import settings
import messages

SELECT_TIMEOUT = 1

class DockerSocket(object):
    def __init__(self):
        self.host = None

    def connect_to_host(self):
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind(("0.0.0.0", settings.DOCKER_EXPOSED_PORT))

        server.listen(1)

        self.host, _ = server.accept()

    def recv_message(self, message_type):
        message = self.host.recv(settings.MESSAGE_SIZE)

        if len(message) != message_type.sizeof():
            raise socket.error("Invalid message size!")

        message = message_type.parse(message)
        return message

    def send_message(self, message):
        if type(message) is str:
            message = bytes(message, "ascii")
        data_message_dict = dict(
            type=messages.MessageType.DATA,
            chunk_size=len(message),
            chunk=message,
        )
        message = messages.DATA_MESSAGE.build(data_message_dict)
        self.host.send(message)

    def get_script(self, location):
        # Get transfer message to start script transfer
        transfer_message = self.recv_message(messages.TRANSFER_FILE_MESSAGE)
        total_size = 0

        # Get script file with data messages
        script_path = os.path.join(location, transfer_message.filename.decode("ascii"))
        with open(script_path, "wb") as script:
            while total_size < transfer_message.file_size:
                data_message = self.recv_message(messages.DATA_MESSAGE)
                total_size += data_message.chunk_size
                script.write(data_message.chunk)

        # Send Done Transfer Message
        done_message_dict = dict(
            type=messages.MessageType.DONE_TRANSFER,
            return_code=0,
        )
        done_message = messages.DONE_TRANSFER_MESSAGE.build(done_message_dict)
        self.host.send(done_message)

        return script_path

    def handle_process_communication(self, process):
        # Send Execution Results Transfer Message
        execution_results_dict = dict(
            type=messages.MessageType.TRANSFER_EXECUTION_RESULTS
        )
        message = messages.TRANSFER_EXECUTION_RESULTS_MESSAGE.build(execution_results_dict)
        self.host.send(message)

        # Send Data Messages between process and client
        try:
            # Pipe data between process and user
            r = [0]  # non-empty list
            while process.poll() is None or len(r) > 0:
                # Wait for IO from client or process
                r, _, _ = select.select([process.stdout, process.stderr, self.host], [], [], SELECT_TIMEOUT)

                # Check for output from the process
                for proc_pipe in [process.stdout, process.stderr]:
                    if proc_pipe not in r:
                        continue

                    try:
                        line = proc_pipe.readline(settings.MAX_CHUNK_SIZE)
                    except OSError:
                        r.remove(proc_pipe)
                        continue

                    if not line:
                        r.remove(proc_pipe)
                        continue

                    line = line.decode("ascii")
                    self.send_message(line)

                # Check for input from the client
                if self.host in r:
                    message = self.recv_message(messages.DATA_MESSAGE)
                    process.stdin.write(message.chunk)
                    process.stdin.flush()

        except Exception as e:
            # Error encountered - Send Error Message
            error_text = e.args[0]
            if type(error_text) is not str:
                if len(e.args) > 1:
                    error_text = e.args[1]
                else:
                    error_text = "Unknown error."
            error_text = bytes(error_text, "ascii")
            error_message_dict = dict(
                type=messages.MessageType.ERROR,
                error_code=settings.ErrorCodes.COMMUNICATION_ERROR,
                error_message_size=len(error_text),
                error_message=error_text,
            )
            message = messages.ERROR_MESSAGE.build(error_message_dict)
            self.host.send(message)

        else:
            # Process finished - Send Done Message
            done_message_dict = dict(
                type=messages.MessageType.DONE_TRANSFER,
                return_code=process.returncode
            )
            message = messages.DONE_TRANSFER_MESSAGE.build(done_message_dict)
            self.host.send(message)
