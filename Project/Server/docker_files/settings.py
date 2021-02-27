import enum

MESSAGE_SIZE = 1024
MAX_CHUNK_SIZE = MESSAGE_SIZE - 5  # 5 = size of type + chunk_size variables
DOCKER_EXPOSED_PORT = 8080

class ErrorCodes(enum.IntEnum):
    UNKNOWN_ERROR =         -0xF000
    TIMED_OUT =             -0xF001
    COMMUNICATION_ERROR =   -0xF002
