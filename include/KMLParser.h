/**
 * @file KMLParser.h
 * @author CIS*2750 F22
 * @date December 2022
 * @brief File containing the mandatory function definitions for the KML parser course assignment.
 */

#ifndef KML_PARSER_H
#define KML_PARSER_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlschemastypes.h>
#include "LinkedListAPI.h"

//M_PI is not declared in the C standard, so we declare it manually
//We will need it for some of the functions
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

//Represents a generic XML namespace and we will read from / write to an xmlNS struct
typedef struct  {
    //Namespace prefix.  May be NULL.
	char 	*prefix;

    //Namespace value.  Must not be NULL or empty.
	char	*value; 
} XMLNamespace;

//Represents a generic KML element/XML node - i.e. some sort of an additinal piece of data, 
// e.g. desciption, visibility, etc..
typedef struct  {
    //KMLElement name.  Must not be NULL or empty.
	char 	*name;

    //KMLElement value.  Must not be NULL or empty.
	char	*value; 
} KMLElement;

//Represents a simplified KML Style element, as typically produced by Google Earth
typedef struct {
    //Style id. Must not be NULL or an empty string
    char *id;

    //LineStyle colour. Must not be NULL or an empty string
    char *colour;

    //LineStyle width;
    int width;

    //PolyStyle fill;
    int fill;
} Style;

//Represents a simplified KML StyleMap element, as typically produced by Google Earth
typedef struct {

    //StyleMap id. Must not be NULL or an empty string.
    char *id;

    //We assume that there are exactly two Pair elements in the StyleMap

    //Key of the first Pair. May be NULL.
    char *key1;

    //URL of the first Pair. May be NULL.
    char *url1;

    //Key of the second Pair. May be NULL.
    char *key2;

    //URL of the second Pair. May be NULL.
    char *url2;

} StyleMap;

//Represents a single coordinate - i.e. a single entry in list stored in the <coordinates> element of a Point or LineString element 
typedef struct {
    //coordinate longitude.  Must be initialized.
    double      longitude;

    //coordinate latitude.  Must be initialized.
    double      latitude;

    //Optional coordinate altitude. When using the altitude, we will use DBL_MAX value to indicate that it is not provided
    //So if it is missing in the original file, we set the struct attribute altitude to DBL_MAX
    double      altitude;
} Coordinate;


//Represents a simplified KML Point element.
typedef struct {
    //Coordinate of the KMLPoint. Must not be NULL.
    Coordinate  *coordinate;

    /* Additional point data - i.e. XML elements that are children of the KML point other than <coordinates> (extrude, altitudeMode).  
    We will assume that all children of Point have no children of their own. 
    We will also ignore all children with a namespace, e.g. gx:altitudeMode
    Keep in mind that <coordinates> already has its own dedicated field in the Point sruct - so do not place the coordinates in this list

    All objects in the list will be of type KMLElement.  It must not be NULL.  It may be empty.
    */
    List        *otherElements;
} Point;


//Represents a simplified KML LineString element.
typedef struct {
    
    //List of coordinates that make up the path. All objects in the list will be of type Coordinate. 
    //Must not me NULL. Must have the minimum length of 2.
    List        *coordinates;

    /* Additional point data - i.e. XML elements that are children of the KML LineString other than <coordinates> (extrude, altitudeMode, tesselate).  
    We will assume that all children of LineString have no children of their own. 
    We will also ignore all children with a namespace, e.g. gx:altitudeOffset
    Keep in mind that <coordinates> already has its own dedicated field in the Point sruct - so do not place the coordinates in this list

    All objects in the list will be of type KMLElement.  It must not be NULL.  It may be empty.
    */
    List        *otherElements;
} Line;


//For placemarks, all Geometry elements other that LineString and Point are ignored

/*
Represents a "single location" Placemark on a map - i.e. a KML Placemark element that has only a Point in it, 
but no other Geometry elements (e.g. no LineString - i.e. "paths", no LookAt, etc.) 
While the KML specification allows any Geometry element in a Placemark, the only Geometry elements we will consider for a PointPlacemark struct 
will be the KML Point element

If a KML Placemark contains a LineString element (another Geometry element), it is considered to represent a path on a map, rather than a single location on a map - 
and we will parse it into a dedicated PathPlacemark struct
*/
typedef struct {
    //Placemark name - i.e. value of the KML element <name>.  May be NULL.  May be an empty string.
    char        *name;
 
    //Point corresponding to the Placemark. Must not be NULL.
    Point       *point;

    /* Additional Placemark data - i.e. XML elements that are children of the KML point other than <coordinates> (visibility, description, etc.).
    We will assume that all children of Placemark have no children of their own. 
    We will also ignore all children with a namespace, e.g. atom:author, etc.

    Since the KML element <name> has its own field in the PointPlacemark struct, it must not be placed into this list
    
    All objects in the list will be of type KMLElement.  It must not be NULL.  It may be empty.
    */
    List        *otherElements;
} PointPlacemark;


