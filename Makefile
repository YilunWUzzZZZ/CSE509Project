#CXX = clang++
CXX = g++
CXXFLAGS := -g -Wall -O2  -std=c++11 

HOSTSRCS = parser.cpp lexer.cpp  error.cpp ast.cpp policy.cpp IR_gen.cpp IR.cpp IR_lifter.cpp code_gen.cpp syscalls.cpp code_template.cpp
TESTSRCS = driver.cpp


SRCS = $(HOSTSRCS) $(TESTSRCS)
HOSTOBJS = $(HOSTSRCS:%.cpp=%.o)


DEPDIR := .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

COMPILE.C = $(CXX) $(CXXFLAGS) $(DEPFLAGS) -c

# Disable default rules. It seems hard to ensure that our patterns rules
# fire, instead of the default rules.
.SUFFIXES:

%.o : %.cpp # (DEPDIR)/%.d
	$(COMPILE.C)  $<


# $(DEPDIR)/%.d: ;
# .PRECIOUS: $(DEPDIR)/%.d

# .PHONY: force

# cxx_flags: force
# 	@echo '$(MCXXFLAGS)' | tr " " '\n' | grep -v '^$$' | sort -u | diff -q $@ - || echo '$(MCXXFLAGS)' | tr " " '\n' | grep -v '^$$' | sort -u  > $@

all: demo

demo: $(HOSTOBJS) driver.o 
	$(CXX) $(LDFLAGS) -o $@ $^ 

parser.cpp: parser.y++
	bison -t -d -v -o parser.cpp parser.y++
#    mv E--_parser.H E--.tab.h


lexer.cpp: lexer.l++
	flex -p -8 -Ce -o lexer.cpp lexer.l++

include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS))))

clean:
	rm -f demo *.o lexer.cpp parser.cpp parser.hpp parser.output .d/*.d
