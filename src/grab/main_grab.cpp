#include <cstdlib>
#include <fstream>
#include <set>

#include <xargs.hpp>
#include <format.hpp>
#include <journal.hpp>
#include <json.hpp>
#include <strex.hpp>

#include <launchdb/config.h>
#include <context.hpp>
#include <postgres/postgres.hpp>
#include <postgres/render.hpp>
#include <postgres/type_conversion.hpp>
#include <sqlite/sqlite.h>
#include <sqlite/render.h>

using json = nlohmann::json;

namespace db {
    void to_json(json& j, const column_t &c) {
        j["name"] = c.column_name;
        j["type"] = c.type_name;

        if (!c.comment.empty())
            j["comment"] = c.comment;

        if (c.not_null)
            j["not_null"] = c.not_null;

        if (c.primary_key)
            j["primary_key"] = c.primary_key;

        if (c.unique_key)
            j["unique"] = c.unique_key;

        if (!c.default_value.empty())
            j["default"] = c.default_value;
    }

    void to_json(json& j, const table_t &t) {
        j["name"] = t.table_name;
        j["schema"] = t.schema_name;

        auto columns = json::array();
        for (const auto &c : t.columns) {
            json jc;
            to_json(jc, c);
            columns.push_back(jc);
        }

        j["columns"] = columns;
    }

    /*void from_json(const json& j, table_t& p) {

    }*/
}

extern json make_json(const db::context &ctx) {
    using namespace db;
    json j;

    auto tables = json::array();
    for (const auto &t : ctx.tables) {
        json jt;
        to_json(jt, t);
        tables.push_back(jt);
    }

    j["tables"] = tables;
    return j;
}


db::context grap_sqlite(const std::string &app, const std::string &db_name) {
    using namespace std;
    using namespace sqlite;

    const auto *app_name = app.c_str();

    auto conn = connection::create(db_name);

    if (!conn) {
        LOG_ERROR(app_name, "%1", conn.error_message());
        exit(EXIT_FAILURE);
    }

    auto get_full_table_name = [] (const auto &schema_name, const auto &table_name) {
        return schema_name.empty() ? table_name : "[" + schema_name + "." + table_name + "]";
    };

    auto get_columns = [&](const auto &s_name, const auto &t_name) {
        const auto tbl_name = get_full_table_name(s_name, t_name);
        auto res = query::execute(conn, "PRAGMA table_info(%1);", tbl_name);

        if (!res) {
            LOG_ERROR(app_name, "%1 %2", res.error_message(), tbl_name);
            return vector<db::column_t>{};
        }

        vector<db::column_t> columns;

        for (const auto &r : res) {
            //const auto column_num = get<int64_t>(r, 0);
            const auto column_name = get<string>(r, 1);
            const auto column_type = get<string>(r, 2);
            const auto column_nullable = get<int64_t>(r, 3);
            auto def_val = get_if<string>(r, 4);
            const auto column_default = def_val == nullptr ? string{} : *def_val;
            const auto is_primary_key = get<int64_t>(r, 5);

            columns.push_back({column_name, string{}, string{}, from_type(column_type, 0, column_default, is_primary_key, false, column_nullable)});
        }

        return columns;
    };

    auto get_tables = [&] () {
        auto res = query::execute(conn, "select "
                                        "   name "
                                        "from "
                                        "   sqlite_master "
                                        "where "
                                        "   type = 'table';");
        if (!res) {
            LOG_ERROR(app_name, "%1", res.error_message());
        }

        vector<db::table_t> tables;
        tables.reserve(res.size());

        for (const auto &r : res) {
            const auto full_table_name = get<string>(r, 0);
            const auto names = strex::split(full_table_name, ".");
            const auto table_name = names.size() == 1 ? names[0] : names[1];
            const auto schema_name = names.size() == 1 ? string{} : names[0];

            auto columns = get_columns(schema_name, table_name);
            tables.push_back(db::table_t{schema_name, table_name, {}, columns, db::primary_key_t{}, {}});
        }

        return tables;
    };

    db::context ctx;
    ctx.tables = get_tables();

    return ctx;
}

