import itertools
import re
import fileinput

lut1: dict[int,tuple[int,int]] = {}
lut2: dict[tuple[int,int],tuple[int,int]] = {}
lut3: dict[tuple[int,int,int],tuple[int,int]] = {}

for line in fileinput.input():
    line = line.strip()
    if (m := re.match(r"1LUT:(\d+): (\d+)/(\d+)$", line)):
        a,b,c = map(int, m.groups())
        lut1[a] = (b,c)
    elif (m := re.match(r"2LUT:(\d+),(\d+): (\d+)/(\d+)$", line)):
        a,b,c,d = map(int, m.groups())
        lut2[(a,b)] = (c,d)
    elif (m := re.match(r"3LUT:(\d+),(\d+),(\d+): (\d+)/(\d+)$", line)):
        a,b,c,d,e = map(int, m.groups())
        lut3[(a,b,c)] = (d,e)
    else:
        print("warning: unknown line:", line)

for tup in itertools.combinations(range(100, 10, -1), 4):
    l1 = sum(lut1[x][0] for x in tup)
    l2 = sum(lut2[x][0] for x in itertools.combinations(tup, 2))
    l3 = sum(lut3[x][0] for x in itertools.combinations(tup, 3))
    l1a = sum(lut1[x][1] for x in tup)
    l2a = sum(lut2[x][1] for x in itertools.combinations(tup, 2))
    l3a = sum(lut3[x][1] for x in itertools.combinations(tup, 3))
    print("1L:" + ",".join(map(str,tup)) + f": {l1}/{l1a}")
    print("2L:" + ",".join(map(str,tup)) + f": {l2}/{l2a}")
    print("3L:" + ",".join(map(str,tup)) + f": {l3}/{l3a}")
