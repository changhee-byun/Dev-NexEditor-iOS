#include "NexTheme_Config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NexTheme_Nodes.h"
#include "NexTheme_Util.h"
#include "NexThemeRenderer_Internal.h"

#define  LOG_TAG    "NexTheme_Nodes"

NXT_RenderItem_KeyValues::NXT_RenderItem_KeyValues(){

}

NXT_RenderItem_KeyValues::~NXT_RenderItem_KeyValues(){

    clear();
}

void NXT_RenderItem_KeyValues::clear(){

    for(iter_t itor = items_.begin(); itor != items_.end(); ++itor){

        delete *itor;
    }
    items_.clear();        
}

NXT_NodeHeader *NXT_AllocateNode( NXT_NodeClass* nodeClass ) {
    if( !nodeClass ) {
        // TODO: error
        return NULL;
    }
    
    NXT_NodeHeader *node = (NXT_NodeHeader*)malloc(nodeClass->size);
    if( !node ) {
        // TODO: error
        return NULL;
    }
    memset(node, 0, nodeClass->size);
    node->isa = nodeClass;
    
    if( node->isa->initNodeFunc ) {
        node->isa->initNodeFunc(node);
    }
    
    return node;
}

void NXT_AddChildNode( NXT_NodeHeader *node, NXT_NodeHeader *newChild ) {
    if( !node || !newChild ) {
        // TODO: error
        return;
    } else if( newChild->parent ) {
        // TODO: error
        return;
    }
    
    if( node->child ) {
        NXT_AddSiblingNode( node->child, newChild );
    } else {
        node->child = newChild;
        newChild->parent = node;
    }
}

NXT_NodeHeader *NXT_RootForNode( NXT_NodeHeader *node ) {
    NXT_NodeHeader *pn = node;
    while( pn->parent )
        pn = pn->parent;
    return pn;
}

void NXT_AddSiblingNode( NXT_NodeHeader *node, NXT_NodeHeader *newSibling ) {
    if( !node || !newSibling ) {
        // TODO: error
        return;
    }
    if( newSibling->parent ) {
        // TODO: error
        return;
    }
    
    NXT_NodeHeader *pn = node;
    while (pn->next) {
        pn = pn->next;
    }
    pn->next = newSibling;
    newSibling->parent = node->parent;
}

void NXT_CompletedNodeChildren( NXT_NodeHeader *node ) {
    if( node && node->isa && node->isa->childrenDoneFunc ) {
        node->isa->childrenDoneFunc(node);
    }
}

void NXT_SetNodeAttr( NXT_NodeHeader *node, const char* attrName, const char* attrValue ) {
    if( !node || !node->isa)
        return;
    if( strcasecmp(attrName, "id")==0 ) {
        NXT_SetNodeId(node, attrValue);
    } else if( node->isa->setAttrFunc ) {
        node->isa->setAttrFunc(node,attrName,attrValue);
    } else {
        // TODO: Error
    }
}

void NXT_DeleteChildNodes(NXT_HThemeRenderer_Context context, NXT_NodeHeader *node, unsigned int isDetachedConext ) {
	NXT_NodeHeader *pcurrent = node->child;
	NXT_NodeHeader *pnext;

	while( pcurrent ) {
		pnext = pcurrent->next;
		NXT_FreeNode(context, pcurrent, isDetachedConext);
		pcurrent = pnext;
    }
    node->child = NULL;
}

void NXT_FreeNode(NXT_HThemeRenderer_Context context, NXT_NodeHeader *node, unsigned int isDetachedConext ) {
    if( !node ) {
        // TODO: error
        return;
    }
    
    NXT_DeleteChildNodes(context, node, isDetachedConext);

    if( node->isa->freeTextureFunc && !isDetachedConext ) 
    {
        node->isa->freeTextureFunc(context, node);
    }

    if( node->isa->freeNodeFunc ) {
        node->isa->freeNodeFunc(node);
    }
    
    if( node->node_id )
        free(node->node_id);
    free(node);
}

