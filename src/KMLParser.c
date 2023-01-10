#include "KMLParser.h"
#include "KMLHelpers.h"

KML * createKML(const char * filename) {
    // Null argument check.
    if (filename == NULL) {
        return NULL;
    }

    // Parse KML file into XML tree.
    xmlDoc * doc = NULL;
    xmlNode * root_node = NULL;
    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL) {
        return NULL;
    }
    root_node = xmlDocGetRootElement(doc);

    // Initialize KML struct as well as it's list fields.
    KML * kml = malloc(sizeof(KML));
    kml->namespaces = initializeList(&XMLNamespaceToString, &deleteXMLNamespace, &compareXMLNamespace);
    kml->pointPlacemarks = initializeList(&pointPlacemarkToString, &deletePointPlacemark, &comparePointPlacemarks);
    kml->pathPlacemarks = initializeList(&pathPlacemarkToString, &deletePathPlacemark, &comparePathPlacemarks);
    kml->styles = initializeList(&styleToString, &deleteStyle, &compareStyles);
    kml->styleMaps = initializeList(&styleMapToString, &deleteStyleMap, &compareStyleMaps);

    // Step through the tree iteratively.
    xmlNode * node = NULL;
    for (node = root_node; node != NULL; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            // Check for kml element.
            if ((strcmp((char *)node->name, "kml") == 0)) {
                // Get namespace(s).
                xmlNs * ns_node = NULL;
                for (ns_node = node->ns; ns_node != NULL; ns_node = ns_node->next) {
                    XMLNamespace * ns = initNameSpace(ns_node);
                    insertBack(kml->namespaces, ns);
                }
                // Go lower.
                node = node->children;
            }
            if ((strcmp((char *)node->name, "Document") == 0)) {
                // Go lower.
                node = node->children;
            }
            // Check for Placemark element.
            if ((strcmp((char *)node->name, "Placemark") == 0)) {
                // Point or LineString Placemark?
                int placemark_type = getPlacemarkType(node->children);
                // Branch for Point Placemark.
                if (placemark_type == 1) {
                    PointPlacemark * pop = initPointPlacemark(node->children, kml);
                    insertBack(kml->pointPlacemarks, pop);
                // Branch for Path Placemark.
                } else if (placemark_type == 0) {
                    PathPlacemark * pap = initPathPlacemark(node->children, kml);
                    insertBack(kml->pathPlacemarks, pap);
                }
            } 
            if ((strcmp((char *)node->name, "Style") == 0)) {
                Style * s = initStyle(node);
                insertBack(kml->styles, s);
            }
            if ((strcmp((char *)node->name, "StyleMap") == 0)) {
                StyleMap * sm = initStyleMap(node);
                insertBack(kml->styleMaps, sm);
            }
        }
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return kml;
}

