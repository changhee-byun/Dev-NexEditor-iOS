#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme.h"
#include "NexTheme_Parser.h"
#include "NexTheme_Internal.h"
#include "NexTheme_Nodes.h"
//#include "NexThemeRenderer_Internal.h"
#include <stdarg.h>

#define IS_WHITESPACE(x) ( (x)==' ' || (x)=='\t' || (x)=='\n' )
#define IS_SYMBOL_START(x) (((x) >= 'a' && (x) <= 'z') || ( (x) >= 'A' && (x) <= 'Z' ) || (x) == '_')
#define IS_SYMBOL_CONTINUE(x) (((x) >= 'a' && (x) <= 'z') || ( (x) >= 'A' && (x) <= 'Z' ) || ( (x) >= '0' && (x) <= '9' ) || (x) == '_')

#define  LOG_TAG    "NexThemeParser"

static int openElement( NXT_HThemeSet hThemeSet, char* elementName );
static int closeElement( NXT_HThemeSet hThemeSet, char* elementName );
static int handleArgument( NXT_HThemeSet hThemeSet, const char* argName, const char* argValue );
static int getTagID( char *tagName );
static const char* getTagName( int tagID );

#ifdef _CHECK_ATTRIBUTES_
static NXT_Attr getAttrID( char *attrName );
static NXT_AttrType getAttrType( NXT_Attr attr );
static char* getAttrName( int attrID );
static int defineSymbol( NXT_HThemeSet hThemeSet, NXT_SymType type, char *symbolName );
static int lookupSymbol( NXT_HThemeSet hThemeSet, NXT_SymType type, char *symbolName );
static char* getSymbolName( NXT_HThemeSet hThemeSet, NXT_SymType type, int symbolId );
#endif

static void parseError( NXT_HThemeSet hThemeSet, const char *errstr, ... );
static NXT_NodeHeader* getCurrentNode(NXT_HThemeSet hThemeSet);


NXT_Error NXT_ThemeParser_Alloc( NXT_HThemeSet hThemeSet ) {
    hThemeSet->themeParser = (NXT_HThemeParser)malloc( sizeof(NXT_ThemeParser) );
    if( !hThemeSet->themeParser ) {
        return NXT_Error_Malloc;
    }
    memset(hThemeSet->themeParser,0,sizeof(NXT_ThemeParser));
    hThemeSet->themeParser->line = 1;
    return NXT_Error_None;
}

void NXT_ThemeParser_Dealloc(NXT_HThemeRenderer_Context context, NXT_HThemeSet hThemeSet, unsigned int isDetachedContext ) {
	if( !hThemeSet ) {
		return;
	}
    if( hThemeSet->themeParser ) {
        int i;
        if( hThemeSet->themeParser->attr_str ) {
            free( hThemeSet->themeParser->attr_str );
        }
        for( i=0; i < hThemeSet->themeParser->symbolCount; i++ ) {
            free(hThemeSet->themeParser->symbolNames[i]);
        }
        free( hThemeSet->themeParser );
        hThemeSet->themeParser = NULL;
    }
	if( hThemeSet->rootNode ) {
		NXT_FreeNode(context, hThemeSet->rootNode, isDetachedContext);
		hThemeSet->rootNode = NULL;
	}
}

