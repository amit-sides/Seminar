#include <cstring>
#include <iostream>

#include "mbedtls/net.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/debug.h"

#define SEED "R4nd0m"
#define CACERT "../../Server/Certificates/ca.pem"


bool initialize_mbedtls(mbedtls_net_context *server_fd, mbedtls_entropy_context *entropy, mbedtls_ctr_drbg_context *ctr_drbg,
                        mbedtls_ssl_context *ssl, mbedtls_ssl_config *conf, mbedtls_x509_crt *cacert)
{
    mbedtls_net_init(server_fd);
    mbedtls_ssl_init(ssl);
    mbedtls_ssl_config_init(conf);
    mbedtls_x509_crt_init(cacert);
    mbedtls_ctr_drbg_init(ctr_drbg);

    mbedtls_entropy_init(entropy);
    if(0 != mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy,
                                   (const unsigned char *) SEED,
                                   strlen(SEED)))
    {
        std::cout << "Error: mbedtls_ctr_drbg_seed failed!" << std::endl;
        return false;
    }

    return true;
}

bool load_certificate(mbedtls_x509_crt *cacert)
{
    if( 0 > mbedtls_x509_crt_parse_file(cacert, CACERT))
    {
        std::cout << "Error: mbedtls_x509_crt_parse_file failed!" << std::endl;
        return false;
    }
    return true;
}

bool connect()
{
    
/*
     * 1. Start the connection
     */
    mbedtls_printf( "  . Connecting to tcp/%s/%s...", SERVER_NAME, SERVER_PORT );
    fflush( stdout );

    if( ( ret = mbedtls_net_connect( &server_fd, SERVER_NAME,
                                         SERVER_PORT, MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_net_connect returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /*
     * 2. Setup stuff
     */
    mbedtls_printf( "  . Setting up the SSL/TLS structure..." );
    fflush( stdout );

    if( ( ret = mbedtls_ssl_config_defaults( &conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_printf( " ok\n" );

    /* OPTIONAL is not optimal for security,
     * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
    mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_debug, stdout );

    if( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
        goto exit;
    }

    if( ( ret = mbedtls_ssl_set_hostname( &ssl, SERVER_NAME ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret );
        goto exit;
    }

    mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

    /*
     * 4. Handshake
     */
    mbedtls_printf( "  . Performing the SSL/TLS handshake..." );
    fflush( stdout );

    while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", (unsigned int) -ret );
            goto exit;
        }
    }

    mbedtls_printf( " ok\n" );

    /*
     * 5. Verify the server certificate
     */
    mbedtls_printf( "  . Verifying peer X.509 certificate..." );

    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( &ssl ) ) != 0 )
    {
        char vrfy_buf[512];

        mbedtls_printf( " failed\n" );

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        mbedtls_printf( "%s\n", vrfy_buf );
    }
    else
        mbedtls_printf( " ok\n" );

    /*
     * 3. Write the GET request
     */
    mbedtls_printf( "  > Write to server:" );
    fflush( stdout );

    len = sprintf( (char *) buf, GET_REQUEST );

    while( ( ret = mbedtls_ssl_write( &ssl, buf, len ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
            goto exit;
        }
    }

    len = ret;
    mbedtls_printf( " %d bytes written\n\n%s", len, (char *) buf );

    /*
     * 7. Read the HTTP response
     */
    mbedtls_printf( "  < Read from server:" );
    fflush( stdout );

    do
    {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = mbedtls_ssl_read( &ssl, buf, len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            mbedtls_printf( "failed\n  ! mbedtls_ssl_read returned %d\n\n", ret );
            break;
        }

        if( ret == 0 )
        {
            mbedtls_printf( "\n\nEOF\n\n" );
            break;
        }

        len = ret;
        mbedtls_printf( " %d bytes read\n\n%s", len, (char *) buf );
    }
    while( 1 );

    mbedtls_ssl_close_notify( &ssl );

    exit_code = MBEDTLS_EXIT_SUCCESS;

exit:

#ifdef MBEDTLS_ERROR_C
    if( exit_code != MBEDTLS_EXIT_SUCCESS )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        mbedtls_printf("Last error was: %d - %s\n\n", ret, error_buf );
    }
#endif

    mbedtls_net_free( &server_fd );

    mbedtls_x509_crt_free( &cacert );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

#if defined(_WIN32)
    mbedtls_printf( "  + Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    mbedtls_exit( exit_code );
}

int main(void)
{
    char *buffer = new char[256];
    nsapi_size_or_error_t result;
    nsapi_size_t size;
    const char query[] = "GET / HTTP/1.1\r\nHost: ifconfig.io\r\nConnection: close\r\n\r\n";

    mbed_trace_init();

    printf("TLSSocket Example.\n");
    printf("Mbed OS version: %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

    NetworkInterface *net = NetworkInterface::get_default_instance();

    if (!net) {
        printf("Error! No network inteface found.\n");
        return 0;
    }

    printf("Connecting to network\n");
    result = net->connect();
    if (result != NSAPI_ERROR_OK) {
        printf("Error! net->connect() returned: %d\n", result);
        return result;
    }

    printf("Connecting to ifconfig.io\n");
    SocketAddress addr;
    result = net->gethostbyname("ifconfig.io", &addr);
    if (result != NSAPI_ERROR_OK) {
	printf("Error! DNS resolution for ifconfig.io failed with %d\n", result);
    }
    addr.set_port(443);

    TLSSocket *socket = new TLSSocket;
    result = socket->open(net);
    if (result != NSAPI_ERROR_OK) {
        printf("Error! socket->open() returned: %d\n", result);
        return result;
    }

    socket->set_hostname("ifconfig.io");

    result = socket->set_root_ca_cert(cert);
    if (result != NSAPI_ERROR_OK) {
        printf("Error: socket->set_root_ca_cert() returned %d\n", result);
        return result;
    }

    result = socket->connect(addr);
    if (result != NSAPI_ERROR_OK) {
        printf("Error! socket->connect() returned: %d\n", result);
        goto DISCONNECT;
    }

    // Send a simple http request
    size = strlen(query);
    result = socket->send(query, size);
    if (result != size) {
        printf("Error! socket->send() returned: %d\n", result);
        goto DISCONNECT;
    }

    // Receieve an HTTP response and print out the response line
    while ((result = socket->recv(buffer, 255)) > 0) {
        buffer[result] = 0;
        printf("%s", buffer);
    }
    printf("\n");

    if (result < 0) {
        printf("Error! socket->recv() returned: %d\n", result);
        goto DISCONNECT;
    }


DISCONNECT:
    delete[] buffer;
    // Close the socket to return its memory
    socket->close();
    delete socket;

    // Bring down the network interface
    net->disconnect();
    printf("Done\n");
}