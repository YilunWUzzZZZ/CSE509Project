allow all;

def openat(dirfd, pathname, flags):
if (is_url(pathname)) {
    pathname = url2file(pathname);
    allow;
}
else {
    allow;
}

def statfs(path, buf):
if (is_url(path)) {
    path = url2file(path);
    allow;
}
else {
    allow;
}
