/**
 * @file KMLParser.h
 * @author CIS*2750 F22
 * @date December 2022
 * @brief File containing the helper function definitions for the KML parser course assignment.
 */

#ifndef KML_HELPERS_H
#define KML_HELPERS_H
#define LIBXML_SCHEMAS_ENABLED
#include "KMLParser.h"
#include <ctype.h>

void treeStepper(xmlNode * node);

/**
 * Print node names.
*/
void print_elem_names(xmlNode * node);

/**
 * Return 1 if a Point node is found, 0 if a LineString node is found,
 * and -1 if neither are found or an error occurs.
*/
int getPlacemarkType(xmlNode * node);

/**
 * Initialize a Point Placemark struct.
*/
PointPlacemark * initPointPlacemark(xmlNode * node, KML * kml);

/**
 * Initialize a Point struct.
*/
Point * initPoint(xmlNode * node);

/**
 * Initialize a Coordinate struct.
*/
Coordinate * initCoordinate(xmlNode * node);

/**
 * Takes a namespace node, populates a namespace struct,
 * and returns it.
*/
XMLNamespace * initNameSpace(xmlNs * ns);

PathPlacemark * initPathPlacemark(xmlNode * node, KML * kml);
Line * initPath(xmlNode * node);
char * lineToString(void * data);
void deleteLine(void * data);

// From https://www.delftstack.com/howto/c/trim-string-in-c/
char * trimString(char *str);

/// @brief Takes a KML node, parses it, and populates a
/// Style struct with its data.
/// @param node A Style XML node.
/// @return A Style struct.
Style * initStyle(xmlNode * node);

StyleMap * initStyleMap(xmlNode * node);

/// @brief Takes a KML element node, parses it, populates a KMLElement
/// struct with its data, and returns it.
/// @param node A KMLElement XML node
/// @return A populated KMLElement struct
KMLElement * initKMLElement(xmlNode * node);

int validateTree(xmlDoc * doc, const char * schemaFile);
xmlDoc * convertToTree(const KML * kml);
bool convertStyleMaps(xmlNode * node, const KML * kml);
bool convertStyles(xmlNode * node, const KML * kml);
bool convertPointPlacemarks(xmlNode * node, const KML * kml);
bool convertPathPlacemarks(xmlNode * node, const KML * kml);

// From https://rosettacode.org/wiki/Haversine_formula#C
double dist(double th1, double ph1, double th2, double ph2);

int updatePoint(char * newName, int i, KML * kml);
int updatePath(char * newName, int i, KML * kml);
int updateStyle(char * newColour, int newWidth, int i, KML * kml);

#endif
