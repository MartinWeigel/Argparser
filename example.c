#define ARGPARSER_IMPLEMENTATION
#include "Argparser.h"
#include <stdbool.h>

// Callback checks if the given value is between 0 and 10 or exits
void test_callback(Argparser* argparser, const ArgparserOption* option)
{ 
    int value = *((int*)option->value);

    if (value < 0 || value > 10)
        Argparser_exitDueToError(argparser, option, "value out of range [0, 10]");

    printf("Callback called with value: %d\n", value);
}

int main(int argc, const char **argv)
{
    // Variables that will be set using Argparser
    // They keep their initial value, if the option is not given.
    bool pBool = 0;
    int pInt = 0;
    float pFloat = 0.f;
    const char *pString = NULL;
    bool pNoShort = 0;
    bool pNoLong = 0;
    bool pUpperCharacters = 0;
    bool pNumeric = 0;
    int pIntCallback = 0;

    // Parse arguments
    Argparser* argparser = Argparser_new();
    Argparser_init(argparser, (ArgparserOption[]) {
        ARGPARSER_OPT_GROUP("Basic Options"),
        ARGPARSER_OPT_HELP(),
        ARGPARSER_OPT_BOOL('c', "check", &pBool, "example for a boolean"),
        ARGPARSER_OPT_INT('n', "number", &pInt, "example for an integer"),
        ARGPARSER_OPT_FLOAT('p', "percent", &pFloat, "example for a float"),
        ARGPARSER_OPT_STRING('l', "lastname", &pString, "example for a string"),

        ARGPARSER_OPT_GROUP("More Options"),
        ARGPARSER_OPT_BOOL('A', "UpperChars", &pUpperCharacters, "example with upper characters"),
        ARGPARSER_OPT_BOOL('\0', "no-short", &pNoShort, "example without short name"),
        ARGPARSER_OPT_BOOL('s', NULL, &pNoLong, "example with no long name"),
        ARGPARSER_OPT_BOOL('7', "seven77", &pNumeric, "example with numeric names"),

        ARGPARSER_OPT_GROUP("Options with Callbacks"),
        ARGPARSER_OPT_INT_CALLBACK('x', "callback", &pIntCallback, "example for an integer in range [0, 10]", test_callback),
        ARGPARSER_OPT_END()
    });
    Argparser_setUsage(argparser, "example [options] [[--] args");
    Argparser_setDescription(argparser, "Optional brief description of what the program does and how it works.");
    Argparser_setEpilog(argparser, "Optional description of the program after the description of the arguments.");
    argc = Argparser_parse(argparser, argc, argv);
    Argparser_clear(argparser);
    Argparser_delete(argparser);

    // Print all values
    printf("\n");
    printf("Boolean (-c / --check):    %d\n", pBool);
    printf("Integer (-n / --number):   %d\n", pInt);
    printf("Float   (-p / --percent):  %f\n", pFloat);
    printf("String  (-l / --lastname): %s\n", pString);
    printf("\n");
    printf("Upper Characters (-A / --UpperChars): %d\n", pUpperCharacters);
    printf("No Short Name    (     --no-short):   %d\n", pNoShort);
    printf("No Long Name     (-s):                %d\n", pNoLong);
    printf("Numeric Names    (-7 / --seven77):    %d\n", pNumeric);
    printf("\n");
    printf("Integer With Callback (-x / --callback): %d\n", pIntCallback);
    printf("\n");

    // Print remaining arguments
    if (argc != 0) {
        printf("argc: %d\n", argc);
        for (int i = 0; i < argc; i++) {
            printf("argv[%d]: %s\n", i, *(argv + i));
        }
    }
    return 0;
}