NXT_Error NXT_ThemeParser_ParseChar( NXT_HThemeSet hThemeSet, char c ) {
    if( c=='\n' ) {
        hThemeSet->themeParser->col = 1;
        hThemeSet->themeParser->line++;
    } else {
        hThemeSet->themeParser->col++;
    }
    for(;;) {
        switch( hThemeSet->themeParser->parseState ) {
            case NXT_ParseState_None:
                if( c == '<' ) {
                    // Begin tag
                    hThemeSet->themeParser->parseState = NXT_ParseState_BeginTag;
                    hThemeSet->themeParser->taglen = 0;
                } else if( IS_WHITESPACE(c) ) {
                    // Ignore
                } else {

                    NXT_NodeHeader* node = getCurrentNode(hThemeSet);
                    if(node && node->isa == &NXT_NodeClass_String) {
                        //ignore
                    }
                    else{

                        parseError(hThemeSet, "unexpected character 0x%02x", (int)c);
                        return NXT_Error_Syntax;
                    }
                    
                    // ignore
                }
                break;
            case NXT_ParseState_BeginTag:
                if( IS_SYMBOL_START(c) || c=='/' || c=='!' ) {
                    hThemeSet->themeParser->parseState = NXT_ParseState_InTag;
                    hThemeSet->themeParser->taglen = 1;
                    hThemeSet->themeParser->tag[0] = c;
                } else if( IS_WHITESPACE(c) ) {
                    parseError(hThemeSet, "missing tag name");
                    return NXT_Error_Syntax;
                } else if( c=='>' ) {
                    parseError(hThemeSet, "missing tag name");
                    return NXT_Error_Syntax;
                    //hThemeSet->themeParser->parseState = NXT_ParseState_None;
                } else {
                    parseError(hThemeSet, "unexpected character 0x%02x", (int)c);
                    return NXT_Error_Syntax;
                }
                break;
            case NXT_ParseState_InTag:
                if( IS_SYMBOL_CONTINUE(c) || c=='-' ) {
                    if( hThemeSet->themeParser->taglen < NXT_MAX_TAGLEN-1 ) {
                        hThemeSet->themeParser->tag[hThemeSet->themeParser->taglen++] = c;
                        if(hThemeSet->themeParser->taglen > 2 
                           && hThemeSet->themeParser->tag[0]=='!' 
                           && hThemeSet->themeParser->tag[1]=='-' 
                           && hThemeSet->themeParser->tag[2]=='-') {
                            hThemeSet->themeParser->parseState = NXT_ParseState_InComment;
                            hThemeSet->themeParser->dashcount = 0;
                        }
                        
                    } else {
                        parseError(hThemeSet, "tag name too long (max %d chars)", NXT_MAX_TAGLEN);
                        return NXT_Error_Limits;
                    }
                } else {
                    hThemeSet->themeParser->tag[hThemeSet->themeParser->taglen++] = 0;
                    if( hThemeSet->themeParser->tag[0]=='/' ) {
                        hThemeSet->themeParser->parseState = NXT_ParseState_InClosingElement;
                    } else {
                        hThemeSet->themeParser->parseState = NXT_ParseState_InElement;
                        NXT_Error e = (NXT_Error)openElement(hThemeSet, hThemeSet->themeParser->tag);
                        if( e )
                            return e;
                    }
                    continue;
                }
                break;
            case NXT_ParseState_InComment:
                if( c=='-' ) {
                    hThemeSet->themeParser->dashcount++;
                } else if( c=='>' && hThemeSet->themeParser->dashcount>=2 ) {
                    hThemeSet->themeParser->parseState = NXT_ParseState_None;
                } else {
                    hThemeSet->themeParser->dashcount = 0;
                }
                break;
            case NXT_ParseState_InElement:
                if( c == '>' ) {
                    hThemeSet->themeParser->parseState = NXT_ParseState_None;
                } else if( c == '/' ) {
                    hThemeSet->themeParser->parseState = NXT_ParseState_EndSlash;
                } else if( IS_SYMBOL_START(c) ) {
                    hThemeSet->themeParser->parseState = NXT_ParseState_InArgName;
                    hThemeSet->themeParser->argnamelen = 1;
                    hThemeSet->themeParser->argname[0] = c;
                } else if( IS_WHITESPACE(c) ) {
                    // Ignore
                } else {
                    parseError(hThemeSet, "unexpected character 0x%02x", (int)c);
                    return NXT_Error_Syntax;
                }
                break;
            case NXT_ParseState_InArgName:
                if( IS_SYMBOL_CONTINUE(c) || c=='-' ) {
                    if( hThemeSet->themeParser->argnamelen < NXT_MAX_ARGNAMELEN-1 ) {
                        hThemeSet->themeParser->argname[hThemeSet->themeParser->argnamelen++] = c;
                    } else {
                        parseError(hThemeSet, "argument name too long (max %d chars)", NXT_MAX_ARGNAMELEN);
                        return NXT_Error_Limits;
                    }
                } else if( c=='=' ) {
                    hThemeSet->themeParser->argname[hThemeSet->themeParser->argnamelen++] = 0;
                    hThemeSet->themeParser->parseState = NXT_ParseState_InArgValue;
                    hThemeSet->themeParser->argvaluelen = 0;
                    hThemeSet->themeParser->startQuote = 0;
                } else {
                    hThemeSet->themeParser->argname[hThemeSet->themeParser->argnamelen++] = 0;
                    hThemeSet->themeParser->parseState = NXT_ParseState_InElement;
                    NXT_Error e = (NXT_Error)handleArgument(hThemeSet, hThemeSet->themeParser->argname, "");
                    if( e )
                        return e;
                    continue;
                }
                break;
            case NXT_ParseState_InArgValue:
                if (hThemeSet->themeParser->argvaluelen == 0 && (c == '"' || c == '\'') && (hThemeSet->themeParser->startQuote == 0))
                {
                    hThemeSet->themeParser->startQuote = c;
                } else if( hThemeSet->themeParser->startQuote == c ) {
                    hThemeSet->themeParser->argvalue[hThemeSet->themeParser->argvaluelen++] = 0;
                    NXT_Error e = (NXT_Error)handleArgument(hThemeSet, hThemeSet->themeParser->argname, hThemeSet->themeParser->argvalue);
                    if( e )
                        return e;
                    hThemeSet->themeParser->parseState = NXT_ParseState_InElement;
                } else if( hThemeSet->themeParser->startQuote ) {
                    if( hThemeSet->themeParser->argvaluelen < NXT_MAX_ARGVALUELEN-1 ) {
                        hThemeSet->themeParser->argvalue[hThemeSet->themeParser->argvaluelen++] = c;
                    } else {
                        parseError(hThemeSet, "argument value too long (max %d chars)", NXT_MAX_ARGVALUELEN);
                        return NXT_Error_Limits;
                    }
                } else if( c == '>' ) {
                    hThemeSet->themeParser->argvalue[hThemeSet->themeParser->argvaluelen++] = 0;
                    NXT_Error e = (NXT_Error)handleArgument(hThemeSet, hThemeSet->themeParser->argname, hThemeSet->themeParser->argvalue);
                    if( e )
                        return e;
                    hThemeSet->themeParser->parseState = NXT_ParseState_InElement;
                    continue;
                } else if( IS_WHITESPACE(c) ) {
                    hThemeSet->themeParser->argvalue[hThemeSet->themeParser->argvaluelen++] = 0;
                    NXT_Error e = (NXT_Error)handleArgument(hThemeSet, hThemeSet->themeParser->argname, hThemeSet->themeParser->argvalue);
                    if( e )
                        return e;
                    hThemeSet->themeParser->parseState = NXT_ParseState_InElement;
                } else {
                    if( hThemeSet->themeParser->argvaluelen < NXT_MAX_ARGVALUELEN-1 ) {
                        hThemeSet->themeParser->argvalue[hThemeSet->themeParser->argvaluelen++] = c;
                    } else {
                        parseError(hThemeSet, "argument value too long (max %d chars)", NXT_MAX_ARGVALUELEN);
                        return NXT_Error_Limits;
                    }
                }
                break;
            case NXT_ParseState_InClosingElement:
                if( c == '>' ) {
                    NXT_Error e = (NXT_Error)closeElement(hThemeSet, hThemeSet->themeParser->tag+1);
                    if( e )
                        return e;
                    hThemeSet->themeParser->parseState = NXT_ParseState_None;
                } else if ( IS_WHITESPACE(c) ) {
                    // Ignore
                } else {
                    parseError(hThemeSet, "unexpected character 0x%02x", (int)c);
                    return NXT_Error_Syntax;
                }
                break;
            case NXT_ParseState_EndSlash:
                if( c == '>' ) {
                    NXT_Error e = (NXT_Error)closeElement(hThemeSet, hThemeSet->themeParser->tag);
                    if( e )
                        return e;
                    hThemeSet->themeParser->parseState = NXT_ParseState_None;
                } else {
                    parseError(hThemeSet, "unexpected character 0x%02x", (int)c);
                    return NXT_Error_Syntax;
                }
                break;
            default:
                return NXT_Error_Internal;
        }
        break;
    }
    return (NXT_Error)0;
}

