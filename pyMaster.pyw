from Tkinter import *
import sys
from code import InteractiveConsole
from imp import new_module
import subprocess
from threading import Thread
import numpy as np
import cPickle as pickle

class Console(InteractiveConsole):
 
            def __init__(self, names=None):
                names = names or {}
                names['console'] = self
                self.superspace = new_module('superspace')
                names['superspace'] = self.superspace
                InteractiveConsole.__init__(self, names)
                
         
            def enter(self, source):
                source = self.preprocess(source)
                self.runcode(source)
         
            @staticmethod
            def preprocess(source):
                return source

class PerFrame():
	def __init__(self, widget):
		self.widget = widget
		self.keywords = {}
		with open("keywords.settings", 'rb') as handle:
			self.keywords = pickle.load(handle)
		
		
	def syntax_tag(self):
		for tag in self.widget.tag_names():
			if tag == "sel":
				continue
			self.widget.remove_tags(tag)
		for key_word in self.keywords:
			self.widget.highlight_pattern(key_word, self.keywords[key_word])
		self.widget.after(100,self.syntax_tag)

class CustomFrame(Frame):
	def __init__(self, *args, **kwargs):
		Frame.__init__(self, *args, background= '#000000', borderwidth=7, **kwargs)

		
class CustomText(Text):
	def __init__(self, *args, **kwargs):
		Text.__init__(self, *args, background= '#000000', **kwargs)

	def highlight_pattern(self, pattern, tag, start="1.0", end="end",
						  regexp=False):
		'''Apply the given tag to all text that matches the given pattern

		If 'regexp' is set to True, pattern will be treated as a regular
		expression according to Tcl's regular expression syntax.
		'''

		start = self.index(start)
		end = self.index(end)
		self.mark_set("matchStart", start)
		self.mark_set("matchEnd", start)
		self.mark_set("searchLimit", end)

		count = IntVar()
		while True:
			index = self.search(pattern, "matchEnd","searchLimit",
								count=count, regexp=regexp)
			if index == "": break
			if count.get() == 0: break # degenerate pattern which matches zero-length strings
			self.mark_set("matchStart", index)
			self.mark_set("matchEnd", "%s+%sc" % (index, count.get()))
			self.tag_add(tag, "matchStart", "matchEnd")
	def remove_tags(self, tag):
		try:
			self.tag_remove(tag,tag + ".first",tag + ".last")
		except:
			pass
			
class IORedirector(object):
            def __init__(self,widget):
                self.widget = widget

class StdoutRedirector(IORedirector):
	def write(self,text):
		self.widget.configure(text= self.widget.cget('text') + text)

class StderrRedirector(IORedirector):
	def write(self,text):
		self.widget.configure(text= self.widget.cget('text') + text)

