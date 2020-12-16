#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "NexTheme.h"
#include "NexTheme_Internal.h"
#include "NexTheme_Nodes.h"
#include "NexTheme_Parser.h"
#include "NexTheme_Util.h"
#include "simplexnoise1234.h"


#define LOG_TAG "NexTheme"

static NXT_HThemeSet createThemeSet( void );
static void deallocThemeSet(NXT_HThemeRenderer_Context renderer, NXT_HThemeSet hThemeSet, unsigned int isDetachedContext );

int NXT_ThemeManager_GetVersionInt( int iComponent ) {
    switch( iComponent ) {
        case 0:         // Major
            return NEXTHEME_VERSION_MAJOR;
        case 1:         // Minor
            return NEXTHEME_VERSION_MINOR;
        case 2:         // Patch
            return NEXTHEME_VERSION_PATCH;
        case 3:         // Build
            return 0;
        default:
            return -1;
    }
}

#ifdef _STDIO_H_
NXT_HThemeSet NXT_ThemeSet_CreateFromFile( FILE* file ) {
    NXT_HThemeSet hThemeSet = createThemeSet();
    if( hThemeSet ) {
        int c;          // Must be -int- to test for EOF condition
        while( (c=getc(file))!=EOF ) {
            if( NXT_ThemeParser_ParseChar(hThemeSet, c) ) {
                return hThemeSet;
            }
        }
    }
    NXT_ThemeParser_Finish(hThemeSet);
    return hThemeSet;
}
#endif

NXT_HThemeSet NXT_ThemeSet_CreateFromString( const char* string ) {
    NXT_HThemeSet hThemeSet = createThemeSet();
    if( hThemeSet ) {
        const char *p;
        for( p = string; *p; p++ ) {
            if( NXT_ThemeParser_ParseChar(hThemeSet, *p) ) {
                return hThemeSet;
            }
        }
    }
    NXT_ThemeParser_Finish(hThemeSet);
    return hThemeSet;
}

NXT_HThemeSet NXT_ThemeSet_Retain( NXT_HThemeSet hThemeSet ) {
    pthread_mutex_lock(&hThemeSet->retainCountLock);
    hThemeSet->retainCount++;
    pthread_mutex_unlock(&hThemeSet->retainCountLock);
    return hThemeSet;
}

int NXT_ThemeSet_Release(NXT_HThemeRenderer_Context context, NXT_HThemeSet hThemeSet, unsigned int  isDetachedContext ) 
{

	unsigned int bFree = 0;
    pthread_mutex_lock(&hThemeSet->retainCountLock);
    hThemeSet->retainCount--;
    if( hThemeSet->retainCount<1 ) {
		bFree = 1;
	}
    pthread_mutex_unlock(&hThemeSet->retainCountLock);
    if( bFree ) {
        deallocThemeSet(context, hThemeSet, isDetachedContext);
        return 1;
    }
    return 0;
}

char* NXT_ThemeSet_GetStringProperty( NXT_HThemeSet themeSet, NXT_ThemeSetProperty property ) {
    
    if( !themeSet ) {
        LOGF("[%s %d] Fatal error; null theme set passed!", __func__, __LINE__);
        return NULL;
    }
    
    NXT_NodeHeader *pn;
    pn = NXT_FindFirstNodeOfClass(themeSet->rootNode,&NXT_NodeClass_ThemeSet);
    NXT_Node_ThemeSet *pThemeSet = ((NXT_Node_ThemeSet*)pn);
    
    if( !pThemeSet ) {
        LOGF("[%s %d] Fatal error; no active theme set root!", __func__, __LINE__);
        return NULL;
    }
    
    switch( property ) {
        case NXT_ThemeSetProperty_DefaultTheme:
            return pThemeSet->defaultTheme;
        case NXT_ThemeSetProperty_DefaultTitleEffect:
            return pThemeSet->defaultTitle;
        case NXT_ThemeSetProperty_DefaultTransitionEffect:
            return pThemeSet->defaultTransition;
        default:
            return NULL;
    }
    
}


