#include <idc.idc>

static IsSubFollowing(addr) {
    auto i, pAddr;
    i = 0;
    while (i < 20) {
        pAddr = Dword(addr) & 0xFFFFFFFFFE;
        if (!isCode(GetFlags(pAddr))) {
            return 0;
        }
        addr = NextHead(addr, MaxEA());
        i = i + 1;
    }
    return 1;
}

static IsStringFollowing(addr) {
    auto i;
    i = 0;
    while (i < 10) {
        if (!isData(GetFlags(addr))) {
            return 0;
        }
        addr = NextHead(addr, MaxEA());
        i = i + 1;
    }
    return 1;
}

static readLine(h) {
    auto line, c;
    line = "";
    while (1) {
        c = fgetc(h);
        if (c == '\n' || c == '\0') {
            break;
        }
        line = line + c;
    }
    return line + '\0';
}

static loadMethods(ea) {
    auto mh, mh_size;
    auto i;
    auto name, addr;

    mh = fopen("./method_name.txt", "r");
    mh_size = atol(readLine(mh));
    i = 0;
    while (i < mh_size) {
        name = readLine(mh);
        addr = Dword(ea) & 0xFFFFFFFFFE;
        MakeNameEx(addr, name, SN_NOWARN);
        ea = ea + 4;
        i = i + 1;
    }
    fclose(mh);
}

static findMethodsStart() {
    auto seg, addr;

    seg = FirstSeg();
    while (seg != BADADDR) {
        seg = NextSeg(seg);
        addr = seg;
        while (SegName(addr) == ".data.rel.ro") {
            if (IsSubFollowing(addr) && IsStringFollowing(Dword(PrevHead(addr, MinEA())))) {
                return addr;
            }
            addr = NextHead(addr, MaxEA());
        }
    }
    return addr;
}

static findStringsEnd(addr) {
    while (1) {
        addr = addr + 4;
        if (!isData(GetFlags(addr))) {
            addr = PrevHead(PrevHead(addr, MinEA()), MinEA());
            return addr;
        }
    }
}

static loadStrings(ea) {
    auto sh, sh_size;
    auto i;
    auto sl, addr;

    sh = fopen("./string_literal.txt", "r");
    sh_size = atol(readLine(sh));

    i = 0;
    while (i < sh_size) {
        sl = readLine(sh);
        addr = Dword(ea);
        MakeNameEx(addr, "StringLiteral_" + sl, SN_NOWARN);
        MakeComm(ea, sl);
        ea = ea - 4;
        i = i + 1;
    }

    fclose(sh);
}

static main()
{
    auto addr;

    addr = findMethodsStart();
    Jump(addr);
    Message("loading methods ...\n");
    loadMethods(addr);
    Message("load methods complete.\n");
    addr = findStringsEnd(Dword(PrevHead(addr, MinEA())));
    Message("loading strings ...\n");
    loadStrings(addr);
    Message("load strings complete.\n");
    Message("Completed.\n");
}
