def gen_strings(template_name, outfile):
  output = "string " + template_name + "_p1 = \n"
  i = 1;
  fname = template_name + ".c"
  with open(fname, 'r') as f:
    lines = f.readlines()
    for l in lines:
      if "/* insert" in l:
        i += 1
        output += ";\nstring " + template_name +"_p" + str(i) + " = \n"
        continue;
      l = l.replace("\\", "\\\\");
      l = l.replace("\"", "\\\"");
 
      if l[-1] == '\n':
        l = l[:-1]
      output += '\"' + l +'\\n\"' + '\n';
    output += ";\n";
  outfile.write(output)



if __name__ == "__main__":
  with open("../code_template.cpp", 'w') as f:
    f.write("#include \"code_template.h\"\n")
    gen_strings("ptrace_template", f)
    gen_strings("seccomp_template", f)