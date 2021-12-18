# Compilation
make

# Usage
./demo source [ptrace_output] [seccomp_output]

[Important] Put all your headers used and all the declaration of helper functions in helpers.h

gcc ptrace_output.cpp helpers.cpp -o sandbox

# RUN
./sandbox [app]

# TODO
Add automatic compilation