
#include <system_error>
#include <sys/types.h>
#include <sys/socket.h>

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

#include "TLSSocket.h"

bool TLSSocket::is_initialized = false;

TLSSocket::TLSSocket()
{
    int result = SSL_ERROR_SSL;

    // Initializes members
    socket_fd = -1;
    connection = nullptr;
    context = nullptr;
    connected = false;

    // Initializes wolfSSL library (if needed)
    if (!is_initialized)
    {
        result = wolfSSL_Init();
        if (SSL_SUCCESS != result)
        {
            throw std::system_error(ENOMEM, std::generic_category());
        }
        is_initialized = true;
    }

    // Create the context for the ssl connection
    context = wolfSSL_CTX_new(wolfTLSv1_3_client_method());
    if (nullptr == context) {
        throw std::system_error(EINVAL, std::generic_category());
    }
}


bool TLSSocket::load_certificate(const char * certificate)
{
    int result = SSL_FATAL_ERROR;

    // Validate parameters
    if (nullptr == certificate)
    {
        return false;
    }

    // Load the certificate
    result = wolfSSL_CTX_load_verify_locations(context, certificate, nullptr);
    if (SSL_SUCCESS != result)
    {
        return false;
    }
    return true;
}


bool TLSSocket::set_cipher_suit(const char *ciphers_list)
{
    int result = SSL_FATAL_ERROR;

    // Validate parameters
    if (nullptr == ciphers_list)
    {
        return false;
    }

    // Set the cipher list
    result = wolfSSL_CTX_set_cipher_list(context, ciphers_list);
    if (SSL_SUCCESS != result)
    {
        return false;
    }
    return true;
}


bool TLSSocket::start_connection(const char *hostname, uint16_t port)
{
    int                 result          = SSL_FATAL_ERROR;
    struct addrinfo     hints           = {0};
    struct addrinfo     *address        = nullptr;
    struct sockaddr_in  server_address  = {0};

    // Validate parameters
    if (nullptr == hostname)
    {
        return false;
    }

    // Make sure the connection is not active already
    if (connected)
    {
        return false;
    }

    // Get IP of host
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    result = getaddrinfo(hostname, nullptr, &hints, &address);
    if (0 != result)
    {
        return false;
    }

    // Create server address struct
    memcpy(&server_address, address, sizeof(server_address));
    server_address.sin_port = htons(port);

    // Free the resources allocated by getaddrinfo
    freeaddrinfo(address);

    // Create the socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > socket_fd)
    {
        return false;
    }

    // Create the TLS connection using the context
    connection = wolfSSL_new(context);
    if (nullptr == connection)
    {
        return false;
    }

    // Connect to the server
    result = connect(socket_fd, (const sockaddr*)&server_address, sizeof(server_address));
    if (0 != result)
    {
        return false;
    }

    // Assign the socket to the TLS connection
    result = wolfSSL_set_fd(connection, socket_fd);
    if (SSL_SUCCESS != result)
    {
        return false;
    }

    // Connect using the TLS connection (Perform TLS handshake...)
    result = wolfSSL_connect(connection);
    if (SSL_SUCCESS != result)
    {
        return false;
    }

    connected = true;
    return true;
}


int TLSSocket::send(const char *message, int message_size)
{
    int result = SSL_FATAL_ERROR;

    // Validate parameters
    if (nullptr == message || 0 >=message_size)
    {
        return result;
    }

    // Make sure the connection is active
    if (!connected)
    {
        return result;
    }

    // Send the message
    result = wolfSSL_write(connection, message, message_size);
    return result;
}


int TLSSocket::recv(char *message, int message_size)
{
    int result = SSL_FATAL_ERROR;

    // Validate parameters
    if (nullptr == message || 0 >=message_size)
    {
        return result;
    }

    // Make sure the connection is active
    if (!connected)
    {
        return result;
    }

    // Receive a message
    result = wolfSSL_read(connection, message, message_size);
    return result;
}

bool TLSSocket::close_connection()
{
    int result = SSL_FATAL_ERROR;

    // Make sure a connection is active
    if (connected)
    {
        // Shutdown the connection
        result = wolfSSL_shutdown(connection);
        if (SSL_SUCCESS != result)
        {
            return false;
        }

        connected = false;
        // Free resources of TLS connection
        wolfSSL_free(connection);
        connection = nullptr;
    }

    // Close socket
    if (-1 != socket_fd)
    {
        result = close(socket_fd);
        if (0 != result)
        {
            return false;
        }
        socket_fd = -1;
    }

    return true;
}


TLSSocket::~TLSSocket(void)
{
    // Shutdown connection
    if (connected)
    {
        // Best effort
        wolfSSL_shutdown(connection);
        connected = false;

        // Free resources of TLS connection
        wolfSSL_free(connection);
    }

    // Close socket
    if (-1 != socket_fd)
    {
        close(socket_fd);
        socket_fd = -1;
    }

    // Free resources of the context
    wolfSSL_CTX_free(context);
}