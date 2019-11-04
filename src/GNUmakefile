.PHONY: all

all: urchub urc-udpsend urc-udprecv ucspi-stream urchubstream cryptoserv urcstream2hub ucspi-client2server ucspi-server2client ucspi-socks4aclient keypair sign_keypair

urchub: urchub.c

urc-udpsend: urc-udpsend.c

urc-udprecv: urc-udprecv.c

ucspi-stream: ucspi-stream.c

urchubstream: urchubstream.c

cryptoserv: LDLIBS=-lnacl
cryptoserv: cryptoserv.c

urcstream2hub: LDLIBS=-ltai
urcstream2hub: urcstream2hub.c

ucspi-client2server: ucspi-client2server.c

ucspi-server2client: ucspi-server2client.c

ucspi-socks4aclient: ucspi-socks4aclient.c

keypair: LDLIBS=-lnacl -lrandombytes
keypair: keypair.c

sign_keypair: LDLIBS=-lnacl -lrandombytes
sign_keypair: sign_keypair.c

#gcc -O2 -fPIC -DPIC $src/liburc.c -shared $PYTHON_INCLUDE -o liburc.so $PYTHON_LIBRARY -l tai -l $nacl || exit 1
#gcc -O2 -fPIC -DPIC $src/nacltaia.c -shared $PYTHON_INCLUDE -o nacltaia.so $PYTHON_LIBRARY -l tai -l $nacl $randombytes || exit 1
#gcc `cat conf-cc` $src/check-taia.c -o check-taia -l tai -l $nacl || exit 1
#gcc `cat conf-cc` -O2 -c build/urcd.c $PYTHON_INCLUDE -o build/urcd.o || exit 1
#gcc `cat conf-cc` -O1 -o urcd build/urcd.o $PYTHON_LIBRARY || exit 1
#gcc `cat conf-cc` -O2 -c build/urc2sd.c $PYTHON_INCLUDE -o build/urc2sd.o || exit 1
#gcc `cat conf-cc` -O1 -o urc2sd build/urc2sd.o $PYTHON_LIBRARY || exit 1
#gcc `cat conf-cc` -O2 -shared -pthread -fPIC -fwrapv -Wall -fno-strict-aliasing $PYTHON_INCLUDE build/taia96n.c -o taia96n.so || exit 1
