#pragma once

#include <vector>
#include <set>
#include <string>

struct db_context {
    typedef std::set<std::string> primary_key_t;

    typedef struct foreign_key_type {
        std::string ref_table;
        std::string ref_schema;
        std::vector<std::string> columns;
        std::vector<std::string> ref_columns;
    } foreign_key_t;

    typedef std::vector<foreign_key_t> foreign_keys_t;

    typedef struct column_type {
        std::string name;
        std::string type;
        std::string comment;
        std::string default_value;
        //std::size_t size;
        bool primary_key = false;
        bool unique_key = false;
        bool not_null = false;
    } column_t;

    typedef struct table_type {
        std::string schema_name;
        std::string table_name;
        std::string comment;
        std::vector<column_t> columns;
        primary_key_t primary_key;
        std::vector<foreign_key_t> foreign_keys;
    } table_t;

    typedef std::string schema_t;

    typedef struct index_type {
        std::string schema_name;
        std::string table_name;
        std::string name;
        std::string column_name;
        //bool        if_exists;
    } index_t;

    std::string owner;

    std::set<schema_t> schemas;
    std::vector<table_t> tables;
    std::vector<index_t> indices;
};

struct db_config {
    bool make_transaction = true;
};
