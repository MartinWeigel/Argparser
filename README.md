# Argparser

*Argparser* is a single-header command line argument-parser library written in C.

Arguments parsing is common task for all kinds of applications.
Parsing the given arguments is an important but cumbersome task.
This library can ease argument parsing in your existing or future C applications.

Using *Argparser* in your application is simple.
Just add `Argparser.h` to your project directory and include it using:

```C
    #define ARGPARSER_IMPLEMENTATION
    #include "Argparser.h"
```

Afterwards, you can specify the options that are used by your application.
*Argparser* figures out how to parse them out of `argc` and `argv`.
The options will automatically be filled in the given variables.

You can also bind callback-functions to specific options.
They are automatically called after the option was processed.
This allows for more control over the entered input.
For example, it can be used to restrict the input range, accept only valid file paths, or to modify a given value.

Last but not least, *Argparser* automatically generates help and usage messages.

An example can be found in [`example.c`](https://github.com/MartinWeigel/Argparser/blob/master/example.c).

*Argparser* is based on [`argparse`](https://github.com/Cofyc/argparse) written by Yecheng Fu.
The library is licensed under the [MIT license](https://github.com/MartinWeigel/Argparser/blob/master/LICENSE)

## Features

- Single-header library for easy integration into existing projects
- Short (`-n`) and long (`--name`) option names
- Callbacks for advanced input control
- Returns the remaining command line arguments for easier processing
- Produces informative usage messages (`--help`)
- Issues errors when invalid arguments are given


## Supported Arguments

*Argparser* currently supports four option types:

1. boolean
2. int
3. float
4. string

They can be specified in two ways:

1. *Short options* consist of one dash (`-`) and one alphanumeric character.
   They are usually followed by their value (`-b 0`/`-b 1`).
   Boolean-options without a value are set to `true`, i.e., `-b` equals `-b 1`).
   In this case, their short options can be bundled, i.e., `-a -b` equals `-ab`.
2. *Long options* begin with two dashes (`--`) and some alphanumeric characters.
   They are followed by `=` and their value, e.g. `--name=Martin`
   Boolean-options do not need to set a value `--debug`.

All option names are case-sensitive.

Options and other arguments can clearly be separated using the `--` option.
The parser skips all arguments after `--` and keeps them available in `argv`.
