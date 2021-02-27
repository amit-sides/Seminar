#ifndef CLIENT_MESSAGES_H
#define CLIENT_MESSAGES_H

#include <stdint.h>

#define MESSAGE_SIZE    (1024)

enum MessageType
{
    TRANSFER_FILE = 1,              // Start file transfer
    TRANSFER_EXECUTION_RESULTS = 2, // Start execution results transfer
    DATA = 3,                       // Data transfer
    DONE_TRANSFER = 4,              // End transfer
    ERROR = 5,                      // Reports an error
};

typedef struct __attribute__((packed)) generic_header_s
{
    uint8_t type;
} generic_header_t;

typedef struct __attribute__((packed)) generic_message_s
{
    generic_header_t    header;
    uint8_t data[MESSAGE_SIZE - sizeof(generic_header_t)];
} generic_message_t;

typedef struct __attribute__((packed)) transfer_file_header_s
{
    uint8_t     type;
    uint32_t    file_size;
    uint8_t     filename_size;
} transfer_file_header_t;

typedef struct __attribute__((packed)) transfer_file_message_s
{
    transfer_file_header_t    header;
    uint8_t filename[MESSAGE_SIZE - sizeof(transfer_file_header_t)];
} transfer_file_message_t;

typedef struct __attribute__((packed)) transfer_execution_results_header_s
{
    uint8_t type;
} transfer_execution_results_header_t;

typedef struct __attribute__((packed)) transfer_execution_results_message_s
{
    transfer_execution_results_header_t header;
    uint8_t data[MESSAGE_SIZE - sizeof(transfer_execution_results_header_t)];
} transfer_execution_results_message_t;

typedef struct __attribute__((packed)) data_header_s
{
    uint8_t     type;
    uint32_t    chunk_size;
} data_header_t;

typedef struct __attribute__((packed)) data_message_s
{
    data_header_t   header;
    uint8_t chunk[MESSAGE_SIZE - sizeof(data_header_t)];
} data_message_t;

typedef struct __attribute__((packed)) done_transfer_header_s
{
    uint8_t type;
    int32_t return_code;
} done_transfer_header_t;

typedef struct __attribute__((packed)) done_transfer_message_s
{
    done_transfer_header_t  header;
    uint8_t unused[MESSAGE_SIZE - sizeof(done_transfer_header_t)];
} done_transfer_message_t;

typedef struct __attribute__((packed)) error_header_s
{
    uint8_t     type;
    int32_t     error_code;
    uint32_t    error_message_size;
} error_header_t;

typedef struct __attribute__((packed)) error_message_s
{
    error_header_t  header;
    uint8_t error_message[MESSAGE_SIZE - sizeof(error_header_t)];
} error_message_t;

#endif //CLIENT_MESSAGES_H