static NXT_NodeHeader* getCurrentNode(NXT_HThemeSet hThemeSet){

    if(hThemeSet->themeParser->nestingLevel > 0)
        return hThemeSet->themeParser->nodeStack[hThemeSet->themeParser->nestingLevel - 1];
    
    return NULL;
}

static int openElement( NXT_HThemeSet hThemeSet, char* elementName ) {

	LOGD("openElement IN : %s",elementName);
    int i;
    if( hThemeSet->themeParser->nestingLevel >= NXT_MAX_NESTING_DEPTH ) {
        parseError(hThemeSet, "elements nested too deeply (max %d levels)", NXT_MAX_NESTING_DEPTH);
        return NXT_Error_Limits;
    }
    NXT_Tag tagid = (NXT_Tag)getTagID(elementName);
    if( tagid==NXT_Tag_NONE ) {
       // parseError(hThemeSet, "unrecognized tag '%s'", elementName);
     //   return NXT_Error_Syntax;
    }
    
    NXT_NodeHeader *newNode = NXT_AllocateNode(NXT_NodeClassByName(elementName));
    
    if( !newNode ) {
        parseError(hThemeSet, "unrecognized tag '%s'", elementName);
        return NXT_Error_Syntax;
    }
    
    if( hThemeSet->themeParser->nestingLevel > 0 ) {
        NXT_AddChildNode(hThemeSet->themeParser->nodeStack[hThemeSet->themeParser->nestingLevel-1], newNode);
    } else if( hThemeSet->themeParser->rootNode ) {
        parseError(hThemeSet, "multiple root nodes; only one allowed '%s'", elementName);
//        NXT_AddChildNode(hThemeSet->themeParser->rootNode, newNode);
    } else {
        hThemeSet->themeParser->rootNode = newNode;
    }
    
    hThemeSet->themeParser->elementStack[hThemeSet->themeParser->nestingLevel]=tagid;
    hThemeSet->themeParser->nodeStack[hThemeSet->themeParser->nestingLevel]=newNode;
    hThemeSet->themeParser->nestingLevel++;
    
    for( i=0; i<NXT_ATTR_MAX; i++ ) {
        hThemeSet->themeParser->attrWasSet[i]=0;
    }
    
    if( hThemeSet->themeParser->attr_str ) {
        free(hThemeSet->themeParser->attr_str);
        hThemeSet->themeParser->attr_str = NULL;
    }
    
    // TODO
    
    ///// TESTING //////
    /*for( i=0; i<hThemeSet->themeParser->nestingLevel-1; i++ ) {
        LOGD("    ");
    }
    LOGD("<%s> (%d)\n",elementName,tagid);*/
    ////////////////////
	LOGD("openElement OUT : %s",elementName);
    
    return 0;
}

