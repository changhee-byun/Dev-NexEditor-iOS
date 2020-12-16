#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexThemeRenderer_Internal.h"

#define LOG_TAG "NexTheme_Node_Effect"

#define LOGFORCE(...)    0
//__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static void nodeInitFunc(NXT_NodeHeader *node);
static void nodeFreeFunc(NXT_NodeHeader *node);
static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue);
static void nodeChildrenDoneFunc(NXT_NodeHeader *node);
static void nodeDebugPrintFunc(NXT_NodeHeader *node);
static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);
static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer);

NXT_NodeClass NXT_NodeClass_Effect = {
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
    "Effect",
    sizeof(NXT_Node_Effect)
};

static void nodeInitFunc(NXT_NodeHeader *node) {
    NXT_Node_Effect *effect = (NXT_Node_Effect*)node;
    effect->defaultDuration = 1000;
    effect->minDuration = 500;
    effect->maxDuration = 2000;
    effect->inTime = 2000;
    effect->outTime = 2000;
    effect->inTimeFirst = 0;
    effect->outTimeLast = 0;
    effect->cycleTime = 2000;
    effect->maxTitleDuration = 0;
    effect->bUserDuration = 1;
    effect->repeatType = NXT_RepeatType_None;
}

static void nodeFreeFunc(NXT_NodeHeader *node) {
    NXT_Node_Effect *effect = (NXT_Node_Effect*)node;
    if( effect->name )
        free( effect->name );
    if( effect->icon )
        free( effect->icon );
    if( effect->categoryTitle )
        free( effect->categoryTitle );
    if( effect->encodedName )
        free( effect->encodedName );
}

static void nodeSetAttrFunc(NXT_NodeHeader *node, const char* attrName, const char* attrValue) {
    NXT_Node_Effect *effect = (NXT_Node_Effect*)node;
    if( strcasecmp(attrName, "name")==0 ) {
        if( effect->name )
            free( effect->name );
        effect->name = (char*)malloc(strlen(attrValue)+1);
        strcpy(effect->name, attrValue);
    } else if( strcasecmp(attrName, "icon")==0 ) {
        if( effect->icon )
            free( effect->icon );
        effect->icon = (char*)malloc(strlen(attrValue)+1);
        strcpy(effect->icon, attrValue);
    } else if( strcasecmp(attrName, "categoryTitle")==0 ) {
        if( effect->categoryTitle )
            free( effect->categoryTitle );
        effect->categoryTitle = (char*)malloc(strlen(attrValue)+1);
        strcpy(effect->categoryTitle, attrValue);
    } else if( strcasecmp(attrName, "type")==0 ) {
        if( strcasecmp(attrValue, "transition")==0 ) {
            effect->effectType = NXT_EffectType_Transition;
            if( !(effect->effectFlags & NXT_EFFECT_FLAG_SETOFFSET) ) {
                effect->effectOffset = -100;
            }
            if( !(effect->effectFlags & NXT_EFFECT_FLAG_SETOVERLAP) ) {
                effect->videoOverlap = 100;
            }
        } else if( strcasecmp(attrValue, "title")==0 ) {
            effect->effectType = NXT_EffectType_Title;
            if( !(effect->effectFlags & NXT_EFFECT_FLAG_SETOFFSET) ) {
                effect->effectOffset = 0;
            }
            if( !(effect->effectFlags & NXT_EFFECT_FLAG_SETOVERLAP) ) {
                effect->videoOverlap = 0;
            }
        } else {
            // TODO: Error
        }
    } else if( strcasecmp(attrName, "repeat")==0 ) {
        if( strcasecmp(attrValue, "no")==0 || strcasecmp(attrValue, "none")==0 ) {
            effect->repeatType = NXT_RepeatType_None;
        } else if( strcasecmp(attrValue, "yes")==0 || strcasecmp(attrValue, "repeat")==0 ) {
            effect->repeatType = NXT_RepeatType_Repeat;
        } else if( strcasecmp(attrValue, "sawtooth")==0 ) {
            effect->repeatType = NXT_RepeatType_Sawtooth;
        } else if( strcasecmp(attrValue, "sine")==0 ) {
            effect->repeatType = NXT_RepeatType_Sine;
        } else {
            // TODO: Error
        }
    } else if( strcasecmp(attrName, "userduration")==0 ) {
        if( strcasecmp(attrValue, "false")==0 ) {
            effect->bUserDuration = 0;
        } else if( strcasecmp(attrValue, "true")==0 ) {
            effect->bUserDuration = 1;
        } else {
            // TODO: Error
        }
    } else if( strcasecmp(attrName, "intime")==0 ) {
        effect->inTime = atoi(attrValue);
    } else if( strcasecmp(attrName, "outtime")==0 ) {
        effect->outTime = atoi(attrValue);
    } else if( strcasecmp(attrName, "intimefirst")==0 ) {
        effect->inTimeFirst = atoi(attrValue);
    } else if( strcasecmp(attrName, "outtimelast")==0 ) {
        effect->outTimeLast = atoi(attrValue);
    } else if( strcasecmp(attrName, "cycleTime")==0 ) {
        effect->cycleTime = atoi(attrValue);
    } else if( strcasecmp(attrName, "minduration")==0 ) {
        effect->minDuration = atoi(attrValue);
    } else if( strcasecmp(attrName, "maxduration")==0 ) {
        effect->maxDuration = atoi(attrValue);
    } else if( strcasecmp(attrName, "maxtitleduration")==0 ) {
        effect->maxTitleDuration = atoi(attrValue);
    } else if( strcasecmp(attrName, "defaultduration")==0 ) {
        effect->defaultDuration = atoi(attrValue);
    } else if( strcasecmp(attrName, "effectoffset")==0 ) {
        effect->effectOffset = atoi(attrValue);
        effect->effectFlags |= NXT_EFFECT_FLAG_SETOFFSET;
    } else if( strcasecmp(attrName, "videooverlap")==0 ) {
        effect->videoOverlap = atoi(attrValue);
        effect->effectFlags |= NXT_EFFECT_FLAG_SETOVERLAP;
    } else {
        // TODO: Error        
    }
}

