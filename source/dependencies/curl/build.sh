#/bin/sh -f

# linux stuff
#export CFLAGS="-m32"
#export LDEMULATION=elf_i386
# --host=i686-pc-linux-gnu (configure)

# mac stuff
#export CFLAGS="-O -g -arch i386"
./configure --disable-shared --enable-static \
--disable-dependency-tracking --enable-ipv6 --disable-ftp --disable-file \
--disable-ldap --disable-ldaps --disable-rtsp --disable-proxy --disable-dict \
--disable-telnet --disable-tftp --disable-pop3 --disable-imap --disable-smtp \
--disable-gopher --disable-sspi --disable-manual --disable-zlib --without-zlib --with-ssl --prefix=$PWD

make
make install
