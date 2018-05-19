#define ARGPARSER_IMPLEMENTATION
#include "Argparser.h"

int test_callback(Argparser* argparser, const ArgparserOption* option)
{ 
    int* value = ((int*)option->value);
    printf("Callback called with value: %d\n", *value);
    return 1;
}

int main(int argc, const char **argv)
{
    // Define variables that will be written using argparse
    int force = 0;
    int test = 0;
    int int_num = 0;
    float flt_num = 0.f;
    const char *path = NULL;
    int perms = 0;

    // Parse arguments
    Argparser* argparser = Argparser_new();
    Argparser_init(argparser, (ArgparserOption[]) {
        ARGPARSER_OPT_GROUP("Basic Options"),
        ARGPARSER_OPT_HELP(),
        ARGPARSER_OPT_BOOL('f', "force", &force, "force to do"),
        ARGPARSER_OPT_STRING('p', "path", &path, "path to read"),
        ARGPARSER_OPT_INT('i', "int", &int_num, "selected integer"),
        ARGPARSER_OPT_FLOAT('s', "float", &flt_num, "selected float"),
        ARGPARSER_OPT_GROUP("Options with Callbacks"),
        ARGPARSER_OPT_INT_CALLBACK('t', "test", &test, "test with callback", test_callback),
        ARGPARSER_OPT_END()
    });
    Argparser_setUsage(argparser, "example [options] [[--] args");
    Argparser_setDescription(argparser, "Optional brief description of what the program does and how it works.");
    Argparser_setEpilog(argparser, "Optional description of the program after the description of the arguments.");
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