unsigned int NXT_ThemeSet_GetThemeCount( NXT_HThemeSet hThemeSet ) {
	
    unsigned int themeCount = 0;
    
    if( !hThemeSet ) {
        return 0;
    }
    
    NXT_NodeHeader *pn;
    pn = NXT_FindFirstNodeOfClass(hThemeSet->rootNode,&NXT_NodeClass_Theme);
    while( pn ) {
        themeCount++;
        pn = NXT_FindNextNodeOfClass(pn,&NXT_NodeClass_Theme);
    }
	
    LOGI("Found %u themes.", themeCount);
    
    return themeCount;
}

NXT_HEffect	NXT_ThemeSet_GetEffectById( NXT_HThemeSet themeSet, const char *effectId ) {
	
    if( !themeSet || !effectId ) {
        return NULL;
    }
    
	NXT_Iter iter = NXT_ForEachNodeOfClass(themeSet->rootNode, &NXT_NodeClass_Effect, NXT_StartAt_Root_IncludeEverything);
	NXT_NodeHeader *effectNode;
	while(( effectNode=iter.next(&iter) )) {
		if( effectNode->node_id && strcmp(effectNode->node_id, effectId)==0 ) {
			return (NXT_HEffect)effectNode;
		}
	}
	return NULL;
}


NXT_HTheme NXT_ThemeSet_GetTheme( NXT_HThemeSet hThemeSet, unsigned int index ) {
    
    unsigned int themeCount = 0;
    
    if( !hThemeSet ) {
        return NULL;
    }
    
    NXT_NodeHeader *pn;
    pn = NXT_FindFirstNodeOfClass(hThemeSet->rootNode,&NXT_NodeClass_Theme);
    while( pn ) {
        if( index==themeCount ) {
            //nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "Returning theme %u", themeCount);
            return (NXT_HTheme)pn;
        }
        themeCount++;
        pn = NXT_FindNextNodeOfClass(pn,&NXT_NodeClass_Theme);
    }
    //nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "Could not find theme for index %u.", index);

    return NULL;
}

char* NXT_Theme_GetID( NXT_HTheme theme ) {
    NXT_Node_Theme *themeNode = (NXT_Node_Theme*)theme;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)themeNode,&NXT_NodeClass_Theme)) {
        return NULL;
    } else {
        return themeNode->header.node_id;
    }
}

char* NXT_Theme_GetName( NXT_HTheme theme ) {
    NXT_Node_Theme *themeNode = (NXT_Node_Theme*)theme;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)themeNode,&NXT_NodeClass_Theme)) {
        return NULL;
    } else {
        return themeNode->name;
    }
}

unsigned int NXT_Theme_GetEffectCount( NXT_HTheme theme ) {
    NXT_Node_Theme *themeNode = (NXT_Node_Theme*)theme;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)themeNode,&NXT_NodeClass_Theme)) {
        return 0;
    } else {
       /* unsigned int effectCount = 0;
        NXT_NodeHeader *pn;
        pn = NXT_FindFirstNodeOfClass((NXT_NodeHeader*)themeNode,&NXT_NodeClass_Effect);
        while( pn ) {
            effectCount++;
            pn = NXT_FindNextNodeOfClass(pn,&NXT_NodeClass_Effect);
        }*/
        unsigned int effectCount = 0;
        NXT_Iter iter = NXT_ForEachNodeOfClass((NXT_NodeHeader*)themeNode, &NXT_NodeClass_Effect, NXT_StartAt_Root_IncludeCurrentTheme);
        while(( iter.next(&iter) ))
            effectCount++;
        
        //LOGI("Found %u effects.", effectCount);
        
        return effectCount;
    }
}

unsigned int NXT_Theme_GetTotalVideoSlot(NXT_HEffect effect){

    unsigned int total_video_slot = 0;

    NXT_Node_Effect* effectinfo = (NXT_Node_Effect*)effect;
    if(effectinfo->effectType != NXT_EffectType_Title)
        return 0;

    NXT_NodeHeader *texNode = NXT_FindFirstNodeOfClass( (NXT_NodeHeader*)effect, &NXT_NodeClass_Texture );
    while( texNode ) {
        
        NXT_Node_Texture* texinfo = (NXT_Node_Texture*)texNode;
        if(texinfo->videoSource)
            total_video_slot++;
        else if(texinfo->kedlitem)
            total_video_slot += NXT_Theme_GetTotalVideoSlot(texinfo->kedlitem->effect);
        texNode = NXT_FindNextNodeOfClass(texNode, &NXT_NodeClass_Texture );
    }

    return total_video_slot;
}

