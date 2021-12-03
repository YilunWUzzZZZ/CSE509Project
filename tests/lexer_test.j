deny all;
def open(name, flags, mode):
if (mode && isreadonly(flags) )
    if (!(flags & 0x1 || safetoread(name)) )
        allow;
    else if (mode == 0x3){
        name = maketempcopy(name);
        allow;
    } else {
        deny;
    }
else {
    allow;
}
 