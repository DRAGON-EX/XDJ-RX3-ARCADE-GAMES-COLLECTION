"""Check occupancy uniqueness and sequential reachability for every level.
Flood-fill uses a 14px ball expanded against ALL bricks, then removes exposed
breakable faces, repeating until all targets can be reached from below.
Steel is never removed. This validates routes, not human difficulty or shot timing.
"""
import json
from pathlib import Path
from collections import deque
levels=json.loads(Path('Games/Native/Equalizarr/level-catalog.json').read_text())
assert len(levels)==50
shapes=set()
stats=[]
for level in levels:
 rows=level['rows']
 assert len(rows)==14 and all(len(r)==16 for r in rows)
 shape=''.join('1' if c!='.' else '.' for row in rows for c in row)
 assert shape not in shapes
 shapes.add(shape)
 alive={i for i,c in enumerate(''.join(rows)) if c!='.'}
 steel={i for i,c in enumerate(''.join(rows)) if c=='5'}
 # Ball center raster, bounded away from field walls.
 W,H=206,166
 blocks=[-1]*(W*H)
 for n in alive:
  x0=28+(n%16)*49-7;y0=62+(n//16)*33-7;x1=x0+43+14;y1=y0+27+14
  for y in range(max(0,(y0-10)//4),min(H,(y1-10)//4+2)):
   for x in range(max(0,(x0-10)//4),min(W,(x1-10)//4+2)):
    px,py=10+x*4,10+y*4
    if x0<=px<=x1 and y0<=py<=y1:blocks[y*W+x]=n
 rounds=0
 while alive-steel:
  seen=bytearray(W*H);start=(H-2)*W+W//2;seen[start]=1;q=deque([start]);exposed=set()
  while q:
   p=q.popleft();x=p%W;y=p//W
   for a in ([p-1] if x else [])+([p+1] if x<W-1 else [])+([p-W] if y else [])+([p+W] if y<H-1 else []):
    if seen[a]:continue
    seen[a]=1;n=blocks[a]
    if n in alive:
     if n not in steel:exposed.add(n)
    else:q.append(a)
  assert exposed,(level['name'],'sealed targets',sorted(alive-steel))
  alive-=exposed;rounds+=1
 assert alive==steel
 stats.append((level['name'],len(steel),rounds))
print('50 different silhouettes: every breakable block reachable after exposed layers clear; steel never removed.')
print('Steel per stage:',min(s for _,s,_ in stats),'to',max(s for _,s,_ in stats),'; maximum clearing layers:',max(r for _,_,r in stats))
