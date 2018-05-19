// Copyright (C) 2012-2015 Yecheng Fu <cofyc.jackson at gmail dot com>
// Copyright (C) 2018 Martin Weigel <mail@MartinWeigel.com>
// License: MIT
#pragma once
#include <stdint.h>
#include <stdbool.h>

// Type definitions
typedef struct Argparser Argparser;
typedef struct ArgparserOption ArgparserOption;
typedef int Argparser_callback(Argparser* self, const ArgparserOption* option);

// Public functions
Argparser* Argparser_new();
void Argparser_init(Argparser* self, ArgparserOption* options, const char *const *usages);
void Argparser_setDescription(Argparser* self, const char* description);
void Argparser_setEpilog(Argparser* self, const char* epilog);
void Argparser_setStopAtNonOption(Argparser* self, bool stop);
int Argparser_parse(Argparser* self, int argc, const char **argv);
void Argparser_clear(Argparser* self);
void Argparser_delete(Argparser* self);

// Public macros for creation of ArgparserOption
#define ARGPARSER_OPT_BOOL(shortName, longName, valuePtr, description) \
    { ARGPARSER_TYPE_BOOLEAN, shortName, longName, valuePtr, description }
#define ARGPARSER_OPT_BOOL_CALLBACK(shortName, longName, valuePtr, description, callback) \
    { ARGPARSER_TYPE_BOOLEAN, shortName, longName, valuePtr, description, callback }

#define ARGPARSER_OPT_INT(shortName, longName, valuePtr, description) \
    { ARGPARSER_TYPE_INTEGER, shortName, longName, valuePtr, description }
#define ARGPARSER_OPT_INT_CALLBACK(shortName, longName, valuePtr, description, callback) \
    { ARGPARSER_TYPE_INTEGER, shortName, longName, valuePtr, description, callback }

#define ARGPARSER_OPT_FLOAT(shortName, longName, valuePtr, description) \
    { ARGPARSER_TYPE_FLOAT, shortName, longName, valuePtr, description }
#define ARGPARSER_OPT_FLOAT_CALLBACK(shortName, longName, valuePtr, description, callback) \
    { ARGPARSER_TYPE_FLOAT, shortName, longName, valuePtr, description, callback }

#define ARGPARSER_OPT_STRING(shortName, longName, valuePtr, description) \
    { ARGPARSER_TYPE_STRING, shortName, longName, valuePtr, description }
#define ARGPARSER_OPT_STRING_CALLBACK(shortName, longName, valuePtr, description, callback) \
    { ARGPARSER_TYPE_STRING, shortName, longName, valuePtr, description, callback }

#define ARGPARSER_OPT_GROUP(description)     { ARGPARSER_TYPE_GROUP, 0, NULL, NULL, description, NULL }
#define ARGPARSER_OPT_END()                  { ARGPARSER_TYPE_END, 0, NULL, NULL, 0, NULL }

int Argparser_help_cb(Argparser* self, const ArgparserOption* option);
#define ARGPARSER_OPT_HELP()       \
    ARGPARSER_OPT_BOOL_CALLBACK('h', "help", NULL, "show this help message and exit", Argparser_help_cb)



// Definition of data structures
// Please only modify them through public functions
enum ArgparserOptionType {
    ARGPARSER_TYPE_END,
    ARGPARSER_TYPE_GROUP,
    ARGPARSER_TYPE_BOOLEAN,
    ARGPARSER_TYPE_INTEGER,
    ARGPARSER_TYPE_FLOAT,
    ARGPARSER_TYPE_STRING,
};

typedef struct ArgparserOption {
    enum ArgparserOptionType type;
    const char short_name;
    const char *long_name;
    void *value;
    const char *help;
    Argparser_callback *callback;
} ArgparserOption;

typedef struct Argparser {
    bool valid;
    const ArgparserOption *options;
    const char *const *usages;
    const char *description;
    const char *epilog;
    bool stopAtNonOption;
    // Internal variables
    int argc;
    const char **argv;
    const char **out;
    int cpidx;
    const char *optvalue;
} Argparser;



#ifdef ARGPARSER_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define ARGPARSER_OPTFLAG_UNSET 1
#define ARGPARSER_OPTFLAG_LONG  (1 << 1)


Argparser* Argparser_new()
{
    Argparser* self = malloc(sizeof(Argparser));
    memset(self, 0, sizeof(*self));
    return self;
}

void Argparser_init(Argparser* self, ArgparserOption* options, const char* const* usages)
{
    self->options = options;
    self->usages = usages;
    self->stopAtNonOption = false;
    self->valid = true;
}

void Argparser_setDescription(Argparser* self, const char* description)
{
    self->description = description;
}

