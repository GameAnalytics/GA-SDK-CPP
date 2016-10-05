rmdir build
mkdir build
cd build
cmake -G "Visual Studio 12" -DBOOST_ROOT="C:\boost_1_60_0" -DOPENSSL_LIBRARIES=..\..\..\..\openssl\1.0.2h\libs\win32\vc120_x86_release -DOPENSSL_INCLUDE_DIR=..\..\..\..\openssl\1.0.2h\include ..
cd..