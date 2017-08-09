#include <cstdlib>

#include <unordered_map>

#include <json.hpp>
#include <journal.hpp>
#include <xargs.hpp>

#include <context.hpp>

#include <version.h>

volatile int log_level = DEFAULT_LOG_LEVEL;

typedef struct migrator_type {

} migrator_t;

extern int main(int argc, char *argv[]) {
    using namespace std;

    const auto app_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : nullptr;

    string from;
    string to;
    string mig_name = "postgres";

    unordered_map<string, migrator_t> migrators {
        {"postgres", {}}
    };

    xargs::args args;
    args.add_arg("DB_PATH_FROM", "Path to database", [&] (const auto &v) {
        from = v;
    }).add_arg("DB_PATH_TO", "Path to database", [&] (const auto &v) {
        to = v;
    }).add_option("-m", "Migrator postgres, mysql, maria. Default: " + mig_name, [&] (const auto &v) {
        if (migrators.find(v) == migrators.end())
        {
            puts(args.usage(argv[0]).c_str());
            LOG_ERROR(app_name, "Migrator not found %s", mig_name.c_str());
            exit(EXIT_SUCCESS);
        }
        mig_name = v;
    }).add_option("-u", "Update (default)", [&] (const auto &v) {
    }).add_option("-d", "Downgrade", [&] (const auto &v) {
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

    if (from.empty() || to.empty()) {
        LOG_ERROR(app_name, "%s", "DB_PATH_FROM or DB_PATH_TO is empty");
        return EXIT_FAILURE;
    }



    return EXIT_SUCCESS;
}