void Argparser_setEpilog(Argparser* self, const char* epilog)
{
    self->epilog = epilog;
}

void Argparser_setStopAtNonOption(Argparser* self, bool stop)
{
    self->stopAtNonOption = stop;
}

void Argparser_clear(Argparser* self)
{
    self->valid = false;
    memset(self, 0, sizeof(*self));
}

void Argparser_delete(Argparser* self)
{
    free(self);
}

void Argparser_error(Argparser* self, const ArgparserOption* opt, const char* reason, int flags)
{
    if (flags & ARGPARSER_OPTFLAG_LONG) {
        fprintf(stderr, "error: option `--%s` %s\n", opt->long_name, reason);
    } else {
        fprintf(stderr, "error: option `-%c` %s\n", opt->short_name, reason);
    }
    exit(1);
}

int Argparser_getvalue(Argparser* self, const ArgparserOption* opt, int flags)
{
    const char *s = NULL;
    if (opt->value) {
        switch (opt->type) {
        case ARGPARSER_TYPE_BOOLEAN:
            if (flags & ARGPARSER_OPTFLAG_UNSET) {
                *(int *)opt->value = *(int *)opt->value - 1;
            } else {
                *(int *)opt->value = *(int *)opt->value + 1;
            }
            if (*(int *)opt->value < 0) {
                *(int *)opt->value = 0;
            }
            break;
        case ARGPARSER_TYPE_STRING:
            if (self->optvalue) {
                *(const char **)opt->value = self->optvalue;
                self->optvalue             = NULL;
            } else if (self->argc > 1) {
                self->argc--;
                *(const char **)opt->value = *++self->argv;
            } else {
                Argparser_error(self, opt, "requires a value", flags);
            }
            break;
        case ARGPARSER_TYPE_INTEGER:
            errno = 0; 
            if (self->optvalue) {
                *(int *)opt->value = strtol(self->optvalue, (char **)&s, 0);
                self->optvalue     = NULL;
            } else if (self->argc > 1) {
                self->argc--;
                *(int *)opt->value = strtol(*++self->argv, (char **)&s, 0);
            } else {
                Argparser_error(self, opt, "requires a value", flags);
            }
            if (errno) 
                Argparser_error(self, opt, strerror(errno), flags);
            if (s[0] != '\0')
                Argparser_error(self, opt, "expects an integer value", flags);
            break;
        case ARGPARSER_TYPE_FLOAT:
            errno = 0; 
            if (self->optvalue) {
                *(float *)opt->value = strtof(self->optvalue, (char **)&s);
                self->optvalue     = NULL;
            } else if (self->argc > 1) {
                self->argc--;
                *(float *)opt->value = strtof(*++self->argv, (char **)&s);
            } else {
                Argparser_error(self, opt, "requires a value", flags);
            }
            if (errno) 
                Argparser_error(self, opt, strerror(errno), flags);
            if (s[0] != '\0')
                Argparser_error(self, opt, "expects a numerical value", flags);
            break;
        default:
            assert(0);
        }
    }

    if (opt->callback) {
        return opt->callback(self, opt);
    }
    return 0;
}

void Argparser_options_check(const ArgparserOption* options)
{
    for (; options->type != ARGPARSER_TYPE_END; options++) {
        switch (options->type) {
        case ARGPARSER_TYPE_END:
        case ARGPARSER_TYPE_BOOLEAN:
        case ARGPARSER_TYPE_INTEGER:
        case ARGPARSER_TYPE_FLOAT:
        case ARGPARSER_TYPE_STRING:
        case ARGPARSER_TYPE_GROUP:
            continue;
        default:
            fprintf(stderr, "wrong option type: %d", options->type);
            break;
        }
    }
}

int Argparser_short_opt(Argparser* self, const ArgparserOption* options)
{
    for (; options->type != ARGPARSER_TYPE_END; options++) {
        if (options->short_name == *self->optvalue) {
            self->optvalue = self->optvalue[1] ? self->optvalue + 1 : NULL;
            return Argparser_getvalue(self, options, 0);
        }
    }
    return -2;
}

int Argparser_long_opt(Argparser* self, const ArgparserOption* options)
{
    for (; options->type != ARGPARSER_TYPE_END; options++) {
        if (!options->long_name)
            continue;

        size_t nameLength = strlen(options->long_name);
        const char *rest = strncmp(self->argv[0] + 2, options->long_name, nameLength) ?
            NULL : self->argv[0] + 2 + nameLength;
        if (rest) {
            if (*rest != '=')
                continue;
            self->optvalue = rest + 1;
            return Argparser_getvalue(self, options, ARGPARSER_OPTFLAG_LONG);
        }
    }
    return -2;
}

