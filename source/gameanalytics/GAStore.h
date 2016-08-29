//
// GA-SDK-CPP
// Copyright 2015 GameAnalytics. All rights reserved.
//

#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <json/json.h>
#include "Foundation/GASingleton.h"


namespace gameanalytics
{
    namespace store
    {
        class GAStore : public GASingleton<GAStore>
        {
         public:
            GAStore();

            sqlite3* getDatabase();

            static bool ensureDatabase(bool dropDatabase);

            static void setState(const std::string& key, const std::string& value);

            static Json::Value executeQuerySync(const std::string& sql);

            static Json::Value executeQuerySync(const std::string& sql, const std::vector<std::string>& parameters);

            static Json::Value executeQuerySync(const std::string& sql, const std::vector<std::string>& parameters, bool useTransaction);

            static long long getDbSizeBytes();

            bool getTableReady();
            static bool isDbTooLargeForEvents();

         private:

            static bool trimEventTable();

            // set when calling "ensureDatabase"
            // using a "writablePath" that needs to be set into the C++ component before
            std::string dbPath;

            // local pointer to database
            sqlite3* sqlDatabase = nullptr;

            // 10 MB limit for database. Will initiate trim logic when exceeded.
            // long maxDbSizeBytes = 10485760;

            // ??
            bool dbReady = false;
            // bool to determine if tables are ensured ready
            bool tableReady = false;

            static const int MaxDbSizeBytes;
            static const int MaxDbSizeBytesBeforeTrim;
        };
    }
}
