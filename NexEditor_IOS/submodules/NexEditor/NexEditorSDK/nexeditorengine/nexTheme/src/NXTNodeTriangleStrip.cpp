//
//  NXTNodeTriangleStrip.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/5/14.
//
//

#include "NXTNodeTriangleStrip.h"
#include "NXTNodeCoordinateSpace.h"
#include "NXTNodePoint.h"
#include "NXTShader.h"
#include "NXTVertexAttr.h"

void NXTNodeTriangleStrip::childrenDone() {
    
    // Find any coodinate space nodes that may
    // apply to this triangle strip
    for( NXTNode* parent: parents() ) {
        if( parent->is<NXTNodeCoordinateSpace>() ) {
            NXTNodeCoordinateSpace *pcs = static_cast<NXTNodeCoordinateSpace*>(parent);
            switch(pcs->getApplyTo()) {
                case NXTCoordType::Vertex:
                    if( !vertexSpace )
                        vertexSpace = pcs;
                    break;
                case NXTCoordType::Mask:
                    if( !maskSpace )
                        maskSpace = pcs;
                    break;
                case NXTCoordType::Texture:
                    if( !textureSpace )
                        textureSpace = pcs;
                    break;
            }
        }
    }
    
    // Count <point> nodes that are children of this node
    numpoints = 0;
    for( NXTNode* child: children() ) {
        if( child->is<NXTNodePoint>() ) ++numpoints;
    }
    
    static const int num_vertex_arrays = 8;
    static const int elements_per_vertex = 4;
    int vertex_array_size = elements_per_vertex * numpoints;
    
    float_buffers.resize(num_vertex_arrays * vertex_array_size);
    component_flags.resize(numpoints);

    float *pbuf = &float_buffers[0];
    
    textureCoord = pbuf;        pbuf += vertex_array_size;
    adjTextureCoord = pbuf;     pbuf += vertex_array_size;
    maskCoord = pbuf;           pbuf += vertex_array_size;
    adjMaskCoord = pbuf;        pbuf += vertex_array_size;
    vertexColor = pbuf;         pbuf += vertex_array_size;
    vertexNormal = pbuf;        pbuf += vertex_array_size;
    vertexCoord = pbuf;         pbuf += vertex_array_size;
    adjVertexCoord = pbuf;      pbuf += vertex_array_size;

    // Process <point> nodes that are children of this node
    int nChild = 0;
    for( NXTNode* child: children() ) {
        if( child->is<NXTNodePoint>() ) {
            NXTNodePoint &p = *static_cast<NXTNodePoint*>(child);
            if( p.location ) {
                animFloatVectorFromString(p.location, vertexCoord+(4*nChild), 4);
                component_flags[nChild] |= PointComponent::location;
            } else if( p.texcoord ) {
                animFloatVectorFromString(p.location, textureCoord+(4*nChild), 4);
                component_flags[nChild] |= PointComponent::texcoord;
            } else if( p.maskcoord ) {
                animFloatVectorFromString(p.location, maskCoord+(4*nChild), 4);
                component_flags[nChild] |= PointComponent::maskcoord;
            } else if( p.color ) {
                animFloatVectorFromString(p.location, vertexColor+(4*nChild), 4);
                component_flags[nChild] |= PointComponent::color;
            } else if( p.normal ) {
                animFloatVectorFromString(p.location, vertexNormal+(4*nChild), 4);
                component_flags[nChild] |= PointComponent::normal;
            }
            ++nChild;
        }
    }
    
    deleteChildNodes();
}

static float limit0_1( float n ) {
    if( n < 0.0 ) return 0.0;
    else if( n > 1.0 ) return 1.0;
    else return n;
}

