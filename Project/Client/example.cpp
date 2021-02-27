#include <iostream>

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>
#include <wolfssl/test.h>
#include <errno.h>
#include "messages.h"

#define SERV_PORT 11111
#define NO_SHA256

int main()
{
    int sockfd;
    WOLFSSL_CTX* ctx;
    WOLFSSL* ssl;
    WOLFSSL_METHOD* method;
    struct  sockaddr_in servAddr;
    const char message[] = "Hello, World!\n";

    /* create and set up socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERV_PORT);



    /* initialize wolfssl library */
    wolfSSL_Init();
    method = wolfTLSv1_3_client_method(); /* use TLS v1.3 */

    /* make new ssl context */
    if ( (ctx = wolfSSL_CTX_new(method)) == NULL) {
          err_sys("wolfSSL_CTX_new error");
    }

        /* Add cert to ctx */
#define CA "../../Server/Certificates/ca.crt"
    system("ls " CA);
    if (wolfSSL_CTX_load_verify_locations(ctx, CA, 0) !=
            SSL_SUCCESS) {
        std::cout << "Error: " << wolfSSL_CTX_load_verify_locations(ctx, CA, 0) << std::endl;
        //err_sys("Error loading ca.crt");
    }

    /* Set cipher list */
#define CIPHER_LIST "TLS_AES_256_GCM_SHA384"
    if (wolfSSL_CTX_set_cipher_list(ctx, CIPHER_LIST) != SSL_SUCCESS) {
        fprintf(stderr, "ERROR: failed to set cipher list\n");
        return -1;
    }

    /* make new wolfSSL struct */
    if ( (ssl = wolfSSL_new(ctx)) == NULL) {
         err_sys("wolfSSL_new error");
    }

    printf("s: %lu\n", MAXIMUM_CHUNK_SIZE);

    /* connect to socket */
    connect(sockfd, (struct sockaddr *) &servAddr, sizeof(servAddr));

    /* Connect wolfssl to the socket, server, then send message */
    wolfSSL_set_fd(ssl, sockfd);
    wolfSSL_connect(ssl);
    wolfSSL_write(ssl, message, strlen(message));
    wolfSSL_shutdown(ssl);
    close(sockfd);

    /* frees all data before client termination */
    wolfSSL_free(ssl);
    wolfSSL_CTX_free(ctx);
    wolfSSL_Cleanup();

    return 99;
}