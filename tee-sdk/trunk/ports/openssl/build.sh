#!/bin/sh

if [ -z "$HOST" ]
then
	HOST=i586-tsvc
fi

if [ -z "$PREFIX" ]
then
	PREFIX=/usr/local/$HOST/usr
fi

if [ -z "$CC" ]
then
	CC=${HOST}-cc
fi
export CC

CFLAGS="-DOPENSSL_NO_DGRAM -DOPENSSL_NO_SOCK"

cd openssl-1.0.0d
./Configure cc --prefix="$PREFIX" no-threads no-zlib no-shared no-sse2 no-dso no-hw $CFLAGS
make
sudo make install

