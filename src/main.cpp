#include <cstdlib>

#include <unordered_map>
#include <tuple>
#include <functional>

#include <iostream>
#include <fstream>

#include <json.hpp>
#include <journal.hpp>
#include <xargs.hpp>

#include <version.h>

#include "context.hpp"

using json = nlohmann::json;

db::context process_json(const json &j);

volatile int log_level = DEFAULT_LOG_LEVEL;

typedef std::function<int (const db::context &, const db::config &)> generator_t;

namespace postgres {
    int output(const db::context &ctx, const db::config &cfg);
}

extern int main(int argc, char *argv[]) {
    using namespace std;

    const auto app_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : nullptr;

    string db_path = "./db.json";
    string gen_name = "postgres";

    unordered_map<string, generator_t> generators {
        {"postgres", postgres::output}
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

    generators[gen_name](process_json(j), db::config{});

    return 0;
}
