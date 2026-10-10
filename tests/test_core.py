import ctypes,random,math
c=ctypes.CDLL('./work/pacman-core.so');c.arcade_audio.restype=ctypes.POINTER(ctypes.c_float)
c.arcade_reset();sounds=0;maxscore=0;deaths=0;lastlives=3
for i in range(18000):
 if i%100==0:k=random.Random(i).choice([1,2,4,8])
 c.arcade_tick(k)
 n=c.arcade_value(7);assert 790<=n<=810,(i,n)
 values=[c.arcade_audio()[j] for j in range(n)]
 assert all(math.isfinite(v) and abs(v)<=1 for v in values)
 sounds+=any(abs(v)>.001 for v in values)
 maxscore=max(maxscore,c.arcade_value(0))
 lives=c.arcade_value(1);deaths+=lives<lastlives;lastlives=lives
 if c.arcade_value(4)==0:c.arcade_reset();lastlives=3
assert sounds>100 and maxscore>0 and deaths>0,(sounds,maxscore,deaths)
c.arcade_reset();c.arcade_tick(0);assert c.arcade_value(0)==0 and c.arcade_value(1)==3
print('18000 ticks passed; audible ticks:',sounds,'max score:',maxscore,'lost lives:',deaths)
