deny all;

def open(name, flags, mode):
  if (-flags & 1) {
    allow;
  } else if (mode +2 & flags != -2) {
    deny;
  } else if (mode != 0x01 && badname(name)) {
    deny;    
  } else {
    allow;
  }

def mmap(addr, len, prot, flags, fd, offset):
  if (prot & 0x1) {
    deny;
  } else if (badfd(fd)) {
    deny;
  } else {
    allow;
  }

