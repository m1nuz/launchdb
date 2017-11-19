#include <json.hpp>

#include "context.hpp"
#include "strex.hpp"

using json = nlohmann::json;
using std::string;

namespace postgres {
    int render(const db::table_t &t, string owner);
    int render(const std::vector<db::index_t> &indices);

    std::string to_type(const db::column_value_type &c);

    int upgrade(const db::context_diff &ctx) {
        using namespace std;

        cout << "BEGIN;\n\n";

        for (const auto &c : ctx.columns) {
            const auto full_table_name = (c.value.schema_name.empty() ? c.value.table_name : c.value.schema_name + "." + c.value.table_name);

            if (c.status == db::context_diff::status_added) {
                cout << "ALTER TABLE " << full_table_name << " ADD COLUMN " << c.value.column_name << " " << to_type(c.value) << ";\n";
                continue;
            }

            if (c.status == db::context_diff::status_removed) {
                cout << "ALTER TABLE " << full_table_name << " DROP COLUMN " << c.value.column_name << ";\n";
                continue;
            }

            if (c.status == db::context_diff::status_changed) {
                if (c.changes.find(db::column_fields::type) != c.changes.end() ||
                    c.changes.find(db::column_fields::size) != c.changes.end() ||
                    c.changes.find(db::column_fields::precision) != c.changes.end() ||
                    c.changes.find(db::column_fields::scale) != c.changes.end()) {

                    // TODO: change type
                }

                // TODO: primary_key

                if (c.changes.find(db::column_fields::default_value) != c.changes.end()) {
                    if (!c.value.default_value.empty())
                        cout << "ALTER TABLE " << full_table_name << " ALTER COLUMN " << c.value.column_name << " SET DEFAULT " << c.value.default_value <<";\n";
                    else
                        cout << "ALTER TABLE " << full_table_name << " ALTER COLUMN " << c.value.column_name << " DROP DEFAULT" << ";\n";
                }

                if (c.changes.find(db::column_fields::not_null) != c.changes.end()) {
                    if (c.value.not_null)
                        cout << "ALTER TABLE " << full_table_name << " ALTER COLUMN " << c.value.column_name << " SET NOT NULL" << ";\n";
                    else
                        cout << "ALTER TABLE " << full_table_name << " ALTER COLUMN " << c.value.column_name << " DROP NOT NULL" << ";\n";
                }

                if (c.changes.find(db::column_fields::unique_key) != c.changes.end()) {
                    if (c.value.unique_key)
                        cout << "ALTER TABLE " << full_table_name << " ADD CONSTRAINT " << "UNIQUE (" << c.value.column_name << ")" << ";\n";
                    else
                        cout << "ALTER TABLE " << full_table_name << " DROP UNIQUE " << "UNIQUE (" << c.value.column_name << ")" << ";\n";
                }

                if (c.changes.find(db::column_fields::rename) != c.changes.end()) {
                    cout << "ALTER TABLE " << full_table_name << " RENAME COLUMN " << c.value.column_renamed << " TO " << c.value.column_name << ";\n";
                }
            }
        }

        cout << '\n';

        for (const auto &t : ctx.tables) {
            if (t.status == db::context_diff::status_added) {
                render(t.value, {});
            }
        }

        for (const auto ix : ctx.indices) {
            if (ix.status == db::context_diff::status_added) {
                render({ix.value});
            }
        }

        cout << '\n';
        cout << "COMMIT;\n";

        return 0;
    }
}
