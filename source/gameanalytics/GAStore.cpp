//
// GA-SDK-CPP
// Copyright 2018 GameAnalytics C++ SDK. All rights reserved.
//

#include "GAStore.h"
#include "GADevice.h"
#include "GAThreading.h"
#include "GALogger.h"
#include "GAUtilities.h"
#include <fstream>
#include <string.h>
#if USE_UWP
#elif USE_TIZEN
#elif _WIN32
#include <direct.h>
#else
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace gameanalytics
{
    namespace store
    {
        const int GAStore::MaxDbSizeBytes = 6291456;
        const int GAStore::MaxDbSizeBytesBeforeTrim = 5242880;

        GAStore::GAStore()
        {
        }

        bool GAStore::executeQuerySync(const char* sql)
        {
            rapidjson::Value v(rapidjson::kObjectType);
            executeQuerySync(sql, v);
            return !v.IsNull();
        }

        void GAStore::executeQuerySync(const char* sql, rapidjson::Value& out)
        {
            executeQuerySync(sql, {}, 0, out);
        }


        void GAStore::executeQuerySync(const char* sql, const char* parameters[], size_t size)
        {
            rapidjson::Value v;
            executeQuerySync(sql, parameters, size, false, v);
        }

        void GAStore::executeQuerySync(const char* sql, const char* parameters[], size_t size, rapidjson::Value& out)
        {
            executeQuerySync(sql, parameters, size, false, out);
        }

        void GAStore::executeQuerySync(const char* sql, const char* parameters[], size_t size, bool useTransaction)
        {
            rapidjson::Value v(rapidjson::kObjectType);
            executeQuerySync(sql, parameters, size, useTransaction, v);
        }

        void GAStore::executeQuerySync(const char* sql, const char* parameters[], size_t size, bool useTransaction, rapidjson::Value& out)
        {
            // Force transaction if it is an update, insert or delete.
            int arraySize = strlen(sql) + 1;
            char sqlUpper[arraySize];
            snprintf(sqlUpper, arraySize, "%s", sql);
            utilities::GAUtilities::uppercaseString(sqlUpper);
            if (utilities::GAUtilities::stringMatch(sqlUpper, "^(UPDATE|INSERT|DELETE)"))
            {
                useTransaction = true;
            }

            // Get database connection from singelton sharedInstance
            sqlite3 *sqlDatabasePtr = sharedInstance()->getDatabase();

            // Create mutable array for results
            rapidjson::Document results;
            results.SetArray();
            rapidjson::Document::AllocatorType& allocator = results.GetAllocator();

            if (useTransaction)
            {
                if (sqlite3_exec(sqlDatabasePtr, "BEGIN;", 0, 0, 0) != SQLITE_OK)
                {
                    logging::GALogger::e(std::string("SQLITE3 BEGIN ERROR: ") + sqlite3_errmsg(sqlDatabasePtr));
                    out = rapidjson::Value();
                    return;
                }
            }

            // Create statement
            sqlite3_stmt *statement;

            // Prepare statement
            if (sqlite3_prepare_v2(sqlDatabasePtr, sql, -1, &statement, nullptr) == SQLITE_OK)
            {
                // Bind parameters
                if (size > 0)
                {
                    for (size_t index = 0; index < size; index++)
                    {
                        sqlite3_bind_text(statement, static_cast<int>(index + 1), parameters[index], -1, 0);
                    }
                }

                // get columns count
                int columnCount = sqlite3_column_count(statement);

                // Loop through results
                while (sqlite3_step(statement) == SQLITE_ROW)
                {
                    rapidjson::Value row(rapidjson::kObjectType);
                    for (int i = 0; i < columnCount; i++)
                    {
                        const char *column = (const char *)sqlite3_column_name(statement, i);
                        const char *value = (const char *)sqlite3_column_text(statement, i);

                        if (!column || !value)
                        {
                            continue;
                        }

                        switch (sqlite3_column_type(statement, i))
                        {
                        case SQLITE_INTEGER:
                        {
                            rapidjson::Value v(column, allocator);
                            row.AddMember(v.Move(), utilities::GAUtilities::parseString<int>(value), allocator);
                            break;
                        }
                        case SQLITE_FLOAT:
                        {
                            rapidjson::Value v(column, allocator);
                            row.AddMember(v.Move(), utilities::GAUtilities::parseString<double>(value), allocator);
                            break;
                        }
                        default:
                        {
                            rapidjson::Value v(column, allocator);
                            rapidjson::Value v1(value, allocator);
                            row.AddMember(v.Move(), v1.Move(), allocator);
                        }
                        }

                        //row[column] = value;
                    }
                    results.PushBack(row, allocator);
                }
            }
            else
            {
                // TODO(nikolaj): Should we do a db validation to see if the db is corrupt here?
                logging::GALogger::e(std::string("SQLITE3 PREPARE ERROR: ") + sqlite3_errmsg(sqlDatabasePtr));
                out = rapidjson::Value();
                return;
            }

            // Destroy statement
            if (sqlite3_finalize(statement) == SQLITE_OK)
            {
                if (useTransaction)
                {
                    if (sqlite3_exec(sqlDatabasePtr, "COMMIT", 0, 0, 0) != SQLITE_OK)
                    {
                        logging::GALogger::e(std::string("SQLITE3 COMMIT ERROR: ") + sqlite3_errmsg(sqlDatabasePtr));
                        out = rapidjson::Value();
                        return;
                    }
                }
            }
            else
            {
                logging::GALogger::d(std::string("SQLITE3 FINALIZE ERROR: ") + sqlite3_errmsg(sqlDatabasePtr));
                results.Clear();
                if (useTransaction)
                {
                    if (sqlite3_exec(sqlDatabasePtr, "ROLLBACK", 0, 0, 0) != SQLITE_OK)
                    {
                        logging::GALogger::e(std::string("SQLITE3 ROLLBACK ERROR: ") + sqlite3_errmsg(sqlDatabasePtr));
                    }
                }
                out = rapidjson::Value();
                return;
            }

            // Return results
            out.CopyFrom(results, allocator);
        }

        sqlite3* GAStore::getDatabase()
        {
            return sqlDatabase;
        }

        bool GAStore::ensureDatabase(bool dropDatabase, const char* key)
        {
            // lazy creation of db path
            if(sharedInstance()->dbPath.empty())
            {
#if USE_UWP
                std::string p(device::GADevice::getWritablePath() + "\\ga.sqlite3");
                sharedInstance()->dbPath = p;
#elif USE_TIZEN
                std::string p(device::GADevice::getWritablePath() + std::string(utilities::GAUtilities::getPathSeparator()) + "ga.sqlite3");
                sharedInstance()->dbPath = p;
#else
                std::string d(device::GADevice::getWritablePath() + std::string(utilities::GAUtilities::getPathSeparator()) + key);
#ifdef _WIN32
                _mkdir(d.c_str());
#else
                mode_t nMode = 0733;
                mkdir(d.c_str(),nMode);
#endif
                std::string p(d + utilities::GAUtilities::getPathSeparator() + "ga.sqlite3");
                sharedInstance()->dbPath = p;
#endif
            }

            // Open database
            if (sqlite3_open(sharedInstance()->dbPath.c_str(), &sharedInstance()->sqlDatabase) != SQLITE_OK)
            {
                sharedInstance()->dbReady = false;
                logging::GALogger::w("Could not open database: " + sharedInstance()->dbPath);
                return false;
            }
            else
            {
                sharedInstance()->dbReady = true;
                logging::GALogger::i("Database opened: " + sharedInstance()->dbPath);
            }

            if (dropDatabase)
            {
                logging::GALogger::d("Drop tables");
                GAStore::executeQuerySync("DROP TABLE ga_events");
                GAStore::executeQuerySync("DROP TABLE ga_state");
                GAStore::executeQuerySync("DROP TABLE ga_session");
                GAStore::executeQuerySync("DROP TABLE ga_progression");
                GAStore::executeQuerySync("VACUUM");
            }

            // Create statements
            auto sql_ga_events = "CREATE TABLE IF NOT EXISTS ga_events(status CHAR(50) NOT NULL, category CHAR(50) NOT NULL, session_id CHAR(50) NOT NULL, client_ts CHAR(50) NOT NULL, event TEXT NOT NULL);";
            auto sql_ga_session = "CREATE TABLE IF NOT EXISTS ga_session(session_id CHAR(50) PRIMARY KEY NOT NULL, timestamp CHAR(50) NOT NULL, event TEXT NOT NULL);";
            auto sql_ga_state = "CREATE TABLE IF NOT EXISTS ga_state(key CHAR(255) PRIMARY KEY NOT NULL, value TEXT);";
            auto sql_ga_progression = "CREATE TABLE IF NOT EXISTS ga_progression(progression CHAR(255) PRIMARY KEY NOT NULL, tries CHAR(255));";

            ;

            if (!GAStore::executeQuerySync(sql_ga_events))
            {
                logging::GALogger::d("ensureDatabase failed: " + std::string(sql_ga_events));
                return false;
            }

            if (!GAStore::executeQuerySync("SELECT status FROM ga_events LIMIT 0,1"))
            {
                logging::GALogger::d("ga_events corrupt, recreating.");
                GAStore::executeQuerySync("DROP TABLE ga_events");
                if (!GAStore::executeQuerySync(sql_ga_events))
                {
                    logging::GALogger::w("ga_events corrupt, could not recreate it.");
                    return false;
                }
            }

            if (!GAStore::executeQuerySync(sql_ga_session))
            {
                return false;
            }

            if (!GAStore::executeQuerySync("SELECT session_id FROM ga_session LIMIT 0,1"))
            {
                logging::GALogger::d("ga_session corrupt, recreating.");
                GAStore::executeQuerySync("DROP TABLE ga_session");
                if (!GAStore::executeQuerySync(sql_ga_session))
                {
                    logging::GALogger::w("ga_session corrupt, could not recreate it.");
                    return false;
                }
            }

            if (!GAStore::executeQuerySync(sql_ga_state))
            {
                return false;
            }

            if (!GAStore::executeQuerySync("SELECT key FROM ga_state LIMIT 0,1"))
            {
                logging::GALogger::d("ga_state corrupt, recreating.");
                GAStore::executeQuerySync("DROP TABLE ga_state");
                if (!GAStore::executeQuerySync(sql_ga_state))
                {
                    logging::GALogger::w("ga_state corrupt, could not recreate it.");
                    return false;
                }
            }

            if (!GAStore::executeQuerySync(sql_ga_progression))
            {
                return false;
            }

            if (!GAStore::executeQuerySync("SELECT progression FROM ga_progression LIMIT 0,1"))
            {
                logging::GALogger::d("ga_progression corrupt, recreating.");
                GAStore::executeQuerySync("DROP TABLE ga_progression");
                if (!GAStore::executeQuerySync(sql_ga_progression))
                {
                    logging::GALogger::w("ga_progression corrupt, could not recreate it.");
                    return false;
                }
            }

            trimEventTable();

            sharedInstance()->tableReady = true;
            logging::GALogger::d("Database tables ensured present");

            return true;
        }

        void GAStore::setState(const char* key, const char* value)
        {
            if (strlen(value) == 0)
            {
                const char* parameterArray[1] = {key};
                executeQuerySync("DELETE FROM ga_state WHERE key = ?;", parameterArray, 1);
            }
            else
            {
                const char* parameterArray[2] = {key, value};
                executeQuerySync("INSERT OR REPLACE INTO ga_state (key, value) VALUES(?, ?);", parameterArray, 2, true);
            }
        }

        // long long is C 64 bit int
        long long GAStore::getDbSizeBytes()
        {
            std::ifstream in(sharedInstance()->dbPath, std::ifstream::ate | std::ifstream::binary);
            return in.tellg();
        }

        bool GAStore::getTableReady()
        {
            return tableReady;
        }

        bool GAStore::isDbTooLargeForEvents()
        {
            return getDbSizeBytes() > MaxDbSizeBytes;
        }


        bool GAStore::trimEventTable()
        {
            if(getDbSizeBytes() > MaxDbSizeBytesBeforeTrim)
            {
                rapidjson::Value resultSessionArray(rapidjson::kArrayType);
                executeQuerySync("SELECT session_id, Max(client_ts) FROM ga_events GROUP BY session_id ORDER BY client_ts LIMIT 3", resultSessionArray);

                if(resultSessionArray.Size() > 0)
                {
                    char sessionDeleteString[257] = "";

                    unsigned int i = 0;
                    for (rapidjson::Value::ConstValueIterator itr = resultSessionArray.Begin(); itr != resultSessionArray.End(); ++itr)
                    {
                        const rapidjson::Value& result = *itr;
                        const char* session_id = result.GetString();
                        char tmp[257] = "";

                        if(i < resultSessionArray.Size() - 1)
                        {
                            snprintf(tmp, 257, "%s%s%s", sessionDeleteString, session_id, ",");
                        }
                        else
                        {
                            snprintf(tmp, 257, "%s%s", sessionDeleteString, session_id);
                        }
                        snprintf(sessionDeleteString, 257, "%s", tmp);
                        ++i;
                    }

                    char deleteOldSessionsSql[513] = "";
                    snprintf(deleteOldSessionsSql, 513, "DELETE FROM ga_events WHERE session_id IN (\"%s\");", sessionDeleteString);
                    logging::GALogger::w("Database too large when initializing. Deleting the oldest 3 sessions.");
                    executeQuerySync(deleteOldSessionsSql);
                    executeQuerySync("VACUUM");

                    return true;
                }
                else
                {
                    return false;
                }
            }

            return true;
        }

    }
}
