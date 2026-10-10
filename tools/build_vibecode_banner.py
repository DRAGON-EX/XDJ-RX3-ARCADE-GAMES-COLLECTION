"""Create the README's animated rainbow lettering (no browser scripting)."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import colorsys
root=Path(__file__).resolve().parents[1]
font=ImageFont.truetype('/usr/share/fonts/TTF/DejaVuSans-Bold.ttf',76)
w,h=1000,148
mask=Image.new('L',(w,h));d=ImageDraw.Draw(mask)
d.text((w//2,h//2),'100% VIBECODED',font=font,anchor='mm',fill=255)
frames=[]
for i in range(36):
 gradient=Image.new('RGB',(w,h));p=gradient.load()
 for x in range(w):
  rgb=tuple(round(c*255) for c in colorsys.hsv_to_rgb((x/w*.8+i/36)%1,.68,1))
  for y in range(h):p[x,y]=rgb
 frame=Image.new('RGB',(w,h),(14,16,24));frame.paste(gradient,(0,0),mask)
 frames.append(frame)
frames[0].save(root/'assets/readme/100-percent-vibecoded.gif',save_all=True,append_images=frames[1:],duration=90,loop=0,optimize=True)