static int closeElement( NXT_HThemeSet hThemeSet, char* elementName ) {
	LOGD("closeElement IN : %s",elementName);
    NXT_CompletedNodeChildren(hThemeSet->themeParser->nodeStack[hThemeSet->themeParser->nestingLevel-1]);
    hThemeSet->themeParser->nestingLevel--;
    NXT_Tag tagid = (NXT_Tag)getTagID(elementName);
    if( hThemeSet->themeParser->elementStack[hThemeSet->themeParser->nestingLevel] != tagid ) {
        parseError(hThemeSet, "expected </%s> but found </%s> instead", 
                   getTagName(hThemeSet->themeParser->elementStack[hThemeSet->themeParser->nestingLevel]), 
                   elementName);
        tagid = hThemeSet->themeParser->elementStack[hThemeSet->themeParser->nestingLevel];
    }
    
    // TODO
    
    ///// TESTING //////
    /*int i;
    for( i=0; i<hThemeSet->themeParser->nestingLevel; i++ ) {
        LOGD("    ");
    }
    LOGD("</%s>\n",elementName);*/
    ////////////////////
	LOGD("closeElement OUT : %s",elementName);
    
    return 0;
}

static int handleArgument( NXT_HThemeSet hThemeSet, const char* argName, const char* argValue ) {
    //int i;

    // TODO
 /*   NXT_Attr attrId = getAttrID(argName);
    if( attrId==NXT_Attr_NONE ) {
        parseError(hThemeSet, "unrecognized attribute '%s'", argName);
        return NXT_Error_Syntax;
    }
    
    if( hThemeSet->themeParser->attrWasSet[attrId] ) {
        parseError(hThemeSet, "duplicate attribute '%s'", argName);
        return NXT_Error_Syntax;
    }
    
    hThemeSet->themeParser->attrWasSet[attrId]=1;
    
    if( hThemeSet->themeParser->nestingLevel<1 ) {
        parseError(hThemeSet, "internal parser error (info=%d)", hThemeSet->themeParser->nestingLevel);
        return NXT_Error_Internal;
    }*/
    
    NXT_SetNodeAttr(hThemeSet->themeParser->nodeStack[hThemeSet->themeParser->nestingLevel-1], argName, argValue);
    
/*    switch( hThemeSet->themeParser->elementStack[hThemeSet->themeParser->nestingLevel-1] ) {
        case NXT_Tag_timingfunction:
            switch( attrId ) {
                case NXT_Attr_id:
                    hThemeSet->themeParser->attr_int = defineSymbol(hThemeSet, NXT_SymType_TimingFunction, argValue);
                    break;
                case NXT_Attr_values:
                    break;
                default:
                    parseError(hThemeSet, "attribute '%s' invalid for element <%s>", argName, 
                               getTagName(hThemeSet->themeParser->elementStack[hThemeSet->themeParser->nestingLevel-1]));
                    return NXT_Error_Syntax;
            }
            break;
    }*/
    
    
    ///// TESTING //////
    /*for( i=0; i<hThemeSet->themeParser->nestingLevel; i++ ) {
        LOGD("    ");
    }
    LOGD("%s=%s\n", argName, argValue);*/
    ////////////////////
    return 0;
}

