#include <cstdlib>

#include <json.hpp>
#include <journal.hpp>
#include <xargs.hpp>

#include "../context.hpp"

#include <migrate_version.h>

extern int main(int argc, char *argv[]) {
    const auto app_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : nullptr;

    xargs::args args;
    args.add_arg("DB_PATH", "Path to database", [&] (const auto &v) {
    }).add_option("-u", "Update", [&] (const auto &v) {
    }).add_option("-d", "Downgrade", [&] (const auto &v) {
    }).add_option("-h", "Display help", [&] () {
        puts(args.usage(argv[0]).c_str());
        exit(EXIT_SUCCESS);
    }).add_option("-v", "Version", [&] () {
        fprintf(stdout, "%s %s\n", app_name, MIGRATEDB_VERSION);
        exit(EXIT_SUCCESS);
    });

    args.dispath(argc, argv);

    if (static_cast<size_t>(argc) < args.count()) {
        puts(args.usage(argv[0]).c_str());
        return EXIT_SUCCESS;
    }

    return 0;
}
