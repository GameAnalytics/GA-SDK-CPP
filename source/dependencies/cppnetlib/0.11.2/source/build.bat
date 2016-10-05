rmdir output
mkdir output
cd output
cmake -G "Visual Studio 14" -DBOOST_ROOT="C:\boost_1_60_0" -DOPENSSL_LIBRARIES=..\..\..\..\openssl\1.0.2h\libs\win64 -DOPENSSL_INCLUDE_DIR=..\..\..\..\openssl\1.0.2h\include64 ..
cd..