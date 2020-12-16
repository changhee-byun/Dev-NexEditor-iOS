//
//  NXTNode.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/8/14.
//
//

#include "NXTNode.h"
#include "NXTNodeEffect.h"
#include "NXTNodeTheme.h"

NXTNodeIteratorParent& NXTNodeIteratorParent::operator++() {
    if( pNode ) {
        pNode = pNode->getParent();
    }
    return *this;
}

NXTNodeIteratorChildren::NXTNodeIteratorChildren( NXTNode *pNode, bool recursive ) {
    this->pNode = pNode->child;
    this->pStartNode = pNode;
    recursive_ = recursive;
}

NXTNodeIteratorChildren& NXTNodeIteratorChildren::operator++() {
    if( pNode ) {
        if( recursive_ ) {
            if( pNode->child ) {
                pNode = pNode->child;
            } else if( pNode->next ) {
                pNode = pNode->next;
            } else {
                while( pNode->parent && pNode->parent!=pStartNode && !pNode->parent->next ) {
                    pNode = pNode->parent;
                }
                if( pNode->parent && pNode->parent!=pStartNode && pNode->parent->next ) {
                    pNode==pNode->parent->next;
                } else {
                    pNode==nullptr;
                }
            }
        } else {
            pNode = pNode->next;
        }
    }
    return *this;
}

NXTNodeEffect* NXTNode::getEffect() {
    for( NXTNode* node: parents() ) {
        if( node->is<NXTNodeEffect>())
            return static_cast<NXTNodeEffect*>(node);
    }
    return nullptr;
}

NXTNodeTheme* NXTNode::getTheme() {
    for( NXTNode* node: parents() ) {
        if( node->is<NXTNodeTheme>())
            return static_cast<NXTNodeTheme*>(node);
    }
    return nullptr;
}

NXTNode* NXTNode::findNodeByRef( const std::string& node_ref ) {
    if( isNodeRef(node_ref) ) {
        return findNodeById(node_ref.substr(1));
    } else {
        return nullptr;
    }
}

NXTNode* NXTNode::findNodeByRef( const std::string& node_ref, DataProviderType providerType ) {
    if( isNodeRef(node_ref) ) {
        NXTNode* pNode = findNodeById(node_ref.substr(1));
        if( pNode && pNode->getDataProviderType()==providerType ) {
            return pNode;
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

NXTNode* NXTNode::findNodeById( const std::string& node_id ) {
    NXTNodeEffect *effect = getEffect();
    if( effect ) {
        for( NXTNode* child: effect->children_recursive() ) {
            if( child->getId() == node_id ) {
                return child;
            }
        }
    }
    return nullptr;
}
bool isNodeRef( const std::string& str ) {
    return ( !str.empty() && str[0]=='@' );
}

