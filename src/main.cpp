#include <cstdlib>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <iostream>

#include <json.hpp>
#include <journal.hpp>
#include <xargs.hpp>
#include <strex.hpp>

#include <version.h>

using namespace std::string_literals;
using json = nlohmann::json;

volatile int log_level = DEFAULT_LOG_LEVEL;

struct db_context {
    typedef std::set<std::string> primary_key_t;

    typedef struct foreign_key_type {
        std::string ref_table;
        std::string ref_schema;
        std::vector<std::string> columns;
        std::vector<std::string> ref_columns;
    } foreign_key_t;

    typedef std::vector<foreign_key_t> foreign_keys_t;

    typedef struct column_type {
        std::string name;
        std::string type;
        std::string comment;
        std::string default_value;
        //std::size_t size;
        bool primary_key = false;
        bool unique_key = false;
        bool not_null = false;
    } column_t;

    typedef struct table_type {
        std::string schema_name;
        std::string table_name;
        std::string comment;
        std::vector<column_t> columns;
        primary_key_t primary_key;
        std::vector<foreign_key_t> foreign_keys;
    } table_t;

    typedef std::string schema_t;

    std::string owner;

    std::set<schema_t> schemas;
    std::vector<table_t> tables;
};

static std::string to_postgres_type(const db_context::column_type &c) {
    std::string type_name;

    switch (strex::hash(c.type)) {
    case strex::hash("int"):
        type_name = "INTEGER"s;
        break;
    case strex::hash("int32"):
        type_name = "INTEGER"s;
        break;
    case strex::hash("int64"):
        type_name = "BIGINT"s;
        break;
    case strex::hash("str"):
        type_name = "TEXT"s;
        break;
    case strex::hash("text"):
        type_name = "TEXT"s;
        break;
    case strex::hash("bool"):
        type_name = "BOOLEAN"s;
        break;
    case strex::hash("buffer"):
        type_name = "bytea"s;
    case strex::hash("timestamp"):
        type_name = "timestamp"s;
    }

    if (c.unique_key)
        type_name += " UNIQUE"s;
    if (c.not_null)
        type_name += " NOT NULL"s;
    if (!c.default_value.empty())
        type_name += " DEFAULT " + c.default_value;

    return type_name;
}

static void json_to_ctx(const json &j, db_context &ctx) {
    using namespace std;

    if (j.find("owner") != j.end())
        ctx.owner = j["owner"].get<string>();

    if (j.find("tables") != j.end())
        for (auto &tbl : j["tables"]) {
            const auto table_name = tbl["name"].get<string>();

            const auto schema_name = tbl.find("schema") != tbl.end() ? tbl["schema"].get<string>() : string{};
            const auto table_comment = tbl.find("comment") != tbl.end() ? tbl["comment"].get<string>() : string{};

            if (!schema_name.empty())
                ctx.schemas.insert(schema_name);

            std::vector<db_context::column_t> all_columns;

            //auto is_primary_key = false;

            db_context::primary_key_t pk;
            db_context::foreign_keys_t fks;

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

                    all_columns.push_back({column_name, column_type, column_comment, default_value, column_pk , unique_column, column_not_null});
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

                    fks.push_back({fk_table, fk_schema, fk_columns, fk_references});
                }

            ctx.tables.push_back({schema_name, table_name, table_comment, all_columns, pk, fks});
        }
}

