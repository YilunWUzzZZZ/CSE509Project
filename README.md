# Compilation
make

# Usage
./demo source [ptrace_output] [seccomp_output]

[Important] Put all your headers used and all the declaration of helper functions in helpers.h

g++ seccomp_output.cpp -o seccomp -fpermissive


g++ ptrace_output.cpp helpers.cpp -o ptrace -fpermissive

# RUN
./ptrace ./seccomp [app]

# TODO
Add automatic compilation