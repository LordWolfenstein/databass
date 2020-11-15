#include <sstream>
//#include <format>
#include <cassert>
#include <regex>

#include "Databass.h"

std::string Databass::_LeftAdjustedCell(std::string cell, int width)
{
    std::stringstream ret;
    ret.fill(' ');
    ret.width(width);
    ret << std::fixed << std::left << cell;
    return ret.str();
}

std::string Databass::_RightAdjustedCell(std::string cell, int width)
{
    std::stringstream ret;
    ret.width(width);
    ret.fill(' ');
    ret << std::fixed << std::right << cell;
    return ret.str();
}

std::string Databass::_CenteredCell(std::string cell, int width)
{
    std::stringstream ret, spaces;
    int padding = width - (int)cell.size();
    for(int i = 0; i < padding / 2; ++i)
        spaces << " ";
    ret << spaces.str() << cell << spaces.str();
    if(padding > 0 && padding % 2 != 0)
        ret << " ";
    return ret.str();
}

std::string Databass::_FillerLine(std::map<std::string, int> width)
{
    std::stringstream ret;
    ret << " ";
    for(auto &col: width)
    {
        ret << "+--";
        for(int i = 0; i < col.second; i++)
            ret << "-";
    }
    ret << "+" << std::endl;
    return ret.str();
}

std::string Databass::_DataLine(BassLine row, std::map<std::string, int> width)
{
    std::stringstream ret;
    for(auto &col: row)
    {
        // is it a number
        if(std::regex_match(col.second, std::regex("-?[0-9]+([.][0-9]+)?")))
            ret << " | " << _RightAdjustedCell(col.second, width[col.first]);
        else
            ret << " | " << _LeftAdjustedCell(col.second, width[col.first]); // It is text.
    }
    ret << " |" << std::endl;
    return ret.str();
}

int Databass::_Callback(void *data, int argc, char **argv, char **column)
{
    Answer *answer = (Answer*)data;
    BassLine line;
    int i;
    for(i = 0; i < argc; i++)
    {
        line[column[i]] = argv[i] ? argv[i] : "";
    }
    answer->list.push_back(line);

    return 0;
}

Databass::Databass(std::string db)
{
    int rc;
    rc = sqlite3_open(db.c_str(), &_db);
    if(rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(_db));
        assert(false);
    }
}

std::set<std::string> Databass::Tables(void)
{
    std::set<std::string> ret;
    Answer data = Query("SELECT `name` FROM `sqlite_master` WHERE type='table';");
    if(data.returnCode == SQLITE_OK)
    {
        for(auto &line: data.list)
        {
            if(line["name"] != "sqlite_sequence")
                ret.insert(line["name"]);
        }
    }

    return ret;
}

void Databass::Drop(std::string table)
{
    if(Tables().count(table) != 0)
    {
        Query("DROP TABLE " + table + ";");
        //Query(std::format("DROP TABLE {};", table));
    }
}

Answer Databass::Query(std::string query)
{
    Answer ret;
    char *errorMessage = NULL;
    ret.returnCode = sqlite3_exec(_db, query.c_str(), Databass::_Callback, &ret, &errorMessage);

    if(ret.returnCode != SQLITE_OK)
    {
        ret.errorMessage = errorMessage;
        sqlite3_free(errorMessage);
    }

    return ret;
}

std::string Databass::Tabulate(BassList data)
{
    std::stringstream ret;
    if(data.size() != 0)
    {
        std::map<std::string, int> width;
        for(auto &col: data[0])
            width[col.first] = (int)col.first.length();

        for(auto &row: data)
        {
            for(auto &key: width)
            {
                if(row[key.first].length() > key.second)
                    key.second = (int)row[key.first].length();
            }
        }

        ret << _FillerLine(width);
        // print headers
        for(auto &col: data[0])
            ret << " | " << _CenteredCell(col.first, width[col.first]);
        ret << " |" << std::endl;
        ret << _FillerLine(width);
        // print data
        for(auto &row: data)
            ret<< _DataLine(row, width);
        ret << _FillerLine(width);
    }
    return ret.str();
}

Databass::~Databass(void)
{
    sqlite3_close(_db);
}
