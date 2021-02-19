import docker

DOCKER_IMAGE_NAME = "runner_image"
DOCKER_BUILD_PATH = "."

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

def build_image():
    client = docker.from_env()

    return client.images.build(tag=DOCKER_IMAGE_NAME, path=DOCKER_BUILD_PATH)[0]


def run_container(image, port):
    client = docker.from_env()

    return client.containers.run(image, detach=True, ports={DOCKER_EXPOSED_PORT : port})



def main():
    return

if __name__ == '__main__':
    main()