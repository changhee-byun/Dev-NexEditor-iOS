from Tkinter import *
import tkFileDialog
import tkMessageBox
import kedl
import appdirs
import os
import shelve

class Application(Frame):
    
    def sel_files(self):
        fnames = tkFileDialog.askopenfilenames(filetypes=(("XML files", "*.xml"),("NFXS files", "*.nfxs"),
                                           ("All files", "*.*") ))
        #fnames = fnames.split() # only for Windows
        current_files = self.file_list_box.get(0,END)
        #print(repr(fnames))
        for fname in fnames:
            if fname in current_files:
                continue
            self.file_list_box.insert(END,fname)

    def remove_sel_file(self):
        sel = self.file_list_box.curselection()
        if len(sel)>0:
            self.file_list_box.delete(sel[0])

    def clear_files(self):
        self.file_list_box.delete(0,self.file_list_box.size()-1)
    
    def test_pkg(self):
        try:
            kedl.kedltool("test",input=self.file_list_box.get(0,END),output=self.tempdir,keys=self.keys_entry.get(),autoclean=True,sdkroot=self.sdk_entry.get())
        except:
            tkMessageBox.showerror( "Packaging Error", str(sys.exc_info()[0]) )
            raise

    def make_pkg(self):
        kedl.kedltool("package",input=self.file_list_box.get(0,END),output=self.output_entry.get(),keys=self.keys_entry.get())

    def browse_output(self):
        selfolder = tkFileDialog.askdirectory(mustexist=True, title="Output Folder")
        if selfolder is not None and len(selfolder)>0:
            self.output_entry.delete(0,END)
            self.output_entry.insert(0,selfolder)
            self.shelf["output_path"] = selfolder
            self.shelf.sync()

    def browse_keys(self):
        selfolder = tkFileDialog.askdirectory(mustexist=True, title="Keys Folder")
        if selfolder is not None and len(selfolder)>0:
            self.keys_entry.delete(0,END)
            self.keys_entry.insert(0,selfolder)
            self.shelf["key_path"] = selfolder
            self.shelf.sync()

    def browse_sdk(self):
        selfolder = tkFileDialog.askdirectory(mustexist=True, title="Android SDK Root")
        if selfolder is not None and len(selfolder)>0:
            self.sdk_entry.delete(0,END)
            self.sdk_entry.insert(0,selfolder)
            self.shelf["sdk_path"] = selfolder
            self.shelf.sync()


    def createWidgets(self):

        bg = 'gray'
        rownum = 0
        
        top=self.winfo_toplevel()
        self.configure(background=bg)
        top.rowconfigure(0, weight=1)
        top.columnconfigure(0, weight=1)
        
        #-----------------------------------------------------------------------
        rownum += 1

        self.label_input = Label(self)
        self.label_input["text"] = "Input Files (.xml)"
        self.label_input.configure(background=bg)
        self.label_input.grid(column=0, row=rownum, columnspan=6, sticky=E+W, pady=(15,0) )

        
        #-----------------------------------------------------------------------
        rownum += 1
        self.file_list_box = Listbox(self)
        self.file_list_box.grid(column=0, row=rownum, columnspan=6, sticky=N+S+E+W, padx=15, pady=(10,0))
        self.rowconfigure(rownum, weight=1)
        self.columnconfigure(1, weight=1)
        
        
        
        #-----------------------------------------------------------------------
        rownum += 1
        
        self.CLEAR = Button(self)
        self.CLEAR.configure(highlightbackground=bg)
        self.CLEAR["text"] = "Clear"
        self.CLEAR["command"] =  self.clear_files
        self.CLEAR.grid(column=0, row=rownum, sticky=S+W, padx=(15,0), pady=(0,20))
        
        self.DELETE = Button(self)
        self.DELETE.configure(highlightbackground=bg)
        self.DELETE["text"] = "Delete"
        self.DELETE["command"] =  self.remove_sel_file
        self.DELETE.grid(column=3, columnspan=2, row=rownum, sticky=S+E, padx=(15,0), pady=(0,20))
        
        self.select_files = Button(self)
        self.select_files.configure(highlightbackground=bg)
        self.select_files["text"] = "Browse..."
        self.select_files["command"] = self.sel_files
        self.select_files.grid(column=5, row=rownum, sticky=S+E, padx=(0,15), pady=(0,20))
        
        
        
        #-----------------------------------------------------------------------
        rownum += 1
        
        self.label_output = Label(self)
        self.label_output["text"] = "Output:"
        self.label_output.configure(background=bg)
        self.label_output.grid(column=0, row=rownum, sticky=E, pady=(15,0), padx=(15,0) )
        
        self.output_entry = Entry(self)
        self.output_entry.configure(highlightbackground=bg)
        self.output_entry.grid(column=1, row=rownum, columnspan=4, sticky=E+W, pady=(15,0), padx=(0,5) )
        
        self.BROWSE = Button(self)
        self.BROWSE.configure(highlightbackground=bg)
        self.BROWSE["text"] = "Browse..."
        self.BROWSE["command"] =  self.browse_output
        self.BROWSE.grid(column=5, row=rownum, sticky=S+W, padx=(0,15), pady=(15,0))

        
        
        
        #-----------------------------------------------------------------------
        rownum += 1
        
        self.label_keys = Label(self)
        self.label_keys["text"] = "Keys:"
        self.label_keys.configure(background=bg)
        self.label_keys.grid(column=0, row=rownum, sticky=E, pady=(15,15), padx=(15,0) )
        
        self.keys_entry = Entry(self)
        self.keys_entry.configure(highlightbackground=bg)
        self.keys_entry.grid(column=1, row=rownum, columnspan=4, sticky=E+W, pady=(15,15), padx=(0,5) )
        
        self.KEY_BROWSE = Button(self)
        self.KEY_BROWSE.configure(highlightbackground=bg)
        self.KEY_BROWSE["text"] = "Browse..."
        self.KEY_BROWSE["command"] =  self.browse_keys
        self.KEY_BROWSE.grid(column=5, row=rownum, sticky=S+W, padx=(0,15), pady=(15,15))
        
        
        
        #-----------------------------------------------------------------------
        rownum += 1
        
        self.label_sdk = Label(self)
        self.label_sdk["text"] = "Android SDK Root:"
        self.label_sdk.configure(background=bg)
        self.label_sdk.grid(column=0, row=rownum, sticky=E, pady=(15,15), padx=(15,0) )
        
        self.sdk_entry = Entry(self)
        self.sdk_entry.configure(highlightbackground=bg)
        self.sdk_entry.grid(column=1, row=rownum, columnspan=4, sticky=E+W, pady=(15,15), padx=(0,5) )
        
        self.SDK_BROWSE = Button(self)
        self.SDK_BROWSE.configure(highlightbackground=bg)
        self.SDK_BROWSE["text"] = "Browse..."
        self.SDK_BROWSE["command"] =  self.browse_sdk
        self.SDK_BROWSE.grid(column=5, row=rownum, sticky=S+W, padx=(0,15), pady=(15,15))
        
        
        
        #-----------------------------------------------------------------------
        rownum += 1

        self.QUIT = Button(self)
        self.QUIT.configure(highlightbackground=bg)
        self.QUIT["text"] = "Quit"
        self.QUIT["command"] =  self.quit
        self.QUIT.grid(column=0, row=rownum, sticky=S+W, padx=15, pady=15)
        
        self.TEST = Button(self)
        self.TEST.configure(highlightbackground=bg)
        self.TEST["text"] = "Test on Device"
        self.TEST["command"] =  self.test_pkg
        self.TEST.grid(column=3, row=rownum, sticky=S+W, padx=(0,30), pady=15)
        
        self.PACKAGE = Button(self)
        self.PACKAGE.configure(highlightbackground=bg)
        self.PACKAGE["text"] = "PUBLISH"
        self.PACKAGE["command"] =  self.make_pkg
        self.PACKAGE.grid(column=5, row=rownum, sticky=S+W, padx=(0,15), pady=15)

        top.update()
        top.minsize(top.winfo_width(), top.winfo_height()-100)
        top.geometry("750x400")

    def loadsettings(self):
        datadir = appdirs.user_data_dir("KineMaster KEDL","NexStreaming")
        if not os.path.isdir(datadir):
            os.makedirs(datadir)
        datafile = os.path.join(datadir,"kedl_pkg_tool.dat")
        self.shelf = shelve.open(datafile,flag='c')
        self.output_entry.delete(0,END)
        if self.shelf.has_key("output_path"):
            self.output_entry.insert(0,str(self.shelf.get("output_path")))
        self.keys_entry.delete(0,END)
        if self.shelf.has_key("key_path"):
            self.keys_entry.insert(0,str(self.shelf.get("key_path")))
        if self.shelf.has_key("sdk_path"):
            self.sdk_entry.insert(0,str(self.shelf.get("sdk_path")))
        self.tempdir = os.path.join(appdirs.user_cache_dir("KineMaster KEDL","NexStreaming"),"tmp_output")
        if not os.path.isdir(self.tempdir):
            os.makedirs(self.tempdir)

    def __init__(self, master=None):
        Frame.__init__(self, master)
        self.grid(sticky=N+S+E+W)
        self.createWidgets()
        self.loadsettings()


def kedlgui():
    app = Application()
    app.master.title('KEDL Packaging Tool v0.1')
    app.mainloop()
    app.shelf.close()

if __name__ == '__main__':
    kedlgui()
