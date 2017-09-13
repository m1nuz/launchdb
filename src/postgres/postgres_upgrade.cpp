#include <json.hpp>

#include "context.hpp"
#include "strex.hpp"

using json = nlohmann::json;

namespace postgres {
    std::string to_type(const db::column_value_type &c);

    int upgrade(const db::context_diff &ctx) {
        using namespace std;

        cout << "BEGIN;\n\n";

        for (const auto &c : ctx.columns) {
            if (c.status == db::context_diff::status_added) {
                cout << "ALTER TABLE " << c.value.table_name << " ADD COLUMN " << c.value.column_name << " " << to_type(c.value) <<";\n";
            }

            if (c.status == db::context_diff::status_removed) {
                cout << "ALTER TABLE " << c.value.table_name << " DROP COLUMN " << c.value.column_name << ";\n";
            }
        }

        cout << '\n';
        cout << "COMMIT;\n";

        return 0;
    }
}
