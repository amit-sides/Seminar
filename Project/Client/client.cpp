#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>

#include "client.h"
#include "messages.h"

std::string Client::send_data_message(const char *data, uint32_t data_size)
{
    int             result = SSL_FATAL_ERROR;
    data_message_t  message = {0};

    // Validate parameters
    if (nullptr == data || 0 >= data_size || data_size > MAXIMUM_CHUNK_SIZE)
    {
        return "Invalid parameters";
    }

    // Make sure the connection is active
    if (!connected)
    {
        return "Connection is closed";
    }

    // Build message struct
    message.header.type = DATA;
    message.header.chunk_size = data_size;
    memcpy(&message.chunk, data, data_size);

    // Send message
    result = send((const char *)&message, sizeof(message));
    if (result != sizeof(message))
    {
        return "Failed to send message";
    }
    return "";
}

std::string Client::transfer_file(std::string filepath)
{
    int                     result = SSL_FATAL_ERROR;
    std::string             output;
    std::string             filename;
    uint8_t                 filename_length = 0;
    uint32_t                size = 0;
    uint32_t                total_size = 0;
    transfer_file_message_t message = {0};
    generic_message_t       recv_message = {0};
    done_transfer_message_t *done_message = nullptr;
    error_message_t         *error_message = nullptr;
    char                    chunk[MAXIMUM_CHUNK_SIZE] = {0};
    char                    err_msg[MESSAGE_SIZE * 2] = {0};

    // Make sure the connection is active
    if (!connected)
    {
        return "Connection is closed";
    }

    // Make sure the file exists
    if (!std::filesystem::exists(filepath))
    {
        return "File doesn't exist";
    }

    // Get the file's name and size
    filename = std::filesystem::path(filepath).filename();
    filename_length = strlen(filename.c_str());
    std::ifstream file(filepath.c_str(), std::ios::in | std::ios::binary);
    if (!file)
    {
        return "Failed to open file";
    }
    file.seekg(0, std::ios::end);
    size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Build transfer message
    message.header.type = TRANSFER_FILE;
    message.header.file_size = size;
    message.header.filename_size = filename_length;
    if (filename_length > sizeof(message.filename))
    {
        return "Filename is too long";
    }
    memcpy(&message.filename, filename.c_str(), filename_length);

    // Send transfer file message
    result = send((const char *)&message, sizeof(message));
    if (sizeof(message) != result)
    {
        return "Failed to send transfer file message";
    }

    // Send file data
    while(!file.eof())
    {
        file.read(chunk, sizeof(chunk));
        total_size += file.gcount();
        output = send_data_message(chunk, file.gcount());
        if (!output.empty())
        {
            return output;
        }
    }

    // Receive end transfer message
    result = recv((char *)&recv_message, sizeof(recv_message));
    if (SSL_SUCCESS != result)
    {
        return "Failed to receive end of transfer message";
    }

    // Determine message
    switch(recv_message.header.type)
    {
            // Success case
        case DONE_TRANSFER:
            done_message = (done_transfer_message_t *)&recv_message;
            if (0 == done_message->header.return_code)
            {
                return "";
            }
            std::snprintf(err_msg, sizeof(err_msg) - 1,
                          "Done message returned with %d", done_message->header.return_code);
            break;

            // Error case
        case ERROR:
            error_message = (error_message_t *)&recv_message;
            std::snprintf(err_msg, sizeof(err_msg) - 1, "Transfer failed with %d: %s",
                               error_message->header.error_code, error_message->error_message);
            break;

            // Unknown message
        default:
            std::snprintf(err_msg, sizeof(err_msg) - 1,
                  "Received unknown message with type %d", recv_message.header.type);
            break;
    }
    return std::string(err_msg);
}

std::string Client::communicate_with_script()
{
    int                                     result = SSL_FATAL_ERROR;
    std::string                             output;
    generic_message_t                       recv_message = {0};
    error_message_t                         *error_message = nullptr;
    char                                    err_msg[MESSAGE_SIZE * 2] = {0};
    fd_set                                  read_fds;
    char                                    chunk[MAXIMUM_CHUNK_SIZE] = {0};
    done_transfer_message_t                 *done_message = nullptr;
    data_message_t                          *data_message = nullptr;




    // Make sure the connection is active
    if (!connected)
    {
        return "Connection is closed";
    }

    // Receive transfer execution results message
    result = recv((char *)&recv_message, sizeof(recv_message));
    if (SSL_SUCCESS != result)
    {
        return "Failed to receive transfer execution results message";
    }

    // Determine message
    switch(recv_message.header.type)
    {
            // Success case
        case TRANSFER_EXECUTION_RESULTS:
            break;

            // Error case
        case ERROR:
            error_message = (error_message_t *)&recv_message;
            std::snprintf(err_msg, sizeof(err_msg) - 1, "Received error message %d: %s",
                               error_message->header.error_code, error_message->error_message);
            return std::string(err_msg);

        default:
            std::snprintf(err_msg, sizeof(err_msg) - 1,
                  "Received unknown message with type %d", recv_message.header.type);
            return std::string(err_msg);
    }

    // Wait for user input and messages simultaneously using select
    while(true)
    {
        // Clear the bits in read_fds
        FD_ZERO(&read_fds);

        // Set the server's fd & stdin fd
        FD_SET(socket_fd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        // Execute select
        result = select(std::max(socket_fd, STDIN_FILENO), &read_fds,
                        nullptr, nullptr, nullptr);
        if (-1 == result)
        {
            return "Select failed";
        }

        // Check for user input
        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            std::cin.getline(chunk, MAXIMUM_CHUNK_SIZE);
            output = send_data_message(chunk, strlen(chunk)+1);
            if (!output.empty())
            {
                return output;
            }
        }

        // Check for incoming messages
        if (FD_ISSET(socket_fd, &read_fds))
        {
            // Receive message
            memset(&recv_message, 0, sizeof(recv_message));
            result = recv((char *)&recv_message, sizeof(recv_message));
            if (SSL_SUCCESS != result)
            {
                return "Failed to receive message";
            }

            // Determine message
            switch(recv_message.header.type)
            {
                    // Data message
                case DATA:
                    data_message = (data_message_t  *)&recv_message;
                    std::cout << data_message->chunk;
                    break;

                    // End message
                case DONE_TRANSFER:
                    done_message = (done_transfer_message_t  *)&recv_message;
                    std::cout << "Execution finished with code " << done_message->header.return_code;
                    return "";

                    // Error message
                case ERROR:
                    error_message = (error_message_t *)&recv_message;
                    std::snprintf(err_msg, sizeof(err_msg) - 1, "Received error message %d: %s",
                                       error_message->header.error_code, error_message->error_message);
                    return std::string(err_msg);

                default:
                    std::snprintf(err_msg, sizeof(err_msg) - 1,
                          "Received unknown message with type %d", recv_message.header.type);
                    return std::string(err_msg);
            }
        }
    }
}