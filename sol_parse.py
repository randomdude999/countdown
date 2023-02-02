import itertools
import fileinput

sums: list[dict[tuple[int,int,int,int],tuple[int,int]]] = [{} for _ in range(5)]

for line in fileinput.input():
    p1,p2 = line.split(": ")
    count, count2 = map(int, p2.split("/"))
    l, args = p1.split("L:")
    l = int(l)
    a,b,c,d = map(int, args.split(","))
    sums[l][(a,b,c,d)] = (count, count2)

print("Verifying all data is present...")
for x in itertools.combinations(range(11,101), 4):
    if not all(x[::-1] in l for l in sums[1:]):
        print("smallest to do:", x[::-1])
        break

def show_top10(n: int, max_: int, inv: bool, real: bool):
    ssums = sorted(sums[n].items(), key=lambda x:-x[1][real], reverse=inv)
    for a,b in ssums[-10:]:
        print(a, ':', b[real], f'  {b[real]/max_*100:.2f}%')