NXT_Error NXT_ThemeParser_Finish( NXT_HThemeSet hThemeSet ) {
    
    hThemeSet->rootNode = hThemeSet->themeParser->rootNode;
    
    NXT_NodeHeader *pn = NXT_FindFirstNodeOfClass(hThemeSet->themeParser->rootNode, NULL);
    while( pn ) {
        NXT_NodeHeader *depthCheck;
        for( depthCheck = pn; depthCheck; depthCheck = depthCheck->parent ) {
            LOGD("    ");
        }
        if (pn->node_id) {
            LOGD("%s: ", pn->node_id );
        }
        if( pn->isa->debugPrintFunc ) {
            pn->isa->debugPrintFunc(pn);
            LOGD("\n" );
        } else {
            LOGD("(%s)\n", pn->isa->className );
        }
        pn = NXT_FindNextNodeOfClass(pn, NULL);
    }
    
    LOGD("[END]\n");
    return (NXT_Error)0;
}

static NXT_TagInfo g_tagTable[] = {
    {NXT_Tag_theme,             "theme",            {NXT_Attr_description, NXT_Attr_thumbnail, (NXT_Attr)0}},
    {NXT_Tag_effect,            "effect",           {NXT_Attr_description, NXT_Attr_type, (NXT_Attr)0}},
    {NXT_Tag_timingfunction,    "timingfunction",   {NXT_Attr_id, NXT_Attr_values, (NXT_Attr)0}},
    {NXT_Tag_animatedvalue,     "animatedvalue",    {NXT_Attr_id, (NXT_Attr)0}},
    {NXT_Tag_keyframe,          "keyframe",         {NXT_Attr_time, NXT_Attr_value, (NXT_Attr)0}},
    {NXT_Tag_texture,           "texture",          {NXT_Attr_id, NXT_Attr_src, NXT_Attr_videosrc, NXT_Attr_textsrc, (NXT_Attr)0}},
    {NXT_Tag_visibility,        "visibility",       {NXT_Attr_starttime, NXT_Attr_endtime, (NXT_Attr)0}},
    {NXT_Tag_trianglestrip,     "trianglestrip",    {NXT_Attr_texture, (NXT_Attr)0}},
    {NXT_Tag_point,             "point",            {NXT_Attr_location, NXT_Attr_color, NXT_Attr_texcoord, (NXT_Attr)0}},
    {NXT_Tag_rotate,            "rotate",           {NXT_Attr_angle, NXT_Attr_axis, (NXT_Attr)0}},
    {NXT_Tag_translate,         "translate",        {NXT_Attr_offset, (NXT_Attr)0}},
    {NXT_Tag_style,             "style",            {NXT_Attr_texture, NXT_Attr_color, NXT_Attr_alpha, NXT_Attr_blendfunc, NXT_Attr_shader, (NXT_Attr)0}},
    {(NXT_Tag)0}
};

