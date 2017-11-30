#include <cstdlib>
#include <fstream>

#include <unordered_map>

#include <json.hpp>
#include <journal.hpp>
#include <xargs.hpp>

#include <context.hpp>
#include <journal.hpp>
#include <launchdb/config.h>

#include <postgres/upgrage.hpp>
#include <sqlite/render.h>

using json = nlohmann::json;

volatile int log_level = DEFAULT_LOG_LEVEL;

typedef std::function<int (const db::context_diff &)> migrator_t;

std::string read_contents(const std::string &filepath) {
    using namespace std;

    ifstream fs(filepath, ios::in | ios::binary);
    if (!fs.is_open()) {
        LOG_ERROR("migrate", "Can't open %1", filepath);
        return {};
    }

    string contents;
    fs.seekg(0, ios::end);
    contents.resize(fs.tellg());
    fs.seekg(0, ios::beg);
    fs.read(&contents[0], contents.size());
    fs.close();

    return contents;
}

db::context process_json(const json &j);
db::context_diff get_diff(const db::context &old_ctx, const db::context &new_ctx);

extern int main(int argc, char *argv[]) {
    using namespace std;

    const auto app_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : nullptr;

    string from_filename;
    string to_filename;
    string mig_name = "postgres";

    unordered_map<string, migrator_t> migrators {
        {"postgres", {postgres::upgrade}},
        {"sqlite", {sqlite::upgrade}}
    };

    xargs::args args;
    args.add_arg("DB_PATH_FROM", "Path to database", [&] (const auto &v) {
        from_filename = v;
    }).add_arg("DB_PATH_TO", "Path to database", [&] (const auto &v) {
        to_filename = v;
    }).add_option("-m", "Migrator postgres, mysql, maria, sqlite. Default: " + mig_name, [&] (const auto &v) {
        if (migrators.find(v) == migrators.end())
        {
            puts(args.usage(argv[0]).c_str());
            LOG_ERROR(app_name, "Migrator not found %1", mig_name);
            exit(EXIT_SUCCESS);
        }
        mig_name = v;
    }).add_option("-u", "Update (default)", [&] (const auto &v) {
        (void)v;
    }).add_option("-d", "Downgrade", [&] (const auto &v) {
        (void)v;
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

    if (from_filename.empty() || to_filename.empty()) {
        LOG_ERROR(app_name, "%1", "DB_PATH_FROM or DB_PATH_TO is empty");
        return EXIT_FAILURE;
    }

    const auto from_contents = read_contents(from_filename);
    const auto to_contents = read_contents(to_filename);

    if (from_contents.empty()) {
        LOG_ERROR("migrate", "input %1 contents empty!", from_filename);
        return EXIT_FAILURE;
    }

    if (to_contents.empty()) {
        LOG_ERROR("migrate", "input %1 contents empty!", from_filename);
        return EXIT_FAILURE;
    }

    auto old_db = process_json(json::parse(from_contents));
    auto new_db = process_json(json::parse(to_contents));

    auto diff_db = get_diff(old_db, new_db);

    migrators[mig_name](diff_db);

    return EXIT_SUCCESS;
}
