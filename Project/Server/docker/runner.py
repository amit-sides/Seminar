import os
import sys
import subprocess
import signal
import contextlib

class TimeoutException(Exception): pass

@contextlib.contextmanager
def time_limit(seconds):
    def signal_handler(signum, frame):
        raise TimeoutException("Timed out!")
    signal.signal(signal.SIGALRM, signal_handler)
    signal.alarm(seconds)
    try:
        yield
    finally:
        signal.alarm(0)


def execute_script(directory, script_name):
    try:
        subprocess.check_call(["pipreqs", directory])
    except subprocess.CalledProcessError:
        print("Error! 1")
        return

    requirements_file = os.path.join(directory, "requirements.txt")
    try:
        subprocess.check_call([sys.executable, "-m", "pip", "install", "-r", requirements_file])
    except subprocess.CalledProcessError:
        print("Error! 2")
        return

    script_path = os.path.join(directory, script_name)
    try:
        subprocess.check_call([sys.executable, script_path])
    except subprocess.CalledProcessError:
        print("Error! 3")
        return
    print("Success!")

def main():
    script = sys.argv[1]
    directory = os.path.dirname(script)
    script_name = os.path.basename(script)

    try:
        with time_limit(30):
            execute_script(directory, script_name)
    except TimeoutException:
        print("Timeout!!!")


    
if __name__ == "__main__":
    main()