NXT_NodeHeader *NXT_FindFirstNodeWithId( NXT_NodeHeader *node, const char *node_id, NXT_NodeClass *nodeClass ) {
    NXT_NodeHeader *pn = NXT_FindFirstNodeOfClass(node,nodeClass);
    while(pn && (!pn->node_id || strcmp(pn->node_id, node_id)!=0) )
        pn = NXT_FindNextNodeOfClass(pn,nodeClass);
    return pn;
}

NXT_NodeHeader *NXT_FindFirstNodeOfClass( NXT_NodeHeader *node, NXT_NodeClass *nodeClass ) {
    
    NXT_NodeHeader *pn;
    
    if( !node ) {
        // TODO: error
        return NULL;
    }
    
    if( nodeClass != &NXT_NodeClass_Theme && nodeClass != &NXT_NodeClass_Effect ) {
        // Find root without passing Effect or Theme
        pn = node;
        while( pn->parent && pn->isa!=&NXT_NodeClass_Theme && pn->isa!=&NXT_NodeClass_Effect )
            pn = pn->parent;
        if( !nodeClass || pn->isa == nodeClass )
            return pn;
        pn = NXT_FindNextNodeOfClass(pn, nodeClass);
        if( pn )
            return pn;
    }
    
    if( nodeClass != &NXT_NodeClass_Theme ) {
        // Find root without passing Theme
        pn = node;
        while( pn->parent && pn->isa!=&NXT_NodeClass_Theme )
            pn = pn->parent;
        if( !nodeClass || pn->isa == nodeClass )
            return pn;
        pn = NXT_FindNextNodeOfClass(pn, nodeClass);
        if( pn )
            return pn;
    }
    
    // Find root
    pn = node;
    while( pn->parent )
        pn = pn->parent;
    if( !nodeClass || pn->isa == nodeClass )
        return pn;
    pn = NXT_FindNextNodeOfClass(pn, nodeClass);
    if( pn )
        return pn;
    
    // Not found
    return NULL;

}

NXT_NodeHeader *NXT_FindNextNodeOfClass( NXT_NodeHeader *node, NXT_NodeClass *nodeClass ) {
    if( !node ) {
        // TODO: error
        return NULL;
    }
    NXT_NodeHeader *pn = node;
    
    for(;;) {
        if( pn->child ) {
            pn = pn->child;
        } else if( pn->next ) {
            pn = pn->next;
        } else {
            while( pn->parent && !pn->parent->next ) {
                pn = pn->parent;
            }
            if( pn->parent && pn->parent->next ) {
                pn = pn->parent->next;
            } else {
                // Not found
                return NULL;
            }
        }
        if( !nodeClass || pn->isa == nodeClass ) {
            return pn;
        }
        if(nodeClass!=&NXT_NodeClass_Theme && 
           (pn->isa==&NXT_NodeClass_Theme || pn->isa==&NXT_NodeClass_Effect))
        {
            // Not found
            return NULL;
        }
    }
}

static unsigned int iterIsIncludeNode(NXT_Iter *pIter, NXT_NodeHeader *pn) {
    /*if( pIter->rootNode==pn )
        return 1;*/
    
    if( pIter->startAt==NXT_StartAt_Root_IncludeEverything )
        return 1;
    
    if( pn->isa==&NXT_NodeClass_Theme ) {
        if( pIter->startAt==NXT_StartAt_Root_IncludeAllThemes )
            return 1;
        else if( pIter->startAt==NXT_StartAt_Root_IncludeCurrentTheme && pIter->themeNode==pn )
            return 1;
        else if( pIter->startAt==NXT_StartAt_Theme_IncludeCurrentEffect && pIter->themeNode==pn )
            return 1;
        else if( pIter->startAt==NXT_StartAt_Root_IncludeCurrentThemeAndEffect && pIter->themeNode==pn )
            return 1;
        else
            return 0;
    } else if( pn->isa==&NXT_NodeClass_Effect ) {
        if( pIter->startAt==NXT_StartAt_Theme_IncludeAllEffects )
            return 1;
        else if( pIter->startAt==NXT_StartAt_Theme_IncludeCurrentEffect && pIter->effectNode==pn )
            return 1;
        else if( pIter->startAt==NXT_StartAt_Root_IncludeAllThemes && pIter->findClass==&NXT_NodeClass_Effect)
            return 1;
        else if( pIter->startAt==NXT_StartAt_Root_IncludeCurrentTheme && pIter->findClass==&NXT_NodeClass_Effect )
            return 1;
        else if( pIter->startAt==NXT_StartAt_Effect && pIter->effectNode==pn )
            return 1;
        else if( pIter->startAt==NXT_StartAt_Root_IncludeCurrentThemeAndEffect && pIter->effectNode==pn )
            return 1;
        else
            return 0;
    } else {
        return 1;
    }
}

