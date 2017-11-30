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
            const auto status = get<0>(c);
            const auto value = get<1>(c);
            const auto changes = get<2>(c);

            const auto full_table_name = (value.schema_name.empty() ? value.table_name : value.schema_name + "." + value.table_name);

            if (status == db::context_diff::status_added) {
                cout << "ALTER TABLE " << full_table_name << " ADD COLUMN " << value.column_name << " " << to_type(value) << ";\n";
                continue;
            }

            if (status == db::context_diff::status_removed) {
                cout << "ALTER TABLE " << full_table_name << " DROP COLUMN " << value.column_name << ";\n";
                continue;
            }

            if (status == db::context_diff::status_changed) {
                if (changes.find(db::column_fields::type) != changes.end() ||
                    changes.find(db::column_fields::size) != changes.end() ||
                    changes.find(db::column_fields::precision) != changes.end() ||
                    changes.find(db::column_fields::scale) != changes.end()) {

                    // TODO: change type
                }

                // TODO: primary_key

                if (changes.find(db::column_fields::default_value) != changes.end()) {
                    if (!value.default_value.empty())
                        cout << "ALTER TABLE " << full_table_name << " ALTER COLUMN " << value.column_name << " SET DEFAULT " << value.default_value <<";\n";
                    else
                        cout << "ALTER TABLE " << full_table_name << " ALTER COLUMN " << value.column_name << " DROP DEFAULT" << ";\n";
                }

                if (changes.find(db::column_fields::not_null) != changes.end()) {
                    if (value.not_null)
                        cout << "ALTER TABLE " << full_table_name << " ALTER COLUMN " << value.column_name << " SET NOT NULL" << ";\n";
                    else
                        cout << "ALTER TABLE " << full_table_name << " ALTER COLUMN " << value.column_name << " DROP NOT NULL" << ";\n";
                }

                if (changes.find(db::column_fields::unique_key) != changes.end()) {
                    if (value.unique_key)
                        cout << "ALTER TABLE " << full_table_name << " ADD CONSTRAINT " << "UNIQUE (" << value.column_name << ")" << ";\n";
                    else
                        cout << "ALTER TABLE " << full_table_name << " DROP UNIQUE " << "UNIQUE (" << value.column_name << ")" << ";\n";
                }

                if (changes.find(db::column_fields::rename) != changes.end()) {
                    cout << "ALTER TABLE " << full_table_name << " RENAME COLUMN " << value.column_renamed << " TO " << value.column_name << ";\n";
                }
            }
        }

        cout << '\n';

        for (const auto &t : ctx.tables) {
            if (get<0>(t) == db::context_diff::status_added) {
                render(get<1>(t) , {});
            }
        }

        for (const auto ix : ctx.indices) {
            if (get<0>(ix)  == db::context_diff::status_added) {
                render({get<1>(ix) });
            }
        }

        cout << '\n';
        cout << "COMMIT;\n";

        return 0;
    }
}