static int getTagID( char *tagName ) {
    NXT_TagInfo *t;
    for( t = g_tagTable; t->tagname; t++ ) {
        if( strcasecmp(t->tagname, tagName)==0 ) {
            return t->tagid;
        }
    }
    return NXT_Tag_NONE;
}

static const char* getTagName( int tagID ) {
    NXT_TagInfo *t;
    for( t = g_tagTable; t->tagname; t++ ) {
        if( t->tagid == tagID )
            return t->tagname;
    }
    return NULL;
}


#ifdef _CHECK_ATTRIBUTES_

static NXT_AttrInfo g_attrTable[] = {
    {NXT_Attr_id,               "id",             NXT_AttrType_Symbol},
    {NXT_Attr_description,      "description",      NXT_AttrType_String},
    {NXT_Attr_thumbnail,        "thumbnail",        NXT_AttrType_String},
    {NXT_Attr_values,           "values",           NXT_AttrType_FloatVector},
    {NXT_Attr_time,             "time",             NXT_AttrType_Float},
    {NXT_Attr_value,            "value",            NXT_AttrType_FloatVector},
    {NXT_Attr_timingfunction,   "timingfunction",   NXT_AttrType_Symbol},
    {NXT_Attr_src,              "src",              NXT_AttrType_String},
    {NXT_Attr_videosrc,         "videosrc",         NXT_AttrType_Integer},
    {NXT_Attr_textsrc,          "textsrc",          NXT_AttrType_Integer},
    {NXT_Attr_starttime,        "starttime",        NXT_AttrType_Float},
    {NXT_Attr_endtime,          "endtime",          NXT_AttrType_Float},
    {NXT_Attr_texture,          "texture",          NXT_AttrType_Symbol},
    {NXT_Attr_location,         "location",         NXT_AttrType_FloatVector},
    {NXT_Attr_texcoord,         "texcoord",         NXT_AttrType_FloatVector},
    {NXT_Attr_color,            "color",            NXT_AttrType_FloatVector},
    {NXT_Attr_alpha,            "alpha",            NXT_AttrType_Float},
    {NXT_Attr_blendfunc,        "blendfunc",        NXT_AttrType_String},
    {NXT_Attr_shader,           "shader",           NXT_AttrType_String},
    {NXT_Attr_type,             "type",             NXT_AttrType_String},
    {NXT_Attr_default,          "default",          NXT_AttrType_Bool},
    {NXT_Attr_angle,            "angle",            NXT_AttrType_FloatVector},
    {NXT_Attr_axis,             "axis",             NXT_AttrType_FloatVector},
    {NXT_Attr_offset,           "offset",           NXT_AttrType_FloatVector},
    {0}
};