KML* createValidKML(const char * fileName, const char * schemaFile) {
    // Null argument check.
    if (fileName == NULL || schemaFile == NULL) {
        return NULL;
    }

    xmlDoc * doc = NULL;
    xmlNode * root_node = NULL;
    doc = xmlReadFile(fileName, NULL, 0);
    if (doc == NULL) {
        return NULL;
    } 

    int ret = validateTree(doc, schemaFile);
    if (ret != 0) {
        return NULL;
    }

    root_node = xmlDocGetRootElement(doc);

    // Initialize KML struct as well as it's list fields.
    KML * kml = malloc(sizeof(KML));
    kml->namespaces = initializeList(&XMLNamespaceToString, &deleteXMLNamespace, &compareXMLNamespace);
    kml->pointPlacemarks = initializeList(&pointPlacemarkToString, &deletePointPlacemark, &comparePointPlacemarks);
    kml->pathPlacemarks = initializeList(&pathPlacemarkToString, &deletePathPlacemark, &comparePathPlacemarks);
    kml->styles = initializeList(&styleToString, &deleteStyle, &compareStyles);
    kml->styleMaps = initializeList(&styleMapToString, &deleteStyleMap, &compareStyleMaps);

    // Step through the tree iteratively.
    xmlNode * node = NULL;
    for (node = root_node; node != NULL; node = node->next) {
        if (node->type == XML_ELEMENT_NODE) {
            // Check for kml element.
            if ((strcmp((char *)node->name, "kml") == 0)) {
                // Get namespace(s).
                xmlNs * ns_node = NULL;
                for (ns_node = node->ns; ns_node != NULL; ns_node = ns_node->next) {
                    XMLNamespace * ns = initNameSpace(ns_node);
                    insertBack(kml->namespaces, ns);
                }
                // Go lower.
                node = node->children;
            }
            if ((strcmp((char *)node->name, "Document") == 0)) {
                // Go lower.
                node = node->children;
            }
            // Check for Placemark element.
            if ((strcmp((char *)node->name, "Placemark") == 0)) {
                // Point or LineString Placemark?
                int placemark_type = getPlacemarkType(node->children);
                // Branch for Point Placemark.
                if (placemark_type == 1) {
                    PointPlacemark * pop = initPointPlacemark(node->children, kml);
                    insertBack(kml->pointPlacemarks, pop);
                // Branch for Path Placemark.
                } else if (placemark_type == 0) {
                    PathPlacemark * pap = initPathPlacemark(node->children, kml);
                    insertBack(kml->pathPlacemarks, pap);
                }
            } 
            if ((strcmp((char *)node->name, "Style") == 0)) {
                Style * s = initStyle(node);
                insertBack(kml->styles, s);
            }
            if ((strcmp((char *)node->name, "StyleMap") == 0)) {
                StyleMap * sm = initStyleMap(node);
                insertBack(kml->styleMaps, sm);
            }
        }
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return kml;
}

bool writeKML(const KML* doc, const char* fileName) {
    if (doc == NULL || fileName == NULL) {
        return false;
    }

    xmlDoc * tree = convertToTree(doc);
    if (tree == NULL) {
        return false;
    }

    int status = xmlSaveFormatFileEnc(fileName, tree, "UTF-8", 1);
    if (status == -1) {
        return false;
    }

    xmlFreeDoc(tree);
    return true;
}

double getPathLen(const PathPlacemark *ppm) {
    if (ppm == NULL) {
        return 0;
    }

    void * elem;
    ListIterator iter = createIterator(ppm->pathData->coordinates);
    int last = getLength(ppm->pathData->coordinates);
    Coordinate * points[last];
    int idx = 0;
    while ((elem = nextElement(&iter)) != NULL) {
        Coordinate * c = (Coordinate * ) elem;
        points[idx] = c;
        idx++;
    }

    double distance = 0;
    for (int i = 0; i < last - 1; i++) {
        distance += dist(points[i]->latitude, points[i]->longitude, points[i + 1]->latitude, points[i + 1]->longitude);
    }

    return distance;
}

bool isLoopPath(const PathPlacemark* ppm, double delta) {
    if (ppm == NULL || delta < 0) {
        return false;
    }

    if (getLength(ppm->pathData->coordinates) < 4) {
        return false;
    }

    int pos = 0;
    int last = getLength(ppm->pathData->coordinates) - 1;
    void * elem;
    double long1 = 0;
    double lat1 = 0;
    double long2 = 0;
    double lat2 = 0;
    ListIterator iter = createIterator(ppm->pathData->coordinates);
    while ((elem = nextElement(&iter)) != NULL) {
        Coordinate * c = (Coordinate *) elem;
        if (pos == 0) {
            long1 = c->longitude;
            lat1 = c->latitude;
        }
        if (pos == last) {
            long2 = c->longitude;
            lat2 = c->latitude;
        }
    }

    double distance = dist(lat1, long1, lat2, long2);

    if (distance > delta) {
        return false;
    }
    
    return true;
}

List * getPathsWithLength(const KML *doc, double len, double delta) {
    if (doc == NULL || len < 0 || delta < 0) {
        return NULL;
    }

    List * paths = initializeList(&pathPlacemarkToString, &deletePathPlacemark, &comparePathPlacemarks);

    void * elem;
    ListIterator iter = createIterator(doc->pathPlacemarks);
    while ((elem = nextElement(&iter)) != NULL) {
        PathPlacemark * p = (PathPlacemark *) elem;
        
        double distance = getPathLen(p);
        if ((len - distance) <= delta) {
            insertBack(paths, p);
        }

    }

    return paths;
}

bool validateKML(const KML *doc, const char* schemaFile) {
    if (doc == NULL || schemaFile == NULL) {
        return false;
    }

    xmlDoc * tree = convertToTree(doc);
    if (tree == NULL) {
        return false;
    }

    int ret = validateTree(tree, schemaFile);
    if (ret != 0) {
        return false;
    }

    return true;     
}

char * KMLToString(const KML * doc) {
    KML * kml = (KML *) doc;

    char * tmpStr = malloc(1000000);
    strcpy(tmpStr, "    KML\n");

    if (getLength(kml->namespaces) > 0) {
        strcat(tmpStr, "\n");
        ListIterator iter = createIterator(kml->namespaces);
        void * elem;
        while ((elem = nextElement(&iter)) != NULL) {
            char * ns = XMLNamespaceToString(elem);
            strcat(tmpStr, ns);
            free(ns);
	    }
    }

    if (getLength(kml->pointPlacemarks) > 0) {
        strcat(tmpStr, "\n    Point Placemarks\n");
        ListIterator iter = createIterator(kml->pointPlacemarks);
        void * elem;
        while ((elem = nextElement(&iter)) != NULL) {
            char * p = pointPlacemarkToString(elem);
            strcat(tmpStr, p);
            free(p);
	    }
    }

    if (getLength(kml->pathPlacemarks) > 0) {
        strcat(tmpStr, "\n    Path Placemarks\n");
        ListIterator iter = createIterator(kml->pathPlacemarks);
        void * elem;
        while ((elem = nextElement(&iter)) != NULL) {
            char * p = pathPlacemarkToString(elem);
            strcat(tmpStr, p);
            free(p);
	    }
    }

    if (getLength(kml->styles) > 0) {
        strcat(tmpStr, "\n    Styles\n");
        ListIterator iter = createIterator(kml->styles);
        void * elem;
        while ((elem = nextElement(&iter)) != NULL) {
            char * p = styleToString(elem);
            strcat(tmpStr, p);
            free(p);
	    }
    }

    if (getLength(kml->styleMaps) > 0) {
        strcat(tmpStr, "\n    StyleMaps\n");
        ListIterator iter = createIterator(kml->styleMaps);
        void * elem;
        while ((elem = nextElement(&iter)) != NULL) {
            char * p = styleMapToString(elem);
            strcat(tmpStr, p);
            free(p);
	    }
    }

    return tmpStr;
}

void deleteKML(KML * doc) {
    if (doc == NULL) {
        return;
    }

    KML * k = (KML * ) doc;

    freeList(k->namespaces);
    freeList(k->pointPlacemarks);
    freeList(k->pathPlacemarks);
    freeList(k->styles);
    freeList(k->styleMaps);
    free(k);
}

void deletePointPlacemark(void * data) {
    PointPlacemark * pl = (PointPlacemark *) data;

    if (pl->name != NULL) {
        free(pl->name);
    }

    deletePoint(pl->point);
    freeList(pl->otherElements);
    free(pl);
}

char * pointPlacemarkToString(void * data) {
    PointPlacemark * pl = (PointPlacemark *) data;

    char * tmpStr = malloc(100000);
    
    if (pl->name) {
        sprintf(tmpStr, "%s;", pl->name);
    } else {
        sprintf(tmpStr, "n/a;");
    }
    
    char * p = pointToString(pl->point);
    strcat(tmpStr, p);
    free(p);

    // if (getLength(pl->otherElements) > 0) {
    //     strcat(tmpStr, "\n    Placemark elements\n");
    //     ListIterator iter = createIterator(pl->otherElements);
    //     void * elem;
    //     while ((elem = nextElement(&iter)) != NULL) {
    //         char * k = KMLElementToString(elem);
    //         strcat(tmpStr, k);
    //         free(k);
	//     }
    // }

    return tmpStr;    
}

int comparePointPlacemarks(const void *first, const void *second) {
    return 0;
}

void deletePathPlacemark(void * data) {
    PathPlacemark * pa = (PathPlacemark *) data;

    if (pa->name != NULL) {
        free(pa->name);
    }

    deleteLine(pa->pathData);
    freeList(pa->otherElements);
    free(pa);

}

char * pathPlacemarkToString(void * data) {
    PathPlacemark * pa = (PathPlacemark *) data;

    char * l = malloc(64);
    strcpy(l, "No");
    bool isLoop = isLoopPath(pa, 10);
    if (isLoop == 1) {
        strcpy(l, "Yes");
    } 
    
    char * tmpStr = malloc(100000);
    double len = getPathLen(pa);

    if (pa->name) {
        sprintf(tmpStr, "%s,%f,%s", pa->name, len, l);
    } else {
        sprintf(tmpStr, "n/a,%f,%s", len, l);
    }


    // char * l = lineToString(pa->pathData);
    // strcat(tmpStr, l);
    // free(l);

    // if (getLength(pa->otherElements) > 0) {
    //     strcat(tmpStr, "\n    Placemark elements\n");
    //     ListIterator iter = createIterator(pa->otherElements);
    //     void * elem;
    //     while ((elem = nextElement(&iter)) != NULL) {
    //         char * k = KMLElementToString(elem);
    //         strcat(tmpStr, k);
    //         free(k);
	//     }
    // }

    free(l);
    return tmpStr;
}

int comparePathPlacemarks(const void *first, const void *second) {
    return 0;
}

void deletePoint(void * data) {
    Point * p = (Point *) data;

    deleteCoordinate(p->coordinate);

    freeList(p->otherElements);

    free(p);
}

char * pointToString(void * data) {
    Point * point = (Point *) data;

    char * c = coordinateToString(point->coordinate);
    char * tmpStr = malloc(100000);
    strcpy(tmpStr, "");
    strcat(tmpStr, c);
    free(c);
    // strcat(tmpStr, "\n");
    
    
    // if (getLength(point->otherElements) > 0) {
    //     strcat(tmpStr, "\n    Point elements\n");
    //     ListIterator iter = createIterator(point->otherElements);
    //     void * elem;
    //     while ((elem = nextElement(&iter)) != NULL) {
    //         char * k = KMLElementToString(elem);
    //         strcat(tmpStr, k);
    //         free(k);
	//     }
    // }

    return tmpStr;
}

int comparePoints(const void *first, const void *second) {
    return 0;
}

void deleteCoordinate(void * data) {
    Coordinate * c = (Coordinate *) data;

    free(c);
}

char * coordinateToString(void * data) {
    Coordinate * coordinate = (Coordinate *) data;

    size_t len = (size_t)snprintf(NULL, 0, "%f %f %f", coordinate->longitude, coordinate->latitude, coordinate->altitude);
    char * tmpStr = malloc(len + 64);

    if (coordinate->altitude == DBL_MAX) {
        snprintf(tmpStr, len, "%f,%f", coordinate->longitude, coordinate->latitude);
    } else {
        snprintf(tmpStr, len, "%f,%f,%f", coordinate->longitude, coordinate->latitude, coordinate->altitude);
    }   

    return tmpStr;
}

int compareCoordinates(const void *first, const void *second) {
    return 0;
}

void deleteKMLElement(void * data) {
    if (data == NULL) {
        return;
    }

    KMLElement * k = (KMLElement *) data;
    
    free(k->name);
    free(k->value);
    free(k);
}

char * KMLElementToString(void * data) {
    KMLElement * kmlElement = (KMLElement *) data;

    char * tmpStr = malloc(strlen(kmlElement->name) + strlen(kmlElement->value) + 64);
    sprintf(tmpStr, "kml elem    %s: %s\n", kmlElement->name, kmlElement->value);

    return tmpStr;
}

int compareKMLElements(const void *first, const void *second) {
    return 0;
}

void deleteXMLNamespace(void * data) {
    if (data == NULL) {
        return;
    }
    
    XMLNamespace * ns = (XMLNamespace *) data;

    if (ns->prefix != NULL) {
        free(ns->prefix);
    }
    free(ns->value);
    free(ns);
}

char * XMLNamespaceToString(void * data) {
    XMLNamespace * ns = (XMLNamespace *) data;

    char * tmpStr;
    if (ns->prefix != NULL) {
        tmpStr = malloc(strlen(ns->prefix) + strlen(ns->value) + 64);
        sprintf(tmpStr, "    %s=%s\n", ns->prefix, ns->value);  
    } else {
        tmpStr = malloc(strlen(ns->value) + 64);
        sprintf(tmpStr, "    %s\n", ns->value);  
    }

    return tmpStr;
}

int compareXMLNamespace(const void *first, const void *second) {
    return 0;
}

void deleteStyle(void * data) {
    Style * s = (Style *) data;

    free(s->id);
    free(s->colour);    
    free(s);
}

char * styleToString(void * data) {
    Style * s = (Style *) data;

    // char * tmpStr = malloc(strlen(s->id) + strlen(s->colour) + 64);
    // sprintf(tmpStr, "    id: %s, colour:%s, fill:%d, width:%d\n", s->id, s->colour, s->fill, s->width);

    char * tmpStr = malloc(1000);
    sprintf(tmpStr, "%s,%d,%d", s->colour, s->width, s->fill);

    return tmpStr;
}

int compareStyles(const void *first, const void *second) {
    return 0;
}

void deleteStyleMap(void * data) {
    StyleMap * map = (StyleMap *) data;
    
    free(map->id);
    free(map->key1);
    free(map->url1);
    free(map->key2);
    free(map->url2);
    free(map);
}

char * styleMapToString(void * data) {
    StyleMap * map = (StyleMap *) data;

    char * tmpStr = malloc(strlen(map->id) + strlen(map->key1) + strlen(map->url1) + strlen(map->key2) + strlen(map->url2) +  64);
    sprintf(tmpStr, "    id: %s, key1: %s, url1: %s, key2: %s, url2: %s\n", map->id, map->key1, map->url1, map->key2, map->url2);

    return tmpStr;
}

int compareStyleMaps(const void *first, const void *second) {
    return 0;
}

int getNumPoints(const KML * doc) {
    if (doc == NULL) {
        return 0;
    }

    return getLength(doc->pointPlacemarks);
}

int getNumPaths(const KML * doc) {
    if (doc == NULL) {
        return 0;
    }

    return getLength(doc->pathPlacemarks);
}

int getNumXMLNamespaces(const KML * doc) {
    if (doc == NULL) {
        return 0;
    }

    return getLength(doc->namespaces);
}

int getNumStyles(const KML * doc) {
    if (doc == NULL) {
        return 0;
    }

    return getLength(doc->styles);
}

int getNumStyleMaps(const KML * doc) {
    if (doc == NULL) {
        return 0;
    }

    return getLength(doc->styleMaps);
}

int getNumKMLElements(const KML * doc) {
    if (doc == NULL) {
        return 0;
    }

    int total = 0;

    void * elem;
    ListIterator iter = createIterator(doc->pathPlacemarks);
	while ((elem = nextElement(&iter)) != NULL) {
        PathPlacemark * pa = (PathPlacemark *) elem;
        total += getLength(pa->otherElements) + getLength(pa->pathData->otherElements);
	}
    elem = NULL;
    iter = createIterator(doc->pointPlacemarks);
    while ((elem = nextElement(&iter)) != NULL) {
        PointPlacemark * pn = (PointPlacemark *) elem;
        total += getLength(pn->otherElements) + getLength(pn->point->otherElements);
	}

    return total;
}

PointPlacemark * getPointPlacemark(const KML * doc, char * name) {
    if (doc == NULL || name == NULL) {
        return NULL;
    }

    void * elem;
    ListIterator iter = createIterator(doc->pointPlacemarks);
    while ((elem = nextElement(&iter)) != NULL) {
        PointPlacemark * p = (PointPlacemark *) elem;

        if (p->name) {
            if (strcmp(name, p->name) == 0) {
            return p;
            }
        }        
    }

    return NULL;
}

PathPlacemark * getPathPlacemark(const KML * doc, char * name) {
if (doc == NULL || name == NULL) {
        return NULL;
    }

    void * elem;
    ListIterator iter = createIterator(doc->pathPlacemarks);
    while ((elem = nextElement(&iter)) != NULL) {
        PathPlacemark * p = (PathPlacemark *) elem;

        if (p->name) {
            if (strcmp(name, p->name) == 0) {
            return p;
            }
        }        
    }

    return NULL;
}

StyleMap * getMapFromPath(const KML * doc, const PathPlacemark * ppm) {
    if (doc == NULL || ppm == NULL) {
        return NULL;
    }

    // Go through PathPlacemark elements and look for a styleUrl element.
    // If found, copy value into url string.
    char * url = NULL;
    void * elem;
    ListIterator iter = createIterator(ppm->otherElements);
    while ((elem = nextElement(&iter)) != NULL) {
        KMLElement * k = (KMLElement *) elem;
        if (strcmp(k->name, "styleUrl") == 0) {
            url = malloc(strlen(k->value) + 1);
            strcpy(url, k->value);
            // Remove leading hashtag.
            if (url[0] == '#') {
                memmove(url, url+1, strlen(url));
            }
            break;
        }
    }

    // If url string is not initialized, then that means either the PathPlacemark 
    // has no non-Geography elements, or there is not a styleUrl element present.
    if (url == NULL) {
        return NULL;
    }

    // Go through StyleMaps and look for StyleMap with an id
    // that matches the url string. If found, return StyleMap.
    elem = NULL;
    iter = createIterator(doc->styleMaps);
    while ((elem = nextElement(&iter)) != NULL) {
        StyleMap * sm = (StyleMap *) elem;
        if (strcmp(sm->id, url) == 0) {
            free(url);
            return sm;
        }
    }
    
    if (url != NULL) {
        free(url);
    }
    return NULL;
}

Style * getStyleFromMap(const KML * doc, const StyleMap * map, int index) {
    if (doc == NULL || map == NULL || index < 0 || index > 2) {
        return NULL;
    }

    // Copy the appropriate StyleMap url determined by the
    // given index.
    char * url;
    if (index == 0) {
        url = malloc(strlen(map->url1) + 1);
        strcpy(url, map->url1);

    } else if (index == 1) {
        url = malloc(strlen(map->url2) + 1);
        strcpy(url, map->url2);
    }

    if (url[0] == '#') {
        memmove(url, url+1, strlen(url));
    }

    // Go through list of Styles in KML struct, if a Style has a
    // matching id, return it.
    void * elem;
    ListIterator iter = createIterator(doc->styles);
    while ((elem = nextElement(&iter)) != NULL) {
        Style * s = (Style *) elem;
        if (strcmp(url, s->id) == 0) {
            free(url);
            return s;
        }
    }

    if (url != NULL) {
        free(url);
    }
    return NULL;
}