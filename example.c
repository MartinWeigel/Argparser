#define ARGPARSER_IMPLEMENTATION
#include "Argparser.h"

static const char *const usages[] = {
    "example [options] [[--] args]",
    "example [options]",
    NULL,
};

int main(int argc, const char **argv)
{
    // Define variables that will be written using argparse
    int force = 0;
    int test = 0;
    int int_num = 0;
    float flt_num = 0.f;
    const char *path = NULL;
    int perms = 0;

    // Define all application options
    ArgparserOption options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic options"),
        OPT_BOOLEAN('f', "force", &force, "force to do"),
        OPT_BOOLEAN('t', "test", &test, "test only"),
        OPT_STRING('p', "path", &path, "path to read"),
        OPT_INTEGER('i', "int", &int_num, "selected integer"),
        OPT_FLOAT('s', "float", &flt_num, "selected float"),
        OPT_END(),
    };

    // Parse arguments
    Argparser* argparser = Argparser_new();
    Argparser_init(argparser, options, usages, 0);
    Argparser_setDescription(argparser, "\nOptional brief description of what the program does and how it works.");
    Argparser_setEpilog(argparser, "\nOptional description of the program after the description of the arguments.");
    argc = Argparser_parse(argparser, argc, argv);
    Argparser_clear(argparser);
    Argparser_delete(argparser);

    // Display all results that are not their default
    if (force != 0)
        printf("force: %d\n", force);
    if (test != 0)
        printf("test: %d\n", test);
    if (path != NULL)
        printf("path: %s\n", path);
    if (int_num != 0)
        printf("int_num: %d\n", int_num);
    if (flt_num != 0)
        printf("flt_num: %g\n", flt_num);
    if (argc != 0) {
        printf("argc: %d\n", argc);
        int i;
        for (i = 0; i < argc; i++) {
            printf("argv[%d]: %s\n", i, *(argv + i));
        }
    }
    if (perms) {
        printf("perms: %d\n", perms);
    }
    return 0;
}
