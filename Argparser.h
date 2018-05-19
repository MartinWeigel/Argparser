// Copyright (C) 2012-2015 Yecheng Fu <cofyc.jackson at gmail dot com>
// Copyright (C) 2018 Martin Weigel <mail@MartinWeigel.com>
// License: MIT
#pragma once
#include <stdint.h>
#include <stdbool.h>

enum argparser_flag {
    ARGPARSER_STOP_AT_NON_OPTION = 1,
};

enum ArgparserOptionType {
    /* special */
    ARGPARSER_OPT_END,
    ARGPARSER_OPT_GROUP,
    /* options with no arguments */
    ARGPARSER_OPT_BOOLEAN,
    /* options with arguments (optional or required) */
    ARGPARSER_OPT_INTEGER,
    ARGPARSER_OPT_FLOAT,
    ARGPARSER_OPT_STRING,
};

enum argparser_option_flags {
    OPT_NONEG = 1,              /* disable negation */
};

// Built-in callbacks
typedef int Argparser_callback(void* self, const void* option);
int Argparser_help_cb(void* self, const void* option);

/**
 *  argparse option
 *
 *  `type`:
 *    holds the type of the option, you must have an ARGPARSER_OPT_END last in your
 *    array.
 *
 *  `short_name`:
 *    the character to use as a short option name, '\0' if none.
 *
 *  `long_name`:
 *    the long option name, without the leading dash, NULL if none.
 *
 *  `value`:
 *    stores pointer to the value to be filled.
 *
 *  `help`:
 *    the short help message associated to what the option does.
 *    Must never be NULL (except for ARGPARSER_OPT_END).
 *
 *  `callback`:
 *    function is called when corresponding argument is parsed.
 *
 *  `data`:
 *    associated data. Callbacks can use it like they want.
 *
 *  `flags`:
 *    option flags.
 */
typedef struct ArgparserOption {
    enum ArgparserOptionType type;
    const char short_name;
    const char *long_name;
    void *value;
    const char *help;
    Argparser_callback *callback;
    intptr_t data;
    int flags;
} ArgparserOption;

// built-in option macros
#define OPT_END()        { ARGPARSER_OPT_END, 0, NULL, NULL, 0, NULL, 0, 0 }
#define OPT_BOOLEAN(...) { ARGPARSER_OPT_BOOLEAN, __VA_ARGS__ }
#define OPT_INTEGER(...) { ARGPARSER_OPT_INTEGER, __VA_ARGS__ }
#define OPT_FLOAT(...)   { ARGPARSER_OPT_FLOAT, __VA_ARGS__ }
#define OPT_STRING(...)  { ARGPARSER_OPT_STRING, __VA_ARGS__ }
#define OPT_GROUP(h)     { ARGPARSER_OPT_GROUP, 0, NULL, NULL, h, NULL, 0, 0 }
#define OPT_HELP()       OPT_BOOLEAN('h', "help", NULL,                 \
                                     "show this help message and exit", \
                                     Argparser_help_cb, 0, 0)

typedef struct Argparser {
    bool valid;
    // user supplied
    const ArgparserOption *options;
    const char *const *usages;
    int flags;
    const char *description;    // a description after usage
    const char *epilog;         // a description at the end
    // internal context
    int argc;
    const char **argv;
    const char **out;
    int cpidx;
    const char *optvalue;       // current option value
} Argparser;


// The following functions should be used from your application
Argparser* Argparser_new();
void Argparser_init(Argparser* self,
    ArgparserOption* options, const char *const *usages, int flags);
void Argparser_setDescription(Argparser* self, const char* description);
void Argparser_setEpilog(Argparser* self, const char* epilog);
int Argparser_parse(Argparser* self, int argc, const char **argv);
void Argparser_clear(Argparser* self);
void Argparser_delete(Argparser* self);



#ifdef ARGPARSER_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#define OPT_UNSET 1
#define OPT_LONG  (1 << 1)

const char* Argparser_prefix_skip(const char *str, const char *prefix)
{
    size_t len = strlen(prefix);
    return strncmp(str, prefix, len) ? NULL : str + len;
}

