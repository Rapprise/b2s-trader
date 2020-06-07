**B2S Trader**:
Application to automate trading process.

**Platforms**:
Windows
Linux
macOS


**Getting Started:**

**Windows**:  
**Pre Requirements**:  
1. Install VS2017  
2. Clone b2s trader from repository. It will be ROOT_DIR.  
3. Create folder 'third-party' inside root directory and download next libraries there.  
4. Donwload [OpenSSL](https://github.com/pocoproject/openssl/tree/master) library.  
5. Download [POCO](https://pocoproject.org/download.html) library.  
6. Download [Qt](https://www.qt.io/download) library.  
7. Download [SQlite](https://www.sqlite.org/download.html) library.
8. Download [CULR](https://curl.haxx.se/download.html) library

**Build third-party libraries:**  
1. Compile OpenSSL library according to the documentation with proper version of msvc. Run buildall.cmd with proper vs_version(VS2017 - 150).  
2. Copy OpenSSL binaries to 'ROOTDIR/third-party/poco/openssl' directory.  
3. Compile Poco library with buildwin.cmd with proper vs_version(VS2017 - 150). Create 'Poco_installed' directory inside 'third-party' and copy directories 'lib64, lib, bin, bin64, include' from poco library.  
4. Install Qt libraries.  
5. Create a folder sqlite3 in 'ROOTDIR/third-party' with include and lib directories.  
6. Copy sqlite3.h and sqlite3.lib to those dirs.(sqlite3.lib can be generated from *.def file using msvc compiler). Also copy sqlite3.dll to sqlite3 folder.
7. Compile Curl library according to the documentation.
7. Add 'ROOTDIR/third-party/binaries' folder to environment path.
8. Create '_build' folder inside ROOT_DIR.  


**CMake dependencies:**  
Folders: 'ROOTDIR/third-party/Qt', 'ROOTDIR/third-party/Poco' should be added to environment variables.
Create '_build' folder in ROOTDIR and run cmake with next parameters:  
OPENSSL_INCLUDE_DIR = (ROOTDIR/third-party/poco/openssl/build/include)  
OPENSSL_LIB_DIR = (ROOTDIR/third-party/poco/openssl/build/win64/lib)  
Poco_INCLUDE_DIRS = (ROOTDIR/third-party/Poco_installed/include)  
POCO_LIB_DIR = (ROOTDIR/third-party/Poco_installed/lib)  
SQLITE3_INCLUDE_DIRS = (ROOTDIR/third-party/sqlite3/include)  
SQLITE_LIB = (ROOTDIR/third-party/sqlite3/lib/sqlite3.lib)
CURL_INCLUDE_DIR = (ROOTDIR/third-party/curl/include)
CURL_LIBRARIES = (ROOTDIR/third-party/curl/builds/libcurl-vc-x64-debug-dll-ipv6-sspi-winssl/lib/libcurl_debug.lib)
GTEST_DIR = (ROOTDIR/third-party/googletest)  
gtest_force_shared_crt - ON (enable shared libs for gtests)  
BINARY_DEPEND_PATH = (ROOTDIR/third-party/binaries)  
ENABLE_TESTS - ON (enable all unit tests)
CMAKE_BUILD_TYPE = Debug or Release(depends on what you need)


Go to '_build' directory.  
Run CMake(example):
cmake -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -DBINARY_DEPEND_PATH=%cd%/../third-party/bin -Dgtest_force_shared_crt=ON -DBUILD_GMOCK=ON -DGTEST_DIR=%cd%/../third-party/googletest -DOPENSSL_LIB_DIR=%cd%/../third-party/poco/openssl/build/win64/lib -DOPENSSL_INCLUDE_DIR=%cd%/../third-party/poco/openssl/build/include -DPoco_INCLUDE_DIRS=%cd%/../third-party/Poco_installed/include -DPOCO_LIB_DIR=%cd%/../third-party/Poco_installed/lib64 -DSQLITE3_INCLUDE_DIRS=%cd%/../third-party/sqlite3/include -DSQLITE_LIB=%cd%/../third-party/sqlite3/lib/sqlite3.lib .. -DLIB_EAY_DEBUG=%cd%/../third-party/poco/lib64/libcryptod.lib -DLIB_EAY_RELEASE=%cd%/../third-party/poco/lib64/libcrypto.lib -DSSL_EAY_DEBUG=%cd%/../third-party/poco/lib64/libssld.lib -DSSL_EAY_RELEASE=%cd%/../third-party/poco/lib64/libssl.lib ..  

Compile B2S trader.  
Before running any target all *.dll from ${BINARY_DEPEND_PATH} should be copied to the CMAKE_CURRENT_BINARY_DIR(folder with all build files).  

**Linux or macOS**:
**Pre Requirements**:  
1. [sudo] apt-get install autoconf automake libtool build-essential g++6 gcc.
2. Download [Poco](https://pocoproject.org/download.html) compile and install.
3. Download [OpenSSL](https://www.openssl.org/source/) 1.1.0k compile and install.
4. Install SQlite library [sudo apt-get install libsqlite3-dev]
5. Download and install [Qt](https://www.qt.io/download) library.
6. Download [GTest](https://github.com/google/googletest) compile and install.
7. Download [CULR](https://curl.haxx.se/download.html) library

**Compile B2S Trader**
1. Create directory 'build'.
2. Go to 'build' directory.
3. Run CMake with all dependencies.

**B2S Main targets**:
ALL_BUILD - target to compile all modules.
auto_trader - target to compile b2s trader only.  
all_tests - target to compile and run unit tests in all modules.

