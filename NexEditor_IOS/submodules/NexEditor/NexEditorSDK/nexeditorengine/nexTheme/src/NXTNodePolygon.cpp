#include "NXTNodePolygon.h"

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


void NXTNodePolygon::tesselate() {
    int i;
    int numVertices = numPolyPoints;
    PolyPoint *poly_alloc = new PolyPoint[numVertices];//malloc(sizeof(PolyPoint)*numVertices);
    PolyPoint *poly = poly_alloc;
    for( i=0; i<numVertices; i++ ) {
        poly[i].index = i;
        poly[i].x = polyPoints[i*2];
        poly[i].y = polyPoints[i*2+1];
        if( i+1 >= numVertices ) {
            poly[i].pNext = poly;
        } else {
            poly[i].pNext = &(poly[i+1]);
        }
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
                if( vertexCount+3 <= vertexCapacity ) {
                    float *v = &vertexCoord[vertexCount*4];
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
                    vertexCount += 3;
                }
                poly->pNext = poly->pNext->pNext;
                poly->isConvex = 2;
                poly->pNext->isConvex = 2;
                lastTrimmedEar = poly;
            }
        }
        poly = poly->pNext;
        if( lastTrimmedEar==poly ) {
//            printf("Malformed polygon");
            break;
        }
    }
    
    delete[] poly_alloc;
}