int NXT_Theme_DoPrecache(NXT_HEffect effect, void* renderer, int* asyncmode, int* max_replaceable){

    LOGI("TM30 KEDL precache begin");

    NXT_Iter iter = NXT_ForEachNodeOfClass((NXT_NodeHeader*)effect,&NXT_NodeClass_Texture,NXT_StartAt_Effect);
    NXT_NodeHeader *node;
    int ret = 0;
    while( (node = iter.next(&iter)) ) {
        if( node->isa->precacheFunc ) {
            LOGI("[%s %d] calling precacheFunc (%s)", __func__, __LINE__, node->isa->className);
            ret = node->isa->precacheFunc( node, (NXT_HThemeRenderer)renderer, NXT_PrecacheMode_Upload, asyncmode, max_replaceable);
        } else {
            LOGI("[%s %d] null precacheFunc (%s)", __func__, __LINE__, node->isa->className);
        }
    }

    iter = NXT_ForEachNodeOfClass((NXT_NodeHeader*)effect,&NXT_NodeClass_KEDL,NXT_StartAt_Effect);
    while( (node = iter.next(&iter)) ) {
        if( node->isa->precacheFunc ) {
            LOGI("[%s %d] calling precacheFunc (%s)", __func__, __LINE__, node->isa->className);
            ret = node->isa->precacheFunc( node, (NXT_HThemeRenderer)renderer, NXT_PrecacheMode_Upload, asyncmode, max_replaceable);
        } else {
            LOGI("[%s %d] null precacheFunc (%s)", __func__, __LINE__, node->isa->className);
        }
    }
    LOGI("TM30 KEDL precache end");
    return ret;
}

int NXT_Theme_GetPrecacheResource(NXT_HEffect effect, void* renderer, void* load_resource_list){

    LOGI("KEDL GetPrecacheResource begin");

    NXT_Iter iter = NXT_ForEachNodeOfClass((NXT_NodeHeader*)effect,&NXT_NodeClass_Texture,NXT_StartAt_Effect);
    NXT_NodeHeader *node;
    int ret = 0;
    while( (node = iter.next(&iter)) ) {
        if(node->isa->getPrecacheResourceFunc) {
            LOGI("[%s %d] calling getPrecacheResourceFunc (%s)", __func__, __LINE__, node->isa->className);
            ret = node->isa->getPrecacheResourceFunc(node, (NXT_HThemeRenderer)renderer, load_resource_list);
        } else {
            LOGI("[%s %d] null getPrecacheResourceFunc (%s)", __func__, __LINE__, node->isa->className);
        }
    }

    iter = NXT_ForEachNodeOfClass((NXT_NodeHeader*)effect,&NXT_NodeClass_KEDL,NXT_StartAt_Effect);
    while( (node = iter.next(&iter)) ) {
        if(node->isa->getPrecacheResourceFunc) {
            LOGI("[%s %d] calling getPrecacheResourceFunc (%s)", __func__, __LINE__, node->isa->className);
            ret = node->isa->getPrecacheResourceFunc(node, (NXT_HThemeRenderer)renderer, load_resource_list);
        } else {
            LOGI("[%s %d] null getPrecacheResourceFunc (%s)", __func__, __LINE__, node->isa->className);
        }
    }
    LOGI("KEDL GetPrecacheResource end");
    return ret;
}

NXT_HTheme NXT_Effect_GetTheme( NXT_HEffect effect ) {
    return (NXT_HTheme)NXT_FindFirstNodeOfClass((NXT_NodeHeader*)effect,&NXT_NodeClass_Theme);
}

