#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Math.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

void NXT_CoordSpace_Adjust( NXT_Node_CoordinateSpace* coordSpace, float* coord, int index );

#define LOG_TAG "NexTheme_Node_TriangleStrip"

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

static void coordSpace_AdjustBounded( NXT_Node_CoordinateSpace* coordSpace, float* coord, int index ) {
    NXT_CoordSpace_Adjust(coordSpace,coord,index);
    if( *coord<0.0 )
        *coord=0.0;
    if( *coord>1.0 )
        *coord=1.0;
}

NXT_NodeClass NXT_NodeClass_TriangleStrip = {
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
    "TriangleStrip",
    sizeof(NXT_Node_TriangleStrip)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_TriangleStrip *nodeData = (NXT_Node_TriangleStrip*)node;
    nodeData->type = GL_TRIANGLE_STRIP;
    nodeData->scale = 1.0f;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_TriangleStrip *nodeData = (NXT_Node_TriangleStrip*)node;

    if( nodeData->buffer ) {
    	free(nodeData->buffer);
        nodeData->buffer = NULL;
    }
    
//    if( nodeData->textureCoord )
//        free( nodeData->textureCoord );
//    
//    if( nodeData->vertexColor )
//        free( nodeData->vertexColor );
//    
//    if( nodeData->vertexNormal )
//        free( nodeData->vertexNormal );
//    
//    if( nodeData->vertexCoord )
//        free( nodeData->vertexCoord );
//    
//    if( nodeData->adjVertexCoord )
//        free( nodeData->adjVertexCoord );
//    
//    if( nodeData->bComponentSet )
//        free( nodeData->bComponentSet );
//    
//    if( nodeData->adjTextureCoord )
//        free( nodeData->adjTextureCoord );
//    
//    if( nodeData->adjMaskCoord )
//        free( nodeData->adjMaskCoord );
//    
//    if( nodeData->maskCoord )
//        free( nodeData->maskCoord );

}