void Argparser_usage(Argparser* self)
{
    if (self->usages) {
        fprintf(stdout, "Usage: %s\n", *self->usages++);
        while (*self->usages && **self->usages)
            fprintf(stdout, "   or: %s\n", *self->usages++);
    } else {
        fprintf(stdout, "Usage:\n");
    }

    // print description
    if (self->description)
        fprintf(stdout, "%s\n", self->description);

    fputc('\n', stdout);

    const struct ArgparserOption *options;

    // figure out best width
    size_t usage_opts_width = 0;
    size_t len;
    options = self->options;
    for (; options->type != ARGPARSER_TYPE_END; options++) {
        len = 0;
        if ((options)->short_name) {
            len += 2;
        }
        if ((options)->short_name && (options)->long_name) {
            len += 2;           // separator ", "
        }
        if ((options)->long_name) {
            len += strlen((options)->long_name) + 2;
        }
        if (options->type == ARGPARSER_TYPE_INTEGER) {
            len += strlen("=<int>");
          }
        if (options->type == ARGPARSER_TYPE_FLOAT) {
            len += strlen("=<flt>");
        } else if (options->type == ARGPARSER_TYPE_STRING) {
            len += strlen("=<str>");
        }
        len = (len + 3) - ((len + 3) & 3);
        if (usage_opts_width < len) {
            usage_opts_width = len;
        }
    }
    usage_opts_width += 4;      // 4 spaces prefix

    options = self->options;
    for (; options->type != ARGPARSER_TYPE_END; options++) {
        size_t pos = 0;
        int pad    = 0;
        if (options->type == ARGPARSER_TYPE_GROUP) {
            fputc('\n', stdout);
            fprintf(stdout, "%s", options->help);
            fputc('\n', stdout);
            continue;
        }
        pos = fprintf(stdout, "    ");
        if (options->short_name) {
            pos += fprintf(stdout, "-%c", options->short_name);
        }
        if (options->long_name && options->short_name) {
            pos += fprintf(stdout, ", ");
        }
        if (options->long_name) {
            pos += fprintf(stdout, "--%s", options->long_name);
        }
        if (options->type == ARGPARSER_TYPE_INTEGER) {
            pos += fprintf(stdout, "=<int>");
        }
        if (options->type == ARGPARSER_TYPE_FLOAT) {
            pos += fprintf(stdout, "=<flt>");
        } else if (options->type == ARGPARSER_TYPE_STRING) {
            pos += fprintf(stdout, "=<str>");
        }
        if (pos <= usage_opts_width) {
            pad = usage_opts_width - pos;
        } else {
            fputc('\n', stdout);
            pad = usage_opts_width;
        }
        fprintf(stdout, "%*s%s\n", pad + 2, "", options->help);
    }

    // print epilog
    if (self->epilog)
        fprintf(stdout, "%s\n", self->epilog);
}

int Argparser_help_cb(Argparser* self, const ArgparserOption* option)
{ 
    Argparser_usage(self);
    exit(0);
}

int Argparser_parse(Argparser* self, int argc, const char **argv)
{
    assert(self->valid);

    self->argc = argc - 1;
    self->argv = argv + 1;
    self->out  = argv;

    Argparser_options_check(self->options);

    for (; self->argc; self->argc--, self->argv++) {
        const char *arg = self->argv[0];
        if (arg[0] != '-' || !arg[1]) {
            if (self->stopAtNonOption) {
                goto end;
            }
            // if it's not option or is a single char '-', copy verbatim
            self->out[self->cpidx++] = self->argv[0];
            continue;
        }
        // short option
        if (arg[1] != '-') {
            self->optvalue = arg + 1;
            switch (Argparser_short_opt(self, self->options)) {
            case -1:
                break;
            case -2:
                goto unknown;
            }
            while (self->optvalue) {
                switch (Argparser_short_opt(self, self->options)) {
                case -1:
                    break;
                case -2:
                    goto unknown;
                }
            }
            continue;
        }
        // if '--' presents
        if (!arg[2]) {
            self->argc--;
            self->argv++;
            break;
        }
        // long option
        switch (Argparser_long_opt(self, self->options)) {
        case -1:
            break;
        case -2:
            goto unknown;
        }
        continue;

unknown:
        fprintf(stderr, "error: unknown option `%s`\n", self->argv[0]);
        Argparser_usage(self);
        exit(1);
    }

end:
    memmove(self->out + self->cpidx, self->argv,
            self->argc * sizeof(*self->out));
    self->out[self->cpidx + self->argc] = NULL;

    return self->cpidx + self->argc;
}

#endif
