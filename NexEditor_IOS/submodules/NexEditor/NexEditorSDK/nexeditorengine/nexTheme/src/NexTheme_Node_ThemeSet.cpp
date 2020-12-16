#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
//#include "NexThemeRenderer_Internal.h"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_ThemeSet = {
    nodeInitFunc,
    nodeFreeFunc,
    nodeSetAttrFunc,
    nodeChildrenDoneFunc,
    nodeDebugPrintFunc,
    nodeBeginRender,
    nodeEndRender,
    NULL,//precacheFunc
    NULL,//getPrecacheResourceFunc
    NULL,//nodeUpdateNotify
    NULL,//freeTextureFunc
    "ThemeSet",
    sizeof(NXT_Node_ThemeSet)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_ThemeSet *themeset = (NXT_Node_ThemeSet*)node;
    if( themeset->name )
        free( themeset->name );
    if( themeset->defaultTheme )
        free( themeset->defaultTheme );
    if( themeset->defaultTransition )
        free( themeset->defaultTransition );
    if( themeset->defaultTitle )
        free( themeset->defaultTitle );
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_ThemeSet *theme = (NXT_Node_ThemeSet*)node;
    if( strcasecmp(attrName, "name")==0 ) {
        if( theme->name )
            free( theme->name );
        theme->name = (char*)malloc(strlen(attrValue)+1);
        strcpy(theme->name, attrValue);
    } else if( strcasecmp(attrName, "defaultTheme")==0 ) {
        if( theme->defaultTheme )
            free( theme->defaultTheme );
        theme->defaultTheme = (char*)malloc(strlen(attrValue)+1);
        strcpy(theme->defaultTheme, attrValue);
    } else if( strcasecmp(attrName, "defaultTitle")==0 ) {
        if( theme->defaultTitle )
            free( theme->defaultTitle );
        theme->defaultTitle = (char*)malloc(strlen(attrValue)+1);
        strcpy(theme->defaultTitle, attrValue);
    } else if( strcasecmp(attrName, "defaultTransition")==0 ) {
        if( theme->defaultTransition )
            free( theme->defaultTransition );
        theme->defaultTransition = (char*)malloc(strlen(attrValue)+1);
        strcpy(theme->defaultTransition, attrValue);
    } else {
        // TODO: Error
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_ThemeSet *themeset = (NXT_Node_ThemeSet*)node;
    LOGD("(ThemeSet; name=%s)", themeset->name );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}

