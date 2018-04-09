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

        Json::Value GAStore::executeQuerySync(const std::string& sql)
        {
            return executeQuerySync(sql, {});
        }


        Json::Value GAStore::executeQuerySync(const std::string& sql, const std::vector<std::string>& parameters)
        {
            return executeQuerySync(sql, parameters, false);
        }

        Json::Value GAStore::executeQuerySync(const std::string& sql, const std::vector<std::string>& parameters, bool useTransaction)
        {
            // Force transaction if it is an update, insert or delete.
            if (utilities::GAUtilities::stringMatch(utilities::GAUtilities::uppercaseString(sql), "^(UPDATE|INSERT|DELETE)"))
            {
                useTransaction = true;
            }

            // Get database connection from singelton sharedInstance
            sqlite3 *sqlDatabasePtr = sharedInstance()->getDatabase();

            // Create mutable array for results
            Json::Value results(Json::arrayValue);

            if (useTransaction)
            {
                if (sqlite3_exec(sqlDatabasePtr, "BEGIN;", 0, 0, 0) != SQLITE_OK)
                {
                    logging::GALogger::e(std::string("SQLITE3 BEGIN ERROR: ") + sqlite3_errmsg(sqlDatabasePtr));
                    return{};
                }
            }

            // Create statement
            sqlite3_stmt *statement;

            // Prepare statement
            if (sqlite3_prepare_v2(sqlDatabasePtr, sql.c_str(), -1, &statement, nullptr) == SQLITE_OK)
            {
                // Bind parameters
                if (!parameters.empty())
                {
                    unsigned parametersCount = parameters.size();
                    for (unsigned index = 0; index < parametersCount; index++)
                    {
                        sqlite3_bind_text(statement, static_cast<int>(index + 1), parameters[index].c_str(), -1, 0);
                    }
                }

                // get columns count
                int columnCount = sqlite3_column_count(statement);

                // Loop through results
                while (sqlite3_step(statement) == SQLITE_ROW)
                {
                    Json::Value row;
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
                            row[column] = utilities::GAUtilities::parseString<int>(value);
                            break;
                        case SQLITE_FLOAT:
                            row[column] = utilities::GAUtilities::parseString<double>(value);
                            break;
                        default:
                            row[column] = value;
                        }

                        //row[column] = value;
                    }
                    results.append(row);
                }
            }
            else
            {
                // TODO(nikolaj): Should we do a db validation to see if the db is corrupt here?
                logging::GALogger::e(std::string("SQLITE3 PREPARE ERROR: ") + sqlite3_errmsg(sqlDatabasePtr));
                results.clear();
            }


            // Destroy statement
            if (sqlite3_finalize(statement) == SQLITE_OK)
            {
                if (useTransaction)
                {
                    if (sqlite3_exec(sqlDatabasePtr, "COMMIT", 0, 0, 0) != SQLITE_OK)
                    {
                        logging::GALogger::e(std::string("SQLITE3 COMMIT ERROR: ") + sqlite3_errmsg(sqlDatabasePtr));
                        results.clear();
                    }
                }
            }
            else
            {
                logging::GALogger::d(std::string("SQLITE3 FINALIZE ERROR: ") + sqlite3_errmsg(sqlDatabasePtr));
                results.clear();
                if (useTransaction)
                {
                    if (sqlite3_exec(sqlDatabasePtr, "ROLLBACK", 0, 0, 0) != SQLITE_OK)
                    {
                        logging::GALogger::e(std::string("SQLITE3 ROLLBACK ERROR: ") + sqlite3_errmsg(sqlDatabasePtr));
                    }
                }
            }

            // Return results
            return results;
        }

        sqlite3* GAStore::getDatabase()
        {
            return sqlDatabase;
        }

        bool GAStore::ensureDatabase(bool dropDatabase, const std::string& key)
        {
            // lazy creation of db path
            if(sharedInstance()->dbPath.empty())
            {
#if USE_UWP
                std::string p(device::GADevice::getWritablePath() + "\\ga.sqlite3");
                sharedInstance()->dbPath = p;
#elif USE_TIZEN
                std::string p(device::GADevice::getWritablePath() + utilities::GAUtilities::getPathSeparator() + "ga.sqlite3");
                sharedInstance()->dbPath = p;
#else
                std::string d(device::GADevice::getWritablePath() + utilities::GAUtilities::getPathSeparator() + key);
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

            auto results = GAStore::executeQuerySync(sql_ga_events);

            if (results.isNull())
            {
                return false;
            }

            if (GAStore::executeQuerySync("SELECT status FROM ga_events LIMIT 0,1").isNull())
            {
                logging::GALogger::d("ga_events corrupt, recreating.");
                GAStore::executeQuerySync("DROP TABLE ga_events");
                results = GAStore::executeQuerySync(sql_ga_events);
                if (results.isNull())
                {
                    logging::GALogger::w("ga_events corrupt, could not recreate it.");
                    return false;
                }
            }

            results = GAStore::executeQuerySync(sql_ga_session);

            if (results.isNull())
            {
                return false;
            }

            if (GAStore::executeQuerySync("SELECT session_id FROM ga_session LIMIT 0,1").isNull())
            {
                logging::GALogger::d("ga_session corrupt, recreating.");
                GAStore::executeQuerySync("DROP TABLE ga_session");
                results = GAStore::executeQuerySync(sql_ga_session);
                if (results.isNull())
                {
                    logging::GALogger::w("ga_session corrupt, could not recreate it.");
                    return false;
                }
            }

            results = GAStore::executeQuerySync(sql_ga_state);
            if (results.isNull())
            {
                return false;
            }

            if (GAStore::executeQuerySync("SELECT key FROM ga_state LIMIT 0,1").isNull())
            {
                logging::GALogger::d("ga_state corrupt, recreating.");
                GAStore::executeQuerySync("DROP TABLE ga_state");
                results = GAStore::executeQuerySync(sql_ga_state);
                if (results.isNull())
                {
                    logging::GALogger::w("ga_state corrupt, could not recreate it.");
                    return false;
                }
            }

            results = GAStore::executeQuerySync(sql_ga_progression);
            if (results.isNull())
            {
                return false;
            }

            if (GAStore::executeQuerySync("SELECT progression FROM ga_progression LIMIT 0,1").isNull())
            {
                logging::GALogger::d("ga_progression corrupt, recreating.");
                GAStore::executeQuerySync("DROP TABLE ga_progression");
                results = GAStore::executeQuerySync(sql_ga_progression);
                if (results.isNull())
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

        void GAStore::setState(const std::string& key, const std::string& value)
        {
            if (value.empty())
            {
                std::vector<std::string> parameterArray;
                parameterArray.push_back(key);
                executeQuerySync("DELETE FROM ga_state WHERE key = ?;", parameterArray);
            }
            else
            {
                std::vector<std::string> parameterArray;
                parameterArray.push_back(key);
                parameterArray.push_back(value);
                executeQuerySync("INSERT OR REPLACE INTO ga_state (key, value) VALUES(?, ?);", parameterArray, true);
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
                Json::Value resultSessionArray = executeQuerySync("SELECT session_id, Max(client_ts) FROM ga_events GROUP BY session_id ORDER BY client_ts LIMIT 3");

                if(resultSessionArray.size() > 0)
                {
                    std::string sessionDeleteString = "";

                    unsigned int i = 0;
                    for (auto result : resultSessionArray)
                    {
                        sessionDeleteString += result.asString();
                        if(i < resultSessionArray.size() - 1)
                        {
                            sessionDeleteString += ",";
                        }
                        ++i;
                    }

                    std::string deleteOldSessionsSql = "DELETE FROM ga_events WHERE session_id IN (\"" + sessionDeleteString + "\");";
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