static NXT_NodeHeader* forEachNodeOfClass_NextFunc(NXT_Iter *pIter) 
{
    if( !pIter ) {
        // TODO: error
        return NULL;
    }
    
    if( !pIter->nextNode ) {
        // Done
        return NULL;
    }
    
    NXT_NodeHeader *pn = pIter->nextNode;
    
    for(;;) {
        if( pn->child && iterIsIncludeNode(pIter,pn) ) {
            pn = pn->child;
            pIter->depth++;
        } else if( pn->next ) {
            pn = pn->next;
        } else {
            while( pn->parent && !pn->parent->next ) {
                pn = pn->parent;
                pIter->depth--;
                if( pn==pIter->rootNode ) {
                    pn = NULL;
                    break;
                }
            }
            if( pn && pn->parent && pn->parent->next && pn->parent!=pIter->rootNode ) {
                pn = pn->parent->next;
                pIter->depth--;
            } else {
                pn = NULL;
                break;
            }
        }
        if( iterIsIncludeNode(pIter,pn) ) {
            if( !pIter->findClass || pn->isa == pIter->findClass ) {
                if( pIter->nextNode && (!pIter->findClass || pIter->nextNode->isa == pIter->findClass) )
                    break;
                pIter->nextNode = pn;
            }
        }
    }
    
    NXT_NodeHeader *returnValue = pIter->nextNode;
    pIter->nextNode = pn;
    return returnValue;
}

NXT_Iter NXT_ForEachNodeOfClass( NXT_NodeHeader *startNode, NXT_NodeClass *nodeClass, NXT_StartAt startAt )
{
    NXT_Iter iter = {0};
    
    if( !startNode ) {
        return iter;
    }
    
    iter.startAt = startAt;
    iter.startNode = startNode;
    iter.findClass = nodeClass;
    NXT_NodeHeader *pn = startNode;
    
    switch( startAt ) {
        case NXT_StartAt_Root:
        case NXT_StartAt_Root_IncludeCurrentTheme:
        case NXT_StartAt_Root_IncludeAllThemes:
        case NXT_StartAt_Root_IncludeCurrentThemeAndEffect:
        case NXT_StartAt_Root_IncludeEverything:
            for(;;) {
                if( pn->isa==&NXT_NodeClass_Theme && !iter.themeNode ) {
                    iter.themeNode=pn;
                } else if( pn->isa==&NXT_NodeClass_Effect && !iter.effectNode ) {
                    iter.effectNode=pn;
                }
                if( pn->parent ) {
                    pn = pn->parent;
                } else {
                    break;
                }
            }
            break;
        case NXT_StartAt_Theme:
        case NXT_StartAt_Theme_IncludeCurrentEffect:
        case NXT_StartAt_Theme_IncludeAllEffects:
            for(;;) {
                if( pn->isa==&NXT_NodeClass_Theme && !iter.themeNode ) {
                    iter.themeNode=pn;
                } else if( pn->isa==&NXT_NodeClass_Effect && !iter.effectNode ) {
                    iter.effectNode=pn;
                }
                if( pn->parent && pn->isa!=&NXT_NodeClass_Theme ) {
                    pn = pn->parent;
                } else {
                    break;
                }
            }
            break;
        case NXT_StartAt_Effect:
            for(;;) {
                if( pn->isa==&NXT_NodeClass_Theme && !iter.themeNode ) {
                    iter.themeNode=pn;
                } else if( pn->isa==&NXT_NodeClass_Effect && !iter.effectNode ) {
                    iter.effectNode=pn;
                }
                if( pn->parent && pn->isa!=&NXT_NodeClass_Theme && pn->isa!=&NXT_NodeClass_Effect) {
                    pn = pn->parent;
                } else {
                    break;
                }
            }
            break;

        case NXT_StartAt_CurrentNode:
        default:
            break;
    }
    iter.nextNode = pn;
    iter.rootNode = pn;
    iter.next = forEachNodeOfClass_NextFunc;
    
    return iter;
}