static NXT_Attr getAttrID( char *attrName ) {
    NXT_AttrInfo *a;
    for( a = g_attrTable; a->attrname; a++ ) {
        if( strcasecmp(a->attrname, attrName)==0 ) {
            return a->attrid;
        }
    }
    return NXT_Attr_NONE;
}

static char* getAttrName( int attrID ) {
    NXT_AttrInfo *a;
    for( a = g_attrTable; a->attrname; a++ ) {
        if( a->attrid == attrID )
            return a->attrname;
    }
    return NULL;
}

static NXT_AttrType getAttrType( NXT_Attr attr ) {
    NXT_AttrInfo *a;
    for( a = g_attrTable; a->attrname; a++ ) {
        if( a->attrid == attr )
            return a->type;
    }
    return NXT_AttrType_NONE;
}

static int defineSymbol( NXT_HThemeSet hThemeSet, NXT_SymType type, char *symbolName ) {
    int i;
    for( i=0; i<hThemeSet->themeParser->symbolCount; i++ ) {
        if( strcmp(hThemeSet->themeParser->symbolNames[i], symbolName)==0 ) {
            parseError(hThemeSet, "name '%s' already used", symbolName);
            return 0;
        }
    }
    if( hThemeSet->themeParser->symbolCount >= NXT_MAX_SYMBOLS ) {
        parseError(hThemeSet, "too many names (max %d); can't add '%s'", NXT_MAX_SYMBOLS, symbolName);
        return 0;
    }
    char *pname = malloc(strlen(symbolName)+1);
    if( !pname ) {
        parseError(hThemeSet, "memory allocation failure");
        return 0;
    }
    strcpy(pname, symbolName);
    hThemeSet->themeParser->symbolTypes[hThemeSet->themeParser->symbolCount] = type;
    hThemeSet->themeParser->symbolNames[hThemeSet->themeParser->symbolCount] = pname;
    hThemeSet->themeParser->symbolCount++;
    return hThemeSet->themeParser->symbolCount;
}

static int lookupSymbol( NXT_HThemeSet hThemeSet, NXT_SymType type, char *symbolName ) {
    int symId = 0;
    int i;
    for( i=0; i<hThemeSet->themeParser->symbolCount; i++ ) {
        if( hThemeSet->themeParser->symbolTypes[i]==type ) {
            symId++;
        }
        if( strcmp(hThemeSet->themeParser->symbolNames[i], symbolName)==0 ) {
            if( hThemeSet->themeParser->symbolTypes[i]!=type ) {
                parseError(hThemeSet, "name '%s' is the wrong type", symbolName);
                return 0;
            } else {
                return symId;
            }
        }
    }
    parseError(hThemeSet, "name '%s' was not defined", symbolName);
    return 0;
}

static char* getSymbolName( NXT_HThemeSet hThemeSet, NXT_SymType type, int symbolId ) {
    int symId = 0;
    int i;
    for( i=0; i<hThemeSet->themeParser->symbolCount; i++ ) {
        if( hThemeSet->themeParser->symbolTypes[i]==type ) {
            symId++;
            if( symId == symbolId )
                return hThemeSet->themeParser->symbolNames[i];
        }
    }
    return NULL;
}
#endif

static void parseError( NXT_HThemeSet hThemeSet, const char *errstr, ... ) {
    va_list arg_ptr;
    
    va_start(arg_ptr, errstr);
    if( hThemeSet->themeParser ) {
        LOGE("%d:%d: error: ", hThemeSet->themeParser->line, hThemeSet->themeParser->col);
    } else {
        LOGE("?:?: error: " );
    }
    vprintf(errstr, arg_ptr);
    LOGD("\n");
    va_end(arg_ptr);
}

