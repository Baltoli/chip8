import re
import sys

def get_bytes(n, l):
    n = int(n, 16)
    bs = []
    while n > 16:
        bs.append(n % 16)
        n //= 16
    bs.append(n % 16)
    while len(bs) < l:
        bs.append(0)
    bs.reverse()
    return bytes(bs)

def get_addr(m):
    return get_bytes(m['addr'], 3)

def get_byte(m):
    return get_bytes(m['byte'], 2)

def get_nibble(m):
    return get_bytes(m['nibble'], 1)

def get_vx(m):
    return get_bytes(m['vx'], 1)

def get_vy(m):
    return get_bytes(m['vy'], 1)

def addr():
    return r"(?P<addr>[0-9A-F]{3})"

def byte():
    return r"(?P<byte>[0-9A-F]{2})"

def nibble():
    return r"(?P<nibble>[0-9A-F])"

def vx():
    return r"V(?P<vx>[0-9A-F])"

def vy():
    return r"V(?P<vy>[0-9A-F])"

def cls(m):
    return [0x0, 0x0, 0xE, 0x0]

def ret(m):
    return [0x0, 0x0, 0xE, 0xE]

def jp_addr(m):
    return [0x1, *get_addr(m)]

def call(m):
    return [0x2, *get_addr(m)]

def se_d(m):
    return [0x3, *get_vx(m), *get_byte(m)]

def sne_d(m):
    return [0x4, *get_vx(m), *get_byte(m)]

def se_id(m):
    return [0x5, *get_vx(m), *get_vy(m), 0x0]

def ld_b(m):
    return [0x6, *get_vx(m), *get_byte(m)]

def add_d(m):
    return [0x7, *get_vx(m), *get_byte(m)]

def alu(code):
    def alu_internal(m):
        y = [0x0]
        if 'vy' in m.groupdict():
            y = get_vy(m)
        return [0x8, *get_vx(m), *y, code]
    return alu_internal

def sne_id(m):
    return [0x9, *get_vx(m), *get_vy(m), 0]

def ld_i(m):
    return [0xA, *get_addr(m)]

def jp_rel(m):
    return [0xB, *get_addr(m)]

def rnd(m):
    return [0xC, *get_vx(m), *get_byte(m)]

def draw(m):
    return [0xD, *get_vx(m), *get_vy(m), *get_nibble(m)]

def skip_p(m):
    return [0xE, *get_vx(m), 0x9, 0xE]

def skip_np(m):
    return [0xE, *get_vx(m), 0xA, 0x1]

def f(c1, c2):
    def f_internal(m):
        return [0xF, *get_vx(m), c1, c2]
    return f_internal

pairs = [
    ("CLS", cls),
    ("RET", ret),
    ("JP " + addr(), jp_addr),
    ("CALL " + addr(), call),
    ("SE " + vx() + ", " + byte(), se_d),
    ("SNE " + vx() + ", " + byte(), sne_d),
    ("SE " + vx() + ", " + vy(), se_id),
    ("LD " + vx() + ", " + byte(), ld_b),
    ("ADD " + vx() + ", " + byte(), add_d),
    ("LD " + vx() + ", " + vy(), alu(0x0)),
    ("OR " + vx() + ", " + vy(), alu(0x1)),
    ("AND " + vx() + ", " + vy(), alu(0x2)),
    ("XOR " + vx() + ", " + vy(), alu(0x3)),
    ("ADD " + vx() + ", " + vy(), alu(0x4)),
    ("SUB " + vx() + ", " + vy(), alu(0x5)),
    ("SHR " + vx(), alu(0x6)),
    ("SUBN " + vx() + ", " + vy(), alu(0x7)),
    ("SHL " + vx(), alu(0xE)),
    ("SNE " + vx() + ", " + vy(), sne_id),
    ("LD I, " + addr(), ld_i),
    ("JP V0, " + addr(), jp_rel),
    ("RND " + vx() + ", " + byte(), rnd),
    ("DRW " + vx() + ", " + vy() + ", " + nibble(), draw),
    ("SKP " + vx(), skip_p),
    ("SKNP " + vx(), skip_np),
    ("LD " + vx() + ", DT", f(0x0, 0x7)),
    ("LD " + vx() + ", K", f(0x0, 0xA)),
    ("LD DT, " + vx(), f(0x1, 0x5)),
    ("LD ST, " + vx(), f(0x1, 0x8)),
    ("LD DT, " + vx(), f(0x1, 0x5)),
    ("ADD I, " + vx(), f(0x1, 0xE)),
    ("LD F, " + vx(), f(0x2, 0x9)),
    ("LD B, " + vx(), f(0x3, 0x3)),
    ("LD [I], " + vx(), f(0x5, 0x5)),
    ("LD " + vx() + ", I", f(0x6, 0x5))
]

def convert_pseudo(line, offsets):
    jp_r = re.compile("LJP (?P<label>.*)")
    m = jp_r.match(line)
    if m:
        return "JP " + "%03X" % offsets[m['label']]
    return line

def instr(line):
    for p in pairs:
        reg = re.compile(p[0])
        m = reg.match(line)
        if m:
            return bytes(p[1](m))
    return None

def label(line):
    line = line.rstrip()
    if line[-1] == ":":
        return line[:-1]
    return None

def pseudo(line, locations):
    pass

def pack(i):
    return bytes([i[0] << 4 | i[1], i[2] << 4 | i[3]])

if __name__ == "__main__":
    in_path = sys.argv[1]
    out_path = sys.argv[2]
    blocks = {}
    current_block = None
    with open(in_path, "r") as in_file:
        for line in in_file:
            if label(line):
                current_block = label(line)
                blocks[current_block] = []
            else:
                blocks[current_block].append(line)
    if '_start' not in blocks:
        sys.exit(2)
    sizes = {}
    for block in blocks:
        sizes[block] = len(blocks[block])
    off = 0x200
    offsets = {}
    for block in sizes:
        offsets[block] = off
        off += sizes[block]*2
    with open(out_path, "wb") as out_file:
        out_file.write(bytes([0x0] * 0x200))
        for b in blocks:
            for i in blocks[b]:
                out_file.write(pack(instr(convert_pseudo(i, offsets))))
        out_file.write(bytes([0x0] * (4096 - out_file.tell())))
