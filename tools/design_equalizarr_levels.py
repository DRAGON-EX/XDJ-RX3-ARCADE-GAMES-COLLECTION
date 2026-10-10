"""Author Equalizarr's 50 distinct stage silhouettes and write the shared C data.
No random levels: each named design has its own geometry and fixed material plan.
Run after changing a design; build_equalizarr.py only compiles the checked-in data.
"""
from pathlib import Path
import json, math
C,R=16,14
stages=[]
def add(name, predicate, steel=(), bombs=()):
    mask=[[bool(predicate(x,y)) for x in range(C)] for y in range(R)]
    n=len(stages)
    rows=[]
    for y in range(R):
        row=[]
        for x in range(C):
            v=0
            if mask[y][x]:
                # Facades and low hanging bricks carry most armor; each act increases it.
                boundary=any(not (0<=x+dx<C and 0<=y+dy<R) or not mask[y+dy][x+dx] for dx,dy in [(1,0),(-1,0),(0,1),(0,-1)])
                v=1
                if (x*3+y+n)%7 < 1+n//10 or (boundary and y>=7 and n>=10):v=2
                if n>=20 and (x+2*y+n)%6 < 1+(n>=40):v=3
            if (x,y) in bombs and v:v=4
            if (x,y) in steel:v=5
            row.append(str(v) if v else '.')
        rows.append(''.join(row))
    stages.append(dict(name=name,rows=rows))
def oval(x,y,cx,cy,rx,ry):return ((x-cx)/rx)**2+((y-cy)/ry)**2
def ring(x,y,cx,cy,rx,ry,inner=.42):return inner<oval(x,y,cx,cy,rx,ry)<1.18
def box(x,y,l,t,r,b):return l<=x<=r and t<=y<=b
def bitmap(lines):
    rows=[line.ljust(C,'.') for line in lines]
    assert all(len(row)==C for row in rows)
    return lambda x,y:y<len(rows) and rows[y][x]!='.'
