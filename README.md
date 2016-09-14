GA-SDK-CPP
==========

GameAnalytics C++ SDK

Supported platforms:

* Mac OS X
* Winddows 32-bit and 64-bit

Dependencies
------------

* python 2.7 or higher (not working for python 3)
  * **Windows:** Go [here](https://www.python.org/downloads/) to download and install Python
  * **Mac:** Should come with Python out of the box or else run *'brew install python'*
* cmake (will be automatically downloaded)
* **Mac:** XCode
* **Windows:** Visual Studio

How to build
------------

To start a build for all supported targets simply call

**Mac: ./build.sh**

**Windows: build.bat**

Or you can run the actual build script directly found at **build/jenkins/main.py** or call **build/jenkins/main.py -h** to display the usage of the script.

How to develop
--------------

If you developing the SDK, you can use the generated project files to work with.

They are located under **build/jenkins/build/[target]** after calling the **build.sh** or **build.bat**.


Folderstructure
---------------

* **build** - Contains scripts for automated project setup, building and creating of the final exstracted library
* **build/cmake** - Contains the cmake files for the project
* **build/jenkins** - Contains the scripts and working directories for the automated builds
* **export** - Target folder for the automated export of the GA lib
* **source** - Contains the complete source code for the project including the dependencies
* **tests** - Contains tests for testing the functionality in the GA SDK, to run tests run **tests/run_tests_osx.py** (mac only)

Lib Dependencies
----------------

* **boost** (*as binary*) - C++ library used for threading, regex and logging.
* **cpp-netlib** (*as binary*) - C++ library build upon latest C++ standard used for HTTP requests.
* **cryptoC++** (*as source*) - collection of functions and classes for cryptography related tasks.
* **jsonCpp** (*as source*) - lightweight C++ library for manipulating JSON values including serialization and deserialization.
* **openssl** (*as binary*) - used by **cpp-netlib** to make HTTPS requests
* **SQLite** (*as source*) - SQLite is a software library that implements a self-contained, serverless, zero-configuration, transactional SQL database engine.

*as source* means the dependency will be compiled with the project itself, *as binary* means the dependency is prebuild and will be linked to the project

Preperation before usage of the SDK
-----------------------------------

* Include the following directories in your C++ project's include paths:
  * **source/gameanalytics**

* In your C++ project, link to the following libraries:
  * All libraries in **source/dependencies/boost/1.60.0/libs/[target]**
  * All libraries in **source/dependencies/cppnetlib/0.11.2/libs/[target]**
  * All libraries in **source/dependencies/openssl/1.0.2h/libs/[target]**
  * Produced library from build script found in **export/[target]-static/Release/**


Usage of the SDK
----------------

Remember to include the GameAnalytics header file wherever you are using the SDK:

``` c++
 #include "GameAnalytics.h"
```

###Configuration

Example:

``` c++
 gameanalytics::GameAnalytics::setEnabledInfoLog(true);
 gameanalytics::GameAnalytics::setEnabledVerboseLog(true);

 gameanalytics::GameAnalytics::configureBuild("0.10");

 {
     std::vector<std::string> list;
     list.push_back("gems");
     list.push_back("gold");
     gameanalytics::GameAnalytics::configureAvailableResourceCurrencies(list);
 }
 {
     std::vector<std::string> list;
     list.push_back("boost");
     list.push_back("lives");
     gameanalytics::GameAnalytics::configureAvailableResourceItemTypes(list);
 }
 {
     std::vector<std::string> list;
     list.push_back("ninja");
     list.push_back("samurai");
     gameanalytics::GameAnalytics::configureAvailableCustomDimensions01(list);
 }
 {
     std::vector<std::string> list;
     list.push_back("whale");
     list.push_back("dolphin");
     gameanalytics::GameAnalytics::configureAvailableCustomDimensions02(list);
 }
 {
     std::vector<std::string> list;
     list.push_back("horde");
     list.push_back("alliance");
     gameanalytics::GameAnalytics::configureAvailableCustomDimensions03(list);
 }
```

###Initialization

Example:

``` c++
 gameanalytics::GameAnalytics::initialize("<your game key>", "<your secret key");
```

###Send events

Example:

``` c++
 gameanalytics::GameAnalytics::addDesignEvent("testEvent");
 gameanalytics::GameAnalytics::addBusinessEvent("USD", 100, "boost", "super_boost", "shop");
 gameanalytics::GameAnalytics::addResourceEvent(gameanalytics::Source, "gems", 10, "lives", "extra_life");
 gameanalytics::GameAnalytics::addProgressionEvent(gameanalytics::Start, "progression01", "progression02");
```

Changelog
---------
**1.0.1**
* fix for empty user id in events

**1.0.0**
* Initial version with Windows and Mac OS X support
