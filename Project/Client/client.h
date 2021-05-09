#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H

#include <string>

#include "tlssocket.h"

class Client : public TLSSocket{
private:
    std::string send_data_message(const char *data, uint32_t data_size);

public:
    std::string transfer_file(std::string filepath);

    std::string communicate_with_script();
};


#endif //CLIENT_CLIENT_H
