
OPENSSL:=openssl
UBUNTU_CERT_DIR:=/usr/local/share/ca-certificates

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

# To test the server certificate, run:
# 	openssl s_client -connect <ip>:<port> -tls1_3
# 	-tls1_3 can be replace with: -tls1_2 / -tls1_1 / -tls1
#
# 	Check that the return code is always 0: "Verify return code: 0 (ok)"


.PHONY: generate clean add remove

generate:
	# Generate self-signed CA
	$(OPENSSL) req -x509 -newkey rsa-pss -pkeyopt rsa_keygen_bits:2048 \
	  -subj "/CN=$(CA_NAME)" -sha384 -nodes -keyout $(CA_KEY) -out $(CA_CERT)
	
	# Generate server end entity certificate
	$(OPENSSL) req -newkey rsa-pss -pkeyopt rsa_keygen_bits:2048 \
	  -subj "/CN=$(SERVER_NAME)" -sha384 -nodes -keyout $(SERVER_KEY) -out $(SERVER_CSR)
	
	$(OPENSSL) x509 -req -CAcreateserial -in $(SERVER_CSR) -sha384 -CA $(CA_CERT) -CAkey $(CA_KEY) -out $(SERVER_CERT)
	$(OPENSSL) x509 -inform PEM -in $(SERVER_CERT) > $(SERVER_PEM)
	$(OPENSSL) x509 -inform PEM -in $(CA_CERT) > $(CA_PEM)


clean:
	rm $(CA_CERT) $(CA_KEY) $(CA_SRL) $(CA_PEM) $(SERVER_CERT) $(SERVER_KEY) $(SERVER_CSR) $(SERVER_PEM)

add:	$(CA_CERT)
	cp $(CA_CERT) $(UBUNTU_CERT_DIR)/$(CA_CERT)
	update-ca-certificates

remove: $(UBUNTU_CERT_DIR)/$(CA_CERT)
	rm $(UBUNTU_CERT_DIR)/$(CA_CERT)
	update-ca-certificates --fresh

