#include <json.hpp>
#include <strex.hpp>

#include <postgres/type_conversion.hpp>

#include "context.hpp"

using json = nlohmann::json;
using std::string;

namespace postgres {
    const auto indent = string{"   "};

    int render(const db::table_t &t, string owner) {
        using namespace std;

        const auto full_table_name = (t.schema_name.empty() ? t.table_name : t.schema_name + "." + t.table_name);

        cout << "CREATE TABLE " << full_table_name << " (\n";

        auto col_idx = 0ul;
        for (const auto &c : t.columns) {
            cout << indent << c.column_name << " " << postgres::to_type(c);
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
                cout << "COMMENT ON COLUMN " << full_table_name << "." << c.column_name << " IS \'" << c.comment << "\';\n";

        if (!owner.empty())
            cout << '\n' << "ALTER TABLE " << full_table_name << " OWNER TO " << owner << ";\n";

        cout << '\n';

        return 0;
    }

    int render(const std::vector<db::index_t> &indices) {
        using namespace std;

        for (const auto &ix : indices) {
            cout << "CREATE INDEX " << "idx_" << ix.table_name << "__" << ix.name << " ON "
                 << ix.schema_name << (ix.schema_name.empty() ? "" : ".") << ix.table_name << " (" << ix.name << ")"
                 << ";\n";
        }

        return 0;
    }

    extern int render(const db::context &ctx, const db::config &cfg) {
        using namespace std;        

        if (cfg.make_transaction)
            cout << "BEGIN;\n\n";

        // create role if needed
        for (const auto &usr : ctx.users) {
            cout << "DO\n$body$\nBEGIN\n";
            cout << indent << "IF NOT EXISTS ( SELECT FROM pg_catalog.pg_user WHERE  usename = \'" << usr.name << "\' ) THEN \n";
            cout << indent << "CREATE ROLE " << usr.name;
            if (usr.password.empty())
                cout << " LOGIN";
            else
                cout << " LOGIN PASSWORD \'" << usr.password << "\';\n";
            cout << indent << "END IF;\n";
            cout << "END\n$body$;\n";
        }

        if (!ctx.users.empty())
            cout << "\n\n";

        // schemas
        for (const auto &s : ctx.schemas) {
            if (s != "public")
                cout << "CREATE SCHEMA " << s << ";\n";

            if (!ctx.owner.empty())
                cout << '\n' << "ALTER SCHEMA " << s << " OWNER TO " << ctx.owner << ";\n";
        }

        if (!ctx.schemas.empty())
            cout << "\n\n";

        for (const auto &t : ctx.tables) {
            render(t, ctx.owner);
            cout << '\n';
        }

        if (!ctx.tables.empty())
            cout << "\n";

        /*for (const auto &ix : ctx.indices) {
            cout << "CREATE INDEX " << "idx_" << ix.table_name << "__" << ix.name << " ON "
                 << ix.schema_name << (ix.schema_name.empty() ? "" : ".") << ix.table_name << " (" << ix.name << ")"
                 << ";\n";
        }*/
        render(ctx.indices);

        if (!ctx.privileges.empty())
            cout << "\n\n";

        for (const auto &gp : ctx.privileges) {
            for (const auto &s : ctx.schemas) {
                cout << "GRANT " << strex::toupper(gp.name) << " PRIVILEGES ON SCHEMA " << s << " TO " << gp.user << ";\n";
                cout << "GRANT " << strex::toupper(gp.name) << " ON ALL SEQUENCES IN SCHEMA " << s << " TO " << gp.user << ";\n";
            }
            for (const auto &t : ctx.tables) {
                const auto full_table_name = (t.schema_name.empty() ? t.table_name : t.schema_name + "." + t.table_name);
                cout << "GRANT " << strex::toupper(gp.name) << " PRIVILEGES ON " << full_table_name << " TO " << gp.user << ";\n";
            }
        }

        cout << "\n\n";

        cout << (cfg.make_transaction ? "COMMIT;\n" : "\n");

        return 0;
    }
}