#include "Matrices.h"

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_TriangleStrip *nodeData = (NXT_Node_TriangleStrip*)node;
    if( strcasecmp(attrName, "texture")==0 ) {
        LOGD("Texture ID='%s'", attrValue);
        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {
            nodeData->texture = (NXT_Node_Texture*)NXT_FindFirstNodeWithId( node, attrValue+1, &NXT_NodeClass_Texture );
            LOGD("Found node: 0x%p", nodeData->texture);
            
        }
    } else if( strcasecmp(attrName, "mask")==0 ) {
        LOGD("Mask ID='%s'", attrValue);
        if( attrValue[0]==NXT_ELEMENT_REF_CHAR ) {
            nodeData->mask = (NXT_Node_Texture*)NXT_FindFirstNodeWithId( node, attrValue+1, &NXT_NodeClass_Texture );
            LOGD("Found node: 0x%p", nodeData->mask);
            
        }
    } else if( strcasecmp(attrName, "type")==0 ) {
        if( strcasecmp(attrValue, "trianglestrip")==0 ) {
            nodeData->type = GL_TRIANGLE_STRIP;
        } else if( strcasecmp(attrValue, "trianglefan")==0 ) {
            nodeData->type = GL_TRIANGLE_FAN;
        } else if( strcasecmp(attrValue, "triangles")==0 ) {
            nodeData->type = GL_TRIANGLES;
        } else if( strcasecmp(attrValue, "strip")==0 ) {
            nodeData->type = GL_TRIANGLE_STRIP;
        } else if( strcasecmp(attrValue, "fan")==0 ) {
            nodeData->type = GL_TRIANGLE_FAN;
        } else if( strcasecmp(attrValue, "triangles")==0 ) {
            nodeData->type = GL_TRIANGLES;
        } else if( strcasecmp(attrValue, "points")==0 ) {
            nodeData->type = GL_POINTS;
        } else if( strcasecmp(attrValue, "lines")==0 ) {
            nodeData->type = GL_LINES;
        } else if( strcasecmp(attrValue, "linestrip")==0 ) {
            nodeData->type = GL_LINE_STRIP;
        } else if( strcasecmp(attrValue, "lineloop")==0 ) {
            nodeData->type = GL_LINE_LOOP;
        }
    } else if( strcasecmp(attrName, "mapping")==0 ) {
        if( strcasecmp(attrValue, "mtv")==0 ) {
            nodeData->mapping = 1;
        }
    } else if( strcasecmp(attrName, "mapping_scale")==0 ) {
        NXT_AnimFloatVectorFromString(node, attrValue, &nodeData->scale, 1 );
    } else {
        // TODO: Error        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
    NXT_Node_TriangleStrip *nodeData = (NXT_Node_TriangleStrip*)node;
    int nChild;
    NXT_NodeHeader *pn;
    
    for( pn=node; pn; pn = pn->parent ) {
        if( pn->isa == &NXT_NodeClass_CoordinateSpace ) {
            NXT_Node_CoordinateSpace *pcs = (NXT_Node_CoordinateSpace*)pn;
            switch( pcs->applyTo ) {
                case NXT_CoordType_Vertex:
                    if( !nodeData->vertexSpace )
                        nodeData->vertexSpace = pcs;
                    break;
                case NXT_CoordType_Mask:
                    if( !nodeData->maskSpace )
                        nodeData->maskSpace = pcs;
                    break;
                case NXT_CoordType_Texture:
                    if( !nodeData->textureSpace )
                        nodeData->textureSpace = pcs;
                    break;
                default:
                    break;
            }
        }
    }
    
    for( pn=node->child, nChild=0; pn; pn = pn->next ) {
        if( pn->isa == &NXT_NodeClass_Point )
            nChild++;
    }
    nodeData->numPoints=nChild;
    
    if( nodeData->buffer ) {
    	free(nodeData->buffer);
        nodeData->buffer = 0;
    }
    
    int numVertexArrays = 8;
    int vertexArraySize = (nodeData->numPoints*4*sizeof(float));
    int numCharArrays = 1;
    int charArraySize = (nodeData->numPoints*sizeof(unsigned char));
    int bufSize = (numVertexArrays * vertexArraySize) + (numCharArrays * charArraySize);
    nodeData->buffer = (void**)malloc(bufSize);
    memset( nodeData->buffer, 0, bufSize );
    char *pbuff = (char*)nodeData->buffer;
    
    nodeData->textureCoord = (float*)pbuff;
    pbuff += vertexArraySize;
    nodeData->adjTextureCoord = (float*)pbuff;
    pbuff += vertexArraySize;
    nodeData->maskCoord = (float*)pbuff;
    pbuff += vertexArraySize;
    nodeData->adjMaskCoord = (float*)pbuff;
    pbuff += vertexArraySize;
    nodeData->vertexColor = (float*)pbuff;
    pbuff += vertexArraySize;
    nodeData->vertexNormal = (float*)pbuff;
    pbuff += vertexArraySize;
    nodeData->vertexCoord = (float*)pbuff;
    pbuff += vertexArraySize;
    nodeData->adjVertexCoord = (float*)pbuff;
    pbuff += vertexArraySize;
    
    nodeData->bComponentSet = (unsigned char*)pbuff;
    pbuff += charArraySize;
    
//    if( nodeData->textureCoord )
//        free( nodeData->textureCoord );
//    
//    if( nodeData->vertexColor )
//        free( nodeData->vertexColor );
//    
//    if( nodeData->vertexNormal )
//        free( nodeData->vertexNormal );
//    
//    if( nodeData->vertexCoord )
//        free( nodeData->vertexCoord );
//    
//    if( nodeData->adjVertexCoord )
//        free( nodeData->adjVertexCoord );
//    
//    if( nodeData->bComponentSet )
//        free( nodeData->bComponentSet );
//    
//    if( nodeData->adjTextureCoord )
//        free( nodeData->adjTextureCoord );
//
//    if( nodeData->adjMaskCoord )
//        free( nodeData->adjMaskCoord );
//    
//    if( nodeData->maskCoord )
//        free( nodeData->maskCoord );

//    nodeData->textureCoord = malloc(nodeData->numPoints*4*sizeof(float));
//    nodeData->adjTextureCoord = malloc(nodeData->numPoints*4*sizeof(float));
//    nodeData->maskCoord = malloc(nodeData->numPoints*4*sizeof(float));
//    nodeData->adjMaskCoord = malloc(nodeData->numPoints*4*sizeof(float));
//    nodeData->vertexColor = malloc(nodeData->numPoints*4*sizeof(float));
//    nodeData->vertexNormal = malloc(nodeData->numPoints*4*sizeof(float));
//    nodeData->vertexCoord = malloc(nodeData->numPoints*4*sizeof(float));
//    nodeData->adjVertexCoord = malloc(nodeData->numPoints*4*sizeof(float));
//    
//    nodeData->bComponentSet = malloc(nodeData->numPoints*sizeof(unsigned char));
//    
//    memset( nodeData->textureCoord, 0, nodeData->numPoints*4*sizeof(float) );
//    memset( nodeData->adjTextureCoord, 0, nodeData->numPoints*4*sizeof(float) );
//    memset( nodeData->maskCoord, 0, nodeData->numPoints*4*sizeof(float) );
//    memset( nodeData->adjMaskCoord, 0, nodeData->numPoints*4*sizeof(float) );
//    memset( nodeData->vertexColor, 0, nodeData->numPoints*4*sizeof(float) );
//    memset( nodeData->vertexNormal, 0, nodeData->numPoints*4*sizeof(float) );
//    memset( nodeData->vertexCoord, 0, nodeData->numPoints*4*sizeof(float) );
//    memset( nodeData->adjVertexCoord, 0, nodeData->numPoints*4*sizeof(float) );
//    memset( nodeData->bComponentSet, 0, nodeData->numPoints*sizeof(unsigned char) );
    
    for( pn=node->child, nChild=0; pn; pn = pn->next ) {
        if( pn->isa == &NXT_NodeClass_Point ) {
            NXT_Node_Point *pPoint = (NXT_Node_Point*)pn;
            if( pPoint->location ) {
                NXT_AnimFloatVectorFromString(node, pPoint->location, nodeData->vertexCoord+(4*nChild), 4);
                nodeData->bComponentSet[nChild] |= NXT_PointComponent_Location;
            }
            if( pPoint->texcoord ) {
                NXT_AnimFloatVectorFromString(node, pPoint->texcoord, nodeData->textureCoord+(4*nChild), 4);
                nodeData->bComponentSet[nChild] |= NXT_PointComponent_TexCoord;
            }
            if( pPoint->maskcoord ) {
                NXT_AnimFloatVectorFromString(node, pPoint->maskcoord, nodeData->maskCoord+(4*nChild), 4);
                nodeData->bComponentSet[nChild] |= NXT_PointComponent_MaskCoord;
            }
            if( pPoint->color ) {
                NXT_AnimFloatVectorFromString(node, pPoint->color, nodeData->vertexColor+(4*nChild), 4);
                nodeData->bComponentSet[nChild] |= NXT_PointComponent_Color;
            }
            if( pPoint->normal ) {
                NXT_AnimFloatVectorFromString(node, pPoint->normal, nodeData->vertexNormal+(4*nChild), 4);
                nodeData->bComponentSet[nChild] |= NXT_PointComponent_Normal;
            }
            nChild++;
        }
    }
    
    LOGD("TriangleStrip::nodeChildrenDoneFunc (%d points)", nodeData->numPoints);
    
    NXT_DeleteChildNodes(NULL, node, 1);
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_TriangleStrip *nodeData = (NXT_Node_TriangleStrip*)node;
    LOGD( "(TriangleStrip / %d points)", nodeData->numPoints );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    
#ifdef NEXTHEME_SUPPORT_OPENGL
    
    int i;
    
    NXT_Node_TriangleStrip *ts = (NXT_Node_TriangleStrip*)node;
    
    for( i=0; i<ts->numPoints; i++ ) {
        if( !(ts->bComponentSet[i] & NXT_PointComponent_Color) ) {
            ts->vertexColor[4*i+0] = renderer->color.e[0];
            ts->vertexColor[4*i+1] = renderer->color.e[1];
            ts->vertexColor[4*i+2] = renderer->color.e[2];
            ts->vertexColor[4*i+3] = renderer->color.e[3];
        }
    }
    
	for( i=0; i<ts->numPoints; i++ ) {
		if( !(ts->bComponentSet[i] & NXT_PointComponent_Normal) ) {
        	if( renderer->shadingMode==NXT_ShadingMode_Flat && ts->numPoints>=3 ) {
            
            	int p1, p2, p3;
            
            	switch( ts->type  ) {
                    case GL_TRIANGLE_STRIP:
                    	if( i==0 ) {
                            p1 = 0;
                            p2 = 1;
                            p3 = 2;
                        } else if( i+1 == ts->numPoints ) {
                            p1 = i-2;
                            p2 = i-1;
                            p3 = i;
                        } else {
                            p1 = i-1;
                            p2 = i;
                            p3 = i+1;
                        }
                    	break;
                    case GL_TRIANGLE_FAN:
                    	if( i<2 ) {
                            p1 = 0;
                            p2 = 1;
                            p3 = 2;
                        } else {
                            p1 = 0;
                            p2 = i-1;
                            p3 = i;
                        }
                    	break;
                    case GL_TRIANGLES:
                    	p1 = i - i%3;
                        p2 = p1+1;
                        p3 = p1+2;
                    	break;
                    case GL_POINTS:
                    case GL_LINES:
                    case GL_LINE_STRIP:
                    case GL_LINE_LOOP:
                    default:
                    	p1 = p2 = p3 = 0;
                    	break;
                }
                
                if( p1==p2 || p2==p3 ) {
                    ts->vertexNormal[4*i+0] = 0;
                    ts->vertexNormal[4*i+1] = 0;
                    ts->vertexNormal[4*i+2] = 1;
                    ts->vertexNormal[4*i+3] = 1;
                } else {
                
                    NXT_Vector4f v1 = NXT_Vector4f(
                        ts->adjVertexCoord[4*p2+0] - ts->adjVertexCoord[4*p1+0],
                        ts->adjVertexCoord[4*p2+1] - ts->adjVertexCoord[4*p1+1],
                        ts->adjVertexCoord[4*p2+2] - ts->adjVertexCoord[4*p1+2],
                        ts->adjVertexCoord[4*p2+3] - ts->adjVertexCoord[4*p1+3]);
                    
                    NXT_Vector4f v2 = NXT_Vector4f(
                        ts->adjVertexCoord[4*p3+0] - ts->adjVertexCoord[4*p1+0],
                        ts->adjVertexCoord[4*p3+1] - ts->adjVertexCoord[4*p1+1],
                        ts->adjVertexCoord[4*p3+2] - ts->adjVertexCoord[4*p1+2],
                        ts->adjVertexCoord[4*p3+3] - ts->adjVertexCoord[4*p1+3]);
                    
                    NXT_Vector4f vec = NXT_Vector4f_Normalize(NXT_Vector4f_CrossProduct(v1,v2));

                    ts->vertexNormal[4*i+0] = vec.e[0];
                    ts->vertexNormal[4*i+1] = vec.e[1];
                    ts->vertexNormal[4*i+2] = vec.e[2];
                    ts->vertexNormal[4*i+3] = vec.e[3];
                }
                
            } else {
                ts->vertexNormal[4*i+0] = 0;
                ts->vertexNormal[4*i+1] = 0;
                ts->vertexNormal[4*i+2] = 1;
                ts->vertexNormal[4*i+3] = 1;
            }
		}
    }
    

    
    NXT_TextureInfo *texture = NULL;
    NXT_TextureInfo *mask = NULL;

    int first_processed = 0;
    Matrix3 homography;
    Matrix3 mask_homography;
    const float* homography_matrix = NULL;
    const float* homography_mask_matrix = NULL;
    

    if( ts->vertexSpace ) {
        for( i=0; i<ts->numPoints; i++ ) {
            ts->adjVertexCoord[4*i+0] = ts->vertexCoord[4*i+0];
            ts->adjVertexCoord[4*i+1] = ts->vertexCoord[4*i+1];
            ts->adjVertexCoord[4*i+2] = ts->vertexCoord[4*i+2];
            ts->adjVertexCoord[4*i+3] = ts->vertexCoord[4*i+3];
            NXT_CoordSpace_Adjust(ts->vertexSpace, &ts->adjVertexCoord[4*i+0], 0);
            NXT_CoordSpace_Adjust(ts->vertexSpace, &ts->adjVertexCoord[4*i+1], 1);
            NXT_CoordSpace_Adjust(ts->vertexSpace, &ts->adjVertexCoord[4*i+2], 2);
            NXT_CoordSpace_Adjust(ts->vertexSpace, &ts->adjVertexCoord[4*i+3], 3);
        }
    } else {
        for( i=0; i<ts->numPoints; i++ ) {
            ts->adjVertexCoord[4*i+0] = ts->vertexCoord[4*i+0];
            ts->adjVertexCoord[4*i+1] = ts->vertexCoord[4*i+1];
            ts->adjVertexCoord[4*i+2] = ts->vertexCoord[4*i+2];
            ts->adjVertexCoord[4*i+3] = ts->vertexCoord[4*i+3];
        }
    }

    mask = ts->mask?&ts->mask->texinfo_preview:NULL;
    texture = ts->texture?&ts->texture->texinfo_preview:NULL;
    
    if( mask && texture ) {

        LOGV( "[NexTheme_Node_TriangleStrip %d] MASK AND TEXTURE", __LINE__ );
        
        {
            
            for( i=0; i<ts->numPoints; i++ ) {
                if(ts->bComponentSet[i] & NXT_PointComponent_TexCoord) {
                    ts->adjTextureCoord[4*i+0] = ts->textureCoord[4*i+0];
                    ts->adjTextureCoord[4*i+1] = ts->textureCoord[4*i+1];
                    ts->adjTextureCoord[4*i+2] = ts->textureCoord[4*i+2];
                    ts->adjTextureCoord[4*i+3] = ts->textureCoord[4*i+3];
                    if( ts->textureSpace ) {
                        coordSpace_AdjustBounded( ts->textureSpace, &ts->adjTextureCoord[4*i+0], 0);
                        coordSpace_AdjustBounded( ts->textureSpace, &ts->adjTextureCoord[4*i+1], 1);
                        NXT_CoordSpace_Adjust( ts->textureSpace, &ts->adjTextureCoord[4*i+2], 2);
                        NXT_CoordSpace_Adjust( ts->textureSpace, &ts->adjTextureCoord[4*i+3], 3);
                    }
                } else {
                    ts->adjTextureCoord[4*i+0] = NexThemeRenderer_AdjTexCoordX(texture,(1.0+ts->adjVertexCoord[4*i+0])/2.0);
                    ts->adjTextureCoord[4*i+1] = NexThemeRenderer_AdjTexCoordY(texture,(1.0+ts->adjVertexCoord[4*i+1])/2.0);
                    ts->adjTextureCoord[4*i+2] = 0;
                    ts->adjTextureCoord[4*i+3] = 0;
                }
            }
        }
        
        {
            float xscale = (float)(mask->srcWidth-1)/(float)mask->textureWidth;
            float yscale = (float)(mask->srcHeight-1)/(float)mask->textureHeight;
            LOGV( "[NexTheme_Node_TriangleStrip %d] MASK xscale=%f yscale=%f", __LINE__, xscale, yscale );
            
            for( i=0; i<ts->numPoints; i++ ) {
                if(ts->bComponentSet[i] & NXT_PointComponent_MaskCoord) {
                    LOGV( "[NexTheme_Node_TriangleStrip %d] POINT %i SET MASK COORD (%f %f)", __LINE__, i, ts->maskCoord[4*i+0], ts->maskCoord[4*i+1] );
                    ts->adjMaskCoord[4*i+0] = ts->maskCoord[4*i+0]*xscale;
                    ts->adjMaskCoord[4*i+1] = (1.0-ts->maskCoord[4*i+1])*yscale;
                    ts->adjMaskCoord[4*i+2] = ts->maskCoord[4*i+2];
                    ts->adjMaskCoord[4*i+3] = ts->maskCoord[4*i+3];
                } else {
                    LOGV( "[NexTheme_Node_TriangleStrip %d] POINT %i DEFAULT MASK COORD", __LINE__, i );
                    ts->adjMaskCoord[4*i+0] = (1.0+ts->adjVertexCoord[4*i+0])/2.0*xscale;
                    ts->adjMaskCoord[4*i+1] = (1.0-((1.0+ts->adjVertexCoord[4*i+1])/2.0))*yscale;
                    ts->adjMaskCoord[4*i+2] = 0;
                    ts->adjMaskCoord[4*i+3] = 0;
                }
            }
        }

        if(ts->mapping){

            NXT_Matrix4f mvp = NXT_Matrix4f_MultMatrix(renderer->basetransform, NXT_Matrix4f_MultMatrix(renderer->proj, renderer->transform));
            NXT_Vector4f vec1 = NXT_Matrix4f_MultVector(mvp, (NXT_Vector4f)&ts->adjVertexCoord[4*0]);
            NXT_Vector4f vec2 = NXT_Matrix4f_MultVector(mvp, (NXT_Vector4f)&ts->adjVertexCoord[4*2]);
            NXT_Vector4f vec3 = NXT_Matrix4f_MultVector(mvp, (NXT_Vector4f)&ts->adjVertexCoord[4*3]);
            NXT_Vector4f vec4 = NXT_Matrix4f_MultVector(mvp, (NXT_Vector4f)&ts->adjVertexCoord[4*1]);

            Vector2 points[4] = {

                Vector2(vec1.e[0], vec1.e[1]),
                Vector2(vec2.e[0], vec2.e[1]),
                Vector2(vec3.e[0], vec3.e[1]),
                Vector2(vec4.e[0], vec4.e[1])
            };

            Vector2 tex_uv[4] = {

                Vector2(ts->adjTextureCoord[4*0+0], ts->adjTextureCoord[4*0+1]),
                Vector2(ts->adjTextureCoord[4*2+0], ts->adjTextureCoord[4*2+1]),
                Vector2(ts->adjTextureCoord[4*3+0], ts->adjTextureCoord[4*3+1]),
                Vector2(ts->adjTextureCoord[4*1+0], ts->adjTextureCoord[4*1+1])
            };

            Vector2 mask_uv[4] = {

                Vector2(ts->adjMaskCoord[4*0+0], ts->adjMaskCoord[4*0+1]),
                Vector2(ts->adjMaskCoord[4*2+0], ts->adjMaskCoord[4*2+1]),
                Vector2(ts->adjMaskCoord[4*3+0], ts->adjMaskCoord[4*3+1]),
                Vector2(ts->adjMaskCoord[4*1+0], ts->adjMaskCoord[4*1+1])
            };

            nexFindHomography(points, tex_uv, homography);
            nexFindHomography(points, mask_uv, mask_homography);
            homography_matrix = homography.get();
            homography_mask_matrix = mask_homography.get();
        }

        for( i=0; i<ts->numPoints; i++ ) {
            LOGV( "[NexTheme_Node_TriangleStrip %d] Point %i: TEXTURE=(%f %f) MASK=(%f %f)", __LINE__, i,
                 ts->adjTextureCoord[4*i+0],
                 ts->adjTextureCoord[4*i+1],
                 ts->adjMaskCoord[4*i+0],
                 ts->adjMaskCoord[4*i+1]);
        }
        
    } else if( texture ) {
        
        for( i=0; i<ts->numPoints; i++ ) {
            if(ts->bComponentSet[i] & NXT_PointComponent_TexCoord) {
                ts->adjTextureCoord[4*i+0] = ts->textureCoord[4*i+0];
                ts->adjTextureCoord[4*i+1] = ts->textureCoord[4*i+1];
                ts->adjTextureCoord[4*i+2] = ts->textureCoord[4*i+2];
                ts->adjTextureCoord[4*i+3] = ts->textureCoord[4*i+3];
                if( ts->textureSpace ) {
                    coordSpace_AdjustBounded( ts->textureSpace, &ts->adjTextureCoord[4*i+0], 0);
                    coordSpace_AdjustBounded( ts->textureSpace, &ts->adjTextureCoord[4*i+1], 1);
                    NXT_CoordSpace_Adjust( ts->textureSpace, &ts->adjTextureCoord[4*i+2], 2);
                    NXT_CoordSpace_Adjust( ts->textureSpace, &ts->adjTextureCoord[4*i+3], 3);
                }
            } else {
                ts->adjTextureCoord[4*i+0] = NexThemeRenderer_AdjTexCoordX(texture,(1.0+ts->adjVertexCoord[4*i+0])/2.0);
                ts->adjTextureCoord[4*i+1] = NexThemeRenderer_AdjTexCoordY(texture,(1.0+ts->adjVertexCoord[4*i+1])/2.0);
                ts->adjTextureCoord[4*i+2] = 0;
                ts->adjTextureCoord[4*i+3] = 0;
            }
        }

        if(ts->mapping){

            NXT_Matrix4f mvp = NXT_Matrix4f_MultMatrix(renderer->basetransform, NXT_Matrix4f_MultMatrix(renderer->proj, renderer->transform));
            NXT_Vector4f vec1 = NXT_Matrix4f_MultVector(mvp, (NXT_Vector4f)&ts->adjVertexCoord[4*0]);
            NXT_Vector4f vec2 = NXT_Matrix4f_MultVector(mvp, (NXT_Vector4f)&ts->adjVertexCoord[4*2]);
            NXT_Vector4f vec3 = NXT_Matrix4f_MultVector(mvp, (NXT_Vector4f)&ts->adjVertexCoord[4*3]);
            NXT_Vector4f vec4 = NXT_Matrix4f_MultVector(mvp, (NXT_Vector4f)&ts->adjVertexCoord[4*1]);

            Vector2 points[4] = {

                Vector2(vec1.e[0], vec1.e[1]),
                Vector2(vec2.e[0], vec2.e[1]),
                Vector2(vec3.e[0], vec3.e[1]),
                Vector2(vec4.e[0], vec4.e[1])
            };

            Vector2 tex_uv[4] = {

                Vector2(ts->adjTextureCoord[4*0+0], ts->adjTextureCoord[4*0+1]),
                Vector2(ts->adjTextureCoord[4*2+0], ts->adjTextureCoord[4*2+1]),
                Vector2(ts->adjTextureCoord[4*3+0], ts->adjTextureCoord[4*3+1]),
                Vector2(ts->adjTextureCoord[4*1+0], ts->adjTextureCoord[4*1+1])
            };

            if(ts->scale != 1.0f){

                Matrix3 find_scaled_pos;
                nexFindHomographyID(points[0], points[1], points[2], points[3], find_scaled_pos);

                float scale = ts->scale - 1.0f;

                Vector3 pos0 = Vector3(0.0f - scale, 0.0f - scale, 1.0f) * find_scaled_pos;
                Vector3 pos1 = Vector3(0.0f - scale, 1.0f + scale, 1.0f) * find_scaled_pos;
                Vector3 pos2 = Vector3(1.0f + scale, 1.0f + scale, 1.0f) * find_scaled_pos;
                Vector3 pos3 = Vector3(1.0f + scale, 0.0f - scale, 1.0f) * find_scaled_pos;

                pos0.x /= pos0.z;
                pos0.y /= pos0.z;

                pos1.x /= pos1.z;
                pos1.y /= pos1.z;

                pos2.x /= pos2.z;
                pos2.y /= pos2.z;

                pos3.x /= pos3.z;
                pos3.y /= pos3.z;

                points[0].x = pos0.x;
                points[0].y = pos0.y;

                points[1].x = pos1.x;
                points[1].y = pos1.y;

                points[2].x = pos2.x;
                points[2].y = pos2.y;

                points[3].x = pos3.x;
                points[3].y = pos3.y;

                ts->adjVertexCoord[4*0 + 0] = points[0].x;
                ts->adjVertexCoord[4*0 + 1] = points[0].y;
                ts->adjVertexCoord[4*2 + 0] = points[1].x;
                ts->adjVertexCoord[4*2 + 1] = points[1].y;
                ts->adjVertexCoord[4*3 + 0] = points[2].x;
                ts->adjVertexCoord[4*3 + 1] = points[2].y;
                ts->adjVertexCoord[4*1 + 0] = points[3].x;
                ts->adjVertexCoord[4*1 + 1] = points[3].y;
            }

            nexFindHomography(points, tex_uv, homography);
            homography_matrix = homography.get();
        }
    }

    mask = NULL;
    texture = NULL;
    
    first_processed = NexThemeRenderer_PrepStateForRender(renderer,ts->texture,ts->mask,&texture,&mask, 1, homography_matrix, homography_mask_matrix);

    if( mask && texture ) {
        
        glVertexAttribPointer(renderer->pMaskedShaderActive->a_position,4, GL_FLOAT, 0, sizeof(GLfloat)*4,ts->adjVertexCoord );
        CHECK_GL_ERROR();

        if(ts->mapping){

        }
        else{

            glVertexAttribPointer(renderer->pMaskedShaderActive->a_texCoord,2, GL_FLOAT, 0, sizeof(GLfloat)*4,ts->adjTextureCoord );
            CHECK_GL_ERROR();
            
            glVertexAttribPointer(renderer->pMaskedShaderActive->a_maskCoord,2, GL_FLOAT, 0, sizeof(GLfloat)*4,ts->adjMaskCoord );
            CHECK_GL_ERROR();
        }
        
    } else if( texture ) {
        
        glVertexAttribPointer(renderer->pTexturedShaderActive->a_position,4, GL_FLOAT, 0, sizeof(GLfloat)*4,ts->adjVertexCoord );
        CHECK_GL_ERROR();

        if(ts->mapping){

        }
        else{

            glVertexAttribPointer(renderer->pTexturedShaderActive->a_texCoord,2, GL_FLOAT, 0, sizeof(GLfloat)*4,ts->adjTextureCoord );
            CHECK_GL_ERROR();
        }
        
        if(IS_VALID_UNIFORM(renderer->pTexturedShaderActive->lightingInfo.a_normal)){

            glEnableVertexAttribArray(renderer->pTexturedShaderActive->lightingInfo.a_normal);
            CHECK_GL_ERROR();

            glVertexAttribPointer(renderer->pTexturedShaderActive->lightingInfo.a_normal,4, GL_FLOAT, 0, sizeof(GLfloat)*4,ts->vertexNormal );
            CHECK_GL_ERROR();
        }

    } else {
        glVertexAttribPointer(renderer->pSolidShaderActive->a_position, 4, GL_FLOAT, 0, sizeof(GLfloat)*4, ts->adjVertexCoord );
        CHECK_GL_ERROR();
        glVertexAttribPointer(renderer->pSolidShaderActive->a_color, 4, GL_FLOAT, 0, 0, ts->vertexColor );
        CHECK_GL_ERROR();
    }
        
    glDrawArrays( ts->type, 0, ts->numPoints );
    CHECK_GL_ERROR();
    
#endif //NEXTHEME_SUPPORT_OPENGL
    
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    return NXT_NodeAction_Next;
}
