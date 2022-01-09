
OPENSSL:=openssl
UBUNTU_CERT_DIR:=/usr/local/share/ca-certificates
CENTOS_CERT_DIR:=/etc/pki/ca-trust/source/anchors
MAC_CERT_KEYCHAIN:=/Library/Keychains/System.keychain

CA_NAME:=self-ca
CA_CERT:=ca.crt
CA_KEY:=ca.key
CA_SRL:=ca.srl
CA_PEM:=ca.pem

SERVER_NAME:=serv
SERVER_CERT:=server.crt
SERVER_KEY:=server.key
SERVER_CSR:=server.csr
SERVER_PEM:=server.pem
SERVER_P12:=server.p12

# Based on https://gist.github.com/fntlnz/cf14feb5a46b2eda428e000157447309

# To test the server certificate, run:
# 	openssl s_client -connect <ip>:<port> -tls1_3
# 	-tls1_3 can be replace with: -tls1_2 / -tls1_1 / -tls1
#
# 	Check that the return code is always 0: "Verify return code: 0 (ok)"
#
# To test a client, run:
# 	openssl s_server -key server.key -cert server.crt --accept <port> -tls1_3


.PHONY: generate clean add remove

generate:
	# Generate CA RSA Key
	$(OPENSSL) genrsa -out $(CA_KEY) 4096
	
	# Generate self-signed CA
	$(OPENSSL) req -x509 -new -nodes -subj "/CN=$(CA_NAME)" \
		   -key $(CA_KEY) -sha384 -out $(CA_CERT)
	
	# Generate server RSA key
	$(OPENSSL) genrsa -out $(SERVER_KEY) 4096
	
	# Create the signing request
	$(OPENSSL) req -new -subj "/CN=$(SERVER_NAME)" \
		   -key $(SERVER_KEY) -out $(SERVER_CSR)
	
	# Generate server certificate
	$(OPENSSL) x509 -req -in $(SERVER_CSR) -CA $(CA_CERT) -CAkey $(CA_KEY) \
		   -CAcreateserial -out $(SERVER_CERT) -sha384
	
	# Generate PEM files
	$(OPENSSL) x509 -inform PEM -in $(SERVER_CERT) > $(SERVER_PEM)
	$(OPENSSL) x509 -inform PEM -in $(CA_CERT) > $(CA_PEM)

	# Generate P12
	$(OPENSSL) pkcs12 -export -out $(SERVER_P12) -inkey $(SERVER_KEY) -in $(SERVER_CERT) -certfile $(CA_CERT)


clean:
	rm $(CA_CERT) $(CA_KEY) $(CA_SRL) $(CA_PEM) $(SERVER_CERT) $(SERVER_KEY) $(SERVER_CSR) $(SERVER_PEM) $(SERVER_P12)

install: install_ubuntu

install_ubutnu:	$(CA_CERT)
	cp $(CA_CERT) $(UBUNTU_CERT_DIR)/$(CA_CERT)
	update-ca-certificates

install_mac:	$(CA_CERT)
	sudo security add-trusted-cert -d -r trustRoot -k $(MAC_CERT_KEYCHAIN) $(CA_CERT)

install_centos:	$(CA_CERT)
	yum install ca-certificates -y
	update-ca-trust force-enable
	cp $(CA_CERT) $(CENTOS_CERT_DIR)/$(CA_CERT)
	update-ca-trust extract

remove: $(UBUNTU_CERT_DIR)/$(CA_CERT)
	rm $(UBUNTU_CERT_DIR)/$(CA_CERT)
	update-ca-certificates --fresh