class App():

    def popup(self,message):
        self.top=Toplevel()
        self.top.overrideredirect(True)
        self.f = CustomFrame(self.top)
        self.f.pack(expand=True, fill='both')
        self.l=Label(self.f,text=message)
        self.l.pack()
        self.e=Entry(self.f)
        self.e.pack()
        self.b=Button(self.f,text='Ok',command=self.cleanup)
        self.b.pack()

        root.wait_window(self.top)
        return self.result
        

    def cleanup(self):
        self.result = self.e.get()
        self.top.destroy()
        
        
    def resize_window(self, event):
        if self.curs_x == None:
            self.curs_x = event.x
        else:
            move_x = event.x - self.curs_x
            if move_x > 0:
                self.width += 1
                self.curs_x = event.x
            elif move_x < 0:
                self.width -= 1
                self.curs_x = event.x

        if self.curs_y == None:
            self.curs_y = event.y
        else:
            move_y = event.y - self.curs_y
            if move_y > 0:
                self.height += 1
                self.curs_y = event.y
            elif move_y < 0:
                self.height -= 1
                self.curs_y = event.y
                
        #Replace this line. Redundant
        self.label.configure(text= "")        
        print "{}, {}".format(root.winfo_width(),root.winfo_height())       
        root.geometry("{}x{}+{}+{}".format(self.width,self.height,self.x,self.y))

    def resize_release(self, event):
        self_curs_x = None
        self_curs_y = None
        
    def move_release(self, event):
        self.second_x = event.x_root
        self.second_y = event.y_root
        self.second_window.geometry("{}x{}+{}+{}".format(self.second_width,self.second_height,self.second_x,self.second_y))

    def cmd(self, command):
        return subprocess.Popen(command.split(), stdout = subprocess.PIPE, stdin = subprocess.PIPE, stderr = subprocess.PIPE).communicate()
        
        

        
        
    def move_window(self, event):
        if self.curs_x == None:
            self.curs_x = event.x
        else:
            move_x = event.x - self.curs_x
            if move_x > 0:
                self.x += 1
                self.curs_x = event.x
            elif move_x < 0:
                self.x -= 1
                self.curs_x = event.x
        

        if self.curs_y == None:
            self.curs_y = event.y
        else:
            move_y = event.y - self.curs_y
            if move_y > 0:
                self.y += 1
                
                self.curs_y = event.y
            elif move_y < 0:
                self.y -= 1
                self.curs_y = event.y

        root.geometry("{}x{}+{}+{}".format(self.width,self.height,self.x,self.y))
        
        
    def run_script_live(self, event):
        self.label.configure(text= "")
        self.console.enter(self.scriptText.get("1.0",'end-1c'))

    def clear_script(self, event):
        self.label.configure(text= "")
        self.cmd(self.popup("whatsup"))
 

    def compile_c(self, event):
        #print subprocess.Popen("pyinstaller --onefile --window --distpath . saved_script.py".split())
        print self.cmd("g++ -o engines\engine c\main.cpp  -lmingw32 -lSDL2main -lSDL2 -lglfw3 -lopengl32 -lglew32 -lgdi32 -lBox2D -lBulletDynamics -lBulletCollision -lBulletSoftBody -lLinearMath -mwindows")

    def poll(self, event):
        t = Thread(target=self.run_program);
        t.daemon = True
        t.start()
        
    def run_program(self):
        print self.cmd("engines\\engine.exe 1200 900 340 55")

    def save_script(self,event):
        with open(self.currentFile, "w") as f:
            f.write(self.scriptText.get("1.0",'end-1c'))
    def open_file(self, event):
        try:
            with open(self.currentFile) as f:
                data = f.read()
                self.scriptText.delete('1.0', END)
                self.scriptText.insert(INSERT, data)
        except IOError as e:
            print "File does not exist"

    def pop_menu(self, event):
        self.menubar.post(event.x_root, event.y_root)

    def custom_cmd(self):
        self.custom_python = self.entry.get()
        self.console.enter(self.custom_python)
	
    def test_print(self, event):
        print "test"

       


            
            
    def __init__(self, master):
        self.frame = CustomFrame(root)
        self.width = 620
        self.height = 580
        self.x = 10
        self.y = 10
        self.custom_python = "dir"

        self.second_width = 300
        self.second_height = 300
        self.second_x = 900
        self.second_y = 300



        
        self.curs_x = None
        self.curs_y = None
        self.currentFile = "py\\saved_script.py"
        root.geometry("{}x{}+{}+{}".format(self.width,self.height,self.x,self.y))
        self.frame.bind_all("<Control-r>", self.run_script_live)
        self.frame.bind_all("<Control-e>", self.clear_script)
        self.frame.bind_all("<Control-q>", self.compile_c)
        self.frame.bind_all("<Control-w>", self.poll)
        self.frame.bind_all("<Control-s>", self.save_script)
        self.frame.bind_all("<Control-o>", self.open_file)
        self.frame.bind("<B3-Motion>", self.resize_window)
        self.frame.bind("<B1-Motion>", self.move_window)
        self.frame.bind_all("<ButtonRelease-1>", self.resize_release)
        self.frame.bind_all("<ButtonRelease-3>", self.resize_release)
        self.frame.bind_all("<Control-p>", self.test_print)
        root.bind("<ButtonRelease-2>", self.pop_menu)
        
        
        
        
        self.frame.pack(expand=True, fill='both')

        
                

        

        
        
        self.menubar = Menu(self.frame)

        menu = Menu(self.menubar, tearoff=0)
        self.menubar.add_cascade(label="File", menu=menu)
        menu.add_command(label="New")

        menu = Menu(self.menubar, tearoff=0)
        self.menubar.add_cascade(label="Edit", menu=menu)
        menu.add_command(label="Cut")
        menu.add_command(label="Copy")
        menu.add_command(label="Paste")
  
        self.scriptText = CustomText(self.frame, foreground='#00FF00', font="system")
        self.scriptText.tag_configure("red", foreground="#ff0000")
        self.scriptText.tag_configure("blue", foreground="#0000ff")

        self.scriptText.pack(expand=True, fill='both')
        self.scriptText.focus_set()

        

        self.second_window = Toplevel()
        self.second_window.geometry("{}x{}+{}+{}".format(self.second_width,self.second_height,self.second_x,self.second_y))
        self.second_window.attributes("-alpha", 0.9)
        self.second_window.wm_attributes('-topmost', 1)
        self.second_window.overrideredirect(True)

        self.second_window.bind("<ButtonRelease-2>", self.pop_menu)
        

        self.second_frame = CustomFrame(self.second_window)
        self.second_frame.pack(expand=True, fill='both')

        self.label = Label(self.second_frame, text="", relief=RAISED, anchor='nw', bg="black", foreground='#00FF00')
        self.label.pack(expand=True, fill='both')

        self.entry=Entry(self.second_frame)
        self.entry.pack()
        
        self.button = Button(self.second_frame, text="OK", command=self.custom_cmd)
        self.button.pack()
        self.second_frame.bind("<ButtonRelease-1>", self.move_release)

        
        

        #Passes sys out and syserr to designated widget
        sys.stdout = StdoutRedirector(self.label)
        sys.stderr = StdoutRedirector(self.label)

        #Start instance of Console
        self.console = Console()
        #Here you can add variables that can survive the console using new module
        self.console.superspace.a = 6
        
        #Setup action per frame instance and call repeating function
        per_frame_action = PerFrame(self.scriptText)
        per_frame_action.syntax_tag()

        

if __name__ == "__main__":
    root = Tk()
    root.overrideredirect(True)
    root.attributes("-alpha", 0.9)
    root.wm_attributes('-topmost', 1)
    root.bind_all("<Escape>", lambda e: e.widget.quit())
    app = App(root)
    root.mainloop()





"""    Code to make a new Label


self.third_window = Toplevel()
        self.third_window.geometry("{}x{}+{}+{}".format(self.second_width,self.second_height,self.second_x,self.second_y-300))
        self.third_window.attributes("-alpha", 0.9)
        self.third_window.wm_attributes('-topmost', 1)
        self.third_window.overrideredirect(True)

        self.third_frame = Frame(self.third_window , background = "black", borderwidth=7)
        self.third_frame.pack(expand=True, fill='both')

"""