NXT_HEffect NXT_Theme_GetEffect( NXT_HTheme theme, unsigned int index ) {
    
    
    NXT_Node_Theme *themeNode = (NXT_Node_Theme*)theme;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)themeNode,&NXT_NodeClass_Theme)) {
        return NULL;
    } else {
        
        unsigned int effectCount = 0;
        NXT_Iter iter = NXT_ForEachNodeOfClass((NXT_NodeHeader*)themeNode, &NXT_NodeClass_Effect, NXT_StartAt_Root_IncludeCurrentTheme);
        NXT_NodeHeader *effectNode;
        while(( effectNode=iter.next(&iter) )) {
            if( effectCount==index ) {
                //LOGI("Returning effect %u.", effectCount);
                return (NXT_HEffect)effectNode;
            }
            effectCount++;
        }
        return NULL;
        
        /*unsigned int effectCount = 0;
        NXT_NodeHeader *pn;
        pn = NXT_FindFirstNodeOfClass((NXT_NodeHeader*)themeNode,&NXT_NodeClass_Effect);
        while( pn ) {
            if( effectCount==index ) {
                LOGI("Returning effect %u.", effectCount);
                return (NXT_HEffect)pn;
            }
            effectCount++;
            pn = NXT_FindNextNodeOfClass(pn,&NXT_NodeClass_Effect);
        }
        LOGI("Could not find effect for index %u.", index);
        
        return NULL;*/
    }
}

char* NXT_Effect_GetID( NXT_HEffect effect ) {
    NXT_Node_Effect *effectNode = (NXT_Node_Effect*)effect;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)effectNode,&NXT_NodeClass_Effect)) {
        return NULL;
    } else {
        return effectNode->header.node_id;
    }
}

int NXT_Effect_GetIntProperty( NXT_HEffect effect, NXT_EffectProperty property ) {
    NXT_Node_Effect *effectNode = (NXT_Node_Effect*)effect;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)effectNode,&NXT_NodeClass_Effect)) {
        return 0;
    } else {
        switch (property) {
            case NXT_EffectProperty_EffectType:
                LOGD("[%s %d] NXT_EffectProperty_EffectType=%d", __func__, __LINE__, effectNode->effectType);
                return effectNode->effectType;
                
            case NXT_EffectProperty_EffectOffset:
                LOGD("[%s %d] NXT_EffectProperty_EffectOffset=%d", __func__, __LINE__, effectNode->effectOffset);
                return effectNode->effectOffset;
                
            case NXT_EffectProperty_VideoOverlap:
                LOGD("[%s %d] NXT_EffectProperty_VideoOverlap=%d", __func__, __LINE__, effectNode->videoOverlap);
                return effectNode->videoOverlap;
                
            case NXT_EffectProperty_NumTitleStrings:
                LOGD("[%s %d] NXT_EffectProperty_NumTitleStrings=%d", __func__, __LINE__, 1);
                return 1;
                
            case NXT_EffectProperty_MaximumDuration:
                return effectNode->maxDuration;
                
            case NXT_EffectProperty_MinimumDuration:
                return effectNode->minDuration;

            case NXT_EffectProperty_DefaultDuration:
                return effectNode->defaultDuration;
                
            case NXT_EffectProperty_IsGlobal:
                if( effectNode->header.parent && effectNode->header.parent->isa==&NXT_NodeClass_ThemeSet )
                    return 1;
                else
                    return 0;
                                
            default:
                return 0;
        }
    }
}


char* NXT_Effect_GetName( NXT_HEffect effect ) {
    NXT_Node_Effect *effectNode = (NXT_Node_Effect*)effect;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)effectNode,&NXT_NodeClass_Effect)) {
        return NULL;
    } else {
        if( effectNode->encodedName )
            return effectNode->encodedName;
        else
            return effectNode->name;
    }
}

char* NXT_Effect_GetIcon( NXT_HEffect effect ) {
    NXT_Node_Effect *effectNode = (NXT_Node_Effect*)effect;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)effectNode,&NXT_NodeClass_Effect)) {
        return NULL;
    } else {
        return effectNode->icon;
    }
}

