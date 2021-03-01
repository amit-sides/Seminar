import os
import sys
import fcntl
import subprocess

import communication
import messages
import settings

USER_DIRECTORY = "user"

def execute_script(script_path):
    print(f"script path: {script_path}")
    directory = os.path.dirname(script_path)

    print(f"dir: {directory}")
    print("create req file")
    sys.stdout.flush()

    subprocess.check_call(["pipreqs", directory])

    print("install reqs")
    sys.stdout.flush()

    requirements_file = os.path.join(directory, "requirements.txt")
    subprocess.check_call([sys.executable, "-m", "pip", "install", "-r", requirements_file])

    print("executing...")
    sys.stdout.flush()

    os.environ["PYTHONUNBUFFERED"] = "1"
    cmd = ["stdbuf", "-oL", "-eL", sys.executable, script_path]
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, bufsize=0)
    for proc_pipe in [process.stdout, process.stderr]:
        fl = fcntl.fcntl(proc_pipe.fileno(), fcntl.F_GETFL)
        fcntl.fcntl(proc_pipe.fileno(), fcntl.F_SETFL, fl | os.O_NONBLOCK)
    print("updated!")
    return process

def main():
    # Connect to host
    docker_socket = communication.DockerSocket()
    docker_socket.connect_to_host()

    try:
        print("getting script...")
        script_path = docker_socket.get_script(USER_DIRECTORY)

        print("executing script...")
        process = execute_script(script_path)
        docker_socket.handle_process_communication(process)
    except Exception as e:
        error_text = bytes(e.args[0], "ascii")
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
