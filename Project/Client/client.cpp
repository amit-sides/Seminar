#include <string>
#include <filesystem>

#include "client.h"
#include "messages.h"

bool Client::send_data_message(const char *data, uint32_t data_length)
{
    int             result = SSL_FATAL_ERROR;
    data_message_t  message = {0};

    // Validate parameters
    if (nullptr == data || 0 >=data_length || data_length > sizeof(message.chunk))
    {
        return false;
    }

    // Make sure the connection is active
    if (!connected)
    {
        return false;
    }

    // Build message struct
    message.header.type = DATA;
    message.header.chunk_size = data_length;
    memcpy(&message.chunk, data, data_length);

    // Send message
    result = send((const char *)&message, sizeof(message));
    if (result != sizeof(message))
    {
        return false;
    }
    return true;
}

bool Client::transfer_file(std::string filepath)
{
    std::string filename;

    // Make sure the file exists
    if (!std::filesystem::exists(filepath))
    {
        return false;
    }

    // Get the file name
    filename = std::filesystem::path(filepath).filename();

    return true;
}

bool Client::communicate_with_script()
{
    return true;
}
