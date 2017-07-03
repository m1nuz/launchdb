#include <json.hpp>

#include "context.hpp"

using json = nlohmann::json;

namespace postgres {
    std::string to_type(const db_context::column_type &c);

    extern int output(const db_context &ctx, const db_config &cfg) {
        using namespace std;

        const auto indent = string{"   "};

        if (cfg.make_transaction)
            cout << "BEGIN;\n\n";

        // schemas
        for (const auto &s : ctx.schemas) {
            if (s != "public")
                cout << "CREATE SCHEMA " << s << ";\n";
        }

        cout << "\n\n";

        for (const auto &t : ctx.tables) {
            const auto full_table_name = (t.schema_name.empty() ? t.table_name : t.schema_name + "." + t.table_name);

            cout << "CREATE TABLE " << full_table_name << " (\n";

            auto col_idx = 0ul;
            for (const auto &c : t.columns) {
                cout << indent << c.name << " " << postgres::to_type(c);
                if ((t.columns.size() > 1 && col_idx != t.columns.size() - 1) || !t.primary_key.empty())
                    cout << ',';
                cout << '\n';
                col_idx++;
            }

            auto pk_idx = 0ul;
            if (!t.primary_key.empty()) {
                cout << indent << "CONSTRAINT " << "pk_" << t.table_name << " PRIMARY KEY (";

                auto pk_elem_idx = 0ul;
                for (const auto &pk : t.primary_key) {
                    if (pk_elem_idx > 0)
                        cout << ' ';
                    cout << pk;
                    if (t.primary_key.size() > 1 && pk_elem_idx != t.primary_key.size() - 1)
                        cout << ',';
                    pk_elem_idx++;
                }
                cout << ')';

                if (pk_idx != 0 || !t.foreign_keys.empty())
                    cout << ',';

                cout << '\n';

                pk_idx++;
            }

            auto fk_idx = 0ul;
            if (!t.foreign_keys.empty()) {
                for (const auto &fk : t.foreign_keys) {
                    cout << indent <<"FOREIGN KEY (";
                    for (const auto &c : fk.columns)
                        cout << c;
                    cout << ") REFERENCES ";
                    cout << fk.ref_schema << (fk.ref_schema.empty() ? "" : ".") << fk.ref_table << " (";
                    for (const auto &ref : fk.ref_columns)
                        cout << ref;
                    cout << ')';

                    if ((fk_idx != 0 || !t.primary_key.empty()) && fk_idx != t.foreign_keys.size() - 1)
                        cout << ',';

                    cout << '\n';

                    fk_idx++;
                }
            }

            cout << ") WITH (OIDS = FALSE);\n";

            if (!t.comment.empty())
                cout << '\n' << "COMMENT ON TABLE " << full_table_name << " IS \'" << t.comment << "\';\n";

            cout << '\n';
            for (const auto &c : t.columns)
                if (!c.comment.empty())
                    cout << "COMMENT ON COLUMN " << full_table_name << "." << c.name << " IS \'" << c.comment << "\';\n";

            if (!ctx.owner.empty())
                cout << '\n' << "ALTER TABLE " << full_table_name << " OWNER TO " << ctx.owner << ";\n";

            cout << '\n';
        }

        for (const auto &ix : ctx.indices) {
            cout << "CREATE INDEX " << "idx_" << ix.table_name << "__" << ix.name << " ON "
                 << ix.schema_name << (ix.schema_name.empty() ? "" : ".") << ix.table_name << " (" << ix.name << ")"
                 << ";\n";
        }

        cout << '\n';

        cout << (cfg.make_transaction ? "COMMIT;\n" : "\n");

        return 0;
    }
}
