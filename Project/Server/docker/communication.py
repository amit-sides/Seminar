import os
import socket

import settings
import messages

class DockerSocket(Object):
    def __init__(self):
        self.host = None

    def connect_to_host(self):
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind(("0.0.0.0", DOCKER_EXPOSED_PORT))

        server.listen(1)

        self.host, _ = server.accept()

    def recv_message(self, type):
        message = self.host.recv(settings.MESSAGE_SIZE)

        if len(message) != type.sizeof():
            raise socket.error("Invalid message size!")

        message = type.parse(message)
        return message

    def send_message(self, message):
        data_message_dict = dict(
            type = messages.MessageType.DATA,
            chunk_size = len(message),
            message = message
        )
        message = messages.DATA_MESSAGE.build(data_message_dict)
        self.host.send(message)

    def get_script(self, location):
        # Get transfer message to start script transfer
        transfer_message = self.recv_message(messages.TRANSFER_FILE_MESSAGE)
        total_size = 0

        # Get script file with data messages
        script_path = os.path.join(location, transfer_message.filename)
        with open(script_path, "wb") as script:
            while total_size < transfer_message.file_size:
                data_message = self.recv_message(messages.DATA_MESSAGE)
                total_size += data_message.chunk_size
                script.write(data_message.chunk)

        # Send Done Transfer Message
        done_message_dict = dict(
            type = MessageType.DONE_TRANSFER,
            return_code = 0,
        )
        done_message = messages.DONE_TRANSFER_MESSAGE.build(done_message_dict)
        self.host.send(done_message)

        return script_path

    def handle_process_communication(self, process):
        # Send Execution Results Transfer Message
        execution_results_dict = dict(
            type = messages.MessageType.TRANSFER_EXECUTION_RESULTS
        )
        message = messages.TRANSFER_EXECUTION_RESULTS_MESSAGE.build(execution_results_dict)
        self.host.send(message)

        # Send Data Messages between process and client
        try:
            # Pipe data between process and user
            while process.poll() is None:
                r, _, _ = select.select([process.stdout, process.stderr, self.host], [], [], timeout=1)
                for proc_pipe in [process.stdout, process.stderr]:
                    if proc_pipe not in r:
                        continue
                    line = proc_pipe.readline()
                    if not line:
                        continue
                    self.send_message(line)


                if self.host in r:
                    message = self.recv_message(messages.DATA_MESSAGE)
                    process.stdin.write(message.chunk)

        except Exception as e:
            # Error encountered - Send Error Message
            error_text = e.args[0]
            error_message_dict = dict(
                type = messages.MessageType.ERROR,
                error_code = -0xFFFF,
                error_message_size = len(error_text),
                error_message = error_text
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














