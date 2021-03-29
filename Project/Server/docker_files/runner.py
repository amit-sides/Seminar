import os
import sys
import fcntl
import subprocess

import communication
import messages
import settings

USER_DIRECTORY = "user"

def execute_script(script_path):
    FNULL = open(os.devnull, 'w')

    # Create requirements.txt file for script
    directory = os.path.dirname(script_path)
    subprocess.check_call(["pipreqs", directory], stdout=FNULL, stderr=FNULL)

    # Install the requirements for the script
    requirements_file = os.path.join(directory, "requirements.txt")
    subprocess.check_call([sys.executable, "-m", "pip", "install", "-r", requirements_file], stdout=FNULL, stderr=FNULL)

    # Run the script in another process
    # A lot of configurations were needed to disable the process's buffering...
    os.environ["PYTHONUNBUFFERED"] = "1"
    cmd = ["stdbuf", "-oL", "-eL", sys.executable, script_path]
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, bufsize=0)
    # Setting the process's IO to be non-blocking
    for proc_pipe in [process.stdout, process.stderr]:
        fl = fcntl.fcntl(proc_pipe.fileno(), fcntl.F_GETFL)
        fcntl.fcntl(proc_pipe.fileno(), fcntl.F_SETFL, fl | os.O_NONBLOCK)

    return process

def main():
    # Connect to host
    docker_socket = communication.DockerSocket()
    docker_socket.connect_to_host()

    try:
        if not os.path.exists(USER_DIRECTORY):
            os.makedirs(USER_DIRECTORY)
        script_path = docker_socket.get_script(USER_DIRECTORY)
        
        process = execute_script(script_path)
        docker_socket.handle_process_communication(process)
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
            error_code=settings.ErrorCodes.UNKNOWN_ERROR,
            error_message_size=len(error_text),
            error_message=error_text
        )
        message = messages.ERROR_MESSAGE.build(error_message_dict)
        docker_socket.host.send(message)
        return -1

    return 0


if __name__ == "__main__":
    sys.exit(main())