int NXT_CountChildNotesOfClass( NXT_NodeHeader *node, NXT_NodeClass *nodeClass ) {
    int count = 0;
    NXT_NodeHeader *pn = node;
    while( pn ) {
        if( pn->isa == nodeClass )
            count++;
        pn = NXT_FindNextNodeOfClass(pn, nodeClass);
    }
    return count;
}

void NXT_SetNodeId( NXT_NodeHeader *node, const char *node_id ) {
    if( !node ) {
        // TODO: error
        return;
    }
    if( node->node_id ) {
        free(node->node_id);
        node->node_id = NULL;
    }
    if( node_id ) {
        node->node_id = (char*)malloc(strlen(node_id)+1);
        strcpy(node->node_id, node_id);
    }
}

static int animFloatVectorFromStringInternal( NXT_NodeHeader *node, const char* string, float* vector, int numElements, int *pNumUpdateTargets, NXT_NodeHeader *notifyNode, void* notifyPrivateData );

int NXT_AnimFloatVectorFromStringNotify( NXT_NodeHeader *node, char* string, float* vector, int numElements, int *pNumUpdateTargets, NXT_NodeHeader *notifyNode, void* notifyPrivateData ) {
    return animFloatVectorFromStringInternal( node, string, vector, numElements, pNumUpdateTargets, notifyNode, notifyPrivateData );
}

int NXT_AnimFloatVectorFromStringEx( NXT_NodeHeader *node, const char* string, float* vector, int numElements, int *pNumUpdateTargets ) {
    return animFloatVectorFromStringInternal( node, string, vector, numElements, pNumUpdateTargets, NULL, NULL );
}

int NXT_AnimFloatVectorFromString( NXT_NodeHeader *node, const char* string, float* vector, int numElements ) {
    int numUpdateTargets = 0;
    return animFloatVectorFromStringInternal( node, string, vector, numElements, &numUpdateTargets, NULL, NULL );
}

static void freeRenderItemKeyValues(NXT_RenderItem_KeyValues* pkeyvalues){

    pkeyvalues->clear();
}

static int procParameterKeyValueFromStringInternal(NXT_NodeHeader* node, const char* string, NXT_RenderItem_KeyValues* pkeyvalues){

    //namespace.key -> renderitem global key

    struct NamespaceDef{

        const char* name;
        int item_type;
        int num_of_elements;
    };

    const NamespaceDef def[] = {

        {"color", NXT_Renderitem_KeyValueType_Color, 4},
        {"selection", NXT_Renderitem_KeyValueType_Selection, 4},
        {"choice", NXT_Renderitem_KeyValueType_Choice, 4},
        {"range", NXT_Renderitem_KeyValueType_Range, 1},
        {"rect", NXT_Renderitem_KeyValueType_Rect, 4},
        {"point", NXT_Renderitem_KeyValueType_Point, 3}
    };

    freeRenderItemKeyValues(pkeyvalues);

    const char* s = string;
    std::string name;
    std::string key;
    std::string valuestr;

    //Structure of Data
    //Type
    //Key - string - char*
    //Value - float* - generally 4 elements

    while(*s){

        //get name_space - from this we can find out what elements should be feeded.
        while (*s==' ' || *s=='\t' )
            s++;
        
        name.clear();
        
        while( *s && *s!=NXT_ELEMENT_REF_CHAR && *s!=' ' && *s!='\t' && *s!='.' && *s!='[' && *s!='(' && *s!=',' && *s!='*' && *s!='+' && *s!='-' && *s!='/'  ) {

            name.push_back(*s++);
        }

        int item_type = 0;

        int num_of_elements = 0;

        const char* cmp_name = name.c_str();

        for(int i =0 ; i < sizeof(def)/sizeof(NamespaceDef); ++i){

            if(!strcasecmp(cmp_name, def[i].name)){

                item_type = def[i].item_type;
                num_of_elements = def[i].num_of_elements;
            }
        }
        //get key
        key.clear();
        if(*s == '.'){

            ++s;

            while (*s==' ' || *s=='\t' )
                s++;
            
            while( *s && *s!=NXT_ELEMENT_REF_CHAR && *s!=' ' && *s!='\t' && *s!='.' && *s!='[' && *s!='(' && *s!=',' && *s!='*' && *s!='+' && *s!='-' && *s!='/'  ) {
                
                key.push_back(*s++);
            }
        }

        //get value string
        valuestr.clear();
        while (*s==' ' || *s=='\t' )
            s++;

        while( *s && *s!= ';') {
            
            valuestr.push_back(*s++);
        }

        if(*s) s++;

        NXT_RenderItem_KeyValue* pitem = new NXT_RenderItem_KeyValue();
        pitem->type_ = item_type;

        pitem->key_ = name + "." + key;
        pitem->value_.resize(num_of_elements);
        int count_of_updated_targets = 0;
        int ret = animFloatVectorFromStringInternal(node, valuestr.c_str(), &pitem->value_.front(), num_of_elements, &count_of_updated_targets, NULL, NULL);
        pkeyvalues->items_.push_back(pitem);
    }

    return 0;
}

