# Client

This client connects to the server and requests the server to execute python scripts.

1. The connection between the client and the server is secured by TLS1.3. It means that the client needs to "trust" the CA certificate that signed the server's.

   Therefore, either the CA certificate is trusted by the machine running the client or the CA certificate needs to be given to the client as the forth argument so that it will configure it as trusted certificate.

2. The client uses the WolfSSL library that implements API for TLS socket communication. During the configuration of the build, the library is automatically cloned by the CMake scripts, and built with the client during the building step.

## Requirements

* Linux machine (Tested on Ubuntu 20.04.2 - 64 bit). Setup instructions demonstrate Ubuntu's setup commands.
* CMake (Tested with version 3.16.3)
* Make (Tested with version 4.2.1)
* gcc 10 (Compiled with gcc 10.1.0)
* g++ 10 (Compiled with g++ 10.1.0)

## Setup

2. Install Make, CMake, GCC, G++, if you don't have them already:

   ```batch
   sudo apt-get install make cmake gcc-10 g++-10
   ```

2. Create the build directory

   ```batch
   mkdir build
   cd build
   ```

3. Run CMake to configure the compilation files

   ```batch
   cmake ..
   ```

   * This might require internet connection to fetch WolfSSL's content from the git repository.
   * This might take some time to clone the entire WolfSSL repo...

4. Run Make to build the project:

   ```batch
   make
   ```

6. You now should be ready to run the client:

   ```batch
   ./Client <Server Host> <Port> <Python Script> [CA Certificate]
   ```

   * Server Host - The IP or hostname of the server to connect.
   * Port - The server's port. Probably 12344.
   * Python Script - Path to a file containing the python code to execute on the server.
   * CA Certificate - The certificate of the CA that signed the server's certificate. If the CA's certificate is already trusted by your machine, this argument is optional.