#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_Rect"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Circle = {
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
    "Circle",
    sizeof(NXT_Node_Circle)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    //NXT_Node_Circle *nodeData = (NXT_Node_Circle*)node;
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Circle *nodeData = (NXT_Node_Circle*)node;
    if( strcasecmp(attrName, "center")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->center.e, sizeof(nodeData->center.e)/sizeof(*(nodeData->center.e)) );
    } else if( strcasecmp(attrName, "radius")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->r, 1 );
    } else if( strcasecmp(attrName, "innerradius")==0 ) {
        NXT_AnimFloatVectorFromString( node, attrValue, &nodeData->inner_r, 1 );
    } else if( strcasecmp(attrName, "color")==0 ) {
        LOGXV("Color: '%s'", attrValue);
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->color.e, sizeof(nodeData->color.e)/sizeof(*(nodeData->color.e)) );
        nodeData->bColorSet = 1;
    } else if( strcasecmp(attrName, "innercolor")==0 ) {
        LOGXV("Color: '%s'", attrValue);
        NXT_AnimFloatVectorFromString( node, attrValue, nodeData->inner_color.e, sizeof(nodeData->inner_color.e)/sizeof(*(nodeData->inner_color.e)) );
        nodeData->bInnerColorSet = 1;
    } else if( strcasecmp(attrName, "segments")==0 ) {
        float segments = 0.0;
        NXT_FloatVectorFromString(attrValue, &segments, 1);
        nodeData->segments = (int)segments;
        if( nodeData->segments < 0 )
            nodeData->segments = 0;
        if( nodeData->segments > 2048 )
            nodeData->segments = 2048;
    } else {
        // TODO: Error        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    LOGD( "(Circle)" );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
#ifdef NEXTHEME_SUPPORT_OPENGL
    //int i;
    
    NXT_Node_Circle *nodeData = (NXT_Node_Circle*)node;
    
    NXT_TextureInfo *texture = NULL;
    NXT_TextureInfo *mask = NULL;
    int first_process = NexThemeRenderer_PrepStateForRender(renderer,NULL,NULL,&texture,&mask, 1);

	NXT_Vector4f color;
    if( nodeData->bColorSet ) {
        color = nodeData->color;
    } else {
        color = renderer->color;
    }
	
	NXT_Vector4f inner_color;
    if( nodeData->bInnerColorSet ) {
        inner_color = nodeData->inner_color;
    } else {
        inner_color = color;
    }
	
	float r0, r1;
	if( nodeData->r < nodeData->inner_r ) {
		r0 = nodeData->r;
		r1 = nodeData->inner_r;
	} else {
		r0 = nodeData->inner_r;
		r1 = nodeData->r;
	}
	
	if( nodeData->r<=0 )
		return;
	
	int segments = nodeData->segments;
	if( segments < 3 ) {
		segments = (int)(sqrtf(r1)*10);
		if( segments < 3 )
			segments = 3;
	}
	
	int maxVertexCount = (segments + 1)*2;
	int vertexCount = 0;
	
	GLfloat vertexCoord[maxVertexCount*4];
	GLfloat vertexColor[maxVertexCount*4];
	
	GLenum mode;
	
	if( r0 > 0 ) {
		
		// Circle algorithm from http://slabode.exofire.net/circle_draw.shtml
		// (original code is marked "public domain" by the author)
		//
		// ... with some modifications (not public domain) by Matthew...
		
		float a = 6.283185307179586 / (float)segments;
		float c = cosf(a);
		float s = sinf(a);
		float t;
		
		float x = r1;
		float y = 0;
		float xi = r0;
		float yi = 0;
		
		int i;
		
		for(i = 0; i <= segments; i++)
		{
			vertexColor[vertexCount*4+0] = inner_color.e[0];
			vertexColor[vertexCount*4+1] = inner_color.e[1];
			vertexColor[vertexCount*4+2] = inner_color.e[2];
			vertexColor[vertexCount*4+3] = inner_color.e[3];
			vertexCoord[vertexCount*4+0] = nodeData->center.e[0] + xi;
			vertexCoord[vertexCount*4+1] = nodeData->center.e[1] + yi;
			vertexCoord[vertexCount*4+2] = nodeData->center.e[2];
			vertexCoord[vertexCount*4+3] = nodeData->center.e[3];
			vertexCount++;
			
			vertexColor[vertexCount*4+0] = color.e[0];
			vertexColor[vertexCount*4+1] = color.e[1];
			vertexColor[vertexCount*4+2] = color.e[2];
			vertexColor[vertexCount*4+3] = color.e[3];
			vertexCoord[vertexCount*4+0] = nodeData->center.e[0] + x;
			vertexCoord[vertexCount*4+1] = nodeData->center.e[1] + y;
			vertexCoord[vertexCount*4+2] = nodeData->center.e[2];
			vertexCoord[vertexCount*4+3] = nodeData->center.e[3];
			vertexCount++;
			
			t = x;
			x = c * x - s * y;
			y = s * t + c * y;
			
			t = xi;
			xi = c * xi - s * yi;
			yi = s * t + c * yi;
		}
		
		mode = GL_TRIANGLE_STRIP;
		
	} else {
		
		// Circle algorithm from http://slabode.exofire.net/circle_draw.shtml
		// (original code is marked "public domain" by the author)
		//
		// ... with some modifications (not public domain) by Matthew...
		
		float a = 6.283185307179586 / (float)segments;
		float c = cosf(a);
		float s = sinf(a);
		float t;
		
		float x = r1;
		float y = 0;
		
		int i;
		
		vertexColor[vertexCount*4+0] = inner_color.e[0];
		vertexColor[vertexCount*4+1] = inner_color.e[1];
		vertexColor[vertexCount*4+2] = inner_color.e[2];
		vertexColor[vertexCount*4+3] = inner_color.e[3];
		vertexCoord[vertexCount*4+0] = nodeData->center.e[0];
		vertexCoord[vertexCount*4+1] = nodeData->center.e[1];
		vertexCoord[vertexCount*4+2] = nodeData->center.e[2];
		vertexCoord[vertexCount*4+3] = nodeData->center.e[3];
		vertexCount++;
		
		for(i = 0; i < segments; i++)
		{
			vertexColor[vertexCount*4+0] = color.e[0];
			vertexColor[vertexCount*4+1] = color.e[1];
			vertexColor[vertexCount*4+2] = color.e[2];
			vertexColor[vertexCount*4+3] = color.e[3];
			vertexCoord[vertexCount*4+0] = nodeData->center.e[0] + x;
			vertexCoord[vertexCount*4+1] = nodeData->center.e[1] + y;
			vertexCoord[vertexCount*4+2] = nodeData->center.e[2];
			vertexCoord[vertexCount*4+3] = nodeData->center.e[3];
			vertexCount++;
			
			t = x;
			x = c * x - s * y;
			y = s * t + c * y;
		}
		
		vertexColor[vertexCount*4+0] = color.e[0];
		vertexColor[vertexCount*4+1] = color.e[1];
		vertexColor[vertexCount*4+2] = color.e[2];
		vertexColor[vertexCount*4+3] = color.e[3];
		vertexCoord[vertexCount*4+0] = vertexCoord[1*4+0];
		vertexCoord[vertexCount*4+1] = vertexCoord[1*4+1];
		vertexCoord[vertexCount*4+2] = vertexCoord[1*4+2];
		vertexCoord[vertexCount*4+3] = vertexCoord[1*4+3];
		vertexCount++;
		
		
		mode = GL_TRIANGLE_FAN;

	}
	
    if( mask && texture ) {
        // Not supported yet
        
    } else if( texture ) {
        // Not supported yet
        
    } else {
        glVertexAttribPointer(renderer->pSolidShaderActive->a_position, 4, GL_FLOAT, 0, sizeof(GLfloat)*4, vertexCoord );
        CHECK_GL_ERROR();
        glVertexAttribPointer(renderer->pSolidShaderActive->a_color, 4, GL_FLOAT, 0, 0, vertexColor );
        CHECK_GL_ERROR();
    }
	
    glDrawArrays( mode, 0, vertexCount );   CHECK_GL_ERROR();
	
    
#endif //NEXTHEME_SUPPORT_OPENGL
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
//    NXT_Node_Circle *nodeData = (NXT_Node_Circle*)node;
    return NXT_NodeAction_Next;
}
