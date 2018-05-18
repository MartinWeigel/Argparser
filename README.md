# Argparse

A single-header command line argument-parser library written in C.

## Description

This library is inspired by parse-options.c (git) and python's argparse
module.

Arguments parsing is common task in cli program, but traditional `getopt`
libraries are not easy to use. This library provides high-level arguments
parsing solutions.

The program defines what arguments it requires, and `argparse` will figure
out how to parse those out of `argc` and `argv`, it also automatically
generates help and usage messages and issues errors when users give the
program invalid arguments.

Please see the file `example.c` to learn how to use the library.


## Features

- Single-header library for easy integration into existing projects
- Handles both optional and positional arguments
- Produces informative usage messages
- Issues errors when invalid arguments are given


## Supported Arguments

There are basically three types of options:

 - boolean options
 - options with mandatory argument
 - options with optional argument

There are basically two forms of options:

 - short option consist of one dash (`-`) and one alphanumeric character.
 - long option begin with two dashes (`--`) and some alphanumeric characters.

Short options may be bundled, e.g. `-a -b` can be specified as `-ab`.

Options are case-sensitive.

Options and non-option arguments can clearly be separated using the `--` option.
