#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_Polygon"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static void tesselatePolygon( NXT_Node_Polygon *nodeData );

NXT_NodeClass NXT_NodeClass_Polygon = {
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
    "Polygon",
    sizeof(NXT_Node_Polygon)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_Polygon *nodeData = (NXT_Node_Polygon*)node;
    
    if( nodeData->polyPoints )
        free( nodeData->polyPoints );
    
    if( nodeData->vertexColor )
        free( nodeData->vertexColor );
    
    if( nodeData->vertexCoord )
        free( nodeData->vertexCoord );
    
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Polygon *nodeData = (NXT_Node_Polygon*)node;
    /*if( strcasecmp(attrName, "texture")==0 ) {
        LOGI("Texture ID='%s'", attrValue);
        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {
            nodeData->texture = (NXT_Node_Texture*)NXT_FindFirstNodeWithId( node, attrValue+1, &NXT_NodeClass_Texture );
            LOGI("Found node: 0x%08x", (unsigned int)nodeData->texture);
            
        }
    } else if( strcasecmp(attrName, "mask")==0 ) {
        LOGI("Mask ID='%s'", attrValue);
        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {
            nodeData->mask = (NXT_Node_Texture*)NXT_FindFirstNodeWithId( node, attrValue+1, &NXT_NodeClass_Texture );
            LOGI("Found node: 0x%08x", (unsigned int)nodeData->mask);
            
        }
    } else*/
    if( strcasecmp(attrName, "fill")==0 ) {
        LOGD("Fill: '%s'", attrValue);
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->polyColor.e, sizeof(nodeData->polyColor.e)/sizeof(*(nodeData->polyColor.e)) );
        nodeData->bSetColor = 1;
    } else if( strcasecmp(attrName, "color")==0 ) {
        LOGD("Color: '%s'", attrValue);
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->polyColor.e, sizeof(nodeData->polyColor.e)/sizeof(*(nodeData->polyColor.e)) );
        nodeData->bSetColor = 1;
    } else if( strcasecmp(attrName, "points")==0 ) {
        
        if( nodeData->polyPoints ) {
            free(nodeData->polyPoints);
            nodeData->polyPoints = NULL;
        }
        
        if( nodeData->vertexCoord ) {
            free(nodeData->vertexCoord);
            nodeData->vertexCoord = NULL;
        }
        
        nodeData->numPolyPoints = NXT_AnimFloatVectorFromString( node, attrValue, NULL, MAX_POLY_POINTS )/2;
        nodeData->polyPoints = (float*)malloc(sizeof(float)*nodeData->numPolyPoints*2);
        nodeData->bNoCache = 1;
        if( nodeData->polyPoints ) {
            nodeData->vertexCapacity = (nodeData->numPolyPoints-2)*3;
            nodeData->vertexCoord = (float*)malloc(sizeof(float)*4*nodeData->vertexCapacity);
            nodeData->vertexColor = (float*)malloc(sizeof(float)*4*nodeData->vertexCapacity);
            int numUpdateTargets = 0;
            NXT_AnimFloatVectorFromStringEx( node, attrValue, nodeData->polyPoints, nodeData->numPolyPoints*2, &numUpdateTargets );
            if( numUpdateTargets > 0 ) {
                nodeData->bNoCache = 0;
            } else {
                tesselatePolygon(nodeData);
            }
        }
    } else {
        // TODO: Error        
    }
}

typedef struct PolyPoint_ PolyPoint;

struct PolyPoint_ {
    PolyPoint *pNext;
    float x;
    float y;
    unsigned int isConvex;
    unsigned int index;
};

// Input:
//      2-element float vectors (f[0]==x, f[1]==y)
// Returns:
//     <0    if p2 is to the right of the line p0-->p1
//     >0    if p2 is to the left of the line p0-->p1
//      0    if p2 is on the line p0-->p1
static float side( PolyPoint *p0, PolyPoint *p1, PolyPoint *p2 ) {
    return (p1->x-p0->x)*(p2->y-p0->y)-(p1->y-p0->y)*(p2->x-p0->x);
}

// Returns true (nonzero) iff the given point is convex
static unsigned int isConvex(PolyPoint *p0, PolyPoint *p1, PolyPoint *p2 ) {
    return (side(p0, p1, p2) < 0.0f);
}

// Returns true (nonzero) iff the point pt is inside the
// triangle defined by v0,v1,v2.
static unsigned int pointInTriangle(PolyPoint *pt, PolyPoint *v0, PolyPoint *v1, PolyPoint *v2) {
    unsigned int b0 = (side(pt, v0, v1) <= 0.0f);
    unsigned int b1 = (side(pt, v1, v2) <= 0.0f);
    unsigned int b2 = (side(pt, v2, v0) <= 0.0f);
    return (b0==b1 && b1==b2);
}

static float fdiff( float a, float b ) {
    return a<b?b-a:a-b;
}

static unsigned int iter_poly( PolyPoint **p ) {
    unsigned int result = (*p)->index < (*p)->pNext->index;
    (*p) = (*p)->pNext;
    return result;
}