int NXT_ProcParamKeyValueFromString(NXT_NodeHeader* node, const char* string, NXT_RenderItem_KeyValues* pkeyvalues){

    return procParameterKeyValueFromStringInternal(node, string, pkeyvalues);
}

void NXT_FreeRenderitemKeyValues(NXT_RenderItem_KeyValues* pkeyvalues){

    freeRenderItemKeyValues(pkeyvalues);
}

static int animFloatVectorFromStringInternal( NXT_NodeHeader *node, const char* string, float* vector, int numElements, int *pNumUpdateTargets, NXT_NodeHeader *notifyNode, void* notifyPrivateData ) {
    
    int numUpdateTargets = 0;
    int remainingElements = numElements;
    const char *s = string;
    float *vec = vector;
    int nFound;
    char tagname[256];
    char fargs[512];
    int taglen;
    int fargslen;
    
    LOGXV("[%s %d] Parsing '%s'", __func__, __LINE__, string );


    while( *s && remainingElements > 0 ) {
        nFound = NXT_PartialFloatVectorFromString((const char**)&s, vec, remainingElements);
        remainingElements -= nFound;
        if( vec )
            vec += nFound;
        LOGXV("[%s %d] Found %d elements; remaining=%d; '%s'", __func__, __LINE__, nFound, remainingElements, string );
        if( remainingElements > 0 ) {
            
            while (*s==' ' || *s=='\t' )
                s++;
            
            if( *s == NXT_ELEMENT_REF_CHAR )
            {
                taglen = 0;
                s++;
                while( *s && *s!=NXT_ELEMENT_REF_CHAR && *s!=' ' && *s!='\t' && *s!='.' && *s!='[' && *s!='(' && *s!=',' && *s!='*' && *s!='+' && *s!='-' && *s!='/'  ) {
                    if( taglen < sizeof(tagname)-1 ) {
                        tagname[taglen]=*s;
                        taglen++;
                        s++;
                    }
                }
                
                if( taglen > 0 ) {
                    NXT_NodeHeader *ref = NULL;
                    NXT_NodeHeader *iternode = node;
                    
                    fargslen = 0;
                    if( *s == '(' ) {
                        s++;
                        while( *s && *s!=')' && fargslen<sizeof(fargs)-1 ) {
                            fargs[fargslen]=*s;
                            s++;
                            fargslen++;
                        }
                    }
                    fargs[fargslen] = 0;
                    
                    float factor = 1.0;
                    float bias = 0.0;
                    
                    tagname[taglen++] = 0; // Null termination
                    int element = -1;
                    
                    if( *s == '.' ) {
                        s++;
                        if( *s=='x' || *s=='r' || *s=='s' ) {
                            s++;
                            element = 0;
                        } else if( *s=='y' || *s=='g' || *s=='t' ) {
                            s++;
                            element = 1;
                        } else if( *s=='z' || *s=='b' || *s=='p' ) {
                            s++;
                            element = 2;
                        } else if( *s=='w' || *s=='a' || *s=='q' ) {
                            s++;
                            element = 3;
                        }
                    } else if ( *s == '[' && *(s+1)>='0' && *(s+1)<='3' && *(s+2)==']') {
                        element = *(s+1) - '0';
                        s += 3;
                    }
                    
					int bSpace = 0;
                    while( *s==' ' || *s=='\t' )
                        s++, bSpace=1;
                    
                    if( *s=='*' ) {
                        s++;
                        float value = 1.0;
                        int bFound = NXT_PartialFloatVectorFromString((const char**)&s, &value, 1);
                        if( bFound ) {
                            factor = value;
							bSpace = 0;
						}
                    } else if( *s=='/' ) {
                        s++;
                        float value = 1.0;
                        int bFound = NXT_PartialFloatVectorFromString((const char**)&s, &value, 1);
                        if( bFound ) {
                            factor = (1.0f/value);
							bSpace = 0;
						}
                    }
                    
                    if( *s=='+' && !bSpace ) {
                        s++;
                        float value = 1.0;
                        int bFound = NXT_PartialFloatVectorFromString((const char**)&s, &value, 1);
                        if( bFound )
                            bias = value;
                    } else if( *s=='-' && !bSpace ) {
                        s++;
                        float value = 1.0;
                        int bFound = NXT_PartialFloatVectorFromString((const char**)&s, &value, 1);
                        if( bFound )
                            bias = (1.0f/value);
                    }
                    
                    LOGXV("[%s %d] Looking for '%s' (element=%i)", __func__, __LINE__, tagname, element );
                    
                    if(iternode != NULL){

                        while( iternode->parent ) {
                            iternode = iternode->parent;
                            if( iternode->node_id && strcmp(iternode->node_id,tagname)==0 ) {
                                ref = iternode;
                                break;
                            }
                        }                        
                    }
                    
                    
                    if( !ref && node != NULL) {
                        NXT_Iter iter = NXT_ForEachNodeOfClass(node, NULL, NXT_StartAt_Root_IncludeCurrentThemeAndEffect);
                        while(( iternode = iter.next(&iter) )) {
                            if( iternode->node_id && strcmp(iternode->node_id,tagname)==0 )
                                ref = iternode;
                        }
                    }
                    
                    
                    if( ref ) {
                        if( ref->isa==&NXT_NodeClass_AnimatedValue ) {
                            if( element == -1 ) {
                                int el_count = NXT_Node_AnimatedValue_GetElementCount((NXT_Node_AnimatedValue*)ref);
                                LOGXV("[%s %d] Found '%s' (element=%i) : animated value el_count=%d", __func__, __LINE__, tagname, element, el_count );
                                if( el_count==0 || el_count>=remainingElements ) {
                                    if( vec )
                                        NXT_Node_AnimatedValue_AddUpdateTarget( (NXT_Node_AnimatedValue*)ref, vec, 0, remainingElements, factor, bias, notifyNode, notifyPrivateData );
                                    remainingElements = 0;
                                } else {
                                    if( vec )
                                        NXT_Node_AnimatedValue_AddUpdateTarget( (NXT_Node_AnimatedValue*)ref, vec, 0, el_count, factor, bias, notifyNode, notifyPrivateData );
                                    remainingElements -= el_count;
                                    if( vec )
                                        vec+=el_count;
                                }
                                numUpdateTargets++;
                            } else {
                                LOGXV("[%s %d] Found '%s' (element=%i) : animated value (one element only)", __func__, __LINE__, tagname, element );
                                if( vec )
                                    NXT_Node_AnimatedValue_AddUpdateTarget( (NXT_Node_AnimatedValue*)ref, vec, element, 1, factor, bias, notifyNode, notifyPrivateData );
                                remainingElements--;
                                if( vec )
                                    vec++;
                                numUpdateTargets++;
                            }
                        } else if( ref->isa==&NXT_NodeClass_Sequence ) {
                            if( element == -1 ) {
                                int el_count = NXT_Node_Sequence_GetElementCount((NXT_Node_Sequence*)ref);
                                LOGXV("[%s %d] Found '%s' (element=%i) : sequence el_count=%d", __func__, __LINE__, tagname, element, el_count );
                                if( el_count==0 || el_count>=remainingElements ) {
                                    if( vec )
                                        NXT_Node_Sequence_AddUpdateTarget( (NXT_Node_Sequence*)ref, vec, 0, remainingElements, factor, bias, notifyNode, notifyPrivateData );
                                    remainingElements = 0;
                                } else {
                                    if( vec )
                                        NXT_Node_Sequence_AddUpdateTarget( (NXT_Node_Sequence*)ref, vec, 0, el_count, factor, bias, notifyNode, notifyPrivateData );
                                    remainingElements -= el_count;
                                    if( vec )
                                        vec+=el_count;
                                }
                                numUpdateTargets++;
                            } else {
                                LOGXV("[%s %d] Found '%s' (element=%i) : sequence (one element only)", __func__, __LINE__, tagname, element );
                                if( vec )
                                    NXT_Node_Sequence_AddUpdateTarget( (NXT_Node_Sequence*)ref, vec, element, 1, factor, bias, notifyNode, notifyPrivateData );
                                remainingElements--;
                                if( vec )
                                    vec++;
                                numUpdateTargets++;
                            }
                        } else if( ref->isa==&NXT_NodeClass_UserField ) {
                            if( element == -1 ) {
                                int el_count = NXT_Node_UserField_GetElementCount((NXT_Node_UserField*)ref);
                                LOGXV("[%s %d] Found '%s' (element=%i) : userfield el_count=%d", __func__, __LINE__, tagname, element, el_count );
                                if( el_count==0 || el_count>=remainingElements ) {
                                    if( vec )
                                        NXT_Node_UserField_AddUpdateTarget( (NXT_Node_UserField*)ref, vec, 0, remainingElements, factor, bias, notifyNode, (char*)notifyPrivateData );
                                    remainingElements = 0;
                                } else {
                                    if( vec )
                                        NXT_Node_UserField_AddUpdateTarget( (NXT_Node_UserField*)ref, vec, 0, el_count, factor, bias, notifyNode, (char*)notifyPrivateData );
                                    remainingElements -= el_count;
                                    if( vec )
                                        vec+=el_count;
                                }
                                numUpdateTargets++;
                            } else {
                                LOGXV("[%s %d] Found '%s' (element=%i) : sequence (one element only)", __func__, __LINE__, tagname, element );
                                if( vec )
                                    NXT_Node_UserField_AddUpdateTarget( (NXT_Node_UserField*)ref, vec, element, 1, factor, bias, notifyNode, (char*)notifyPrivateData );
                                remainingElements--;
                                if( vec )
                                    vec++;
                                numUpdateTargets++;
                            }
                        } else if( ref->isa==&NXT_NodeClass_Function ) {
                            if( element == -1 ) {
                                int el_count = NXT_Node_Function_GetElementCount((NXT_Node_Function*)ref);
                                LOGXV("[%s %d] Found '%s' (element=%i) : sequence el_count=%d", __func__, __LINE__, tagname, element, el_count );
                                if( el_count==0 || el_count>=remainingElements ) {
                                    if( vec )
                                        NXT_Node_Function_AddUpdateTarget( (NXT_Node_Function*)ref, vec, 0, remainingElements, factor, bias, node, fargs );
                                    remainingElements = 0;
                                } else {
                                    if( vec )
                                        NXT_Node_Function_AddUpdateTarget( (NXT_Node_Function*)ref, vec, 0, el_count, factor, bias, node, fargs );
                                    remainingElements -= el_count;
                                    if( vec )
                                        vec+=el_count;
                                }
                                numUpdateTargets++;
                            } else {
                                LOGXV("[%s %d] Found '%s' (element=%i) : sequence (one element only)", __func__, __LINE__, tagname, element );
                                if( vec )
                                    NXT_Node_Function_AddUpdateTarget( (NXT_Node_Function*)ref, vec, element, 1, factor, bias, node, fargs );
                                remainingElements--;
                                if( vec )
                                    vec++;
                                numUpdateTargets++;
                            }
                        } else if( ref->isa==&NXT_NodeClass_Const ) {
                            float cv[4] = {0.0, 0.0, 0.0, 0.0};
                            NXT_Node_Const *constNode = (NXT_Node_Const*)ref;
                            int foundElements = NXT_FloatVectorFromString(constNode->value, cv, 4);
                            if( element == -1 ) {
                                LOGXV("[%s %d] Found '%s' (element=%i) : constant value foundElements=%d", __func__, __LINE__, tagname, element, foundElements );
                                int i;
                                for( i=0; i<foundElements; i++ ) {
                                    if( remainingElements>0 ) {
                                        if( vec ) {
                                            *vec = cv[i];
                                            vec++;
                                        }
                                        remainingElements--;
                                    }
                                }
                            } else {
                                LOGXV("[%s %d] Found '%s' (element=%i) : constant value (only one) foundElements=%d", __func__, __LINE__, tagname, element, foundElements );
                                if( vec ) {
                                    *vec = cv[element];
                                    vec++;
                                }
                                remainingElements--;
                            }
                        } else {
                            LOGW("[%s %d] Found '%s' UNKNOWN NODE TYPE", __func__, __LINE__, tagname );
                        }
                    } else {
                        LOGE("[%s %d] Couldn't find node with id '%s'", __func__, __LINE__, tagname );
                    }

                }
                
                while( *s==' ' || *s=='\t' )
                    s++;
                if( *s==',' || *s==')' )
                    s++;
                while( *s==' ' || *s=='\t' )
                    s++;
            }
            else if( *s )
            {
                LOGE("[%s %d] Bad character (0x%02X '%c') in vector '%s'", __func__, __LINE__, *s, *s, string );
                s++;
            }
        }
    }
    
    int elementsFound = numElements-remainingElements;
    if( vec ) {
        while (remainingElements>0) {
            if( numElements==4 && remainingElements==1 )
                *vec = 1.0f;
            else
                *vec = 0.0f;
            vec++;
            remainingElements--;
        }
    }
    
    if( pNumUpdateTargets )
        *pNumUpdateTargets = numUpdateTargets;
    
    return elementsFound;
}

