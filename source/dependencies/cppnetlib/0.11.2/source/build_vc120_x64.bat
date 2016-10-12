rmdir build
mkdir build
cd build
cmake -G "Visual Studio 12 Win64" -DBOOST_ROOT="C:\boost_1_60_0" -DOPENSSL_LIBRARIES=..\..\..\..\openssl\1.0.2h\libs\win64\vc120_x64_release -DOPENSSL_INCLUDE_DIR=..\..\..\..\openssl\1.0.2h\include ..
cd..