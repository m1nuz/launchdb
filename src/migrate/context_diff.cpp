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
            return col.column_name == v.column_name;
        }) != columns.end();
    };

    // removed tables
    for (const auto &t : old_ctx.tables) {
        if (!contains_table(new_ctx.tables, t)) {
            ctx_diff.tables.emplace_back(db::context_diff::status_removed, t);
        }
    }

    // added tables
    for (const auto &t: new_ctx.tables) {
        if (!contains_table(old_ctx.tables, t)) {
            ctx_diff.tables.emplace_back(db::context_diff::status_added, t);

            // indices from foreign keys
            for (const auto &fk : t.foreign_keys) {
                string ix_name;
                string ix_column_name;

                for (const auto &n : fk.columns)
                    ix_name += (ix_name.empty() ? "" : "_") + n;

                for (const auto &n : fk.ref_columns)
                    ix_column_name = (ix_column_name.empty() ? "" : "_") + n;

                ctx_diff.indices.emplace_back(db::context_diff::status_added, db::index_t{t.schema_name, t.table_name, ix_name, ix_column_name});
            }
        }
    }

    // columns
    for (const auto &nt: new_ctx.tables) {
        for (const auto &ot : old_ctx.tables) {
            if (same_table(nt, ot)) {
                for (const auto &c : nt.columns) {
                    if (!contains_column(ot.columns, c)) {
                        const set<db::column_fields> no_changes;
                        ctx_diff.columns.emplace_back(db::context_diff::status_added, db::context_diff::column_t{nt.schema_name, nt.table_name, c.column_name, c.column_renamed, c}, no_changes);
                    }
                }

                for (const auto &c : ot.columns) {
                    if (!contains_column(nt.columns, c)) {
                        const set<db::column_fields> no_changes;
                        ctx_diff.columns.emplace_back(db::context_diff::status_removed, db::context_diff::column_t{ot.schema_name, nt.table_name, c.column_name, c.column_renamed, c}, no_changes);
                    }
                }

                for (const auto &nc : nt.columns)
                    for (const auto &oc : ot.columns)
                        if ((nc.column_name == oc.column_name) || (nc.column_renamed == oc.column_name)) {
                            auto col_diff = get_diff(nc, oc);
                            if (!col_diff.empty()) {
                                ctx_diff.columns.emplace_back(db::context_diff::status_changed, db::context_diff::column_t{nt.schema_name, nt.table_name, nc.column_name, nc.column_renamed, nc}, col_diff);
                            } else {
                                const set<db::column_fields> no_changes;
                                ctx_diff.columns.emplace_back(db::context_diff::status_unchanged, db::context_diff::column_t{nt.schema_name, nt.table_name, nc.column_name, nc.column_renamed, nc}, no_changes);
                            }

                        }
            }
        }
    }

    return ctx_diff;
}
