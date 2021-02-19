import construct
import enum

import settings

class MessageType(enum.IntEnum):
    TRANSFER_FILE = 1               # Start file transfer
    TRANSFER_EXECUTION_RESULTS = 2  # Start execution results transfer
    DATA = 3                        # data transfer
    DONE_TRANSFER = 4               # End transfer
    ERROR = 5                       # Reports an error

GENERIC_MESSAGE =                       construct.FixedSized(settings.MESSAGE_SIZE,
                                            construct.Struct(
                                                "type"  / construct.Enum(construct.Byte, MessageType),
                                                "data"  / construct.Bytes(settings.MESSAGE_SIZE - construct.Byte.sizeof())
                                            ))

TRANSFER_FILE_MESSAGE =                 construct.FixedSized(settings.MESSAGE_SIZE,
                                            construct.Struct(
                                                "type"          / construct.Const(MessageType.TRANSFER_FILE.value, construct.Byte),
                                                "file_size"     / construct.Int32ub,
                                                "filename_size" / construct.Int8ub,
                                                "filename"      / construct.Bytes(lambda ctx: ctx.filename_size),
                                            ))

TRANSFER_EXECUTION_RESULTS_MESSAGE =    construct.FixedSized(settings.MESSAGE_SIZE,
                                            construct.Struct(
                                                "type"          / construct.Const(MessageType.TRANSFER_EXECUTION_RESULTS.value, construct.Byte),
                                            ))

DATA_MESSAGE =                          construct.FixedSized(settings.MESSAGE_SIZE,
                                            construct.Struct(
                                                "type"          / construct.Const(MessageType.DATA.value, construct.Byte),
                                                "chunk_size"    / construct.Int32ub,
                                                "chunk"         / construct.Bytes(lambda ctx: ctx.chunk_size),
                                            ))

DONE_TRANSFER_MESSAGE =                 construct.FixedSized(settings.MESSAGE_SIZE,
                                            construct.Struct(
                                                "type"          / construct.Const(MessageType.DONE_TRANSFER.value, construct.Byte),
                                                "return_code"   / construct.Int32sb,
                                            ))

ERROR_MESSAGE =                         construct.FixedSized(settings.MESSAGE_SIZE,
                                            construct.Struct(
                                                "type"                  / construct.Const(MessageType.ERROR.value, construct.Byte),
                                                "error_code"            / construct.Int32sb,
                                                "error_message_size"    / construct.Int32ub,
                                                "error_message"         / construct.Bytes(lambda ctx: ctx.error_message_size),
                                            ))