static void nodeChildrenDoneFunc(NXT_NodeHeader *node) {
    char *p, *d;
    NXT_Node_Effect *effect = (NXT_Node_Effect*)node;
    int namelen = effect->name?(int)strlen(effect->name):0;
    int catlen = effect->categoryTitle?(int)strlen(effect->categoryTitle):0;
    int extra = 0;
    for( p = effect->name; p && *p; p++ )
        extra += (*p=='\t'||*p=='\\');
    for( p = effect->categoryTitle; p && *p; p++ )
        extra += (*p=='\t'||*p=='\\');
    if(effect->encodedName)
        free(effect->encodedName);
    effect->encodedName = (char*)malloc(namelen+catlen+extra+2);
    d = effect->encodedName;
    for( p = effect->name; p && *p; p++ ) {
        if( *p=='\t' ) {
            *d++ = '\\';
            *d++ = 't';
        } else if( *p=='\\' ) {
            *d++ = '\\';
            *d++ = '\\';
        } else {
            *d++ = *p;
        }
    }
    *d++ = '\t';
    for( p = effect->categoryTitle; p && *p; p++ ){
        if( *p=='\t' ) {
            *d++ = '\\';
            *d++ = 't';
        } else if( *p=='\\' ) {
            *d++ = '\\';
            *d++ = '\\';
        } else {
            *d++ = *p;
        }
    }
    *d++ = 0;
}

static void nodeDebugPrintFunc(NXT_NodeHeader *node) {
    NXT_Node_Effect *effect = (NXT_Node_Effect*)node;
    LOGD( "(Effect; type=%d; name=%s; offset=%d; overlap=%d; categoryTitle=%s)", effect->effectType, effect->name, effect->effectOffset, effect->videoOverlap, effect->categoryTitle );
}

static void nodeBeginRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Effect *effect = (NXT_Node_Effect*)node;
    LOGFORCE("Begin render (%s)", effect->header.node_id);
}

static NXT_NodeAction nodeEndRender(NXT_NodeHeader *node, NXT_HThemeRenderer renderer) {
    NXT_Node_Effect *effect = (NXT_Node_Effect*)node;
    LOGFORCE("End render (%s)", effect->header.node_id);
    return NXT_NodeAction_Stop;
}

