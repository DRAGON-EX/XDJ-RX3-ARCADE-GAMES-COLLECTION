(async()=>{
 const q=new URLSearchParams(location.search);
 if(q.get('story')!=='1'&&(sessionStorage.getItem('rx3-story-seen')||['music','debug','credits','volume','gain','finale','menu','unplug'].some(k=>q.has(k))))return;
 const response=await fetch('content/launcher-help.json');if(!response.ok)return;
 const {story}=await response.json();const rows=[];
 for(const paragraph of story){let line='';for(const word of paragraph.replaceAll('’',"'").toUpperCase().split(/\s+/)){if(line&&line.length+word.length+1>54){rows.push(line);line='';}line+=(line?' ':'')+word;}rows.push(line,'');}while(!rows.at(-1))rows.pop();
 let offset=0;
 const host=document.createElement('section');host.setAttribute('role','dialog');host.setAttribute('aria-label','Cat Rescue Saga');host.setAttribute('aria-modal','true');Object.assign(host.style,{position:'absolute',inset:'0',zIndex:100,background:'#0008'});
 const canvas=document.createElement('canvas');canvas.width=1280;canvas.height=800;Object.assign(canvas.style,{width:'100%',height:'100%',imageRendering:'pixelated'});host.append(canvas);const c=canvas.getContext('2d');const artwork=new Image();artwork.onload=()=>paint();artwork.src='assets/story-artwork.png?v=updated-dialogue';
 const punctuation={'.':[0,0,0,0,0,0,4],"'":[4,4,0,0,0,0,0],'?':[14,17,1,2,4,0,4],'!':[4,4,4,4,4,0,4],',':[0,0,0,0,0,4,8],'-':[0,0,0,31,0,0,0],':':[0,4,0,0,4,0,0],'/':[1,2,2,4,8,8,16]};
 function text(x,y,size,value,color){c.fillStyle=color;for(const char of value){const n=char>='0'&&char<='9'?char.charCodeAt(0)-48:char>='A'&&char<='Z'?char.charCodeAt(0)-55:-1;const glyph=n>=0?retroFont[n]:punctuation[char];if(glyph)for(let j=0;j<7;j++)for(let i=0;i<5;i++)if(glyph[j]&(1<<(4-i)))c.fillRect(x+i*size,y+j*size,size,size);x+=6*size;}}
 function paint(){const accent=theme==='red'?'#ff315b':'#64ffe1';c.clearRect(0,0,1280,800);c.fillStyle=theme==='red'?'#211015':'#10243c';c.fillRect(80,130,1120,610);c.fillStyle=accent;c.fillRect(80,130,1120,3);text(110,170,4,'CAT RESCUE SAGA',accent);for(let i=0;i<10&&offset+i<rows.length;i++)text(110,270+i*32,2,rows[offset+i],'#fff');for(const [x,y,w,label] of [[1070,160,100,'SKIP'],[110,682,180,'PREVIOUS'],[940,682,230,offset+10<rows.length?'NEXT':'CONTINUE']]){c.fillStyle=theme==='red'?'#31141e':'#14283d';c.fillRect(x,y,w,42);text(x+16,y+13,2,label,accent);}if(artwork.complete&&artwork.naturalWidth){c.imageSmoothingEnabled=false;c.drawImage(artwork,790,315,384,256);}text(450,695,2,'PAGE '+(offset/10+1)+' OF '+Math.ceil(rows.length/10),'#8fa1b5');host.setAttribute('aria-description',rows.slice(offset,offset+10).join(' '));}
 function close(){sessionStorage.setItem('rx3-story-seen','1');host.remove();document.removeEventListener('keydown',key,true);}
 function next(){if(offset+10<rows.length){offset+=10;paint();}else close();}
 function key(e){e.stopImmediatePropagation();if(['Enter','ArrowRight','ArrowDown',' '].includes(e.key)){e.preventDefault();next();}else if(['ArrowLeft','ArrowUp'].includes(e.key)){e.preventDefault();offset=Math.max(0,offset-10);paint();}else if(e.key==='Escape')close();}
 host.onclick=e=>{const r=host.getBoundingClientRect(),x=(e.clientX-r.left)*1280/r.width,y=(e.clientY-r.top)*800/r.height;if(x>=1070&&x<1170&&y>=160&&y<206)close();else if(y>=682&&y<724){if(x>=110&&x<290){offset=Math.max(0,offset-10);paint();}else if(x>=940&&x<1170)next();}};
 host.onwheel=e=>{e.preventDefault();e.stopPropagation();offset=Math.max(0,Math.min(Math.floor((rows.length-1)/10)*10,offset+(e.deltaY>0?10:-10)));paint();};
 document.querySelector('#ready').hidden=true;document.querySelector('main').append(host);document.addEventListener('keydown',key,true);paint();
})().catch(console.error);
