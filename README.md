# Compilation
To build the compiler files you need to run ```make``` on the parent directory

    make

# Usage

To generate the ptrace and seccomp source files for a given ruleset, defined as per the language,
you need to run the command shown below.

`source` should point to the rules file defined in our language.
`ptrace_output` and `seccomp_output` should be names of the 
ptrace and seccomp source files you want the compiler
to generate. 


    ./demo source [ptrace_output] [seccomp_output]


#### Important

*All helper functions should be defined in `helpers.c` and the respective
headers defined in `helpers.h`*

Use the command below to generate the sandbox binary with your helper functions.

    gcc ptrace_output.c seccomp_output.c helpers.c -o sandbox

# Run

To run a program with the sandbox.

    ./sandbox [app] <args,...>


# Demo programs

We have added several demo programs to test our compiler with. All the demo
programs and their rulesets are placed in `tests/` folder. There are 4 different
applications that you can use.

1. `tests/safex`: A file access sandboxing application. 
Any process trying to modify a *protected* file 
should instead be redirected to a temp file
and all modifications should be made to the *temp* file instead. 
For simplicity, we have hardcoded the *protected* file names into our program, 
namely "important.txt", "important2.txt", "important3.txt", etc. If app opens
any of these filenames, our sandbox will intercept the program and create
a temp copy of the same file in /tmp folder and redirect application
to that file instead.

Rule set is defined in tests/safex/safex_dsl.p

        cd tests/safex
        ./build.sh      
        ./sandbox vim important.txt

2. `tests/url2file`: Functionality extension using ptrace. We can try to open
any URL with a file reading application, and the sandbox program will download
the webpage to `/tmp/url2file/file.tmp` and pass this file path to the application.

        cd tests/url2file
        ./build.sh
        ./sandbox vim www.facebook.com



# TODO
Add automatic compilation