NXT_EffectType NXT_Effect_GetType( NXT_HEffect effect ) {
    NXT_Node_Effect *effectNode = (NXT_Node_Effect*)effect;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)effectNode,&NXT_NodeClass_Effect)) {
        return NXT_EffectType_NONE;
    } else {
        return effectNode->effectType;
    }
}

void NXT_Effect_UpdateUserFields( NXT_HEffect effect, void *renderer ) {
    
    NXT_Node_Effect *effectNode = (NXT_Node_Effect*)effect;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)effectNode,&NXT_NodeClass_Effect)) {
        return;
    }
    
    NXT_NodeHeader *pn = NXT_FindFirstNodeOfClass((NXT_NodeHeader*)effectNode, &NXT_NodeClass_UserField);
    NXT_Node_UserField *pUserField;
    
    while( pn ) {
        
        pUserField = (NXT_Node_UserField*)pn;

        NXT_Node_UserField_ProcessUpdateTargets(pUserField,(NXT_HThemeRenderer)renderer);
        
        pn = NXT_FindNextNodeOfClass(pn, &NXT_NodeClass_UserField);
        
    }
}

void NXT_Effect_Seek( NXT_HEffect effect, float currentTimeInPart, float currentTimeOverall, NXT_PartType part, int clip_index, int clip_count ) 
{
    NXT_Node_Effect *effectNode = (NXT_Node_Effect*)effect;
    if(!NXT_Node_ClassCheck((NXT_NodeHeader*)effectNode,&NXT_NodeClass_Effect)) {
        return;
    }

    /*void NXT_Theme_InterpolateForTime( NXT_HThemeSet hThemeSet, float time ) {
    if( !hThemeSet ) {
        return;
    }*/

    /*NXT_NodeHeader *pn = NXT_FindFirstNodeOfClass(hThemeSet->rootNode, &NXT_NodeClass_AnimatedValue);*/

    NXT_NodeHeader *pn = NXT_FindFirstNodeOfClass((NXT_NodeHeader*)effectNode, &NXT_NodeClass_AnimatedValue);
    
    //LOGI( "Start pn=0x%08x", (unsigned int)pn );
    LOGI("[%s %d] Seek to time %f (%f in part %d)", __func__, __LINE__, currentTimeOverall, currentTimeInPart, part );
 
    /*NXT_Iter iter = NXT_ForEachNodeOfClass((NXT_NodeHeader*)effectNode, &NXT_NodeClass_AnimatedValue, NXT_StartAt_Effect);
    while(( pn = iter.next(&iter) )) {*/
    
    NXT_RandSeed rseed;
    
    NXT_SRandom( &rseed, 0x1E32A7B * clip_index, 0x59B1C13 + clip_index + clip_count );

    int i;
    for( i=0; i<clip_index; i++ ) {
        NXT_FRandom(&rseed);
    }
    
    while( pn ) {
        
        int i;
        float time;
        
        NXT_Node_AnimatedValue *pAnimVal = (NXT_Node_AnimatedValue*)pn;
        
        NXT_Vector4f timing;
        timing.e[0] = pAnimVal->timing[0];
        timing.e[1] = pAnimVal->timing[1];
        timing.e[2] = pAnimVal->timing[2];
        timing.e[3] = pAnimVal->timing[3];
        
        if( pAnimVal->basis==NXT_InterpBasis_ClipIndex ) {
            timing = NXT_Vector4f(0,0,1,1);
            time = (float)clip_index / ((float)clip_count-1.0);
        } else if( pAnimVal->basis==NXT_InterpBasis_ClipIndexPermuted ) {
            timing = NXT_Vector4f(0,0,1,1);
            time = NXT_FRandom(&rseed);
        } else if( pAnimVal->bUseOverallTime ) {
            time = currentTimeOverall;
        } else {
            time = currentTimeInPart;
        }
        
        if( pAnimVal->noiseType==NXT_NoiseType_PerlinSimplex ) {
            int octave;
            float factor = 1.0;
            float noise = 0.0;
            float maxval = 0.0;
            float p = time*pAnimVal->noiseSampleScale + pAnimVal->noiseSampleBias;
            switch( pAnimVal->noiseFunc ) {
                case NXT_NoiseFunction_Fractal:
                    for( octave = 0; octave < pAnimVal->noiseOctaves; octave++ ) {
                        noise += (1.0/factor)*simplex_noise1(p*factor);
                        maxval += (1.0/factor);
                        factor++;
                    }
                    time = (noise/maxval)*pAnimVal->noiseScale + pAnimVal->noiseBias;
                    break;
                case NXT_NoiseFunction_Turbulence:
                    for( octave = 0; octave < pAnimVal->noiseOctaves; octave++ ) {
                        float n = simplex_noise1(p*factor);
                        if( n<0 )
                            noise += (1.0/factor)*(-n);
                        else
                            noise += (1.0/factor)*n;
                        maxval += (1.0/factor);
                        factor++;
                    }
                    time = (noise/maxval)*pAnimVal->noiseScale + pAnimVal->noiseBias;
                    break;
                case NXT_NoiseFunction_WaveFractal:
                    for( octave = 0; octave < pAnimVal->noiseOctaves; octave++ ) {
                        float n = simplex_noise1(p*factor);
                        if( n<0 )
                            noise += (1.0/factor)*(-n);
                        else
                            noise += (1.0/factor)*n;
                        factor++;
                    }
                    noise = sinf( p + noise );
                    time = noise*pAnimVal->noiseScale + pAnimVal->noiseBias;
                    break;
                case NXT_NoiseFunction_Pure:
                default:
                    time = simplex_noise1(p)*pAnimVal->noiseScale + pAnimVal->noiseBias;
                    break;
            }
            if( time < 0.0 )
                time = 0.0;
            if( time > 1.0 )
                time = 1.0;
        }
        
        float e[4] = {0};
        
        NXT_KeyFrameSet *kfset = NULL;
        NXT_KeyFrameSet *pDefaultSet = NULL;
        unsigned int bSkip;
        
        
        LOGD( "KEYFRAME sets: %d", pAnimVal->numKeyframeSets );
        
        for( i=0; i<pAnimVal->numKeyframeSets; i++ ) {
            bSkip = 0;
            if( pAnimVal->keyframeSet[i].bDefault ) {
                pDefaultSet = &pAnimVal->keyframeSet[i];
                bSkip = 1;
            }
            if( pAnimVal->keyframeSet[i].partType!=NXT_PartType_All && part != pAnimVal->keyframeSet[i].partType ) {
                bSkip = 1;
            }
            switch( pAnimVal->keyframeSet[i].clipType ) {
                case NXT_ClipType_All:
                    break;
                case NXT_ClipType_First:
                    if( clip_index != 0 )
                        bSkip = 1;
                    break;
                case NXT_ClipType_Middle:
                    if( clip_count>0 && clip_index <= 0 && clip_index>=(clip_count-1) )
                        bSkip = 1;
                    break;
                case NXT_ClipType_Last:
                    if( clip_index<(clip_count-1) )
                        bSkip = 1;
                    break;
                case NXT_ClipType_Even:
                    if( (clip_index % 2)!=0 )
                        bSkip = 1;
                    break;
                case NXT_ClipType_Odd:
                    if( (clip_index % 2)==0 )
                        bSkip = 1;
                    break;
                case NXT_ClipType_NotFirst:
                    if( clip_index == 0 )
                        bSkip = 1;
                    break;
                case NXT_ClipType_NotMiddle:
                    if( !(clip_count>0 && clip_index <= 0 && clip_index>=(clip_count-1)) )
                        bSkip = 1;
                    break;
                case NXT_ClipType_NotLast:
                    if( clip_index>=(clip_count-1) )
                        bSkip = 1;
                    break;
                case NXT_ClipType_NotEven:
                    if( (clip_index % 2)==0 )
                        bSkip = 1;
                    break;
                case NXT_ClipType_NotOdd:
                    if( (clip_index % 2)!=0 )
                        bSkip = 1;
                    break;
                default:
                    break;
            }
            LOGV( "   sets: %d (bSkip=%d)", i, bSkip );
            if( !bSkip ) {
                kfset = &pAnimVal->keyframeSet[i];
                break;
            }

        }
        
        if( !kfset && pDefaultSet ) {
            kfset = pDefaultSet;
        }
        
        if( !kfset ) {
            //LOGV( "NO KEYFRAMES" );
            e[0] = 0;
            e[1] = 0;
            e[2] = 0;
            e[3] = 0;
        } else if( kfset->numKeyFrames == 1 ) {
            //LOGV( "ONE KEYFRAME" );
            e[0] = kfset->keyframes[0].e[0];
            e[1] = kfset->keyframes[0].e[1];
            e[2] = kfset->keyframes[0].e[2];
            e[3] = kfset->keyframes[0].e[3];
        } else if( kfset->numKeyFrames > 0 ) {
        
            if( time < kfset->keyframes[0].t ) {
                //LOGV( "BEFORE FIRST" );
                e[0] = kfset->keyframes[0].e[0];
                e[1] = kfset->keyframes[0].e[1];
                e[2] = kfset->keyframes[0].e[2];
                e[3] = kfset->keyframes[0].e[3];
            } else if( time > kfset->keyframes[kfset->numKeyFrames-1].t ) {
                //LOGV( "AFTER LAST" );
                e[0] = kfset->keyframes[kfset->numKeyFrames-1].e[0];
                e[1] = kfset->keyframes[kfset->numKeyFrames-1].e[1];
                e[2] = kfset->keyframes[kfset->numKeyFrames-1].e[2];
                e[3] = kfset->keyframes[kfset->numKeyFrames-1].e[3];
            } else {
                
                float animTotal = kfset->keyframes[kfset->numKeyFrames-1].t - kfset->keyframes[0].t;
                float animElapsed = time - kfset->keyframes[0].t;
                float progress = animElapsed/animTotal;
                float interp = NXT_CubicBezierAtTime(progress,
                                                     timing.e[0],timing.e[1],
                                                     timing.e[2],timing.e[3],
                                                     10.0);
                float adjtime = kfset->keyframes[0].t + (animTotal*interp);
                
                //LOGV("time=%f adjusted=%f interp=%f [%f %f %f %f] total=%f elapsed=%f (progress=%f)", time, adjtime,interp,
                     //timing.e[0],timing.e[1],
                     //timing.e[2],timing.e[3], animTotal, animElapsed, progress);

                
                for( i=0; i<kfset->numKeyFrames-1; i++ ) {
                    if( adjtime >= kfset->keyframes[i].t) {
                        if(adjtime <= kfset->keyframes[i+1].t || 
                            ((i+1) == (kfset->numKeyFrames-1))) {

                            float elapsedTime = adjtime - kfset->keyframes[i].t;
                            float totalTime = kfset->keyframes[i+1].t - kfset->keyframes[i].t;
                            float factor;
                            if( totalTime<=0.0 ) {
                                factor = 1.0;
                            } 
                            else if(kfset->framefit){

                                factor = 0.0f;
                            }
                            else if(kfset->keyframes[i].framefit){

                                factor = 0.0f;
                            }
                            else {
                                factor = elapsedTime/totalTime;
                                factor = NXT_CubicBezierAtTime(factor,
                                                                 kfset->keyframes[i+1].timing[0],kfset->keyframes[i+1].timing[1],
                                                                 kfset->keyframes[i+1].timing[2],kfset->keyframes[i+1].timing[3],
                                                                 10.0);
                            }
                            e[0] = kfset->keyframes[i].e[0] + (kfset->keyframes[i+1].e[0] - kfset->keyframes[i].e[0])*factor;
                            e[1] = kfset->keyframes[i].e[1] + (kfset->keyframes[i+1].e[1] - kfset->keyframes[i].e[1])*factor;
                            e[2] = kfset->keyframes[i].e[2] + (kfset->keyframes[i+1].e[2] - kfset->keyframes[i].e[2])*factor;
                            e[3] = kfset->keyframes[i].e[3] + (kfset->keyframes[i+1].e[3] - kfset->keyframes[i].e[3])*factor;
                            LOGI( "FOUND AT %i; factor=%f; e=[%f %f %f %f]", i, factor, e[0], e[1], e[2], e[3] );
                            break;
                        }
                    }
                }
            }
            
        }
        
        LOGD( "Updating %d targets", pAnimVal->numUpdateTargets );
        for( i=0; i<pAnimVal->numUpdateTargets; i++ ) {
            int j;
            LOGV( "  %d: 0x%08x (%d)", i, (unsigned int)(size_t)(pAnimVal->updateTargets[i].targetVector), pAnimVal->updateTargets[i].numElements );
            if( pAnimVal->updateTargets[i].targetVector ) {
                for( j=0; j<pAnimVal->updateTargets[i].numElements; j++ ) {
                    pAnimVal->updateTargets[i].targetVector[j] = e[j+pAnimVal->updateTargets[i].elementOffset] * pAnimVal->updateTargets[i].factor + pAnimVal->updateTargets[i].bias;
                }
            }
        }
        
        pn = NXT_FindNextNodeOfClass(pn, &NXT_NodeClass_AnimatedValue);
//        LOGI( "Next pn=0x%08x", (unsigned int)pn );
    }
    
    
    pn = NXT_FindFirstNodeOfClass((NXT_NodeHeader*)effectNode, &NXT_NodeClass_Function);
    
    while( pn ) {
        NXT_Node_Function *pFunc = (NXT_Node_Function*)pn;
        pn = NXT_FindNextNodeOfClass(pn, &NXT_NodeClass_Function);
        
        NXT_Node_Function_ProcessUpdateTargets( pFunc );
    }
    
}

