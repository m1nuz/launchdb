#include <unordered_map>
#include <json.hpp>

#include "context.hpp"
#include "strex.hpp"

using json = nlohmann::json;
using std::string;

namespace sqlite {
    void render(const db::table_t &tbl, const bool);
    int render(const std::vector<db::index_t> &indices);

    string to_type(const db::column_value_type &c) noexcept;

    const auto indent = string{"   "};

    int upgrade(const db::context_diff &ctx) {
        using namespace std;

        auto get_full_table_name = [] (const auto &schema_name, const auto &table_name) {
            return schema_name.empty() ? table_name : "[" + schema_name + "." + table_name + "]";
        };

        auto get_simple_table_name = [] (const auto &schema_name, const auto &table_name) {
            return schema_name.empty() ? table_name : schema_name + "." + table_name;
        };

        auto get_temp_table_name = [] (const auto &full_tbl_name) {
            return full_tbl_name.find('.') ? "[temp_" + full_tbl_name + ']' : "temp_" + full_tbl_name;
        };

        auto get_table_name = [] (const auto &full_tbl_name) {
            return full_tbl_name.find('.') ? '[' + full_tbl_name + ']' : full_tbl_name;
        };

        cout << "BEGIN;\n";

        unordered_map<string, vector<tuple<db::context_diff::status_type, db::context_diff::column_t, set<db::column_fields>>>> table_columns;

        for (const auto &c : ctx.columns) {
            //const auto status = get<0>(c);
            const auto value = get<1>(c);
            //const auto changes = get<2>(c);

            //const auto [status, value, changes] = c;

            const auto simple_table_name = get_simple_table_name(value.schema_name, value.table_name);

            if (table_columns.find(simple_table_name) != table_columns.end()) {
                table_columns[simple_table_name].push_back(c);
            } else {
                vector<tuple<db::context_diff::status_type, db::context_diff::column_t, set<db::column_fields>>> vv;
                vv.push_back(c);
                table_columns.emplace(simple_table_name, vv);
            }
        }

        auto needed_status = [] (const auto st) {
            return st == db::context_diff::status_unchanged || st == db::context_diff::status_added || st == db::context_diff::status_changed;
        };

        auto unchanged_status = [] (const auto st) {
            return st == db::context_diff::status_unchanged;
        };

        auto get_column_list = [=] (const auto &columns, const auto check_status) {
            string column_list;

            for (const auto &c : columns) {
                const auto status = get<0>(c);
                const auto value = get<1>(c);

                if (check_status(status)) {
                    if (column_list.empty())
                        column_list += value.column_name;
                    else
                        column_list += ", " + value.column_name;
                }
            }

            return column_list;
        };

        for (const auto &tc : table_columns) {
            const auto table_name = get_table_name(get<0>(tc));
            const auto temp_table_name = get_temp_table_name(get<0>(tc));

            const bool any_remove_flag = find_if(tc.second.begin(), tc.second.end(), [] (const auto c) {
                return get<0>(c) == db::context_diff::status_removed || get<0>(c) == db::context_diff::status_added;
            }) != tc.second.end();

            if (any_remove_flag) {
                cout << "ALTER TABLE " << table_name << " RENAME TO " << temp_table_name << ";\n";

                cout << "CREATE TABLE " << table_name << "(\n";

                auto col_idx = 0ul;
                for (const auto &c : tc.second) {
                    const auto status = get<0>(c);
                    const auto value = get<1>(c);

                    if (needed_status(status)) {
                        // TODO: AUTOINCREMENT

                        if (col_idx != 0)
                            cout << ',' << '\n';
                        cout << indent << value.column_name << " " << to_type(value) << (value.primary_key ? " PRIMARY KEY" : string{});
                        col_idx++;
                    }

                }

                cout << ");\n";

                const auto column_list = get_column_list(tc.second, unchanged_status);

                cout << "INSERT INTO " << table_name << " (";
                cout << column_list << ")\n";
                cout << indent << "SELECT " << column_list << '\n' <<indent << "FROM " << temp_table_name << ";\n";

                cout << "DROP TABLE " << temp_table_name << ";\n";

                continue;
            }

            /*for (const auto &c : tc.second) {
                const auto status = get<0>(c);
                const auto value = get<1>(c);

                if (status == db::context_diff::status_added)
                    cout << "ALTER TABLE " << table_name << " ADD COLUMN " << value.column_name << " " << to_type(value) << ";\n";
            }*/

            cout << '\n';
        }

        for (const auto &t : ctx.tables) {
            if (get<0>(t) == db::context_diff::status_added) {
                render(get<1>(t), false);
            }
        }

        for (const auto ix : ctx.indices) {
            if (get<0>(ix) == db::context_diff::status_added) {
                render({get<1>(ix)});
            }
        }

        cout << '\n' << '\n';
        cout << "COMMIT;\n";

        return 0;
    }
}
