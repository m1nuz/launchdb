#include <algorithm>
#include <context.hpp>

db::context_diff get_diff(const db::context &old_ctx, const db::context &new_ctx) {
    using namespace std;

    db::context_diff ctx_diff;

    auto same_table = [] (const auto &t1, const auto &t2) {
        return t1.schema_name == t2.schema_name && t1.table_name == t2.table_name;
    };

    auto contains_table = [] (const auto &tables, const auto &tb) {
        return find_if(tables.begin(), tables.end(), [&] (const auto &v) {
            return v.schema_name == tb.schema_name && v.table_name == tb.table_name;
        }) != tables.end();
    };

    auto contains_column = [] (const auto &columns, const auto &col) {
        return find_if(columns.begin(), columns.end(), [&] (const auto &v) {
            return col.name == v.name;
        }) != columns.end();
    };

    // removed tables
    for (const auto &t : old_ctx.tables) {
        if (!contains_table(new_ctx.tables, t)) {
            auto dt = t;
            dt.columns.clear();
            ctx_diff.tables.emplace_back(db::context_diff::status_removed, db::table_t{dt.schema_name, dt.table_name, dt.comment});
        }
    }

    // added tables
    for (const auto &t: new_ctx.tables) {
        if (!contains_table(old_ctx.tables, t)) {
            auto dt = t;
            dt.columns.clear();
            ctx_diff.tables.emplace_back(db::context_diff::status_added, db::table_t{dt.schema_name, dt.table_name, dt.comment});
        }
    }

    // columns
    for (const auto &nt: new_ctx.tables) {
        for (const auto &ot : old_ctx.tables) {
            if (same_table(nt, ot)) {
                for (const auto &c : nt.columns) {
                    if (!contains_column(ot.columns, c)) {
                        ctx_diff.columns.emplace_back(db::context_diff::status_added, db::context_diff::column_t{nt.schema_name, nt.table_name, c.name, c});
                    }
                }

                for (const auto &c : ot.columns) {
                    if (!contains_column(nt.columns, c)) {
                        ctx_diff.columns.emplace_back(db::context_diff::status_removed, db::context_diff::column_t{ot.schema_name, nt.table_name, c.name, c});
                    }
                }
            }
        }
    }

    return ctx_diff;
}
