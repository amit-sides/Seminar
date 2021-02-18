#include <iostream>
#include <cstring>

// Compiled with -fno-stack-protector

bool authenticate()
{
    char password[20] = { 0 };
    int authenticated = 0;

    std::cout << "Insert secret password: ";
    std::cin >> password;
    if (strncmp(password, "MY_SECRET_PASSWORD", 20) == 0)
    {
        authenticated = 1;
    }

    return authenticated;
}

int main() {
    bool result = authenticate();
    std::cout << "Authenticated succeeded? " << result;
    return 0;
}
