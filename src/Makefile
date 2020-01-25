.PHONY: all

all: urchub urc-udpsend urc-udprecv ucspi-stream urchubstream cryptoserv urcstream2hub ucspi-client2server ucspi-server2client ucspi-socks4aclient keypair sign_keypair liburc.so nacltaia.so taia96n.so urccache

urchub: urchub.c

urccache: LDLIBS=-lnacl -lrandombytes -ltai
urccache: urccache.c

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

liburc.so:
	gcc -O2 -fPIC -DPIC liburc.c -shared `python-config --includes` -o liburc.so `python-config --libs` -l tai -lnacl
nacltaia.so:
	gcc -O2 -fPIC -DPIC nacltaia.c -shared `python-config --includes` -o nacltaia.so `python-config --libs` -l tai -l nacl -l randombytes

taia96n.c: taia96n.pyx
	cython taia96n.pyx -o taia96n.c

taia96n.so: taia96n.c
	gcc -O2 -shared -pthread -fPIC -fwrapv -Wall -fno-strict-aliasing `python-config --includes` taia96n.c -o taia96n.so

#gcc $src/check-taia.c -o check-taia -l tai -l $nacl || exit 1
#gcc -O2 -c build/urcd.c $PYTHON_INCLUDE -o build/urcd.o || exit 1
#gcc -O1 -o urcd build/urcd.o $PYTHON_LIBRARY || exit 1
#gcc -O2 -c build/urc2sd.c $PYTHON_INCLUDE -o build/urc2sd.o || exit 1
#gcc -O1 -o urc2sd build/urc2sd.o $PYTHON_LIBRARY || exit 1
