#/bin/sh -f

./configure --disable-shared --enable-static \
--disable-dependency-tracking --enable-ipv6 --disable-ftp --disable-file \
--disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict \
--disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp \
--disable-gopher --disable-sspi --disable-manual --disable-zlib --without-zlib --with-ssl --prefix=$PWD

make
make install
