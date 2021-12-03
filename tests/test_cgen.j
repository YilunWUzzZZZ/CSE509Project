def open(name, flags, mode):
  if (flags & 2 || mode == 0x1 || name == badname()) {
    deny;
  } else if (mode == 4 ) {
    name = okname();
    allow;
  } else {
    allow;
  }