#!/bin/bash

OPENSSL_VERSION="1.0.2h"

curl --insecure -O https://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz
tar -xvzf openssl-$OPENSSL_VERSION.tar.gz
mv openssl-$OPENSSL_VERSION openssl_i386
tar -xvzf openssl-$OPENSSL_VERSION.tar.gz
mv openssl-$OPENSSL_VERSION openssl_x86_64
cd openssl_i386
# mac
./Configure darwin-i386-cc -static

# linux
#./Configure -m32 linux-generic32 -static
make
cd ../
cd openssl_x86_64
# mac
./Configure darwin64-x86_64-cc -static

# linux
#./Configure -m64 linux-generic64 -static
make
cd ../
lipo -create openssl_i386/libcrypto.a openssl_x86_64/libcrypto.a -output libcrypto.a
lipo -create openssl_i386/libssl.a openssl_x86_64/libssl.a -output libssl.a
rm openssl-$OPENSSL_VERSION.tar.gz
