#include "KMLHelpers.h"
#include "KMLParser.h"

int validateTree(xmlDoc * doc, const char * schemaFile) {
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr c1;

    xmlLineNumbersDefault(1);
    c1 = xmlSchemaNewParserCtxt(schemaFile);
    xmlSchemaSetParserErrors(c1, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema = xmlSchemaParse(c1);
    xmlSchemaFreeParserCtxt(c1);

    xmlSchemaValidCtxtPtr c2 = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(c2, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    int ret = xmlSchemaValidateDoc(c2, doc);

    xmlSchemaFreeValidCtxt(c2);
    if(schema != NULL) {
        xmlSchemaFree(schema);
    }
    xmlSchemaCleanupTypes();

    return ret;
}

xmlDoc * convertToTree(const KML * kml) {
    xmlDocPtr tree = NULL;
    xmlNodePtr kml_node = NULL;
    
    // Create xml tree and kml root node.
    tree = xmlNewDoc(BAD_CAST "1.0");
    kml_node = xmlNewNode(NULL, BAD_CAST "kml");
    xmlDocSetRootElement(tree, kml_node);

    // Validity checks.
    if (kml->namespaces == NULL) {
        return NULL;
    }
    if (kml->pointPlacemarks == NULL) {
        return NULL;
    }
    if (kml->pathPlacemarks == NULL) {
        return NULL;
    }
    if (kml->styles == NULL) {
        return NULL;
    }
    if (kml->styleMaps == NULL) {
        return NULL;
    }

    if (getLength(kml->namespaces) > 0) {
        // Iterate through namespace list in struct and set namespace for kml node.
        void * elem;
        ListIterator iter = createIterator(kml->namespaces);
        int count = 0;
        while ((elem = nextElement(&iter)) != NULL) {
            XMLNamespace * ns = (XMLNamespace *) elem;

            // Validity checks.
            if (ns->value == NULL) {
                return NULL;
            }
            if (strcmp(ns->value, "") == 0) {
                return NULL;
            }

            xmlNs * ns_node = xmlNewNs(kml_node, (xmlChar *) ns->value, (xmlChar *) ns->prefix);
            if (count == 0) {
                xmlSetNs(kml_node, ns_node);
            }
            count++;
        }
    }  

    // Determine if a Document element is needed.
    int document_required = 0;
    if (getLength(kml->pathPlacemarks) > 1) {
        document_required = 1;
    } else if (getLength(kml->pointPlacemarks) > 1) {
        document_required = 1;
    } else if (getLength(kml->styleMaps) > 0) {
        document_required = 1;
    }

    // Create document node. Initialize if needed.
    xmlNode * document_node = NULL;;
    if (document_required == 1) {
        document_node = xmlNewChild(kml_node, NULL, BAD_CAST "Document", NULL);
    }    

    if (getLength(kml->styleMaps) > 0) {
        if (document_required == 1) {
            bool status = convertStyleMaps(document_node, kml);
            if (status == false) {
                return NULL;
            }
        } else {
            bool status = convertStyleMaps(kml_node, kml);
            if (status == false) {
                return NULL;
            }
        }
    }

    if (getLength(kml->styles) > 0) {
        if (document_required == 1) {
            bool status = convertStyles(document_node, kml);
            if (status == false) {
                return NULL;
            }
        } else {
            bool status = convertStyles(kml_node, kml);
            if (status == false) {
                return NULL;
            }
        }

    }

    if (getLength(kml->pointPlacemarks) > 0) {
        if (document_required == 1) {
            bool status = convertPointPlacemarks(document_node, kml);
            if (status == false) {
                return NULL;
            }
        } else {
            bool status = convertPointPlacemarks(kml_node, kml);
            if (status == false) {
                return NULL;
            }
        }
      
    }

    if (getLength(kml->pathPlacemarks) > 0) {
        if (document_required == 1) {
            bool status = convertPathPlacemarks(document_node, kml);
            if (status == false) {
                return NULL;
            }
        } else {
            bool status = convertPathPlacemarks(kml_node, kml);
            if (status == false) {
                return NULL;
            }
        }        
    }

    return tree;
}

bool convertStyleMaps(xmlNode * node, const KML * kml) {
    void * elem;
    ListIterator iter = createIterator(kml->styleMaps);
    while ((elem = nextElement(&iter)) != NULL) {
        StyleMap * sm = (StyleMap *) elem;

        if (sm->id == NULL) {
            return false;
        }
        if (strcmp(sm->id, "") == 0) {
            return false;
        }

        xmlNode * sm_node = xmlNewChild(node, NULL, BAD_CAST "StyleMap", NULL);
        xmlNewProp(sm_node, (xmlChar *) "id", (xmlChar *) sm->id);

        for (int i = 0; i < 2; i++) {
            xmlNode * p = xmlNewChild(sm_node, NULL, (xmlChar *) "Pair", NULL);

            if (i == 0) {
                xmlNewChild(p, NULL, (xmlChar *) "key", (xmlChar *) sm->key1);
                xmlNewChild(p, NULL, (xmlChar *) "styleUrl", (xmlChar *) sm->url1);
            } else {
                xmlNewChild(p, NULL, (xmlChar *) "key", (xmlChar *) sm->key2);
                xmlNewChild(p, NULL, (xmlChar *) "styleUrl", (xmlChar *) sm->url2);
            } 
        }
    }

    return true;
}

bool convertStyles(xmlNode * node, const KML * kml) {
    void * elem;
    ListIterator iter = createIterator(kml->styles);
    while ((elem = nextElement(&iter)) != NULL) {
        Style * s = (Style *) elem;

        // Validity checks.
        if (s->id == NULL) {
            return false;
        }
        if (strcmp(s->id, "") == 0) {
            return false;
        }
        if (s->colour == NULL) {
            return false;
        }
        if (strcmp(s->colour, "") == 0) {
            return false;
        }

        xmlNode * s_node = xmlNewChild(node, NULL, (xmlChar *) "Style", NULL);
        xmlNewProp(s_node, (xmlChar *) "id", (xmlChar *) s->id);

        xmlNode * lnstyle_node = xmlNewChild(s_node, NULL, (xmlChar *) "LineStyle", NULL);
        
        xmlNewChild(lnstyle_node, NULL, (xmlChar *) "color", (xmlChar *) s->colour);

        if (s->width != -1) {
            char * width = malloc(64);
            sprintf(width, "%d", s->width);
            xmlNewChild(lnstyle_node, NULL, (xmlChar *) "width", (xmlChar *) width);

            free(width);
        }

        if (s->fill != -1) {
            xmlNode * plystyle_node = xmlNewChild(s_node, NULL, (xmlChar *) "PolyStyle", NULL);
            
            char * fill = malloc(64);
            sprintf(fill, "%d", s->fill);
            xmlNewChild(plystyle_node, NULL, (xmlChar *) "fill", (xmlChar *) fill);

            free(fill);
        }
    }

    return true;
}

bool convertPointPlacemarks(xmlNode * node, const KML * kml) {
    void * elem;
    ListIterator iter = createIterator(kml->pointPlacemarks);
    while ((elem = nextElement(&iter)) != NULL) {
        PointPlacemark * pointPlacemark = (PointPlacemark *) elem;

        // Validity checks.
        if (pointPlacemark->point == NULL) {
            return false;
        }
        if (pointPlacemark->otherElements == NULL) {
            return false;    
        }

        // Initialize Placemark node.
        xmlNode * placemark_node = xmlNewChild(node, NULL, (xmlChar *) "Placemark", NULL);

        // Initialize name node.
        if (pointPlacemark->name != NULL) {
            xmlNewChild(placemark_node, NULL, (xmlChar *) "name", (xmlChar *) pointPlacemark->name);
        }

        if (getLength(pointPlacemark->otherElements) > 0) {
            void * elem2;
            ListIterator iter2 = createIterator(pointPlacemark->otherElements);
            while ((elem2 = nextElement(&iter2)) != NULL) {
                KMLElement * k = (KMLElement *) elem2;

                // Validity checks.
                if (k->name == NULL) {
                    return false;
                }
                if (strcmp(k->name, "") == 0) {
                    return false;
                }
                if (k->value == NULL) {
                    return false;
                }
                if (strcmp(k->value, "") == 0) {
                    return false;
                }

                xmlNewChild(placemark_node, NULL, (xmlChar *) k->name, (xmlChar *) k->value);
            }
        }

        xmlNode * point_node = xmlNewChild(placemark_node, NULL, (xmlChar *) "Point", NULL);
        if (getLength(pointPlacemark->point->otherElements) > 0) {
            void * elem2;
            ListIterator iter2 = createIterator(pointPlacemark->point->otherElements);
            while ((elem2 = nextElement(&iter2)) != NULL) {
                KMLElement * k = (KMLElement *) elem2;

                // Validity checks.
                if (k->name == NULL) {
                    return false;
                }
                if (strcmp(k->name, "") == 0) {
                    return false;
                }
                if (k->value == NULL) {
                    return false;
                }
                if (strcmp(k->value, "") == 0) {
                    return false;
                }

                xmlNewChild(point_node, NULL, (xmlChar *) k->name, (xmlChar *) k->value);
            }
        }

        // Validity checks.
        if (pointPlacemark->point->coordinate->latitude == -1) {
            return false;
        }
        if (pointPlacemark->point->coordinate->longitude == -1) {
            return false;
        }

        char * coordinates = malloc(1024);
        if (pointPlacemark->point->coordinate->altitude == DBL_MAX) {
            sprintf(coordinates, "%f,%f", pointPlacemark->point->coordinate->longitude, pointPlacemark->point->coordinate->latitude);
        } else {
            sprintf(coordinates, "%f,%f,%f", pointPlacemark->point->coordinate->longitude, pointPlacemark->point->coordinate->latitude, pointPlacemark->point->coordinate->altitude);
        }

        xmlNewChild(point_node, NULL, (xmlChar *) "coordinates", (xmlChar *) coordinates);
    }

    return true;     
}

bool convertPathPlacemarks(xmlNode * node, const KML * kml) {
    void * elem;
    ListIterator iter = createIterator(kml->pathPlacemarks);
    while ((elem = nextElement(&iter)) != NULL) {
        PathPlacemark * pathPlacemark = (PathPlacemark *) elem;

        if (pathPlacemark->pathData == NULL) {
            return false;
        }
        if (pathPlacemark->otherElements == NULL) {
            return false;
        }
        if (pathPlacemark->pathData->coordinates == NULL) {
            return false;
        }
        if (getLength(pathPlacemark->pathData->coordinates) < 2) {
            return false;
        }
        if (pathPlacemark->pathData->otherElements == NULL) {
            return false;
        }

        xmlNode * placemark_node = xmlNewChild(node, NULL, (xmlChar *) "Placemark", NULL);

        if (pathPlacemark->name != NULL) {
            xmlNewChild(placemark_node, NULL, (xmlChar *) "name", (xmlChar *) pathPlacemark->name);
        }

        if (getLength(pathPlacemark->otherElements) > 0) {
            void * elem2;
            ListIterator iter2 = createIterator(pathPlacemark->otherElements);
            while ((elem2 = nextElement(&iter2)) != NULL) {
                KMLElement * k = (KMLElement *) elem2;

                // Validity checks.
                if (k->name == NULL) {
                    return false;
                }
                if (strcmp(k->name, "") == 0) {
                    return false;
                }
                if (k->value == NULL) {
                    return false;
                }
                if (strcmp(k->value, "") == 0) {
                    return false;
                }

                xmlNewChild(placemark_node, NULL, (xmlChar *) k->name, (xmlChar *) k->value);
            }
        }

        xmlNode * path_node = xmlNewChild(placemark_node, NULL, (xmlChar *) "LineString", NULL);
        if (getLength(pathPlacemark->pathData->otherElements) > 0) {
            void * elem2;
            ListIterator iter2 = createIterator(pathPlacemark->pathData->otherElements);
            while ((elem2 = nextElement(&iter2)) != NULL) {
                KMLElement * k = (KMLElement *) elem2;

                // Validity checks.
                if (k->name == NULL) {
                    return false;
                }
                if (strcmp(k->name, "") == 0) {
                    return false;
                }
                if (k->value == NULL) {
                    return false;
                }
                if (strcmp(k->value, "") == 0) {
                    return false;
                }

                xmlNewChild(path_node, NULL, (xmlChar *) k->name, (xmlChar *) k->value);
            }
        }

        char * coordinates = malloc(1024);
        strcpy(coordinates, "");
        void * elem2;
        ListIterator iter2 = createIterator(pathPlacemark->pathData->coordinates);
        while ((elem2 = nextElement(&iter2)) != NULL) {
            Coordinate * c = (Coordinate *) elem2;

            // Validity checks.
            if (c->longitude == -1) {
                return false;
            }
            if (c->latitude == -1) {
                return false;
            }

            char * coordinate = malloc(1024);
            if (c->altitude == DBL_MAX) {
                sprintf(coordinate, "%f,%f", c->longitude, c->latitude);
            } else {
                sprintf(coordinate, "%f,%f,%f", c->longitude, c->latitude, c->altitude);
            }

            strcat(coordinates, coordinate);
            strcat(coordinates, " ");
            coordinates = realloc(coordinates, 1024);
            free(coordinate);
        }

        xmlNewChild(path_node, NULL, (xmlChar *) "coordinates", (xmlChar *) coordinates);
        free(coordinates);
    }

    return true;
}

int getPlacemarkType(xmlNode * node) {
    // Null argument check.
    if (node == NULL) {
        return -1;
    }

    xmlNode * curr_node = NULL;
    for (curr_node = node; curr_node != NULL; curr_node = curr_node->next) {
        if (curr_node->type == XML_ELEMENT_NODE) {
            if (strcmp((char *) curr_node->name, "Point") == 0) {
                return 1;
            } else if (strcmp((char *) curr_node->name, "LineString") == 0){
                return 0;
            }
        }
    }

    return -1;
}

PointPlacemark * initPointPlacemark(xmlNode * node, KML * kml) {
    PointPlacemark * pl = malloc(sizeof(PointPlacemark));
    pl->name = NULL;
    pl->otherElements = initializeList(&KMLElementToString, &deleteKMLElement, &compareKMLElements);

    xmlNode * curr_node = NULL;
    for (curr_node = node; curr_node != NULL; curr_node = curr_node->next) {
        if (curr_node->type == XML_ELEMENT_NODE) {
            // Branch for name node..
            if (strcmp((char *) curr_node->name, "name") == 0) {
                char * c = (char *) xmlNodeGetContent(curr_node);
                pl->name = malloc(strlen(c) + 1);
                strcpy(pl->name, c);
                free(c);
            } else if (strcmp((char *) curr_node->name, "Point") == 0) {
                Point * point = initPoint(curr_node->children);
                pl->point = point;
            } else {
                KMLElement * k = initKMLElement(curr_node);
                insertBack(pl->otherElements, k);
            }
        }
    }

    return pl;
}

PathPlacemark * initPathPlacemark(xmlNode * node, KML * kml) {
    PathPlacemark * pa = malloc(sizeof(PathPlacemark));
    pa->name = NULL;
    pa->otherElements = initializeList(&KMLElementToString, &deleteKMLElement, &compareKMLElements);

    xmlNode * curr_node = NULL;
    for (curr_node = node; curr_node != NULL; curr_node = curr_node->next) {
        if (curr_node->type == XML_ELEMENT_NODE) {
            // Branch for name node.
            if (strcmp((char *) curr_node->name, "name") == 0) {
                char * c = (char *) xmlNodeGetContent(curr_node);
                pa->name = malloc(strlen(c) + 1);
                strcpy(pa->name, c);
                free(c);
            } else if (strcmp((char *) curr_node->name, "LineString") == 0) {
                Line * line = initPath(curr_node->children);
                pa->pathData = line;
            } else {
                KMLElement * k = initKMLElement(curr_node);
                insertBack(pa->otherElements, k);
            }
        }
    }

    return pa;
}

Point * initPoint(xmlNode * node) {
    Point * point = malloc(sizeof(Point));
    point->otherElements = initializeList(&KMLElementToString, &deleteKMLElement, &compareKMLElements);

    xmlNode * curr_node = NULL;
    for (curr_node = node; curr_node != NULL; curr_node = curr_node->next) {
        if (curr_node->type == XML_ELEMENT_NODE) {
            if (strcmp((char *) curr_node->name, "coordinates") == 0) {
                Coordinate * coordinate = initCoordinate(curr_node);
                point->coordinate = coordinate;
            } else {
                KMLElement * k = initKMLElement(curr_node);
                insertBack(point->otherElements, k);
            }
        } 
    }

    return point;
}

Line * initPath(xmlNode * node) {
    Line * path = malloc(sizeof(Line));
    path->otherElements = initializeList(&KMLElementToString, &deleteKMLElement, &compareKMLElements);
    path->coordinates = initializeList(&coordinateToString, &deleteCoordinate, &compareCoordinates);

    xmlNode * curr_node = NULL;
    for (curr_node = node; curr_node != NULL; curr_node = curr_node->next) {
        if (curr_node->type == XML_ELEMENT_NODE) {
            if (strcmp((char *) curr_node->name, "coordinates") == 0) {
                char * nodeContent = (char *) xmlNodeGetContent(curr_node);
                char * coordinates = malloc(strlen(nodeContent) + 1);
                char * coordinatesCpy = coordinates;
                strcpy(coordinates, nodeContent);
                coordinates = trimString(coordinates);
                char * token = strtok_r(coordinates, " ", &coordinates);
                do {
                    char * subtoken = strtok_r(token, ",", &token);
                    int pos = 0;
                    Coordinate * c = malloc(sizeof(Coordinate));
                    do {
                        if (pos == 0) {
                            c->longitude = atof(subtoken);
                        } else if (pos == 1) {
                            c->latitude = atof(subtoken);
                        } else if (pos == 2) {
                            c->altitude = atof(subtoken);
                        }
                        subtoken = strtok_r(token, ",", &token);
                        pos++;
                    } while (subtoken != NULL);
                    insertBack(path->coordinates, c);
                    token = strtok_r(coordinates, " ", &coordinates);
                } while (token != NULL);
                free(nodeContent);
                free(coordinatesCpy);
            } else {
                KMLElement * k = initKMLElement(curr_node);
                insertBack(path->otherElements, k);
            }
        } 
    }

    return path;
}

Coordinate * initCoordinate(xmlNode * node) {
    // Initialize Coordinate struct.
    Coordinate * coordinate = malloc(sizeof(Coordinate));
    coordinate->longitude = -1;
    coordinate->latitude = -1;
    coordinate->altitude = DBL_MAX;

    // Get the coordinate string.
    char * c = (char *) xmlNodeGetContent(node);
    char * content = malloc(strlen(c) + 1);
    strcpy(content, c);

    // Parse the coordinate string and assign to struct fields.
    char delim[] = ",";
    char * coordinateStr = strtok(content, delim);  
    int pos = 0;
    while (coordinateStr != NULL) {
        if (pos == 0) {
            coordinate->longitude = atof(coordinateStr);
        } else if (pos == 1) {
            coordinate->latitude = atof(coordinateStr);
        } else if (pos == 2) {
            coordinate->altitude = atof(coordinateStr);
        }
        coordinateStr = strtok(NULL, delim);
        pos++;
    }

    free(c);
    free(content);
    return coordinate;
}

XMLNamespace * initNameSpace(xmlNs * nsNode) {
    XMLNamespace * ns = malloc(sizeof(XMLNamespace));

    char * href = (char *) nsNode->href;
    ns->value = malloc(strlen(href) + 1);
    strcpy(ns->value, href);

    char * prefix;
    if (nsNode->prefix != NULL) {
        prefix = (char *) nsNode->prefix;
        ns->prefix = malloc(strlen(prefix) + 1);
        strcpy(ns->prefix, prefix);
    } else {
        ns->prefix = NULL;
    }
    
    return ns;
}

char * lineToString(void * data) {
    Line * l = (Line *) data;

    char * tmpStr = malloc(100000);
    strcpy(tmpStr, "");

    if (getLength(l->coordinates) > 0) {
        ListIterator iter = createIterator(l->coordinates);
        void * elem;
        while ((elem = nextElement(&iter)) != NULL) {
            char * c = coordinateToString(elem);
            strcat(tmpStr, c);
            strcat(tmpStr, "\n");
            free(c);
	    }
    }
    
    // if (getLength(l->otherElements) > 0) {
    //     strcat(tmpStr, "\n    LineStyle elements\n");
    //     ListIterator iter = createIterator(l->otherElements);
    //     void * elem;
    //     while ((elem = nextElement(&iter)) != NULL) {
    //         char * k = KMLElementToString(elem);
    //         strcat(tmpStr, k);
    //         free(k);
	//     }
    // }

    return tmpStr;
}

void deleteLine(void * data) {
    Line * l = (Line *) data;

    freeList(l->otherElements);
    freeList(l->coordinates);

    free(l);
}

Style * initStyle(xmlNode * node) {
    Style * s = malloc(sizeof(Style));
    s->fill = -1;
    s->width = -1;

    // KML spec states: "Color and opacity (alpha) values are expressed in hexadecimal notation. The range of values for any one color is 0 to 255 (00 to ff)."
    // So colour may be of maximum 8 bytes. Colour is initialized to a default value.
    s->colour = malloc(64);
    strcpy(s->colour, "#ffffffff");

    // Get style id from Style element properties.
    xmlAttr * attr = node->properties;
    char * id = (char *) xmlNodeGetContent(attr->children);
    s->id = malloc(strlen(id) + 1);
    strcpy(s->id, id);
    free(id);

    // Get children of Style element.
    xmlNode * style_node;
    for (style_node = node->children; style_node != NULL; style_node = style_node->next) {
        if (style_node->type == XML_ELEMENT_NODE) {
            // If the child node is a LineStyle element.
            if (((strcmp((char *)style_node->name, "LineStyle") == 0))) {
                xmlNode * linestyle_node;
                for (linestyle_node = style_node->children; linestyle_node != NULL; linestyle_node = linestyle_node->next) {
                    if (linestyle_node->type == XML_ELEMENT_NODE) {
                        if (((strcmp((char *)linestyle_node->name, "width") == 0))) {
                            char * width = (char *) xmlNodeGetContent(linestyle_node);
                            s->width = atoi(width);
                            free(width);
                        } else if (((strcmp((char *)linestyle_node->name, "color") == 0))) {
                            char * colour = (char *) xmlNodeGetContent(linestyle_node);
                            // s->colour = malloc(strlen(colour) + 1);
                            strcpy(s->colour, colour);
                            free(colour);
                        }
                    }
                }
            } 
            if (((strcmp((char *)style_node->name, "PolyStyle") == 0))) {
                xmlNode * polystyle_node;
                for (polystyle_node = style_node->children; polystyle_node != NULL; polystyle_node = polystyle_node->next) {
                    if (polystyle_node->type == XML_ELEMENT_NODE) {
                        if (((strcmp((char *)polystyle_node->name, "fill") == 0))) {
                            char * fill = (char *) xmlNodeGetContent(polystyle_node);
                            s->fill = atoi(fill);
                            free(fill);
                        }
                    }
                }
            }
        }
    }

    return s;
}

StyleMap * initStyleMap(xmlNode * node) {
    StyleMap * sm = malloc(sizeof(StyleMap));

    // Get style id from StyleMap element properties.
    xmlAttr * attr = node->properties;
    char * id = (char *) xmlNodeGetContent(attr->children);
    sm->id = malloc(strlen(id) + 1);
    strcpy(sm->id, id);
    free(id);

    // Get children of StyleMap element.
    xmlNode * mapNode;
    int index = 0;
    for (mapNode = node->children; mapNode != NULL; mapNode = mapNode->next) {
        if (mapNode->type == XML_ELEMENT_NODE) {
            if (((strcmp((char *)mapNode->name, "Pair") == 0))) {
                xmlNode * pairNode;
                for (pairNode = mapNode->children; pairNode != NULL; pairNode = pairNode->next) {
                    if (pairNode->type == XML_ELEMENT_NODE) {
                        if (((strcmp((char *)pairNode->name, "key") == 0))) {
                            char * key = (char *) xmlNodeGetContent(pairNode);
                            if (index == 0) {
                                sm->key1 = malloc(strlen(key) + 1);
                                strcpy(sm->key1, key);
                            } else if (index == 1) {
                                sm->key2 = malloc(strlen(key) + 1);
                                strcpy(sm->key2, key);
                            }
                            free(key);
                        }
                        if (((strcmp((char *)pairNode->name, "styleUrl") == 0))) {
                            char * url = (char *) xmlNodeGetContent(pairNode);
                            if (index == 0) {
                                sm->url1 = malloc(strlen(url) + 1);
                                strcpy(sm->url1, url);
                            } else if (index == 1) {
                                sm->url2 = malloc(strlen(url) + 1);
                                strcpy(sm->url2, url);
                            }
                            free(url);
                        }
                    }
                }
                index++;
            } 
        }
    }
    
    return sm;
}

KMLElement * initKMLElement(xmlNode * node) {
        KMLElement * k = malloc(sizeof(KMLElement));
        k->name = malloc(strlen((char *) node->name) + 1);
        strcpy(k->name, (char *) node->name);
        char * c = (char *) xmlNodeGetContent(node);
        k->value = malloc(strlen(c) + 1);
        strcpy(k->value, c);
        free(c);

        return k;
}

char * trimString(char *str)
{
    char *end;

    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';

    return str;
}

#define R 6371e3
#define TO_RAD (3.1415926536 / 180)
double dist(double th1, double ph1, double th2, double ph2) {
	double dx, dy, dz;
	ph1 -= ph2;
	ph1 *= TO_RAD, th1 *= TO_RAD, th2 *= TO_RAD;

	dz = sin(th1) - sin(th2);
	dx = cos(ph1) * cos(th1) - cos(th2);
	dy = sin(ph1) * cos(th1);
	return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * R;
}

int updatePoint(char * newName, int i, KML * kml) {
    ListIterator iter = createIterator(kml->pointPlacemarks);
    int index = 0;
    void * elem;
    while ((elem = nextElement(&iter)) != NULL) {
        if (index == i) {
            PointPlacemark * p = (PointPlacemark *) elem;
            p->name = malloc(strlen(newName) + 1);
            strcpy(p->name, newName);
            return 1;
        }
        index++;
    }
    return 0;
}

int updatePath(char * newName, int i, KML * kml) {
    ListIterator iter = createIterator(kml->pathPlacemarks);
    int index = 0;
    void * elem;
    while ((elem = nextElement(&iter)) != NULL) {
        if (index == i) {
            PathPlacemark * p = (PathPlacemark *) elem;
            p->name = malloc(strlen(newName) + 1);
            strcpy(p->name, newName);
            return 1;
        }
        index++;
    }
    return 0;
}

int updateStyle(char * newColour, int newWidth, int i, KML * kml) {
    ListIterator iter = createIterator(kml->styles);
    int index = 0;
    void * elem;
    while ((elem = nextElement(&iter)) != NULL) {
        if (index == i) {
            Style * s = (Style *) elem;
            s->colour = malloc(strlen(newColour) + 1);
            strcpy(s->colour, newColour);
            s->width = newWidth;
            return 1;
        }
        index++;
    }
    return 0;
}