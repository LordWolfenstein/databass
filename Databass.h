#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <winsqlite/winsqlite3.h>
#include <sstream>

typedef std::map<std::string, std::string> BassLine;
typedef std::vector<BassLine> BassList;

struct Answer
{
    BassList list;
    int returnCode = 0;
    std::string errorMessage;
};

//  Databass - SQLite for humans.
//
//  An interface for making SQLite actually useful.
//  All returned queries are a vector of maps.
//  All data returned are strings. Just convert to numbers yourself.
//
class Databass
{
private:
    static std::string _LeftAdjustedCell(std::string cell, int width);
    static std::string _RightAdjustedCell(std::string cell, int width);
    static std::string _CenteredCell(std::string cell, int width);
    static std::string _FillerLine(std::map<std::string, int> width);
    static std::string _DataLine(BassLine row, std::map<std::string, int> width);

    sqlite3 *_db;

    // Query backend callback function
    static int _Callback(void *data, int argc, char **argv, char **column);

public:
    // db: SQLite database file.
    Databass(std::string db);

    // Return a set of the tables in the database.
    std::set<std::string> Tables(void);

    // Drops a table
    void Drop(std::string table);

    // Executes a query
    Answer Query(std::string query);

    // print a pretty ASCII table
    static std::string Tabulate(BassList data);

    ~Databass(void);
};
