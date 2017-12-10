#include <json.hpp>
#include <strex.hpp>

#include <launchdb/types.hpp>

#include "context.hpp"

using json = nlohmann::json;
using std::string;

namespace sqlite {
    const auto indent = string{"   "};

    string to_type(const db::column_value_type &c) noexcept {
        using namespace std;
        using namespace launchdb;

        string type_name = c.type_name;

        switch (strex::hash(type_name)) {
        case strex::hash(types::int_type):
        case strex::hash(types::bool_type):
            type_name = "INTEGER";
            break;
        case strex::hash(types::float_type):
        case strex::hash(types::decimal_type):
            type_name = "REAL";
            break;
        case strex::hash(types::str_type):
        case strex::hash(types::text_type):
        case strex::hash(types::time_type):
        case strex::hash(types::date_type):
        case strex::hash(types::datetime_type):
        case strex::hash(types::uuid_type):
        case strex::hash(types::json_type):
            type_name = "TEXT";
            break;
        case strex::hash(types::bytes_type):
            type_name = "BLOB";
            break;
        }

        if (!c.default_value.empty())
            type_name += " DEFAULT " + c.default_value;
        if (c.not_null)
            type_name += " NOT NULL";
        if (c.unique_key)
            type_name += " UNIQUE";


        // TODO: AUTOINCREMENT
        /*if (type_name == "INTEGER" && c.primary_key)
            type_name += " AUTOINCREMENT";*/

        return type_name;
    }

    int render(const std::vector<db::index_t> &indices) {
        using namespace std;

        auto get_full_table_name = [] (const auto &schema_name, const auto &table_name) {
            return schema_name.empty() ? table_name : "[" + schema_name + "." + table_name + "]";
        };

        for (const auto &ix : indices) {
            cout << "CREATE INDEX " << "idx_" << ix.table_name << "__" << ix.name << " ON "
                 << get_full_table_name(ix.schema_name, ix.table_name) << " (" << ix.name << ")"
                 << ";\n";
        }

        return 0;
    }

    void render(const db::table_t &tbl, const bool exists_only) {
        using namespace std;

        auto get_full_table_name = [] (const auto &schema_name, const auto &table_name) {
            return schema_name.empty() ? table_name : "[" + schema_name + "." + table_name + "]";
        };

        const auto full_table_name = get_full_table_name(tbl.schema_name, tbl.table_name);

        cout << "CREATE TABLE " << (exists_only ? string{} : "IF NOT EXISTS ") << full_table_name << " (\n";

        auto col_idx = 0ul;
        for (const auto &c : tbl.columns) {
            cout << indent << c.column_name << " " << to_type(c);
            if ((tbl.columns.size() > 1 && col_idx != tbl.columns.size() - 1) || !tbl.primary_key.empty())
                cout << ',';
            cout << '\n';
            col_idx++;
        }

        auto pk_idx = 0ul;
        if (!tbl.primary_key.empty()) {
            cout << indent << "CONSTRAINT " << "pk_" << tbl.table_name << " PRIMARY KEY (";

            auto pk_elem_idx = 0ul;
            for (const auto &pk : tbl.primary_key) {
                if (pk_elem_idx > 0)
                    cout << ' ';
                cout << pk;
                if (tbl.primary_key.size() > 1 && pk_elem_idx != tbl.primary_key.size() - 1)
                    cout << ',';
                pk_elem_idx++;
            }
            cout << ')';

            if (pk_idx != 0 || !tbl.foreign_keys.empty())
                cout << ',';

            cout << '\n';

            pk_idx++;
        }

        auto fk_idx = 0ul;
        if (!tbl.foreign_keys.empty()) {
            for (const auto &fk : tbl.foreign_keys) {
                cout << indent <<"FOREIGN KEY (";
                for (const auto &c : fk.columns)
                    cout << c;
                cout << ") REFERENCES ";
                cout << get_full_table_name(fk.ref_schema, fk.ref_table) << " (";
                for (const auto &ref : fk.ref_columns)
                    cout << ref;
                cout << ')';

                if ((fk_idx != 0 || !tbl.primary_key.empty()) && fk_idx != tbl.foreign_keys.size() - 1)
                    cout << ',';

                cout << '\n';

                fk_idx++;
            }
        }

        cout << ");\n";

        //cout << ") WITHOUT ROWID;\n";
    }

    extern int render(const db::context &ctx, const db::config &cfg) {
        using namespace std;

        if (cfg.make_transaction)
            cout << "BEGIN;\n\n";

        for (const auto &t : ctx.tables) {
            render(t, true);
            cout << '\n';
        }

        if (!ctx.tables.empty())
            cout << "\n";

        render(ctx.indices);

        cout << "\n\n";

        cout << (cfg.make_transaction ? "COMMIT;\n" : "\n");

        return 0;
    }

    db::column_value_type from_type(const std::string &t, const size_t size, const std::string &def, const bool _primary_key, const bool _unique_key, const bool _not_null) noexcept {
        using namespace std;
        using namespace launchdb;

        const auto def_value = _primary_key ? string{} : def;

        const auto type_hash = strex::hash(strex::tolower(t));

        if (type_hash == strex::hash("integer"))
            return db::column_value_type{types::int_type, size, def_value, _primary_key, _unique_key, _not_null};

        if (type_hash == strex::hash("text"))
            return db::column_value_type{types::text_type, size, def_value, _primary_key, _unique_key, _not_null};

        if (type_hash == strex::hash("blob"))
            return db::column_value_type{types::bytes_type, size, def_value, _primary_key, _unique_key, _not_null};

        if (type_hash == strex::hash("real"))
            return db::column_value_type{types::decimal_type, size, def_value, _primary_key, _unique_key, _not_null};

        return db::column_value_type{t, size, def_value, _primary_key, _unique_key, _not_null};
    }
}
