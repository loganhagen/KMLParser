import sys
import tkinter as tk
from tkinter import ttk
from ctypes import *
from tkinter.simpledialog import askstring
from tkinter.messagebox import askyesno
from tkinter import messagebox

class KML(Structure):
    _fields_ = [
        ("namespaces", c_void_p),
        ("pointPlacemarks", c_void_p),
        ("pathPlacemarks", c_void_p),
        ("styles", c_void_p),
        ("styleMaps", c_void_p)]

kmllibPath = './libkmlparser.so'
kmllib = CDLL(kmllibPath)
schemaFile = "ogckml22.xsd"

createValidKML = kmllib.createValidKML
createValidKML.argtypes = [c_char_p, c_char_p]
createValidKML.restype = POINTER(KML)

validateKML = kmllib.validateKML
validateKML.argtypes = [c_void_p, c_char_p]
validateKML.restype = int

listLen = kmllib.getLength
listLen.argtypes = [c_void_p]
listLen.restype = int

elemToStr = kmllib.toString
elemToStr.argtypes = [c_void_p]
elemToStr.restype = c_char_p

updatePoint = kmllib.updatePoint
updatePoint.argtpyes = [c_char_p, int, c_void_p]
updatePoint.restype = int

updatePath = kmllib.updatePath
updatePath.argtpyes = [c_char_p, int, c_void_p]
updatePath.restype = int

updateStyle = kmllib.updateStyle
updateStyle.argtpyes = [c_char_p, c_char_p, int, c_void_p]
updateStyle.restype = int

writeKML = kmllib.writeKML
writeKML.argtypes = [c_void_p, c_char_p]
writeKML.restype = int

isLoop = kmllib.isLoopPath
isLoop.argtypes = [c_void_p, c_double]
isLoop.restype = int

deleteKML = kmllib.deleteKML
deleteKML.argtypes = [POINTER(KML)]

kmlPtr = POINTER(KML)()
filename = -1

def openKML(*args):
    clearTrees()
    setRootTitle()

    global filename
    global kmlPtr

    filename = askstring("Open KML", "Filename:", parent=root) 
    
    if filename and not filename.isspace():
        fStr = filename.encode("UTF-8")
        xsdStr = schemaFile.encode("UTF-8")

        kmlPtr = createValidKML(fStr, xsdStr)

        if not kmlPtr:
            msg = "<{}> not opened.".format(filename)
            logPanel.insert(tk.END, msg)
            return

        msg = "<{}> successfully opened and KML created.".format(filename)
        logPanel.insert(tk.END, msg)     

        root.title(filename)
        kmlContents = kmlPtr.contents       

        if listLen(kmlContents.pointPlacemarks) > 0:
            elemStr = elemToStr(kmlContents.pointPlacemarks)
            elemStr = elemStr.decode("utf-8")

            arr = elemStr.split("\n")
            arr = list(filter(None, arr))

            i = 0
            for a in arr:
                t = a.split(";")
                pointTree.insert(parent="", iid=i, index="end", text="", values=(t[0], t[1])) 
                i += 1

        if listLen(kmlContents.pathPlacemarks) > 0:
            elemStr = elemToStr(kmlContents.pathPlacemarks)
            elemStr = elemStr.decode("utf-8")

            arr = elemStr.split("\n")
            arr = list(filter(None, arr))

            i = 0
            for a in arr:
                t = a.split(",")
                pathTree.insert(parent="", iid=i, index="end", text="", values=(t[0], t[1], t[2]))
                i += 1

        if listLen(kmlContents.styles) > 0:
            elemStr = elemToStr(kmlContents.styles)
            elemStr = elemStr.decode("utf-8")

            arr = elemStr.split("\n")
            arr = list(filter(None, arr))

            i = 0
            for a in arr:
                t = a.split(",")
                styleTree.insert(parent="", iid=i, index="end", text="", values=(t[0], t[1], t[2]))
                i += 1    

def saveKML(*args):
    if filename == -1:
        return

    i = 0
    for row in pointTree.get_children():
        arr = pointTree.item(row)["values"]
        updatePoint(arr[0].encode("UTF-8"), i, kmlPtr)
        i += 1

    i = 0
    for row in pathTree.get_children():
        arr = pathTree.item(row)["values"]
        updatePath(arr[0].encode("UTF-8"), i, kmlPtr)
        i += 1

    i = 0
    for row in styleTree.get_children():
        arr = styleTree.item(row)["values"]
        s1 = str(arr[0]).encode("UTF-8")
        s2 = str(arr[1]).encode("UTF-8")
        updateStyle(s1, s2, i, kmlPtr)
        i += 1


    valid = validateKML(kmlPtr, schemaFile.encode("UTF-8"))

    if valid == 1:
        msg = "<{}> successfully validated.".format(filename)
        logPanel.insert(tk.END, msg)
        written = writeKML(kmlPtr, filename.encode("UTF-8"))
        if written == 1:
            msg = "<{}> successfully saved.".format(filename)
            logPanel.insert(tk.END, msg)
        elif written == 0:
            msg = "<{}> failed to saved.".format(filename)
            logPanel.insert(tk.END, msg)
    elif valid == 0:
        msg = "Data failed to validate. File not saved."
        logPanel.insert(tk.END, msg)
        return    

