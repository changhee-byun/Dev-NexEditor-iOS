//
//  NexTheme_Parser.h
//  NexTheme
//
//  Created by Matthew Feinberg on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef NEXTHEME_PARSER_H
#define NEXTHEME_PARSER_H
#include "NexTheme.h"
#include "NexTheme_Nodes.h"

NXT_Error NXT_ThemeParser_ParseChar( NXT_HThemeSet hThemeSet, char c );
NXT_Error NXT_ThemeParser_Finish( NXT_HThemeSet hThemeSet );
NXT_Error NXT_ThemeParser_Alloc( NXT_HThemeSet hThemeSet );
void NXT_ThemeParser_Dealloc(NXT_HThemeRenderer_Context context, NXT_HThemeSet hThemeSet, unsigned int isDetachedContext );

#define NXT_MAX_TAGLEN 256
#define NXT_MAX_ARGNAMELEN 256
#define NXT_MAX_ARGVALUELEN 4096
#define NXT_MAX_NESTING_DEPTH 256
#define NXT_MAX_SYMBOLS 1024

typedef enum NXT_ParseState_ {
    NXT_ParseState_None = 0,
    NXT_ParseState_BeginTag,
    NXT_ParseState_InTag,
    NXT_ParseState_InElement,
    NXT_ParseState_EndSlash,
    NXT_ParseState_InArgName,
    NXT_ParseState_InArgValue,
    NXT_ParseState_InClosingElement,
    NXT_ParseState_InComment
} NXT_ParseState;

typedef enum NXT_Tag_ {
    NXT_Tag_NONE = 0,
    NXT_Tag_theme,
    NXT_Tag_effect,
    NXT_Tag_timingfunction,
    NXT_Tag_animatedvalue,
    NXT_Tag_keyframe,
    NXT_Tag_texture,
    NXT_Tag_visibility,
    NXT_Tag_trianglestrip,
    NXT_Tag_point,
    NXT_Tag_rotate,
    NXT_Tag_style,
    NXT_Tag_translate
} NXT_Tag;

typedef enum NXT_Attr_ {
    NXT_Attr_NONE = 0,
    NXT_Attr_id,
    NXT_Attr_description,
    NXT_Attr_thumbnail,
    NXT_Attr_values,
    NXT_Attr_time,
    NXT_Attr_value,
    NXT_Attr_timingfunction,
    NXT_Attr_src,
    NXT_Attr_videosrc,
    NXT_Attr_textsrc,
    NXT_Attr_starttime,
    NXT_Attr_endtime,
    NXT_Attr_texture,
    NXT_Attr_location,
    NXT_Attr_texcoord,
    NXT_Attr_color,
    NXT_Attr_alpha,
    NXT_Attr_blendfunc,
    NXT_Attr_shader,
    NXT_Attr_type,
    NXT_Attr_default,
    NXT_Attr_angle,
    NXT_Attr_axis,
    NXT_Attr_offset,
    NXT_ATTR_MAX
} NXT_Attr;

typedef enum NXT_SymType_ {
    NXT_SymType_NONE    = 0,
    NXT_SymType_Const,
    NXT_SymType_TimingFunction,
    NXT_SymType_AnimatedValue,
    NXT_SymType_Texture
} NXT_SymType;

typedef enum NXT_AttrType_ {
    NXT_AttrType_NONE = 0,
    NXT_AttrType_String,
    NXT_AttrType_Float,
    NXT_AttrType_FloatVector,
    NXT_AttrType_Symbol,
    NXT_AttrType_Integer,
    NXT_AttrType_Bool
} NXT_AttrType;

typedef struct NXT_ThemeParser_ {
    NXT_ParseState parseState;
    char tag[NXT_MAX_TAGLEN];
    int taglen;
    char argname[NXT_MAX_ARGNAMELEN];
    int argnamelen;
    char argvalue[NXT_MAX_ARGVALUELEN];
    int dashcount;
    int argvaluelen;
    int line;
    int col;
    NXT_Tag elementStack[NXT_MAX_NESTING_DEPTH];
    char *symbolNames[NXT_MAX_SYMBOLS];
    NXT_SymType symbolTypes[NXT_MAX_SYMBOLS];
    NXT_NodeHeader *nodeStack[NXT_MAX_NESTING_DEPTH];
    NXT_NodeHeader *rootNode;
    int symbolCount;
    int nestingLevel;
    char startQuote;
    char attrWasSet[NXT_ATTR_MAX];
    
    // Attributes
    char *attr_str;
    float attr_fvec[4];
    float attr_float;
    int attr_int;
    
} NXT_ThemeParser, *NXT_HThemeParser;;

typedef struct NXT_TagInfo_ {
    NXT_Tag tagid;
    const char *tagname;
    NXT_Attr validAttrs[16];
} NXT_TagInfo;

typedef struct NXT_AttrInfo_ {
    NXT_Attr attrid;
    const char *attrname;
    NXT_AttrType type;
} NXT_AttrInfo;


#endif //NEXTHEME_PARSER_H