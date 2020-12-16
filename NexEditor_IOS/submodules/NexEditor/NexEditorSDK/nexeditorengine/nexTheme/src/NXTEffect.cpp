//
//  NXTEffect.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/26/14.
//
//

#include "NXTEffect.h"
#include <cstring>
#include "NXTNodeAnimatedValue.h"
#include "NXTNodeEffect.h"
#include "NXTNodeFunction.h"
#include "NXTNodeUserField.h"
#include "NXTNodeFieldLabel.h"
#include "NXTNodeOption.h"
#include "NXTNodeKeyFrame.h"
#include "NXTNodeRotate.h"
#include "NXTNodeStyle.h"
#include "NXTNodeTexture.h"
#include "NXTNodeTheme.h"
#include "NXTNodeThemeSet.h"
#include "NXTNodeTimingFunction.h"
#include "NXTNodeTriangleStrip.h"
#include "NXTNodePolygon.h"
#include "NXTNodeVisibility.h"
#include "NXTNodeTranslate.h"
#include "NXTNodePoint.h"
#include "NXTNodeConst.h"
#include "NXTNodeCoordinateSpace.h"
#include "NXTNodeScale.h"
#include "NXTNodeClear.h"
#include "NXTNodeCull.h"
#include "NXTNodeCircle.h"
#include "NXTNodeRect.h"
#include "NXTNodePart.h"
#include "NXTNodeProjection.h"
#include "NXTNodeSequence.h"

typedef NXTNode* (*ElementFactoryFunction)();

template <typename T>
static NXTNode* elementFactory() {
    return new T();
}


static struct ElementTableEntry {
    const char* name;
    ElementFactoryFunction factory;
} elementTable[] = {
    "AnimatedValue",    elementFactory<NXTNodeAnimatedValue>,
    "Effect",           elementFactory<NXTNodeEffect>,
    "Function",         elementFactory<NXTNodeFunction>,
    "UserField",        elementFactory<NXTNodeUserField>,
    "FieldLabel",       elementFactory<NXTNodeFieldLabel>,
    "Option",           elementFactory<NXTNodeOption>,
    "KeyFrame",         elementFactory<NXTNodeKeyframe>,
    "Rotate",           elementFactory<NXTNodeRotate>,
    "Style",            elementFactory<NXTNodeStyle>,
    "Texture",          elementFactory<NXTNodeTexture>,
    "Theme",            elementFactory<NXTNodeTheme>,
    "ThemeSet",         elementFactory<NXTNodeThemeSet>,
    "TimingFunc",       elementFactory<NXTNodeTimingFunction>,
    "TriangleStrip",    elementFactory<NXTNodeTriangleStrip>,
    "Polygon",          elementFactory<NXTNodePolygon>,
    "Visibility",       elementFactory<NXTNodeVisibility>,
    "Translate",        elementFactory<NXTNodeTranslate>,
    "Point",            elementFactory<NXTNodePoint>,
    "Const",            elementFactory<NXTNodeConst>,
    "CoordinateSpace",  elementFactory<NXTNodeCoordinateSpace>,
    "Scale",            elementFactory<NXTNodeScale>,
    "Clear",            elementFactory<NXTNodeClear>,
    "Cull",             elementFactory<NXTNodeCull>,
    "Circle",           elementFactory<NXTNodeCircle>,
    "Rect",             elementFactory<NXTNodeRect>,
    "Part",             elementFactory<NXTNodePart>,
    "Projection",       elementFactory<NXTNodeProjection>,
    "Sequence",         elementFactory<NXTNodeSequence>
};

NXT_Error NXTEffect::loadFromString( const std::string& effect_data ) {
    
    if( is_effect_loaded_ ) {
        return NXT_Error_InvalidState;
    }
    
    effect_data_.assign(effect_data.begin(), effect_data.end());
    
    rapidxml::xml_document<> doc;
    try {
        doc.parse<0>(&effect_data_[0]);
        root_node_ = process_node(&doc);
    } catch( const rapidxml::parse_error& err ) {
        int line=1;
        int col=0;
        char prev=0;
        char* line_start = nullptr;
        for( char& c : effect_data_ ) {
            if( c==0x0d || (c==0x0a && prev!=0x0d) ) {
                line++;
                line_start = (&c)+1;
            }
            if( &c >= err.where<char>() ) {
                col = (&c)-line_start;
                break;
            }
            prev = c;
        }
        std::stringstream errmsg;
        errmsg << "effect " << line << ":" << col << ": " << err.what();
        parse_error_ = errmsg.str();
        return NXT_Error_InvalidXML;
    }
    
    
    if( !root_node_ ) {
        parse_error_ = "effect 1:0: no valid root node found";
        return NXT_Error_Syntax;
    }
    
    is_effect_loaded_ = true;
}