def saveKMLas(*args):
    global filename

    if filename == -1:
        return

    newFilename = askstring("filename", "Filename:", parent=root) 

    if newFilename == filename:
        confirm = askyesno(title="Confirm Overwrite", message="Are you sure you want to overwrite <{}>?".format(filename))
        
        if not confirm:
            msg = "Save operation cancelled."
            logPanel.insert(tk.END, msg)
            return

    filename = newFilename

    i = 0
    for row in pointTree.get_children():
        arr = pointTree.item(row)["values"]
        updatePoint(arr[0].encode("UTF-8"), i, kmlPtr)
        i += 1

    i = 0
    for row in pathTree.get_children():
        arr = pathTree.item(row)["values"]
        updatePath(arr[0].encode("UTF-8"), i, kmlPtr)
        i += 1

    i = 0
    for row in styleTree.get_children():
        arr = styleTree.item(row)["values"]
        updateStyle(arr[0].encode("UTF-8"), arr[1], i, kmlPtr)
        i += 1

    valid = validateKML(kmlPtr, schemaFile.encode("UTF-8"))

    if valid == 1:
        msg = "<{}> successfully validated.".format(filename)
        logPanel.insert(tk.END, msg)
        written = writeKML(kmlPtr, filename.encode("UTF-8"))
        if written == 1:
            msg = "<{}> successfully saved.".format(filename)
            logPanel.insert(tk.END, msg)
        elif written == 0:
            msg = "<{}> failed to saved.".format(filename)
            logPanel.insert(tk.END, msg)
    elif valid == 0:
        msg = "Data failed to validate. File not saved."
        logPanel.insert(tk.END, msg)
        return    

def getSelectedPoint(*args):
    pointTxt.delete('1.0', tk.END)
    selected = pointTree.focus()
    pointTxt.insert(tk.END, pointTree.item(selected, "values")[0])

def getSelectedPath(*args):
    pathTxt.delete('1.0', tk.END)
    selected = pathTree.focus()
    pathTxt.insert(tk.END, pathTree.item(selected, "values")[0])

def getSelectedStyle(*args):
    widthTxt.delete('1.0', tk.END)
    colourTxt.delete('1.0', tk.END)
    selected = styleTree.focus()
    colourTxt.insert(tk.END, styleTree.item(selected, "values")[0])
    widthTxt.insert(tk.END, styleTree.item(selected, "values")[1])

def editPoint(*args):
    selected = pointTree.focus()
    if not selected:
        return

    newRecord = pointTxt.get("1.0", "end-1c")
    oldValues = pointTree.item(selected, "values")
    pointTree.item(selected, text="", values=(newRecord, oldValues[1]))

def editPath(*args):
    selected = pathTree.focus()
    if not selected:
        return

    newRecord = pathTxt.get("1.0", "end-1c")
    oldValues = pathTree.item(selected, "values")
    pathTree.item(selected, text="", values=(newRecord, oldValues[1], oldValues[2]))

def editStyle(*args):
    selected = styleTree.focus()
    if not selected:
        return

    newColour = colourTxt.get("1.0", "end-1c")
    newWidth = widthTxt.get("1.0", "end-1c")
    oldValues = styleTree.item(selected, "values")
    styleTree.item(selected, text="", values=(newColour, newWidth, oldValues[2]))

def clearTrees():
    for row in pointTree.get_children():
        pointTree.delete(row)

    for row in pathTree.get_children():
        pathTree.delete(row)
    
    for row in styleTree.get_children():
        styleTree.delete(row)

def clearLog():
    logPanel.delete(0, tk.END)

def setRootTitle():
    root.title("KMLParser")

def showAbout():
    str = """KMLParser, 2022.\nAuthor: Logan Hagen\nDisplays select contents of a given KML file for viewing and editing."""
    messagebox.showinfo("About", str)

def quitApp(*args):
    confirm = askyesno(title="Quit?", message="Are you sure you want to quit the app?")

    if confirm:
        sys.exit()   


root = tk.Tk()
root.geometry("1000x600")
setRootTitle()
root.rowconfigure(0, weight=1)
root.rowconfigure(1, weight=1)
root.rowconfigure(2, weight=1)
root.rowconfigure(3, weight=1)
root.rowconfigure(4, weight=1)
root.rowconfigure(5, weight=1)
root.rowconfigure(6, weight=1)
root.columnconfigure(0, weight=1)
root.columnconfigure(1, weight=1)

