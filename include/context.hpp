#pragma once

#include <vector>
#include <set>
#include <string>

namespace db {
    struct context {
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
            //bool if_exists;
        } index_t;

        typedef struct user_type {
            std::string name;
            std::string password;
            //bool if_exists;
        } user_t;

        typedef struct privilege_type {
            std::string user;
            std::string name;
        } privilege_t;

        std::string owner;

        std::set<schema_t> schemas;
        std::vector<table_t> tables;
        std::vector<index_t> indices;
        std::vector<user_t> users;
        std::vector<privilege_t> privileges;
    };

    struct config {
        bool make_transaction = true;
    };

    struct context_diff {
        typedef struct table_type {
            table_type() = default;
            table_type(const std::string &_schema, const std::string &_name, const std::string &_comment) : schema{_schema}, name{_name}, comment{_comment} {

            }

            std::string schema;
            std::string name;
            std::string comment;
        } table_t;

        typedef struct column_type {
            column_type() = default;
            column_type(const std::string &_schema, const std::string &_name) : schema{_schema}, name{_name} {

            }

            std::string schema;
            std::string name;
            std::string type;
            std::string comment;
            std::string default_value;
            //std::size_t size;
            bool primary_key = false;
            bool unique_key = false;
            bool not_null = false;
        } column_t;

        enum status_type {
            status_added,
            status_removed,
            status_changed
        };

        template <typename T, typename V>
        struct modification_t {
            modification_t() = default;
            modification_t(const T &t, const V &v) : status{t}, value{v} {

            }

            T status;
            V value;
        };

        std::vector<modification_t<status_type, table_type>> tables;
        std::vector<modification_t<status_type, column_t>> columns;
    };
}