NXTEffect::~NXTEffect() {
    if( root_node_ ) {
        delete root_node_;
        root_node_ = nullptr;
    }
}

void NXTEffect::render(IRenderContext &renderer) {
    
    if(!root_node_)
        return;
    
    update_time(renderer);
    
    NXTNode* pn = root_node_;

    for( NXTNode* child: pn->children_recursive() ) {
        if( child->is<NXTNodeTexture>() ) {
            child->renderBegin(renderer);
        }
    }
    
    NXTNode::Action action = NXTNode::Action::Next;
    
    while( pn && action!=NXTNode::Action::Stop ) {
        
        renderer.skipChildren(false);
        
        if( action!=NXTNode::Action::Repeat )
            pn->renderBegin(renderer);
        
        action = NXTNode::Action::Next;
        
        if( pn->child && !renderer.isSkipChildren() ) {
            
            pn = pn->child;
            
        } else if( pn->next ) {
            
            action = pn->renderEnd(renderer);
            if( action!=NXTNode::Action::Next )
                continue;
            pn = pn->next;
            
        } else {
            
            while( pn->parent && !pn->parent->next ) {
                action = pn->renderEnd(renderer);
                if( action!=NXTNode::Action::Next )
                    break;
                pn = pn->parent;
            }
            
            if( action!=NXTNode::Action::Next )
                continue;
            
            action = pn->renderEnd(renderer);
            
            if( action!=NXTNode::Action::Next )
                continue;

            if( pn->parent && pn->parent->next ) {
                
                pn = pn->parent;
                action = pn->renderEnd(renderer);
                if( action!=NXTNode::Action::Next )
                    continue;
                pn = pn->next;
                
            } else {
                break;
            }
        }
    }
    
}

void NXTEffect::update_time(IRenderContext &renderer) {
    
    if( !root_node_ )
        return;
    
    int clip_index = renderer.getCurrentClipIndex();
    int clip_count = renderer.getTotalClipCount();
    
    // TODO: Effect seeking
    //    float overallTime = (float)cur_time / (float)renderer->max_time;
    //
    //    NXT_Effect_Seek( effect, renderer->time, /*renderer->settime*/overallTime, renderer->part, renderer->clip_index, renderer->clip_count );

    NXT_RandSeed rseed;
    NXT_SRandom( &rseed, 0x1E32A7B * clip_index, 0x59B1C13 + clip_index + clip_count );
    
    int i;
    for( i=0; i<clip_index; i++ ) {
        NXT_FRandom(&rseed);
    }
    
//    const std::string pre_render_order[] = {
//        *NXTNodeAnimatedValue::CLASS_NAME(),
//        *NXTNodeAnimatedValue::CLASS_NAME(),
//    };

    for( NXTNode* pn: root_node_->children_recursive() ) {
        pn->preRender(renderer,rseed);
    }
    
}

NXTNode* NXTEffect::process_node( rapidxml::xml_node<>* node ) {
    
    if( node->type()!=rapidxml::node_element )
        return nullptr;

    std::auto_ptr<NXTNode> kedl_node;
    
    for( auto& entry: elementTable ) {
        if( strcasecmp(entry.name,node->name())==0 ) {
            kedl_node = std::auto_ptr<NXTNode>(entry.factory());
            break;
        }
    }
    
    if( !kedl_node.get() )
        return nullptr;
    
    for (rapidxml::xml_attribute<> *attr = node->first_attribute();
         attr; attr = attr->next_attribute())
    {
        try {
            kedl_node->setAttr(attr->name(), attr->value());
        } catch( const NXTNodeParseError& err ) {
            throw rapidxml::parse_error(err.what(),attr->name());
        }
    }

    NXTNode *last_child = nullptr;
    for(rapidxml::xml_node<>* child_node = node->first_node();
        child_node; child_node = child_node->next_sibling() )
    {
        NXTNode *child_kedl_node = process_node(child_node);
        if( child_kedl_node ) {
            if( last_child ) {
                last_child->add_sibling(child_kedl_node);
            } else {
                kedl_node->add_child(child_kedl_node);
            }
            last_child = child_kedl_node;
        }
    }
    
    try {
        kedl_node->childrenDone();
    } catch( const NXTNodeParseError& err ) {
        throw rapidxml::parse_error(err.what(),node->name());
    }

    return kedl_node.release();
}

const std::string& NXTEffect::get_parse_error() const  {
    return parse_error_;
}
