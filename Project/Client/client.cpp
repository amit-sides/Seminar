#include <string>
#include <fstream>
#include <filesystem>

#include "client.h"
#include "messages.h"

bool Client::send_data_message(const char *data, uint32_t data_length)
{
    int             result = SSL_FATAL_ERROR;
    data_message_t  message = {0};

    // Validate parameters
    if (nullptr == data || 0 >=data_length || data_length > MAXIMUM_CHUNK_SIZE)
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
    int                     result = SSL_FATAL_ERROR;
    std::string             filename;
    uint8_t                 filename_length = 0;
    uint32_t                size = 0;
    uint32_t                total_size = 0;
    transfer_file_message_t message = {0};
    char                    chunk[MAXIMUM_CHUNK_SIZE] = {0};

    // Make sure the connection is active
    if (!connected)
    {
        return false;
    }

    // Make sure the file exists
    if (!std::filesystem::exists(filepath))
    {
        return false;
    }

    // Get the file's name and size
    filename = std::filesystem::path(filepath).filename();
    filename_length = strlen(filename.c_str());
    std::ifstream file(filepath.c_str(), std::ios::in | std::ios::binary);
    if (!file)
    {
        return false;
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
        return false;
    }
    memcpy(&message.filename, filename.c_str(), filename_length);

    // Send transfer file message
    result = send((const char *)&message, sizeof(message));
    if (sizeof(message) != result)
    {
        return false;
    }

    // Send file data
    while(!file.eof())
    {
        file.read(chunk, sizeof(chunk));
        total_size += file.gcount();
        if (!send_data_message(chunk, file.gcount()))
    }

    return true;
}

bool Client::communicate_with_script()
{
    return true;
}
