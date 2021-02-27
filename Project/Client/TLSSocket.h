#ifndef CLIENT_TLSSOCKET_H
#define CLIENT_TLSSOCKET_H

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

class TLSSocket {

private:
    static bool         is_initialized;

    WOLFSSL_CTX *       context;
    WOLFSSL *           connection;
    int                 socket_fd;
    bool                connected;

public:
    TLSSocket();

    bool load_certificate(const char * certificate);

    bool set_cipher_suit(const char *ciphers_list);

    bool start_connection(const char *hostname, uint16_t port);

    int send(const char *message, int message_size);

    int recv(char *message, int message_size);

    bool close_connection();

    ~TLSSocket();

};


#endif //CLIENT_TLSSOCKET_H
