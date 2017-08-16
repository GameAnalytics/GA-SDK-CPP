#/bin/sh -f

# linux stuff
#export CFLAGS="-m32"
#export LDEMULATION=elf_i386
# --host=i686-pc-linux-gnu (configure)

# mac stuff

#32-bit
#CFLAGS='-m32 -arch i386' LDFLAGS='-arch i386' PKG_CONFIG_PATH=/path/to/openssl/pkgconfig ./configure --host=i386-apple --disable-shared --enable-static --disable-dependency-tracking --enable-ipv6 --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp --disable-gopher --disable-sspi --disable-manual --disable-zlib --without-zlib --with-ssl --prefix=$PWD

# 64-bit
PKG_CONFIG_PATH=/path/to/openssl/pkgconfig ./configure --disable-shared --enable-static --disable-dependency-tracking --enable-ipv6 --disable-ftp --disable-file --disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict --disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp --disable-gopher --disable-sspi --disable-manual --disable-zlib --without-zlib --with-ssl --prefix=$PWD

make
make install
