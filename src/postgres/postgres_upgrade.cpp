#include <json.hpp>

#include "context.hpp"
#include "strex.hpp"

using json = nlohmann::json;
using std::string;

namespace postgres {
    int output(const db::table_t &t, string owner);
    int output(const std::vector<db::index_t> &indices);

    std::string to_type(const db::column_value_type &c);

    int upgrade(const db::context_diff &ctx) {
        using namespace std;

        cout << "BEGIN;\n\n";

        for (const auto &c : ctx.columns) {
            const auto full_table_name = (c.value.schema_name.empty() ? c.value.table_name : c.value.schema_name + "." + c.value.table_name);

            if (c.status == db::context_diff::status_added) {
                cout << "ALTER TABLE " << full_table_name << " ADD COLUMN " << c.value.column_name << " " << to_type(c.value) <<";\n";
            }

            if (c.status == db::context_diff::status_removed) {
                cout << "ALTER TABLE " << full_table_name << " DROP COLUMN " << c.value.column_name << ";\n";
            }
        }

        cout << '\n';

        for (const auto &t : ctx.tables) {
            if (t.status == db::context_diff::status_added) {
                output(t.value, {});
            }
        }

        for (const auto ix : ctx.indices) {
            if (ix.status == db::context_diff::status_added) {
                output({ix.value});
            }
        }

        cout << '\n';
        cout << "COMMIT;\n";

        return 0;
    }
}
