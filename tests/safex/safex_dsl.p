allow all;

def openat(dirfd, pathname, flags):
if (protected_file(pathname)) {
    pathname = safex(pathname);
    allow;
}
else {
    allow;
}