# 1–10: broad shots, silhouette recognition, then deeper returns.
add('SPECTRUM RISE',lambda x,y: 1<=x<=14 and 2<=y<=11 and y>=11-[4,6,8,5,9,7,3][(x-1)//2],bombs=[(7,8)])
add('SPLIT GATE',lambda x,y:box(x,y,1,1,6,6) or box(x,y,9,1,14,6) or (y in (9,10) and 3<=x<=12),bombs=[(4,3),(11,4)])
add('HEARTBREAKER',lambda x,y:(y in (1,2) and (2<=x<=5 or 10<=x<=13)) or (3<=y<=5 and 1<=x<=14) or (6<=y<=11 and abs(x-7.5)<=12-y))
add('VINYL GROOVE',lambda x,y:ring(x,y,7.5,6,6.8,5.6,.28) or oval(x,y,7.5,6,1.8,1.8)<1.2,bombs=[(7,6)])
add('CAT FACE',bitmap(['.11..........11.','.111........111.','.1111......1111.','.11111111111111.','.11111111111111.','.11..111111..11.','.11..111111..11.','.11111111111111.','..1111.11.1111..','...111....111...','....11111111....','.....111111.....']))
add('DOWN THE STAIRS',lambda x,y:(x//2<=5 and x//2<=y<=x//2+2) or (x>=4 and y in (10,11)) or box(x,y,12,7,14,9))
add('TWIN SPEAKERS',lambda x,y: (box(x,y,1,1,6,11) and (x in (1,6) or y in (1,6,11) or ring(x,y,3.5,3.5,1.7,1.7,.25) or ring(x,y,3.5,8.5,1.7,1.7,.25))) or (box(x,y,9,1,14,11) and (x in (9,14) or y in (1,6,11) or ring(x,y,11.5,3.5,1.7,1.7,.25) or ring(x,y,11.5,8.5,1.7,1.7,.25))))
add('BROKEN HOURGLASS',lambda x,y:1<=y<=12 and (abs(x-7.5)<=abs(y-6.5)+.5) and (x+y)%5!=0,bombs=[(7,6),(8,7)])
add('ZIGZAG ALLEY',lambda x,y:1<=x<=14 and min(abs(y-(x%8+1)),abs(y-(12-x%8)))<=1,steel=[(3,6),(12,6)])
add('CLUB GATE',lambda x,y:box(x,y,1,1,14,3) or box(x,y,1,4,3,11) or box(x,y,12,4,14,11) or box(x,y,6,6,9,9),steel=[(2,10),(13,10)],bombs=[(7,2)])
# 11–20: pockets, oblique rebounds and recognisable pixel-art stages.
add('DIAMOND QUARTET',lambda x,y:any(abs(x-cx)+abs(y-cy)<3.3 for cx,cy in [(3,3),(12,3),(5,9),(10,9)]),steel=[(7,6),(8,6)])
add('CRESCENT',lambda x,y:oval(x,y,7,6,6.8,5.9)<1 and oval(x,y,10,4.7,5.1,4.2)>1,bombs=[(4,7)])
add('CIRCUIT RUN',lambda x,y: (x in (1,2,7,8,13,14) and 1<=y<=10) or (y in (2,7,11) and 1<=x<=14 and (x+y)%7!=0),steel=[(4,7),(11,7)])
add('PIXEL INVADER',bitmap(['...11......11...','....11....11....','...1111111111...','..111111111111..','.111..1111..111.','.111..1111..111.','.11111111111111.','...11......11...','..11..1111..11..','.11...1..1...11.']),steel=[(7,11),(8,11)],bombs=[(7,5)])
add('RIBBON DANCE',lambda x,y:1<=x<=14 and abs(y-(6+4*math.sin(x*.57)))<1.8,steel=[(2,12),(13,12)])
add('DOUBLE HELIX',lambda x,y:1<=y<=12 and (abs(x-(7.5+5*math.sin(y*.65)))<1.3 or abs(x-(7.5-5*math.sin(y*.65)))<1.3 or (y in (3,7,11) and 3<=x<=12)),bombs=[(7,7)])
add('TRIDENT',lambda x,y: (x in (2,3,7,8,12,13) and 1<=y<=7) or (y in (7,8) and 2<=x<=13) or box(x,y,6,9,9,12),steel=[(7,1),(8,1)])
add('OFFSET GATES',lambda x,y:(y in (1,2,5,6,9,10) and ((x<12 and y<3) or (x>3 and 4<y<7) or (x<11 and y>8))),steel=[(2,10),(3,10),(12,6),(13,6)])
add('DRAGON WINGS',bitmap(['1..............1','11............11','111...1..1...111','1111..1111..1111','11111.1111.11111','1111111111111111','.11111111111111.','..11.111111.11..','.....111111.....','......1111......','.....11..11.....','....11....11....']),bombs=[(7,5)],steel=[(0,3),(15,3)])
add('CROWN OF BASS',lambda x,y:(y<6 and ((x in (1,2,13,14)) or (x in (5,6,9,10) and y>=2))) or box(x,y,1,6,14,9) or (y in (11,12) and x%3!=0),steel=[(6,9),(9,9)],bombs=[(3,7),(12,7)])
# 21–30: dense sculpted layouts, guarded centers, deeper corridors.
add('SPIRAL CUT',lambda x,y:(x in (1,14) and 1<=y<=11) or (y in (1,11) and 1<=x<=14) or (x in (4,11) and 4<=y<=8) or (y in (4,8) and 4<=x<=11) or box(x,y,7,6,8,6),steel=[(1,7),(14,5)],bombs=[(7,6)])
add('PINBALL LANES',lambda x,y: (1<=y<=10 and x in (1,2,6,9,13,14)) or (y in (1,2) and 1<=x<=14) or (y in (7,8) and x in (3,4,11,12)),steel=[(6,10),(9,10),(3,8),(12,8)])
add('MIXTAPE',lambda x,y:box(x,y,1,2,14,11) and not ((3<=y<=6 and x in (3,4,5,10,11,12)) or (8<=y<=10 and 5<=x<=10)),steel=[(4,11),(11,11)],bombs=[(7,4),(8,4)])
add('FALLING TEETH',lambda x,y:1<=x<=14 and ((y<=3) or (x%4<2 and y<=7+x//5) or (y==12 and 4<=x<=11)),steel=[(5,9),(9,10),(13,11)])
add('DIAMOND MAZE',lambda x,y:2<abs(x-7.5)+abs(y-6.5)<7.5 and (x+y)%7!=0,steel=[(4,6),(11,7),(7,3),(8,10)],bombs=[(3,6)])
add('SIDE POCKETS',lambda x,y:box(x,y,0,1,4,5) or box(x,y,11,4,15,8) or box(x,y,3,9,8,12) or box(x,y,7,1,9,3),steel=[(2,6),(3,6),(12,9),(13,9)])
add('SERPENTINE',lambda x,y: (y in (1,2,6,7,11,12) and 1<=x<=14) or (x in (1,2) and 2<=y<=6) or (x in (13,14) and 7<=y<=11),steel=[(5,7),(6,7),(9,11),(10,11)])
add('STAR DETONATOR',lambda x,y:abs(x-7.5)<1.5 or abs(y-6.5)<1.5 or abs(abs(x-7.5)-abs(y-6.5))<1,steel=[(1,1),(14,1),(1,12),(14,12)],bombs=[(7,6),(8,7)])
add('BUTTERFLY EFFECT',lambda x,y:oval(x,y,3.4,4,3.3,3.8)<1 or oval(x,y,11.6,4,3.3,3.8)<1 or oval(x,y,4.5,9.5,2.7,2.7)<1 or oval(x,y,10.5,9.5,2.7,2.7)<1 or box(x,y,7,3,8,12),steel=[(6,8),(9,8)],bombs=[(3,3),(12,3)])
add('BASS CITADEL',lambda x,y: ((x in (0,1,4,5,10,11,14,15) and 0<=y<=8) or box(x,y,0,5,15,8) or box(x,y,3,10,12,12)),steel=[(4,8),(5,8),(10,8),(11,8)],bombs=[(7,6),(8,6)])
# 31–40: narrow firing lanes and armored targets under steel overhangs.
add('WINDMILL',lambda x,y:box(x,y,6,1,8,10) or box(x,y,3,5,14,7) or box(x,y,9,1,13,3) or box(x,y,1,8,5,11),steel=[(7,6),(8,6)],bombs=[(11,2)])
add('TRIPLE TOWERS',lambda x,y:(box(x,y,1,1,4,11) or box(x,y,6,3,9,12) or box(x,y,11,0,14,10)) and not(y%4==2 and x in (2,3,7,8,12,13)),steel=[(2,11),(3,11),(7,12),(8,12),(12,10),(13,10)])
add('STAGGERED LOCKS',lambda x,y: ((y in (1,2,5,6,9,10) and (x+y//4)%7<5) or (y==12 and 5<=x<=10)),steel=[(0,6),(1,6),(7,10),(8,10),(13,2),(14,2)])
add('SKULL DROP',bitmap(['....11111111....','..111111111111..','.11111111111111.','.111..1111..111.','.11....11....11.','.111..1111..111.','..11111..11111..','...1111..1111...','....11111111....','....1.1..1.1....','....1.1..1.1....','.....111111.....']),steel=[(3,6),(12,6)],bombs=[(7,2)])
add('SATELLITE',lambda x,y:oval(x,y,7.5,6.5,3,4.5)<1 or (abs(y-(10.5-x*.55))<1.2 and 1<=x<=14),steel=[(2,9),(13,3)],bombs=[(7,6)])
add('KEYHOLE',lambda x,y:ring(x,y,7.5,4.5,5.8,4,.35) or (8<=y<=12 and 4<=x<=11 and abs(x-7.5)>=1.5),steel=[(4,11),(11,11)],bombs=[(7,1)])
add('INVERTED CITY',lambda x,y: (0<=y<=2) or (x in (0,1,4,5,8,9,12,13) and y<=[10,7,12,9][x//4]) or box(x,y,3,12,5,13),steel=[(0,9),(1,9),(8,11),(9,11)])
add('FRACTURE LINE',lambda x,y:1<=y<=12 and abs(x-(3+(y*3)%10))>1.8 and ((x<8 and y<9) or (x>=8 and y>=3)),steel=[(6,5),(10,9),(3,11)],bombs=[(2,2),(13,11)])
add('NEON RAIN',lambda x,y:1<=x<=14 and (y-(x*5)%8 in (0,1,2,5,6)) or box(x,y,5,10,10,12),steel=[(4,8),(8,7),(12,6)])
add('DRAGON THRONE',lambda x,y:box(x,y,4,1,11,8) or box(x,y,1,6,3,10) or box(x,y,12,6,14,10) or (y in (11,12) and x in (3,4,5,10,11,12)),steel=[(3,10),(12,10),(6,8),(9,8)],bombs=[(7,3),(8,3)])
# 41–50: different finale layouts, not recolored repeats.
add('SHATTERED HALO',lambda x,y:ring(x,y,7.5,6.5,7.2,6,.45) and (x+y)%6!=0 or oval(x,y,7.5,6.5,2,2)<1,steel=[(7,2),(8,2),(2,6),(13,7)],bombs=[(7,6)])
add('VOLTAGE',lambda x,y:(2<=y<=6 and 10-y<=x<=13-y) or (6<=y<=8 and 4<=x<=11) or (8<=y<=13 and 14-y<=x<=18-y) or box(x,y,12,1,14,4) or box(x,y,1,2,3,5),steel=[(5,8),(10,6)])
add('CHECKMATE',lambda x,y: (x//2+y//2)%2==0 and 1<=y<=12,steel=[(4,4),(5,4),(10,8),(11,8)],bombs=[(8,4),(6,8)])
add('TURNTABLE PAIR',lambda x,y:ring(x,y,3.5,5,3.5,4.5,.15) or ring(x,y,11.5,7,3.5,4.5,.15) or box(x,y,6,11,9,13),steel=[(3,8),(12,10)],bombs=[(3,2),(12,4)])
add('DUNGEON DOORS',lambda x,y:box(x,y,1,1,5,9) or box(x,y,10,4,14,12) or box(x,y,7,0,8,5),steel=[(1,9),(2,9),(4,9),(5,9),(10,12),(11,12),(13,12),(14,12)],bombs=[(3,4),(12,7)])
add('FORKED LIGHTNING',lambda x,y: (abs(x-(7.5-y*.5))<1.5 or abs(x-(7.5+y*.5))<1.5 or (y>6 and abs(x-7.5)<1.5)) and 1<=y<=12,steel=[(2,10),(13,10)],bombs=[(7,3)])
add('CAT RESCUE',bitmap(['..11........11..','.1111......1111.','.11111111111111.','.11111111111111.','.11..111111..11.','.11111111111111.','..11111..11111..','...1111111111...','.....11..11.....','.11..11..11..11.','.111111..111111.','.111111..111111.','..1111....1111..']),steel=[(3,10),(12,10),(7,8),(8,8)],bombs=[(7,3)])
add('FORTY EIGHT KHZ',lambda x,y: (box(x,y,1,1,6,12) and (x in (1,2,5,6) or y in (5,6))) or (box(x,y,9,1,14,12) and (x in (9,10,13,14) or y in (1,2,6,7,11,12))),steel=[(5,9),(6,9),(9,9),(10,9)])
add('FINAL MIX',lambda x,y: (y in (0,1,4,5,8,9,12,13) and (x+2*(y//4))%9<7) or (x in (1,14) and 2<=y<=11),steel=[(2,5),(3,5),(12,9),(13,9),(6,13),(9,13)],bombs=[(7,1),(8,8)])
add('THE EQUALIZARR',bitmap(['1..11......11..1','11.111....111.11','1111111..1111111','.11111111111111.','..111111111111..','...11..11..11...','..111..11..111..','.11111111111111.','1111111111111111','111..111111..111','11....1111....11','.11...1111...11.','..111.1111.111..','...1111111111...']),steel=[(2,9),(13,9),(6,11),(9,11)],bombs=[(7,3),(8,8)])
assert len(stages)==50
# Occupancy uniqueness ignores armor/colors; material-only copies cannot pass.
silhouettes=[''.join('1' if c!='.' else '.' for row in s['rows'] for c in row) for s in stages]
assert len(set(silhouettes))==50
assert len(set(s['name'] for s in stages))==50
for s in stages:
    count=sum(c not in '.5' for row in s['rows'] for c in row)
    assert 24<=count<=200,(s['name'],count)
out='/* Generated by tools/design_equalizarr_levels.py. 50 separately authored silhouettes. */\n#define EQ_COLS 16\n#define EQ_ROWS 14\n#define EQ_CELLS (EQ_COLS*EQ_ROWS)\n#define EQ_BX 28\n#define EQ_BY 62\n#define EQ_DX 49\n#define EQ_DY 33\n#define EQ_BW 43\n#define EQ_BH 27\n'
out+='static const char *eq_level_names[50]={\n'+''.join(json.dumps(s['name'])+',\n' for s in stages)+'};\n'
out+='static const char eq_levels[50][EQ_ROWS][EQ_COLS+1]={\n'+''.join('{'+','.join(json.dumps(row) for row in s['rows'])+'},\n' for s in stages)+'};\n'
Path('Games/Native/Equalizarr/levels.h').write_text(out)
Path('Games/Native/Equalizarr/level-catalog.json').write_text(json.dumps(stages,indent=2)+'\n')
print('50 individually designed layouts written; all silhouettes distinct.')
