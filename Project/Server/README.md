# Server

This server executes python scripts given by it's clients.

1. When a client initiates a connection to the server, it starts sending the server it's python script.
2. After the script was transferred, the server runs a docker container and runs the script inside it.
3. All the input/output of the script is transferred to the client through the server. This way, a client can see the output immediately after the script prints it and send input to the script.
4. When the script finishes, it's exit code is sent back to the client and the connection is closed.
5. The server uses a pool of processes to allow multiple clients execute scripts at the same time.
6. Each client session lasts at most 60 seconds. After that, the server's process raises a timeout exception and the connection is closed in a graceful manner.
7. The connection between the server and the clients is secured by TLS1.3. It means that the client needs to "trust" the CA certificate that signed the server's.

## Requirements

* Linux machine (Tested on Ubuntu 20.04.2 - 64 bit). Setup instructions demonstrate Ubuntu's setup commands.
* Python 3 (Tested on Python 3.8.5 - 64bit)
* Pip for python 3 (Tested with version 20.0.2)
* Docker (Tested with version 19.03.8)
* Make (Tested with version 4.2.1)
* OpenSSL (Tested with version 1.1.1f)
* Python packages (install using requirements.txt):
  * docker (Tested with version 4.4.4)
  * construct (Tested with version 2.10.61)
  * timeout_decorator (Tested with version 0.5.0)

## Setup

1. Install python 3 and pip, if you don't have it already:

   ```batch
   sudo apt-get install python3 python3-pip
   ```

2. Install Docker, Make, OpenSSL, if you don't have them already:

   ```batch
   sudo apt-get install docker.io make openssl
   ```

3. Run the following commands to install the required python packages:

   ```batch
   python3 -m pip install -r requirements.txt
   ```

4. Generate the CA & server certificates by running the following commands:

   ```batch
   cd Certificates
   make generate
   cd ..
   ```

   * If you ever need to delete all the files created by this step, run the same commands but replace `make generate` with `make clean`.

5. Make sure the user that runs the server has permissions to run docker containers:

   * If it doesn't, add the user to the `docker` group:

   ```batch
   sudo groupadd docker
   sudo gpasswd -a $USER docker
   newgrp docker
   ```

6. You now should be ready to run the server:

   ```batch
   python3 main.py
   ```

   * The server might need internet connection to download pre-built docker image of python.

   This should run the server on port 12344. Use this port in the client to connect to the server.

## Known Issues

* The docker runs the script to protect the host (server) from running malicious code. In order to send the output back to the user, the script is ran as a sub-process in the docker. As a result, the output is configured to be line-buffered. Most terminal executables are ran using line-buffered mode, but for some reason when running python scripts in the terminal some output is printed without flushing the standard output nor printing a new-line character. This behavior might cause the output that's printed to the client to appear later than it should have (compared to normal terminal execution). This issue is purely aesthetic and is not considered to be a security flaw.
  * This issue can be observed using the test script `multiple_inputs_nl.py`.