db::context grap_postgres(const std::string &app, const std::string &db_name, const std::string &db_host, const std::string &db_user, const std::string &pass) {
    using namespace std;

    const auto *app_name = app.c_str();

    const string conn_info = "dbname=" + db_name
            + " host=" + db_host
            + " user=" + db_user
            + (!pass.empty() ? " password=" + pass : string{});

    using namespace postgres;
    auto conn = connection::create(conn_info);

    if (!conn) {
        LOG_ERROR(app_name, "%1", conn.error_message());
        exit(EXIT_FAILURE);
    }

    auto get_primary_keys = [&] (const auto & t_name) {
        auto res = query::execute(conn, "select "
                                        "   c.column_name, "
                                        "   c.data_type "
                                        "from "
                                        "   information_schema.table_constraints tc "
                                        "   join "
                                        "      information_schema.constraint_column_usage as ccu using (constraint_schema, constraint_name) "
                                        "   join "
                                        "      information_schema.columns as c "
                                        "      on c.table_schema = tc.constraint_schema "
                                        "      and tc.table_name = c.table_name "
                                        "      and ccu.column_name = c.column_name "
                                        "where "
                                        "   constraint_type = 'PRIMARY KEY' "
                                        "   and tc.table_name = '%1';", t_name);

        if (!res) {
            LOG_ERROR(app_name, "%1", res.error_message());
        }

        set<string> primary_keys;

        for (const auto &r : res) {
            const auto column_name = get<string>(r, 0);

            if (!column_name.empty())
                primary_keys.insert(column_name);
        }

        return primary_keys;
    };

    auto get_comment = [&] (const auto &s_name, const auto &t_name, const auto &c_name) {
        auto res = query::execute(conn, "select "
                                        "   description "
                                        "from "
                                        "   pg_catalog.pg_description "
                                        "where "
                                        "   objsubid = "
                                        "   ( "
                                        "      select "
                                        "         ordinal_position "
                                        "      from "
                                        "         information_schema.columns "
                                        "      where "
                                        "         table_name = '%2' "
                                        "         and column_name = '%3' "
                                        "         and table_schema = '%1' "
                                        "   ) "
                                        "   and objoid = "
                                        "   ( "
                                        "      select "
                                        "         oid "
                                        "      from "
                                        "         pg_class "
                                        "      where "
                                        "         relname = '%2' "
                                        "         and relnamespace = "
                                        "         ( "
                                        "            select "
                                        "               oid "
                                        "            from "
                                        "               pg_catalog.pg_namespace "
                                        "            where "
                                        "               nspname = '%1' "
                                        "         ) "
                                        "   ) ", s_name, t_name, c_name);

        if (!res) {
            LOG_ERROR(app_name, "s=%1 t=%2 c=%3", s_name, t_name, c_name);
            LOG_ERROR(app_name, "%1", res.error_message());
        }

        return res.size() > 0 ? get<string>(res[0], 0) : string{};
    };

    auto get_columns = [&](const auto &s_name, const auto &t_name) {
        auto res = query::execute(conn, "select "
                                        "   column_name, data_type, column_default, is_nullable "
                                        "from "
                                        "   information_schema.columns "
                                        "where "
                                        "   table_schema = '%1' "
                                        "   and table_name = '%2' "
                                        "order by ordinal_position ", s_name, t_name);

        if (!res) {
            LOG_ERROR(app_name, "%1", res.error_message());
        }

        const auto primary_keys = get_primary_keys(t_name);

        vector<db::column_t> columns;

        for (const auto &r : res) {
            const auto column_name = get<string>(r, 0);
            const auto column_type = get<string>(r, 1);
            const auto column_default = get<string>(r, 2);
            const auto column_nullable = get<bool>(r, 3);
            const auto column_comment = get_comment(s_name, t_name, column_name);

            const auto is_primary_key = primary_keys.find(column_name) != primary_keys.cend();

            columns.push_back({column_name, {}, column_comment, from_type(column_type, 0, column_default, is_primary_key, false, !column_nullable)});
        }

        return columns;
    };

    auto get_tables = [&] () {
        auto res = query::execute(conn, "select "
                                        "   table_schema, "
                                        "   table_name "
                                        "from "
                                        "   information_schema.tables "
                                        "where "
                                        "   table_schema not in "
                                        "   ( "
                                        "      'pg_catalog', "
                                        "      'information_schema' "
                                        "   )");
        if (!res) {
            LOG_ERROR(app_name, "%1", res.error_message());
        }

        vector<db::table_t> tables;
        tables.reserve(res.size());

        for (const auto &r : res) {
            const auto schema_name = get<string>(r, 0);
            const auto table_name = get<string>(r, 1);

            auto columns = get_columns(schema_name, table_name);
            tables.push_back(db::table_t{schema_name, table_name, string{}, columns, db::primary_key_t{}, {}});
        }

        return tables;
    };

    db::context ctx;
    ctx.tables = get_tables();

    return ctx;
}

volatile int log_level = DEFAULT_LOG_LEVEL;

extern int main(int argc, char *argv[]) {
    using namespace std;

    const auto app_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : nullptr;

    string db_name;
    string db_type = "postgres";
    string host = "localhost";
    string user = "james";
    string pass;

    xargs::args args;
    args.add_arg("DB_NAME", "Database name", [&] (const auto &v) {
        db_name = v;
    }).add_option("-g", "Grap from postgres, mysql, maria, sqlite. Default: " + db_name, [&] (const auto &v) {
        db_type = v;
    }).add_option("-h", "Display help", [&] () {
        puts(args.usage(argv[0]).c_str());
        exit(EXIT_SUCCESS);
    }).add_option("-u", "User name for connection", [&] (const auto &v) {
        user = v;
    }).add_option("-p", "Password of user for connection", [&] (const auto &v) {
        pass = v;
    }).add_option("-v", "Version", [&] () {
        fprintf(stdout, "%s %s\n", app_name, LAUNCHDB_VERSION);
        exit(EXIT_SUCCESS);
    });

    args.dispath(argc, argv);

    if (static_cast<size_t>(argc) < args.count()) {
        puts(args.usage(argv[0]).c_str());
        return EXIT_SUCCESS;
    }

    const auto get_ctx = [&] () {
        if (db_type == "postgres")
            return grap_postgres(app_name, db_name, host, user, pass);

        if (db_type == "sqlite")
            return grap_sqlite(app_name, db_name);

        return db::context{};
    };

    const auto j = make_json(get_ctx());

    cout << setw(4) << j << endl;

    return 0;
}
