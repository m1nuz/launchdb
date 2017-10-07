#include <strex.hpp>

#include "context.hpp"

namespace postgres {
    std::string to_type(const db::column_value_type &c) {
        using namespace std;
        string type_name;

        switch (strex::hash(c.type_name)) {
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

    db::column_value_type from_type(const std::string &t, const std::string def) {
        if (t == "integer")
            return db::column_value_type{"int", def};

        if (t == "text")
            return db::column_value_type{"text", def};

        if (t == "bytea")
            return db::column_value_type{"buffer", def};

        if (t == "numeric")
            return db::column_value_type{"decimal", def};

        if (t == "boolean")
            return db::column_value_type{"bool", def};

        return db::column_value_type{};
    }
}
