//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include "rapidjson/document.h"
#include "Foundation/GASingleton.h"
#include "GameAnalytics.h"

namespace gameanalytics
{
    namespace store
    {
        class GAStore : public GASingleton<GAStore>
        {
         public:
            GAStore();

            sqlite3* getDatabase();

            static bool ensureDatabase(bool dropDatabase, const char* key = "");

            static void setState(const char* key, const char* value);

            static bool executeQuerySync(const char* sql);
            static void executeQuerySync(const char* sql, rapidjson::Value& out);

            static void executeQuerySync(const char* sql, const char* parameters[], size_t size);
            static void executeQuerySync(const char* sql, const char* parameters[], size_t size, rapidjson::Value& out);

            static void executeQuerySync(const char* sql, const char* parameters[], size_t size, bool useTransaction);
            static void executeQuerySync(const char* sql, const char* parameters[], size_t size, bool useTransaction, rapidjson::Value& out);

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
