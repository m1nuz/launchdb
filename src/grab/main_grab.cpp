#include <cstdlib>
#include <xargs.hpp>

#include <version.h>

extern int main(int argc, char *argv[]) {
    using namespace std;

    const auto app_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : nullptr;

    string db_name;
    string db_type = "postgres";
    xargs::args args;
    args.add_arg("DB_NAME", "Database name", [&] (const auto &v) {
        db_name = v;
    }).add_option("-d", "Grap from postgres, mysql, maria. Default: " + db_name, [&] (const auto &v) {
        db_type = v;
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

    return 0;
}
