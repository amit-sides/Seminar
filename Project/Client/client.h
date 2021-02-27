#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <string>

#include "tlssocket.h"

class Client : public TLSSocket{

public:
    bool send_data_message(const char *data, uint32_t data_length);

    bool transfer_file(std::string filepath);

    bool communicate_with_script();
};


#endif //CLIENT_CLIENT_H
