import enum

MESSAGE_SIZE = 1024
DOCKER_EXPOSED_PORT = 8080
SYNC_MESSAGE = b"connected?"

class ErrorCodes(enum.IntEnum):
    UNKNOWN_ERROR =         -0xF000
    TIMED_OUT =             -0xF001
    COMMUNICATION_ERROR =   -0xF002