void NXTNodeTriangleStrip::renderBegin( IRenderContext& renderer ) {
    
    float r = renderer.getColor()[0];
    float g = renderer.getColor()[1];
    float b = renderer.getColor()[2];
    float a = renderer.getColor()[3];
    
    for( int i=0; i<numpoints; i++ ) {
        if( !(component_flags[i] & PointComponent::color ) ) {
            vertexColor[4*i+0] = r;
            vertexColor[4*i+1] = g;
            vertexColor[4*i+2] = b;
            vertexColor[4*i+3] = a;
        }
    }
    
	for( int i=0; i<numpoints; i++ ) {
		if( !(component_flags[i] & PointComponent::normal) ) {
        	if( renderer.getShadingMode()==NXTShadingMode::Flat && numpoints>=3 ) {
                
            	int p1, p2, p3;
                
            	switch( type  ) {
                    case NXTDrawArraysMode::TriangleStrip:
                    	if( i==0 ) {
                            p1 = 0;
                            p2 = 1;
                            p3 = 2;
                        } else if( i+1 == numpoints ) {
                            p1 = i-2;
                            p2 = i-1;
                            p3 = i;
                        } else {
                            p1 = i-1;
                            p2 = i;
                            p3 = i+1;
                        }
                    	break;
                    case NXTDrawArraysMode::TriangleFan:
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
                    case NXTDrawArraysMode::Triangles:
                    	p1 = i - i%3;
                        p2 = p1+1;
                        p3 = p1+2;
                    	break;
                    case NXTDrawArraysMode::Points:
                    case NXTDrawArraysMode::Lines:
                    case NXTDrawArraysMode::LineStrip:
                    case NXTDrawArraysMode::LineLoop:
                    	p1 = p2 = p3 = 0;
                    	break;
                }
                
                if( p1==p2 || p2==p3 ) {
                    vertexNormal[4*i+0] = 0;
                    vertexNormal[4*i+1] = 0;
                    vertexNormal[4*i+2] = 1;
                    vertexNormal[4*i+3] = 1;
                } else {
                    
                    NXT_Vector4f v1 = (NXT_Vector4f){{
                        adjVertexCoord[4*p2+0] - adjVertexCoord[4*p1+0],
                        adjVertexCoord[4*p2+1] - adjVertexCoord[4*p1+1],
                        adjVertexCoord[4*p2+2] - adjVertexCoord[4*p1+2],
                        adjVertexCoord[4*p2+3] - adjVertexCoord[4*p1+3]}};
                    
                    NXT_Vector4f v2 = (NXT_Vector4f){{
                        adjVertexCoord[4*p3+0] - adjVertexCoord[4*p1+0],
                        adjVertexCoord[4*p3+1] - adjVertexCoord[4*p1+1],
                        adjVertexCoord[4*p3+2] - adjVertexCoord[4*p1+2],
                        adjVertexCoord[4*p3+3] - adjVertexCoord[4*p1+3]}};
                    
                    NXT_Vector4f vec = NXT_Vector4f_Normalize(NXT_Vector4f_CrossProduct(v1,v2));
                    
                    vertexNormal[4*i+0] = vec.e[0];
                    vertexNormal[4*i+1] = vec.e[1];
                    vertexNormal[4*i+2] = vec.e[2];
                    vertexNormal[4*i+3] = vec.e[3];
                }
                
            } else {
                vertexNormal[4*i+0] = 0;
                vertexNormal[4*i+1] = 0;
                vertexNormal[4*i+2] = 1;
                vertexNormal[4*i+3] = 1;
            }
		}
    }
    
    
    renderer.setMask(mask_);
    renderer.setTexture(texture_);
    renderer.prepStateForRender();
    
    if( vertexSpace ) {
        for( int i=0; i<numpoints; i++ ) {
            adjVertexCoord[4*i+0] = vertexCoord[4*i+0];
            adjVertexCoord[4*i+1] = vertexCoord[4*i+1];
            adjVertexCoord[4*i+2] = vertexCoord[4*i+2];
            adjVertexCoord[4*i+3] = vertexCoord[4*i+3];
            vertexSpace->adjustCoord( adjVertexCoord[4*i+0], 0);
            vertexSpace->adjustCoord( adjVertexCoord[4*i+1], 1);
            vertexSpace->adjustCoord( adjVertexCoord[4*i+2], 2);
            vertexSpace->adjustCoord( adjVertexCoord[4*i+3], 3);
        }
    } else {
        for( int i=0; i<numpoints; i++ ) {
            adjVertexCoord[4*i+0] = vertexCoord[4*i+0];
            adjVertexCoord[4*i+1] = vertexCoord[4*i+1];
            adjVertexCoord[4*i+2] = vertexCoord[4*i+2];
            adjVertexCoord[4*i+3] = vertexCoord[4*i+3];
        }
    }
    
    renderer.getShader().vertexLocations().set(adjVertexCoord, numpoints*4, 4);
 
    if( renderer.isTextureActive() ) {
        
        for( int i=0; i<numpoints; i++ ) {
            if(component_flags[i] & PointComponent::texcoord) {
                adjTextureCoord[2*i+0] = textureCoord[4*i+0];
                adjTextureCoord[2*i+1] = textureCoord[4*i+1];
                if( textureSpace ) {
                    textureSpace->adjustCoordBounded( adjTextureCoord[2*i+0], 0);
                    textureSpace->adjustCoordBounded( adjTextureCoord[2*i+1], 1);
                }
            } else {
                adjTextureCoord[2*i+0] = limit0_1((1.0+adjVertexCoord[4*i+0])/2.0);
                adjTextureCoord[2*i+1] = limit0_1((1.0+adjVertexCoord[4*i+1])/2.0);
            }
        }

        renderer.getShader().vertexTexCoords().set(adjTextureCoord, numpoints*2, 2);
    }
    
    if( renderer.isMaskActive() ) {
        {
            // TODO: Check calculate xscale and yscale as below OR check if we can use matrix transform for this on the renderer side
            //            float xscale = (float)(mask->srcWidth-1)/(float)mask->textureWidth;
            //            float yscale = (float)(mask->srcHeight-1)/(float)mask->textureHeight;
        }
        float xscale = 1.0;
        float yscale = 1.0;
        
        for( int i=0; i<numpoints; i++ ) {
            if(component_flags[i] & PointComponent::maskcoord) {
                adjMaskCoord[2*i+0] = maskCoord[4*i+0]*xscale;
                adjMaskCoord[2*i+1] = (1.0-maskCoord[4*i+1])*yscale;
            } else {
                adjMaskCoord[2*i+0] = (1.0+adjVertexCoord[4*i+0])/2.0*xscale;
                adjMaskCoord[2*i+1] = (1.0-((1.0+adjVertexCoord[4*i+1])/2.0))*yscale;
            }
        }
        renderer.getShader().vertexMaskCoords().set(adjTextureCoord, numpoints*2, 2);
        
    }
    
    renderer.getShader().vertexNormals().set(vertexNormal, numpoints*4, 4);
    renderer.getShader().vertexColors().set(vertexColor, numpoints*4, 4);

    renderer.drawArrays(type);
}
