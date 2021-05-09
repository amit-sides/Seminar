import docker

from docker_files import settings

DOCKER_IMAGE_NAME = "runner_image"
DOCKER_BUILD_PATH = "."


def build_image():
    client = docker.from_env()
    image = client.images.build(tag=DOCKER_IMAGE_NAME, path=DOCKER_BUILD_PATH)[0]
    return image


def run_container(image, port):
    client = docker.from_env()

    return client.containers.create(image, detach=True, ports={settings.DOCKER_EXPOSED_PORT: port})