static void tesselatePolygon( NXT_Node_Polygon *nodeData ) {
    int i;
    int numVertices = nodeData->numPolyPoints;
    PolyPoint *poly_alloc = (PolyPoint*)malloc(sizeof(PolyPoint)*numVertices);
    PolyPoint *poly = poly_alloc;
    //LOGV("Tesselating polygon: ");
    for( i=0; i<numVertices; i++ ) {
        poly[i].index = i;
        poly[i].x = nodeData->polyPoints[i*2];
        poly[i].y = nodeData->polyPoints[i*2+1];
        if( i+1 >= numVertices ) {
            poly[i].pNext = poly;
        } else {
            poly[i].pNext = &(poly[i+1]);
        }
        LOGV("-  %03d:  %0.2f, %0.2f", i, poly[i].x, poly[i].y);
    }

    // Remove duplicate consecutive points
    do {
        if( fdiff(poly->x,poly->pNext->x)<0.000001 && fdiff(poly->y,poly->pNext->y)<0.000001 )
            poly->pNext = poly->pNext->pNext;
    } while( iter_poly(&poly) );
    
    // Flag convex vertices
    do {
        poly->pNext->isConvex = isConvex(poly, poly->pNext, poly->pNext->pNext);
    } while( iter_poly(&poly) );
    
    // Find and trim ears
    PolyPoint *lastTrimmedEar = poly;
    unsigned int isEar;
    while( poly->pNext->pNext != poly ) {
        if( poly->pNext->isConvex==2 ) {
            poly->pNext->isConvex = isConvex(poly, poly->pNext, poly->pNext->pNext);
        }
        if( poly->pNext->isConvex ) {
            // Check all points except the three that make up this triangle
            isEar = 1;
            PolyPoint *p;
            for( p = poly->pNext->pNext->pNext; p != poly; p = p->pNext ) {
                if( p->isConvex )
                    continue;
                if( pointInTriangle(p, poly, poly->pNext, poly->pNext->pNext) ) {
                    isEar = 0;
                    break;
                }
            }
            
            // If this an ear, trim it
            if( isEar ) {
                //LOGV( "-  Triangle:   %.2f,%.2f;   %.2f,%.2f;   %.2f,%.2f",
                //       poly->x, poly->y,
                //       poly->pNext->x, poly->pNext->y,
                //       poly->pNext->pNext->x, poly->pNext->pNext->y );
                if( nodeData->vertexCount+3 <= nodeData->vertexCapacity ) {
                    float *v = &nodeData->vertexCoord[nodeData->vertexCount*4];
                    v[0]  = poly->x;
                    v[1]  = poly->y;
                    v[2]  = 0.0f;
                    v[3]  = 1.0f;
                    v[4]  = poly->pNext->x;
                    v[5]  = poly->pNext->y;
                    v[6]  = 0.0f;
                    v[7]  = 1.0f;
                    v[8]  = poly->pNext->pNext->x;
                    v[9]  = poly->pNext->pNext->y;
                    v[10] = 0.0f;
                    v[11] = 1.0f;
                    nodeData->vertexCount += 3;
                }
                poly->pNext = poly->pNext->pNext;
                poly->isConvex = 2;
                poly->pNext->isConvex = 2;
                lastTrimmedEar = poly;
            }
        }
        poly = poly->pNext;
        if( lastTrimmedEar==poly ) {
            LOGD("Malformed polygon");
            break;
        }
    }
    
    free(poly_alloc);
}



static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_Polygon *nodeData = (NXT_Node_Polygon*)node;
    LOGD( "(Polygon / %d points)", nodeData->numPolyPoints );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
#ifdef NEXTHEME_SUPPORT_OPENGL
    int i;
    
    NXT_Node_Polygon *poly = (NXT_Node_Polygon*)node;
    
    if( !poly->bNoCache ) {
        tesselatePolygon(poly);
    }
    
    NXT_Vector4f color;
    if( poly->bSetColor ) {
        color = poly->polyColor;
    } else {
        color = renderer->color;
    }
    for( i=0; i<poly->vertexCount; i++ ) {
        poly->vertexColor[4*i+0] = color.e[0];
        poly->vertexColor[4*i+1] = color.e[1];
        poly->vertexColor[4*i+2] = color.e[2];
        poly->vertexColor[4*i+3] = color.e[3];
    }
    
    NXT_TextureInfo *texture = NULL;
    NXT_TextureInfo *mask = NULL;
//    NexThemeRenderer_PrepStateForRender(renderer,poly->texture,poly->mask,&texture,&mask);
    int first_processed = NexThemeRenderer_PrepStateForRender(renderer,NULL,NULL,&texture,&mask, 1);
    
    if( mask && texture ) {
        // Not supported yet
        
    } else if( texture ) {
        // Not supported yet
        
    } else {
        glVertexAttribPointer(renderer->pSolidShaderActive->a_position, 4, GL_FLOAT, 0, sizeof(GLfloat)*4, poly->vertexCoord );
        CHECK_GL_ERROR();
        glVertexAttribPointer(renderer->pSolidShaderActive->a_color, 4, GL_FLOAT, 0, 0, poly->vertexColor );
        CHECK_GL_ERROR();
    }
        
    glDrawArrays( GL_TRIANGLES, 0, poly->vertexCount );     CHECK_GL_ERROR();
    
#endif //NEXTHEME_SUPPORT_OPENGL
    
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}