/*
Represents a "path" Placemark on a map - i.e. a KML PLacemark element that has only a LineString in it, 
but no other Geometry elements (e.g. no Point, no LookAt, etc.) 
While the KML specification allows any Geometry element in a Placemark, the only Geometry elements we will consider for a PathPlacemark struct 
will be the KML LineString element

If a KML Placemark contains a Point element (another Geometry element), it is considered to represent a single location on a map, rather than a path - 
and we will parse it into a dedicated PointPlacemark struct
*/
typedef struct {
    //Placemark name - i.e. value of the KML element <name>.  May be NULL.
    char        *name;
 
    //A representation of the LineString object. Must not be NULL.
    Line        *pathData;

    /* Additional Placemark data - i.e. XML elements that are children of the KML point other than <coordinates> (visibility, description, etc.).
    We will assume that all children of Placemark have no children of their own. 
    We will also ignore all children with a namespace, e.g. atom:author, etc.

    Since the KML element <name> has its own field in the PathPlacemark struct, it must not be placed into this list
    
    All objects in the list will be of type KMLElement.  It must not be NULL.  It may be empty.
    */
    List        *otherElements;
} PathPlacemark;


typedef struct {
    
    //Namespaces associated with our KML doc.  Must not be NULL or empty. Since a KML KML doc might have
    //multiple namespaces associated with it, we place them in a list
    List        *namespaces;
    
    //Placemarks representing paths in a KML file. All objects in the list will be of type PointPlacemark.  It must not be NULL.  It may be empty.
    List        *pointPlacemarks;

    //Placemarks representing paths in a KML file. All objects in the list will be of type PathPlacemark.  It must not be NULL.  It may be empty.
    List        *pathPlacemarks;

    //Styles in a KML file. All objects in the list will be of type Style.  It must not be NULL.  It may be empty.
    List        *styles;

    //Style Maps in a KML file. All objects in the list will be of type StyleMap.  It must not be NULL.  It may be empty.
    List        *styleMaps;
    
} KML;

/* Public API - read/write/delete/toString */

/** Function to create an KML object based on the contents of an KML file.
 *@pre File name cannot be an empty string or NULL.
       File represented by this name must exist and must be readable.
 *@post KML has not been modified in any way
        Also, either:
        A valid KML struct has been created and its address was returned
		or 
		An error occurred, and NULL was returned
 *@return the pinter to the new struct or NULL
 *@param fileName - a string containing the name of the KML file
**/
KML* createKML( const char *fileName);

/** Function to create a string representation of an KML object.
 *@pre KML object exists, is not NULL, and is valid
 *@post KML has not been modified in any way, and a string representing the KML contents has been created
 *@return a string contaning a humanly readable representation of an KML object
 *@param obj - a pointer to an KML struct
**/
char* KMLToString(const KML *doc);


/** Function to delete doc content and free all the memory.
 *@pre KML object exists, is not NULL, and has not been freed
 *@post KML object had been freed
 *@return none
 *@param obj - a pointer to an KML struct
**/
void deleteKML(KML* doc);


/* Public API - utility getter functions */

/** For the "get..." functions below, return the count of specified entities from the file.  
They all share the same format and only differ in what they have to count.
 
 *@pre KML object exists, is not NULL, and has not been freed
 *@post KML object has not been modified in any way
 *@return the number of entities in the KMLdoc object
 *@param obj - a pointer to an KML struct
 */

//Total number of "point" Placemarks in the KML file - i.e. the total number of PointPlacemarks
int getNumPoints(const KML* doc);

//Total number of "path" Placemarks in the KML file - i.e. the total number of PathPlacemarks
int getNumPaths(const KML* doc);

//Total number of KMLElements in the document
int getNumKMLElements(const KML* doc);

//Total number of XMLNamespaces in the document
int getNumXMLNamespaces(const KML* doc);

//Total number of Styles in the document
int getNumStyles(const KML* doc);

//Total number of StyleMaps in the document
int getNumStyleMaps(const KML* doc);



/** For the two "get..." functions below, return the pointer to the first struct that matches the name.  
They all share the same format and only differ in what they have to return.
 
 *@pre 
    - KML object exists, is not NULL, and has not been freed
    - name exists, is not NULL, and has not been freed
 *@post 
    - KML object has not been modified in any way
    - name has not been modified in any way
 *@return returns a struct representing the specific KML element with the given name.  
    - a pointer to a matching struct if the element was found
    - NULL otherwise
 *@param 
    - doc - a pointer to an KML struct
    - name - the name of thr element we are looking for
 */

// Function that returns a PointPlacemark with the given name.  If more than one exists, return the first one.  
// Return NULL if the PointPlacemark does not exist
PointPlacemark* getPointPlacemark(const KML* doc, char* name);

// Function that returns a PathPlacemark with the given name.  If more than one exists, return the first one. 
// Return NULL if the PathPlacemark does not exist 
PathPlacemark* getPathPlacemark(const KML* doc, char* name);

