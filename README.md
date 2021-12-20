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


# Language description

This section provides a high level overview of the language we have defined and that 
can be used with our compiler. 

A sample example is shown below. 
        
        deny all;
        def open ( x, y, z):
            if ( x > 0 ) {
                allow;
            }
        def close(y):
            deny;

In our language we start by first defining the default policy of our sandbox
application, followed by the specific system calls that we want to intercept,
and the associated logic for modification. In this simple example, we intercept
all `open()` and `close()` calls made by the tracee and only allow the `open()`
based on a specific condition.

One important thing to note here is that the order of arguments being passed to 
each system call should be preserved. Because our compiler uses code templates
of all system calls to map the arguments, if the order or count of the arguments
used in the language differs from the template it will lead to errors.


Another example, shown below, is from the `tests/safex` implementation. 

        allow all;

        def openat(dirfd, pathname, flags):
        if (protected_file(pathname)) {
            pathname = safex(pathname);
            allow;
        }
        else {
            allow;
        }

This policy basically dictates that we allow the tracee program to make all
the system calls, however we only intercept the `openat()` calls that it 
makes. For every `openat()` call we check whether the file being accessed by 
the tracee program is a protected file. Here the function `protected_file()` 
is a helper function for our tracer and needs to be defined in the `helpers.c` 
file. If the file is a protected file, the tracer creates a temporary copy
of that file and passes it to the tracer, done using the helper function 
`safex()`. 
All other `openat()` calls made by the tracee to files that are not protected
files are allowed by the tracer.

***Note:** The formatting of the language needs to be consistent with
the examples shown above.*

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


3.	`tests/chat`: a client/server program for chatting. With this test case, we want to
show the capability of our language in generating ptrace and seccom rules and limit 
the system calls and arguments that each program(client/server) can call without 
impacting on the functionalities.  
```
        cd tests/chat
       ./build.sh
       ./sandbox ./client or ./server
```
4.	`tests/exploit`: this part contains all exploits which are related to the exploit assignment.
We can detect all exploits with applying two rules:

    -	**A rule for write system call**: we limit using $ and % at the same time in the printf function with
    checking it in the buf argument of write system call. 
    -	**A rule for execve system call**: we check the pathname argument of execve to be in the allowed paths.
    In this way, the attacker can not execute any other program. 
```
       cd tests/exploit
       ./build.sh
       ./sandbox ./driver_auth_db or ./driver_return2_helper or ./driver_return2_helper2 or ./driver_return2_injectedcode
```


# TODO
Add automatic compilation
