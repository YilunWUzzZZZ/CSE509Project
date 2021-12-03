deny all;
def open(name, flags, mode):
  if (name == legalname()) {
    flags = flags & 0x10;
  }
  name = alloc_name();
  if (flags & 0x10) {
    deny;
  }
  
  allow;
 