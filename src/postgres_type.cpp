#include <strex.hpp>

#include "context.hpp"

namespace postgres {
    std::string to_type(const db_context::column_type &c) {
        using namespace std;
        string type_name;

        switch (strex::hash(c.type)) {
        case strex::hash("int"):
            type_name = "INTEGER";
            break;
        case strex::hash("int32"):
            type_name = "INTEGER";
            break;
        case strex::hash("int64"):
            type_name = "BIGINT";
            break;
        case strex::hash("str"):
            type_name = "TEXT";
            break;
        case strex::hash("text"):
            type_name = "TEXT";
            break;
        case strex::hash("bool"):
            type_name = "BOOLEAN";
            break;
        case strex::hash("buffer"):
            type_name = "bytea";
            break;
        case strex::hash("timestamp"):
            type_name = "timestamp";
            break;
        case strex::hash("serial"):
            type_name = "SERIAL";
            break;
        case strex::hash("decimal"):
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
}