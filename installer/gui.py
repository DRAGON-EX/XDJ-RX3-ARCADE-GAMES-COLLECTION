"""Portable Windows/Linux Tk GUI. Worker threads never access Tk widgets."""
from __future__ import annotations
import pathlib, queue, sys, threading, tkinter as tk, webbrowser
from tkinter import filedialog, messagebox, ttk, font
from installer import core

class Installer(tk.Tk):
    def __init__(self):
        super().__init__();self.title('DJ DRAGON-X Cat Rescue Saga Installer')
        self.geometry('860x840');self.minsize(820,800)
        self.events=queue.Queue();self.key=None;self.valid_paths=None;self.busy=False;self.targets=[]
        self.protocol('WM_DELETE_WINDOW',self.close)
        family = 'Segoe UI' if sys.platform == 'win32' else 'DejaVu Sans'
        for name in ('TkDefaultFont', 'TkTextFont', 'TkMenuFont', 'TkHeadingFont'):
            font.nametofont(name).configure(family=family, size=10)
        style=ttk.Style(self)
        if sys.platform == 'win32' and 'vista' in style.theme_names(): style.theme_use('vista')
        style.configure('TButton', padding=7)
        body=ttk.Frame(self,padding=26);body.pack(fill='both',expand=True)
        ttk.Label(body,text='DJ DRAGON-X Cat Rescue Saga Installer',font=(family,17,'bold')).pack(anchor='w')
        ttk.Label(body,text='Install Cat Rescue Saga on your USB stick · Version '+core.VERSION).pack(anchor='w',pady=(4,22))
        ttk.Label(body,text='1   Choose your USB stick',font=(family,12,'bold')).pack(anchor='w')
        row=ttk.Frame(body);row.pack(fill='x',pady=8)
        self.drive=ttk.Combobox(row,state='readonly');self.drive.pack(side='left',fill='x',expand=True)
        self.drive.bind('<<ComboboxSelected>>',lambda e:self.ready())
        self.refresh_button=ttk.Button(row,text='Refresh',command=self.refresh);self.refresh_button.pack(side='left',padx=(10,0))
        ttk.Label(body,text='FAT32 or exFAT USB drives only. No formatting. Music and playlists stay on the stick.',wraplength=750).pack(anchor='w')
        ttk.Label(body,text='2   Add the two Pioneer source packages',font=(family,12,'bold')).pack(anchor='w',pady=(23,8))
        ttk.Label(body,text='Please obtain the XDJ-RX3 source-code files yourself from Pioneer DJ’s website and read their terms. This installer does not download those files for you. Download both parts, then select Part 1 and Part 2 below. Original ZIP downloads and extracted .00 / .01 files are accepted.',wraplength=748).pack(anchor='w')
        ttk.Button(body,text='Open Pioneer DJ source-code page',command=lambda:webbrowser.open('https://www.pioneerdj.com/en/support/open-source-code-distribution/gnu-open-source-license/')).pack(anchor='w',pady=7)
        self.paths=[tk.StringVar(),tk.StringVar()];self.pick_buttons=[]
        for i in range(2):
            row=ttk.Frame(body);row.pack(fill='x',pady=4)
            ttk.Label(row,text='Part '+str(i+1),width=7).pack(side='left')
            ttk.Entry(row,textvariable=self.paths[i],state='readonly').pack(side='left',fill='x',expand=True)
            b=ttk.Button(row,text='Choose…',command=lambda i=i:self.pick(i));b.pack(side='left',padx=(10,0));self.pick_buttons.append(b)
        self.agree=tk.BooleanVar();self.agree_button=ttk.Checkbutton(body,text='I have read and accepted the manufacturer’s source-package terms.',variable=self.agree,command=self.invalidate);self.agree_button.pack(anchor='w',pady=8)
        self.check_button=ttk.Button(body,text='Check source packages',command=self.check);self.check_button.pack(anchor='w')
        row=ttk.Frame(body);row.pack(fill='x',pady=(18,6))
        ttk.Label(row,text='GitHub token (only for private repository access)').pack(anchor='w')
        self.token=tk.StringVar();ttk.Entry(row,textvariable=self.token,show='•').pack(fill='x',pady=4)
        ttk.Label(row,text='Read-only Contents permission. The token is used only for this installation and is not saved.',font=(family,9)).pack(anchor='w')
        self.persistent=tk.BooleanVar(value=False)
        self.save_button=ttk.Checkbutton(body,text='PERSISTENT SAVE FILE',variable=self.persistent)
        self.save_button.pack(anchor='w',pady=(12,3))
        ttk.Label(body,text='Not recommended: unplugging the USB stick while saving can corrupt its filesystem. Enable only if you want highscores, rescued cats and unlocks to survive a restart. Never unplug while SAVING is shown.',foreground='#b00020',wraplength=790).pack(anchor='w')
        self.status=tk.StringVar(value='Choose a USB stick and both source packages to begin.')
        ttk.Label(body,textvariable=self.status,wraplength=740).pack(anchor='w',pady=(12,8))
        self.bar=ttk.Progressbar(body,mode='indeterminate');self.bar.pack(fill='x')
        self.install_button=ttk.Button(body,text='Install to USB',state='disabled',command=self.install);self.install_button.pack(fill='x',pady=(15,5))
        self.after(100,self.poll);self.refresh()
    def close(self):
        if self.busy: messagebox.showinfo('Installation in progress','Please wait until the current operation finishes.');return
        self.key=None;self.token.set('');self.destroy()
    def invalidate(self): self.key=None;self.valid_paths=None;self.ready()
    def pick(self,i):
        path=filedialog.askopenfilename(title='Pioneer XDJ-RX3 source package — Part '+str(i+1),filetypes=[('Source packages','*.zip *.00 *.01'),('All files','*')])
        if path: self.paths[i].set(path);self.invalidate()
    def refresh(self):
        try:
            self.targets=core.drives();self.drive['values']=[f'{x.label} — {x.root} — {x.size/1024**3:.1f} GiB' for x in self.targets]
            self.drive.set('Select a USB stick' if self.targets else 'No mounted FAT32/exFAT USB stick found')
        except Exception as e: self.status.set(str(e));self.targets=[]
        self.ready()
    def ready(self):
        valid=not self.busy and self.key is not None and self.agree.get() and 0<=self.drive.current()<len(self.targets) and tuple(x.get() for x in self.paths)==self.valid_paths
        self.install_button.configure(state='normal' if valid else 'disabled')
    def start(self,fn,kind):
        self.busy=True;self.bar.start(15);self.ready()
        for b in self.pick_buttons+[self.check_button,self.refresh_button,self.agree_button,self.save_button]: b.configure(state='disabled')
        self.drive.configure(state='disabled')
        def worker():
            try: self.events.put((kind,fn(lambda s:self.events.put(('status',s)))))
            except Exception as e: self.events.put(('error',str(e)))
        threading.Thread(target=worker,daemon=True).start()
    def check(self):
        if not self.agree.get(): messagebox.showinfo('Pioneer terms','Read and accept the manufacturer’s terms first.');return
        paths=tuple(x.get() for x in self.paths);self.key=None;self.valid_paths=None
        self.start(lambda progress:(core.validate_parts(paths,progress),paths),'checked')
    def install(self):
        self.ready()
        if self.install_button['state']=='disabled': return
        target=self.targets[self.drive.current()]
        if not messagebox.askyesno('Install to selected USB stick',f'{target.label}\n{target.root}\n\nInstall the latest game package here? An existing autoexec.bin will be backed up before replacement. No formatting.'):return
        key=self.key;token=self.token.get();self.token.set('');persistent=self.persistent.get()
        self.start(lambda progress:core.run(target,key,token,progress,persistent=persistent),'installed')
    def poll(self):
        try:
            while True:
                kind,value=self.events.get_nowait()
                if kind=='status': self.status.set(value);continue
                self.busy=False;self.bar.stop()
                for b in self.pick_buttons+[self.check_button,self.refresh_button,self.agree_button,self.save_button]: b.configure(state='normal')
                self.drive.configure(state='readonly')
                if kind=='checked': self.key,self.valid_paths=value;self.status.set('Both Pioneer parts are valid. Ready to install.')
                elif kind=='installed':
                    self.status.set('Version '+value+' installed and verified. Safely eject your USB stick before unplugging it.')
                    messagebox.showinfo('USB stick ready',self.status.get())
                else: self.status.set(value);messagebox.showerror('Could not complete operation',value)
                self.ready()
        except queue.Empty: pass
        self.after(100,self.poll)

def main(): Installer().mainloop()
if __name__=='__main__':main()
