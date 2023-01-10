<h1 align="center">KMLParser</h1>

KMLParser is a lightweight parser and editor for KML files, built as part of coursework for a sophomore-level software engineering course. It is built with a combination of C (back-end) and Python (front-end). The libxml2 library is used to parse XML files in C, and the Tk toolkit for Python was used to build the user interface. 

Editable KML fields: Names of points; Colours and widths of styles.

### KML Structure Contraints
- (1) <KML> element.
- Maximum (1) <Document> element.
- (0) or more <Placemark>, <Style>, and <StyleMap> elements.
Please see the provided test files for compatible KML files. 

<br>Compilation: Enter `make parser` at root level to compile source files.
<br>To run: Enter `python3 KMLParser.py` in bin after compilation. Ensure that any KML files intended for use are also in bin.

### Keyboard Shortcuts
- Exit: CTRL-X
- Open: CTRL-O
- Save: CTRL-S
- Save-as: CTRL-SHFT-S
