#include <algorithm>
#include <context.hpp>

db::contex_diff get_diff(const db::context &old_ctx, const db::context &new_ctx) {
    using namespace std;

    db::contex_diff ctx_diff;

    auto contains_table = [] (const auto &tables, const auto &t) {
        return find_if(tables.begin(), tables.end(), [&] (const auto &v) {
            return v.schema_name == t.schema_name && v.table_name == t.table_name;
        }) != tables.end();
    };

    for (const auto &t : old_ctx.tables) {
        if (!contains_table(new_ctx.tables, t)) {
            auto dt = t;
            dt.columns.clear();
            ctx_diff.removed.tables.push_back(dt);
        }
    }
}
