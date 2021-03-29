#include <iostream>
#include <climits>

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

#include "client.h"

enum Arguments
{
    EXECUTABLE = 0, // The program's executable
    HOSTNAME,       // Server's hostname
    PORT,           // Server's port
    PYTHON_FILE,    // Python script to send to the server
    CERTIFICATE,    // CA Certificate that has signe to server's certificate

    COUNT
};

#define USAGE       (" <Hostname> <Port> <Python File> [CA Certificate]")
#define CIPHER_LIST ("TLS_AES_256_GCM_SHA384")

int execute_client(int argc, char **argv)
{
    int         result = SSL_FATAL_ERROR;
    std::string error_message;
    Client      client = Client();
    uint64_t    port = 0;

    // Check arguments
    if (argc < CERTIFICATE || argc > COUNT)
    {
        std::cerr << "Error: Invalid number of arguments!" << std::endl;
        std::cerr << "Usage: " << argv[EXECUTABLE] << USAGE << std::endl;
        return -1;
    }

    // Convert port to integer
    port = std::stoi(argv[PORT]);
    if (port >=USHRT_MAX)
    {
        std::cerr << "Error: Invalid port number: " << argv[PORT] << std::endl;
        return -2;
    }

    // Connect to the server
    result = client.start_connection(argv[HOSTNAME], port);
    if (SSL_SUCCESS != result)
    {
        std::cerr << "Error: Failed to start connection!" << std::endl;
        std::cerr << "Error code: " << result << std::endl;
        return result;
    }

    // Send script file to server
    error_message = client.transfer_file(argv[PYTHON_FILE]);
    if (!error_message.empty())
    {
        std::cerr << "Error: Failed to send script file!" << std::endl;
        std::cerr << "Error message: " << error_message << std::endl;
        return -3;
    }

    // Transfer input & output of script to server and back
    error_message = client.communicate_with_script();
    if (!error_message.empty())
    {
        std::cerr << "Error: Failed to execute file!" << std::endl;
        std::cerr << "Error message: " << error_message << std::endl;
        return -4;
    }

    printf("\n");
    return 0;
}

int main(int argc, char**argv)
{
    try
    {
        return execute_client(argc, argv);
    }
    catch (const std::exception &exc)
    {
        std::cerr << "Error: Exception was thrown!" << std::endl;
        std::cerr << "Exception message: " << exc.what() << std::endl;
        return -5;
    }
}