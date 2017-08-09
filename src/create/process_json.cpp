#include <json.hpp>

#include "context.hpp"

using json = nlohmann::json;

extern db::context process_json(const json &j) {
    using namespace std;

    db::context ctx;

    // TODO: alloc vectors in ctx

    if (j.find("privileges") != j.end())
        for (auto &pr : j["privileges"]) {
            const auto user_name = pr.begin().key();
            const auto privilege = pr.begin().value().get<string>();

            ctx.privileges.push_back(db::context::privilege_t{user_name, privilege});
        }

    if (j.find("owner") != j.end())
        ctx.owner = j["owner"].get<string>();

    if (j.find("users") != j.end())
        for (auto &usr : j["users"]) {
            const auto user_name = usr["name"].get<string>();
            const auto user_pwd = usr.find("password") != usr.end() ? usr["password"].get<string>() : string{};

            ctx.users.push_back(db::context::user_t{user_name, user_pwd});
        }

    if (j.find("tables") != j.end())
        for (auto &tbl : j["tables"]) {
            const auto table_name = tbl["name"].get<string>();

            const auto schema_name = tbl.find("schema") != tbl.end() ? tbl["schema"].get<string>() : string{};
            const auto table_comment = tbl.find("comment") != tbl.end() ? tbl["comment"].get<string>() : string{};

            if (!schema_name.empty())
                ctx.schemas.insert(schema_name);

            std::vector<db::context::column_t> all_columns;

            //auto is_primary_key = false;

            db::context::primary_key_t pk;
            db::context::foreign_keys_t fks;

            if (tbl.find("columns") != tbl.end())
                for (auto &col : tbl["columns"]) {
                    const auto column_name = col["name"].get<string>();
                    const auto column_type = col["type"].get<string>();
                    const auto column_not_null = col.find("not_null") != col.end() ? col["not_null"].get<bool>() : false;
                    const auto unique_column = col.find("unique") != col.end() ? col["unique"].get<bool>() : false;
                    const auto column_comment = col.find("comment") != col.end() ? col["comment"].get<string>() : string{};
                    const auto default_value = col.find("default") != col.end() ? col["default"].get<string>() : string{};
                    const auto column_pk = col.find("primary_key") != col.end() ? col["primary_key"].get<bool>() : false;

                    if (column_pk)
                        pk.insert(column_name);

                    all_columns.push_back(db::context::column_t{column_name, column_type, column_comment, default_value, column_pk, unique_column, column_not_null});
                }

            if (tbl.find("foreign_keys") != tbl.end())
                for (auto &fk : tbl["foreign_keys"]) {
                    const auto fk_table = fk.find("table") != fk.end() ? fk["table"].get<string>() : string{};
                    const auto fk_schema = fk.find("schema") != fk.end() ? fk["schema"].get<string>() : string{};

                    vector<string> fk_columns;
                    vector<string> fk_references;

                    if (fk.find("columns") != fk.end()) {
                        for (auto &fkc : fk["columns"])
                            fk_columns.emplace_back(fkc.get<string>());
                    }

                    if (fk.find("references") != fk.end()) {
                        for (auto &fkref : fk["references"])
                            fk_references.emplace_back(fkref.get<string>());
                    }

                    fks.push_back(db::context::foreign_key_t{fk_table, fk_schema, fk_columns, fk_references});
                }

            // indices from foreign keys
            for (const auto &fk : fks) {
                std::string ix_name;
                std::string ix_column_name;

                for (const auto &n : fk.columns)
                    ix_name += (ix_name.empty() ? "" : "_") + n;

                for (const auto &n : fk.ref_columns)
                    ix_column_name = (ix_column_name.empty() ? "" : "_") + n;

                ctx.indices.push_back(db::context::index_t{schema_name, table_name, ix_name, ix_column_name});
            }

            ctx.tables.push_back(db::context::table_t{schema_name, table_name, table_comment, all_columns, pk, fks});
        }

    return ctx;
}
