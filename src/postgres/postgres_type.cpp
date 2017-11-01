#include <strex.hpp>

#include "context.hpp"
#include <launchdb/types.hpp>

namespace postgres {
    using namespace launchdb;

    std::string to_type(const db::column_value_type &c) {
        using namespace std;
        string type_name = c.type_name;

        switch (strex::hash(type_name)) {
        case strex::hash(types::int_type):
            switch (c.size) {
            case 0: // default size
                type_name = c.primary_key ? "SERIAL" : "INTEGER";
                break;
            case 8:
                break;
            case 16:
                type_name = "SMALLINT";
                break;
            case 32:
                type_name = c.primary_key ? "SERIAL" : "INTEGER";
                break;
            case 64:
                type_name = c.primary_key ? "BIGSERIAL" : "BIGINT";
                break;
            }
            break;
        case strex::hash(types::str_type):
            type_name = "TEXT";
            break;
        case strex::hash(types::text_type):
            type_name = "TEXT";
            break;
        case strex::hash(types::bool_type):
            type_name = "BOOLEAN";
            break;
        case strex::hash(types::bytes_type):
            type_name = "bytea";
            break;
        case strex::hash(types::time_type):
            type_name = "timestamp";
            break;
        case strex::hash(types::decimal_type):
            type_name = "DECIMAL(12, 2)";
            break;
        }

        if (c.unique_key)
            type_name += " UNIQUE";
        if (c.not_null)
            type_name += " NOT NULL";
        if (!c.default_value.empty())
            type_name += " DEFAULT " + c.default_value;

        return type_name;
    }

    db::column_value_type from_type(const std::string &t, const size_t size, const std::string def, const bool _primary_key, const bool _unique_key, const bool _not_null) {
        const auto def_value = _primary_key ? std::string{} : def;

        if (t == "integer")
            return db::column_value_type{"int", size, def_value, _primary_key, _unique_key, _not_null};

        if (t == "text")
            return db::column_value_type{"text", size, def_value, _primary_key, _unique_key, _not_null};

        if (t == "bytea")
            return db::column_value_type{"buffer", size, def_value, _primary_key, _unique_key, _not_null};

        if (t == "numeric")
            return db::column_value_type{"decimal", size, def_value, _primary_key, _unique_key, _not_null};

        if (t == "boolean")
            return db::column_value_type{"bool", size, def_value, _primary_key, _unique_key, _not_null};

        return db::column_value_type{t, size, def_value, _primary_key, _unique_key, _not_null};
    }
}
