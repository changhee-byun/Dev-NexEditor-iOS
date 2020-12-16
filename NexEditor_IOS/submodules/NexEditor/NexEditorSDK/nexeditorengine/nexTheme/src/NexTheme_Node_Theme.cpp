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

NXT_NodeClass NXT_NodeClass_Theme = {
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
    "Theme",
    sizeof(NXT_Node_Theme)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_Theme *theme = (NXT_Node_Theme*)node;
    if( theme->name )
        free( theme->name );
    if( theme->thumbnailPath )
        free( theme->thumbnailPath );
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Theme *theme = (NXT_Node_Theme*)node;
    if( strcasecmp(attrName, "description")==0 ) {
        if( theme->name )
            free( theme->name );
        theme->name = (char*)malloc(strlen(attrValue)+1);
        strcpy(theme->name, attrValue);
    } else if( strcasecmp(attrName, "thumbnail")==0 ) {
        if( theme->thumbnailPath )
            free( theme->thumbnailPath );
        theme->thumbnailPath = (char*)malloc(strlen(attrValue)+1);
        strcpy(theme->thumbnailPath, attrValue);
    } else {
        // TODO: Error
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_Theme *theme = (NXT_Node_Theme*)node;
    LOGD( "(Theme; name=%s; Thumb=%s)", theme->name, theme->thumbnailPath );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}

