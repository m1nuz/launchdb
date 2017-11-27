#include <strex.hpp>

#include "context.hpp"
#include <launchdb/types.hpp>

using std::string;

namespace postgres {

    string to_type(const db::column_value_type &c) noexcept {
        using namespace std;
        using namespace launchdb;
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
            type_name = c.size == 0 ? "TEXT" : "VARCHAR(" + to_string(c.size) + ")";
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
            type_name = "DECIMAL("
                    + (c.precision == 0 ? "12" : to_string(c.precision))
                    + ","
                    + (c.scale == 0 ? "2" : to_string(c.scale))
                    + ")";
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

    db::column_value_type from_type(const string &t, const size_t size, const string &def, const bool _primary_key, const bool _unique_key, const bool _not_null) noexcept {
        using namespace std;
        using namespace launchdb;

        const auto def_value = _primary_key ? string{} : def;

        const auto type_hash = strex::hash(strex::tolower(t));

        if (type_hash == strex::hash("integer"))
            return db::column_value_type{types::int_type, size, def_value, _primary_key, _unique_key, _not_null};

        if (type_hash == strex::hash("text"))
            return db::column_value_type{types::text_type, size, def_value, _primary_key, _unique_key, _not_null};

        if (type_hash == strex::hash("bytea"))
            return db::column_value_type{types::bytes_type, size, def_value, _primary_key, _unique_key, _not_null};

        if (type_hash == strex::hash("numeric"))
            return db::column_value_type{types::decimal_type, size, def_value, _primary_key, _unique_key, _not_null};

        if (type_hash == strex::hash("boolean"))
            return db::column_value_type{types::bool_type, size, def_value, _primary_key, _unique_key, _not_null};

        return db::column_value_type{t, size, def_value, _primary_key, _unique_key, _not_null};
    }
}