unsigned int NXT_Node_ClassCheck( NXT_NodeHeader *node, NXT_NodeClass *nodeClass ) {
    if( !node )
        return 0;
    if( !node->isa )
        return 0;
    if( node->isa != nodeClass )
        return 0;
    return 1;
}


static NXT_NodeClass *g_nodeClassList[] = {
    &NXT_NodeClass_AnimatedValue,
    &NXT_NodeClass_Effect,
    &NXT_NodeClass_Function,
    &NXT_NodeClass_UserField,
    &NXT_NodeClass_Parameter,
    &NXT_NodeClass_FieldLabel,
    &NXT_NodeClass_Option,
    &NXT_NodeClass_String,
    &NXT_NodeClass_KeyFrame,
    &NXT_NodeClass_Rotate,
    &NXT_NodeClass_Style,
    &NXT_NodeClass_Texture,
    &NXT_NodeClass_Theme,
    &NXT_NodeClass_ThemeSet,
    &NXT_NodeClass_TimingFunc,
    &NXT_NodeClass_TriangleStrip,
    &NXT_NodeClass_Polygon,
    &NXT_NodeClass_Visibility,
    &NXT_NodeClass_Translate,
    &NXT_NodeClass_Point,
    &NXT_NodeClass_Const,
    &NXT_NodeClass_CoordinateSpace,
    &NXT_NodeClass_Scale,
    &NXT_NodeClass_Clear,
    &NXT_NodeClass_Cull,
    &NXT_NodeClass_Circle,
    &NXT_NodeClass_Rect,
    &NXT_NodeClass_Part,
    &NXT_NodeClass_Projection,
    &NXT_NodeClass_Sequence,
    &NXT_NodeClass_RenderItem,
    &NXT_NodeClass_KEDL,
    &NXT_NodeClass_EffectQueue,
    &NXT_NodeClass_EffectItem,
    NULL
};

NXT_NodeClass *NXT_NodeClassByName( char* name ) {
    NXT_NodeClass **ppNodeClass;
    for( ppNodeClass = g_nodeClassList; *ppNodeClass; ppNodeClass++ ) {
        if( strcasecmp((*ppNodeClass)->className, name)==0 ) {
            return *ppNodeClass;
        }
    }
    return NULL;
}