pointFrame = tk.LabelFrame(root, text="Points")
pointScroll = tk.Scrollbar(pointFrame)
pointScroll.pack(side=tk.RIGHT, fill=tk.Y)
pointTree = ttk.Treeview(pointFrame, yscrollcommand=pointScroll.set)
pointTree['columns'] = ("Name", "Coordinate")
pointTree.column("#0", width=0, stretch=tk.NO)
pointTree.column("Name", width=120)
pointTree.column("Coordinate", width=120)
pointTree.heading("#0", text="#0")
pointTree.heading("Name", text="Name")
pointTree.heading("Coordinate", text="Coordinate")
pointTree.pack(fill=tk.BOTH, expand=True)
pointScroll.config(command=pointTree.yview)
pointEdit = tk.Button(root, text="Edit", command=editPoint)
pointLbl = tk.Label(root, text="Name")
pointTxt = tk.Text(root, height=1, width=40)

pathFrame = tk.LabelFrame(root, text="Paths")
pathScroll = tk.Scrollbar(pathFrame)
pathScroll.pack(side=tk.RIGHT, fill=tk.Y)
pathTree = ttk.Treeview(pathFrame, yscrollcommand=pathScroll.set)
pathTree['columns'] = ("Name", "Length", "IsLoop")
pathTree.column("#0", width=0, stretch=tk.NO)
pathTree.column("Name", width=120)
pathTree.column("Length", width=120)
pathTree.column("IsLoop", width=80)
pathTree.heading("#0", text="#0")
pathTree.heading("Name", text="Name")
pathTree.heading("Length", text="Length")
pathTree.heading("IsLoop", text="IsLoop")
pathTree.pack(fill=tk.BOTH, expand=True)
pathScroll.config(command=pathTree.yview)
pathEdit = tk.Button(root, text="Edit", command=editPath)
pathLbl = tk.Label(root, text="Name")
pathTxt = tk.Text(root, height=1, width=40)

styleFrame = tk.LabelFrame(root, text="Styles")
styleScroll = tk.Scrollbar(styleFrame)
styleScroll.pack(side=tk.RIGHT, fill=tk.Y)
styleTree = ttk.Treeview(styleFrame, yscrollcommand=styleScroll.set)
styleTree['columns'] = ("Colour", "Width", "Fill")
styleTree.column("#0", width=0, stretch=tk.NO)
styleTree.column("Colour", width=120)
styleTree.column("Width", width=120)
styleTree.column("Fill", width=80)
styleTree.heading("Colour", text="Colour")
styleTree.heading("Width", text="Width")
styleTree.heading("Fill", text="Fill")
styleTree.pack(fill=tk.BOTH, expand=True)
styleScroll.config(command=styleTree.yview)
styleEdit = tk.Button(root, text="Edit", command=editStyle)
widthLbl = tk.Label(root, text="Width")
widthTxt = tk.Text(root, height=1, width=40)
colourTxt = tk.Text(root, height=1, width=40)
colourLbl = tk.Label(root, text="Colour")


logFrame = tk.LabelFrame(root, text="Log")
logPanel = tk.Listbox(logFrame)
logPanel.pack(fill=tk.BOTH, expand=True)
logClear = tk.Button(root, text="Clear", command=clearLog)

menuBar = tk.Menu(root)
fileMenu = tk.Menu(menuBar, tearoff=0)
fileMenu.add_command(label="Open", command=openKML)
fileMenu.add_command(label="Save", command=saveKML)
fileMenu.add_command(label="Save As...", command=saveKMLas)
fileMenu.add_command(label="About", command=showAbout)
fileMenu.add_separator()
fileMenu.add_command(label="Exit", command=root.quit)
menuBar.add_cascade(label="File", menu=fileMenu)
root.config(menu=menuBar)

root.bind('<Control-x>', quitApp)
root.bind('<Control-o>', openKML)
root.bind('<Control-s>', saveKML)
root.bind('<Control-S>', saveKMLas)
pointTree.bind("<<TreeviewSelect>>", getSelectedPoint)
pathTree.bind("<<TreeviewSelect>>", getSelectedPath)
styleTree.bind("<<TreeviewSelect>>", getSelectedStyle)

pointLbl.grid(row=1, column=0, sticky="w")
pointTxt.grid(row=1, column=0)
pointEdit.grid(row=1, column=0, sticky="e")
pathLbl.grid(row=1, column=1, sticky="w")
pathTxt.grid(row=1, column=1)
pathEdit.grid(row=1, column=1, sticky="e")
pointFrame.grid(row=0, column=0, sticky="nsew")
pathFrame.grid(row=0, column=1, sticky="nsew")
styleFrame.grid(row=2, column=0, sticky="nsew")
styleEdit.grid(row=3, column=0, sticky="n")
widthLbl.grid(row=5, column=0, sticky="w")
widthTxt.grid(row=5, column=0, stick="e")
colourLbl.grid(row=4, column=0, sticky="w")
colourTxt.grid(row=4, column=0, sticky="e")
logFrame.grid(row=2, column=1, sticky="nsew")
logClear.grid(row=3, column=1, sticky="n")

root.mainloop()
deleteKML(kmlPtr)