deny all;

def open(name, flags, mode):
  if (-flags & 1) {
    allow;
  } else if (mode +2 & flags != -2) {
    deny;
  } else if (name + 2 + 3 & flags) {
    deny;    
  } else {
    allow;
  }