void NXT_TextStyle_Free( NXT_TextStyle *textStyle ) {
    
    if( textStyle->typeface ) {
        free(textStyle->typeface);
        textStyle->typeface=NULL;
    }

    if( textStyle->default_typeface ) {
        free(textStyle->default_typeface);
        textStyle->default_typeface=NULL;
    }
}

unsigned int NXT_TextStyle_Compare( NXT_TextStyle *textStyle1, NXT_TextStyle *textStyle2 ) {
    if(textStyle1->textSkewX != textStyle2->textSkewX ||
       textStyle1->textScaleX != textStyle2->textScaleX ||
       textStyle1->textSize != textStyle2->textSize ||
       textStyle1->textStrokeWidth != textStyle2->textStrokeWidth ||
       textStyle1->textFlags != textStyle2->textFlags ||
       textStyle1->textAlign != textStyle2->textAlign ||
       !NXT_Vector4f_Compare(textStyle1->fillColor, textStyle2->fillColor ) ||
       !NXT_Vector4f_Compare(textStyle1->strokeColor, textStyle2->strokeColor ) ||
       !NXT_Vector4f_Compare(textStyle1->shadowColor, textStyle2->shadowColor ) ||
       !NXT_Vector4f_Compare(textStyle1->shadowOffset, textStyle2->shadowOffset ) ||
       textStyle1->shadowRadius != textStyle2->shadowRadius ||
       strcmp(textStyle1->typeface,textStyle2->typeface)!=0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static NXT_HThemeSet createThemeSet( void ) {
    LOGI("[%s %d] NexTheme (%d.%d.%d.%d)", __func__, __LINE__, NEXTHEME_VERSION_MAJOR, NEXTHEME_VERSION_MINOR, NEXTHEME_VERSION_PATCH, 0);
    NXT_HThemeSet hThemeSet = (NXT_HThemeSet)malloc( sizeof(NXT_ThemeSet) );
    if( !hThemeSet )
        return NULL;
    memset(hThemeSet,0,sizeof(NXT_ThemeSet));
    NXT_Error result = NXT_ThemeParser_Alloc( hThemeSet );
    if( result!=NXT_Error_None ) {
        free(hThemeSet);
        return NULL;
    }
    hThemeSet->retainCount = 1;
    pthread_mutex_init(&hThemeSet->retainCountLock,NULL);
    return hThemeSet;
}

static void deallocThemeSet(NXT_HThemeRenderer_Context context, NXT_HThemeSet hThemeSet, unsigned int isDetachedContext ) {
    pthread_mutex_destroy(&hThemeSet->retainCountLock);
    NXT_ThemeParser_Dealloc(context, hThemeSet, isDetachedContext);
    free(hThemeSet);
}