/** Function that returns a StyelMap associated with the provided PathPlacemark.  If more than one exists, return the first one.  
 *@pre 
    - KML object exists, is not NULL, and has not been freed
    - ppm exists, is not NULL, and has not been freed
 *@post 
    - KML object has not been modified in any way
    - ppm has not been modified in any way
 *@return returns a struct representing the StyleMap from the PathPlacemark.  
    - a pointer to a matching struct if the element was found
    - NULL otherwise
 *@param 
    - doc - a pointer to an KML struct
    - ppm - the PathPlacemark whose StyleMap we want to get
*/
StyleMap* getMapFromPath(const KML *doc, const PathPlacemark *ppm);

/** Function that returns a Styel at the specified index in the specified StyleMap.
 *@pre 
    - KML object exists, is not NULL, and has not been freed
    - map exists, is not NULL, and has not been freed
    - index is a non-negative number
 *@post 
    - KML object has not been modified in any way
    - map has not been modified in any way
 *@return returns a struct representing the Style from the StyleMap.  
    - a pointer to a matching struct if the element was found
    - NULL otherwise
 *@param 
    - doc - a pointer to an KML struct
    - map - the StyleMap referencing the style we want
    - index - the index of the style we want, since the StyleMap may have more than on Style 
    
*/
Style* getStyleFromMap(const KML *doc, const StyleMap *map, int index);


/* ******************************* A2 functions *************************** */
/** Function to create a KML struct based on the contents of an KML file.
 * This function must validate the XML tree generated by libxml against a KML schema file
 * before attempting to traverse the tree and create a KML struct
 *@pre File name cannot be an empty string or NULL.
       File represented by this name must exist and must be readable.
       Schema file name is not NULL/empty, and represents a valid schema file
 *@post Either:
        A valid KML struct has been created and its address was returned
		or 
		An error occurred, or KML file was invalid, and NULL was returned
 *@return the pinter to the new struct or NULL
 *@param fileName - a string containing the name of the KML file
**/
KML* createValidKML(const char *fileName, const char* schemaFile);


/** Function to validating an existing a KML struct against a KML schema file
 *@pre 
    KML struct exists and is not NULL
    schema file name is not NULL/empty, and represents a valid schema file
 *@post KML struct has not been modified in any way
 *@return the boolean and indicating whether the KML struct is valid
 *@param obj - a pointer to a KML struct
 *@param obj - the name iof a schema file
 **/
bool validateKML(const KML *doc, const char* schemaFile);


/** Function to writing a KML struct into a file in KML format.
 *@pre
    KML object exists, is valid, and and is not NULL.
    fileName is not NULL, has the correct extension
 *@post 
    - KML has not been modified in any way
    - file name has not been modified in any way
    - a file representing the KML contents in KML format has been created
 *@return a boolean value indicating success or failure of the write
 *@param
    - doc - a pointer to a KML struct
 	- fileName - the name of the output file
 **/
bool writeKML(const KML* doc, const char* fileName);


/** Function that returns the length of the path in a PathPlacemark
 *@pre PathPlacemark object exists, is not null, and has not been freed
 *@post PathPlacemark object had been modified in any way
 *@return length of the path in meters
 *@param ppm - a pointer to a PathPlacemark struct
**/
double getPathLen(const PathPlacemark *ppm);


/** Function that checks if the current path is a loop
 *@pre PathPlacemark object exists, is not null
 *@post PathPlacemark object exists, is not null, has not been modified
 *@return true if the path is a loop, false otherwise
 *@param ppm - a pointer to a PathPlacemark struct
 *@param delta - the tolerance used for comparing distances between start and end points
**/
bool isLoopPath(const PathPlacemark* ppm, double delta);


/** Function that returns the the list of paths with the specified length, using the provided tolerance 
 * to compare path lengths
 *@pre KML struct exists, is not null
 *@post KML struct exists, is not null, has not been modified
 *@return the list of PathPlacemark with the specified length
 *@param doc - a pointer to a KML struct
 *@param len - search track length
 *@param delta - the tolerance used for comparing track lengths
**/
List* getPathsWithLength(const KML *doc, double len, double delta);

void deleteKMLElement( void* data);
char* KMLElementToString( void* data);
int compareKMLElements(const void *first, const void *second);

void deleteXMLNamespace( void* data);
char* XMLNamespaceToString( void* data);
int compareXMLNamespace(const void *first, const void *second);

void deleteStyle( void* data);
char* styleToString( void* data);
int compareStyles(const void *first, const void *second);

void deleteStyleMap( void* data);
char* styleMapToString( void* data);
int compareStyleMaps(const void *first, const void *second);

void deleteCoordinate(void* data);
char* coordinateToString( void* data);
int compareCoordinates(const void *first, const void *second);

void deletePoint(void* data);
char* pointToString(void* data);
int comparePoints(const void *first, const void *second);

void deletePointPlacemark(void* data);
char* pointPlacemarkToString(void* data);
int comparePointPlacemarks(const void *first, const void *second);

void deletePathPlacemark(void* data);
char* pathPlacemarkToString(void* data);
int comparePathPlacemarks(const void *first, const void *second);


#endif