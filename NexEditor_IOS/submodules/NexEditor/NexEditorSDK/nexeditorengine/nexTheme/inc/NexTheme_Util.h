#ifndef NexTheme_NexTheme_Util_h
#define NexTheme_NexTheme_Util_h

/* NXT_PartialFloatVectorFromString : Parses as many valid space/comma-separated 
 * floats as possible up to numElements, but stops if an invalid character is 
 * encountered, and returns the number of elements found.
 *
 * The string pointer is updated to the first invalid character encountered.
 */

int NXT_PartialFloatVectorFromString( const char** string, float* vector, int numElements );

int NXT_FloatVectorFromString( const char* string, float* vector, int numElements );
float NXT_CubicBezierAtTime(float t,float p1x,float p1y,float p2x,float p2y,float duration);

typedef enum {
    TFUTrue = 1, TFUFalse = 2, TFUUnknown = 0
} TFU;

TFU NXT_ParseBool(const char* string);

#endif