void Argparser_error(Argparser* self, const ArgparserOption* opt, const char* reason, int flags)
{
    if (flags & OPT_LONG) {
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
        case ARGPARSER_OPT_BOOLEAN:
            if (flags & OPT_UNSET) {
                *(int *)opt->value = *(int *)opt->value - 1;
            } else {
                *(int *)opt->value = *(int *)opt->value + 1;
            }
            if (*(int *)opt->value < 0) {
                *(int *)opt->value = 0;
            }
            break;
        case ARGPARSER_OPT_STRING:
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
        case ARGPARSER_OPT_INTEGER:
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
        case ARGPARSER_OPT_FLOAT:
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
    for (; options->type != ARGPARSER_OPT_END; options++) {
        switch (options->type) {
        case ARGPARSER_OPT_END:
        case ARGPARSER_OPT_BOOLEAN:
        case ARGPARSER_OPT_INTEGER:
        case ARGPARSER_OPT_FLOAT:
        case ARGPARSER_OPT_STRING:
        case ARGPARSER_OPT_GROUP:
            continue;
        default:
            fprintf(stderr, "wrong option type: %d", options->type);
            break;
        }
    }
}

int Argparser_short_opt(Argparser* self, const ArgparserOption* options)
{
    for (; options->type != ARGPARSER_OPT_END; options++) {
        if (options->short_name == *self->optvalue) {
            self->optvalue = self->optvalue[1] ? self->optvalue + 1 : NULL;
            return Argparser_getvalue(self, options, 0);
        }
    }
    return -2;
}

int Argparser_long_opt(Argparser* self, const ArgparserOption* options)
{
    for (; options->type != ARGPARSER_OPT_END; options++) {
        const char *rest;
        int opt_flags = 0;
        if (!options->long_name)
            continue;

        rest = Argparser_prefix_skip(self->argv[0] + 2, options->long_name);
        if (!rest) {
            // negation disabled?
            if (options->flags & OPT_NONEG) {
                continue;
            }
            // only OPT_BOOLEAN/OPT_BIT supports negation
            if (options->type != ARGPARSER_OPT_BOOLEAN) {
                continue;
            }

            if (strncmp(self->argv[0] + 2, "no-", 3)) {
                continue;
            }
            rest = Argparser_prefix_skip(self->argv[0] + 2 + 3, options->long_name);
            if (!rest)
                continue;
            opt_flags |= OPT_UNSET;
        }
        if (*rest) {
            if (*rest != '=')
                continue;
            self->optvalue = rest + 1;
        }
        return Argparser_getvalue(self, options, opt_flags | OPT_LONG);
    }
    return -2;
}


Argparser* Argparser_new()
{
    Argparser* self = malloc(sizeof(Argparser));
    memset(self, 0, sizeof(*self));
    return self;
}

void Argparser_init(Argparser* self,
    ArgparserOption* options, const char* const* usages, int flags)
{
    self->options     = options;
    self->usages      = usages;
    self->flags       = flags;
    self->valid       = true;
}

void Argparser_setDescription(Argparser* self, const char* description)
{
    self->description = description;
}

void Argparser_setEpilog(Argparser* self, const char* epilog)
{
    self->epilog      = epilog;
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
    for (; options->type != ARGPARSER_OPT_END; options++) {
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
        if (options->type == ARGPARSER_OPT_INTEGER) {
            len += strlen("=<int>");
          }
        if (options->type == ARGPARSER_OPT_FLOAT) {
            len += strlen("=<flt>");
        } else if (options->type == ARGPARSER_OPT_STRING) {
            len += strlen("=<str>");
        }
        len = (len + 3) - ((len + 3) & 3);
        if (usage_opts_width < len) {
            usage_opts_width = len;
        }
    }
    usage_opts_width += 4;      // 4 spaces prefix

    options = self->options;
    for (; options->type != ARGPARSER_OPT_END; options++) {
        size_t pos = 0;
        int pad    = 0;
        if (options->type == ARGPARSER_OPT_GROUP) {
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
        if (options->type == ARGPARSER_OPT_INTEGER) {
            pos += fprintf(stdout, "=<int>");
        }
        if (options->type == ARGPARSER_OPT_FLOAT) {
            pos += fprintf(stdout, "=<flt>");
        } else if (options->type == ARGPARSER_OPT_STRING) {
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
            if (self->flags & ARGPARSER_STOP_AT_NON_OPTION) {
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

int Argparser_help_cb(void* self, const void* option)
{ 
    Argparser_usage(self);
    exit(0);
}

#endif