static int postgress_generator(const json &j) {
    using namespace std;

    const auto indent = "   "s;

    db_context ctx;
    json_to_ctx(j, ctx);

    cout << "BEGIN;\n\n";

    // schemas
    for (const auto &s : ctx.schemas) {
        if (s != "public")
            cout << "CREATE SCHEMA " << s << ";\n";
    }

    cout << "\n\n";

    for (const auto &t : ctx.tables) {
        const auto full_table_name = (t.schema_name.empty() ? t.table_name : t.schema_name + "." + t.table_name);

        cout << "CREATE TABLE " << full_table_name << " (\n";

        auto col_idx = 0ul;
        for (const auto &c : t.columns) {
            cout << indent << c.name << " " << to_postgres_type(c);
            if ((t.columns.size() > 1 && col_idx != t.columns.size() - 1) || !t.primary_key.empty())
                cout << ',';
            cout << '\n';
            col_idx++;
        }

        auto pk_idx = 0ul;
        if (!t.primary_key.empty()) {
            cout << indent << "CONSTRAINT " << "pk_" << t.table_name << " PRIMARY KEY (";

            auto pk_elem_idx = 0ul;
            for (const auto &pk : t.primary_key) {
                if (pk_elem_idx > 0)
                    cout << ' ';
                cout << pk;
                if (t.primary_key.size() > 1 && pk_elem_idx != t.primary_key.size() - 1)
                    cout << ',';
                pk_elem_idx++;
            }
            cout << ')';

            if (pk_idx != 0 || !t.foreign_keys.empty())
                cout << ',';

            cout << '\n';

            pk_idx++;
        }

        auto fk_idx = 0ul;
        if (!t.foreign_keys.empty()) {
            for (const auto &fk : t.foreign_keys) {
                cout << indent <<"FOREIGN KEY (";
                for (const auto &c : fk.columns)
                    cout << c;
                cout << ") REFERENCES ";
                cout << fk.ref_table << " (";
                for (const auto &ref : fk.ref_columns)
                    cout << ref;
                cout << ')';

                if ((fk_idx != 0 || !t.primary_key.empty()) && fk_idx != t.foreign_keys.size() - 1)
                    cout << ',';

                cout << '\n';

                fk_idx++;
            }
        }

        cout << ") WITH (OIDS = FALSE);\n";

        if (!t.comment.empty())
            cout << '\n' << "COMMENT ON TABLE " << full_table_name << " IS \'" << t.comment << "\';\n";

        cout << '\n';
        for (const auto &c : t.columns)
            if (!c.comment.empty())
                cout << "COMMENT ON COLUMN " << full_table_name << "." << c.name << " IS \'" << c.comment << "\';\n";

        if (!ctx.owner.empty())
            cout << '\n' << "ALTER TABLE " << full_table_name << " OWNER TO " << ctx.owner << ";\n";

        cout << '\n';
    }

    cout << "COMMIT;\n";

    return 0;
}

extern int main(int argc, char *argv[]) {
    using namespace std;

    const auto app_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : nullptr;

    string db_path = "./db.json";
    string gen_name = "postgres";

    unordered_map<string, function<int (const json &j)>> generators {
    {"postgres", postgress_generator}
    };

    xargs::args args;
    args.add_arg("DB_PATH", "Path to database", [&] (const auto &v) {
        db_path = v;
    }).add_option("-g", "Generator postgres, mysql, maria. Default: " + gen_name, [&] (const auto &v) {
        if (generators.find(v) == generators.end())
        {
            puts(args.usage(argv[0]).c_str());
            LOG_ERROR(app_name, "Generator not found %s", gen_name.c_str());
            exit(EXIT_SUCCESS);
        }
        gen_name = v;
    }).add_option("-h", "Display help", [&] () {
        puts(args.usage(argv[0]).c_str());
        exit(EXIT_SUCCESS);
    }).add_option("-v", "Version", [&] () {
        fprintf(stdout, "%s %s\n", app_name, LAUNCHDB_VERSION);
        exit(EXIT_SUCCESS);
    });

    args.dispath(argc, argv);

    if (static_cast<size_t>(argc) < args.count()) {
        puts(args.usage(argv[0]).c_str());
        return EXIT_SUCCESS;
    }

    ifstream fs(db_path, ios::in | ios::binary);
    if (!fs.is_open()) {
        LOG_ERROR(app_name, "Can't open %s", db_path.c_str());
        return EXIT_FAILURE;
    }

    string contents;
    fs.seekg(0, ios::end);
    contents.resize(fs.tellg());
    fs.seekg(0, ios::beg);
    fs.read(&contents[0], contents.size());
    fs.close();

    json j = json::parse(contents);

    generators[gen_name](j);

    return 0;
}
