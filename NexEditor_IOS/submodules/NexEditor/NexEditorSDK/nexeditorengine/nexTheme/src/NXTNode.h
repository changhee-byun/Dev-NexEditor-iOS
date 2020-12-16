//
//  NXTNode.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/8/14.
//
//

#ifndef __NexVideoEditor__NXTNode__
#define __NexVideoEditor__NXTNode__

#include <iostream>
#include <vector>
#include <stdexcept>
//#include "NexThemeRenderer_Internal.h"
//#include "NXTRenderer.h"
#include "NXTUpdateTarget.h"
#include "NexTheme_Math.h"
#include "NXTVector.h"
#include "NXTImageSettings.h"
#include "IRenderContext.h"

class NXTUpdateTarget;

class NXTNode;
class NXTNodeEffect;
class NXTNodeTheme;


class NXTNodeParseError : public std::exception {
    
public:
    NXTNodeParseError(const char* what) {
        what_ = what;
    }
    
    virtual const char* what() const throw() {
        return what_;
    }
    
private:
    const char* what_;
};

// For parent iteration
class NXTNodeIteratorParent {

private:
    NXTNode *pNode;
    
public:
    
    NXTNodeIteratorParent(NXTNode *pNode ) {
        this->pNode = pNode;
    }
    
    NXTNodeIteratorParent& operator++();
    bool operator!=(NXTNodeIteratorParent const& other) {
        return( other.pNode != pNode );
    }
    bool operator==(NXTNodeIteratorParent const& other) {
        return( other.pNode == pNode );
    }
    NXTNode* operator*() const {
        return pNode;
    }
};

class NXTNodeRangeParent  {
public:
    NXTNodeRangeParent(NXTNode *pNode) {
        this->pNode = pNode;
    }
    NXTNodeIteratorParent begin() {
        return NXTNodeIteratorParent(pNode);
    }
    NXTNodeIteratorParent end() {
        return NXTNodeIteratorParent(nullptr);
    }
private:
    NXTNode *pNode;
};

// For child iteration

class NXTNodeIteratorChildren {
    
private:
    NXTNode *pStartNode;
    NXTNode *pNode;
    bool recursive_;

public:
    
    NXTNodeIteratorChildren(NXTNode *pNode, bool recursive );
    
    NXTNodeIteratorChildren& operator++();
    bool operator!=(NXTNodeIteratorChildren const& other) {
        return( other.pNode != pNode );
    }
    bool operator==(NXTNodeIteratorChildren const& other) {
        return( other.pNode == pNode );
    }
    NXTNode* operator*() const {
        return pNode;
    }
};

class NXTNodeRangeChildren  {
public:
    NXTNodeRangeChildren(NXTNode *pNode, bool recursive) {
        this->pNode = pNode;
        recursive_ = recursive;
    }
    NXTNodeIteratorChildren begin() {
        return NXTNodeIteratorChildren(pNode,recursive_);
    }
    NXTNodeIteratorChildren end() {
        return NXTNodeIteratorChildren(nullptr,recursive_);
    }
private:
    bool recursive_;
    NXTNode *pNode;
};


// NXTNode

class NXTNode {
    
    friend class NXTNodeIteratorChildren;
    friend class NXTEffect;
    
private:
    std::string nodeId;
    NXTNode *child = nullptr;
    NXTNode *next = nullptr;
    NXTNode *parent = nullptr;
    
public:
    virtual const char* nodeClassName() = 0;
    
    enum class Action {
        Next, Repeat, Stop
    };
    
    enum class DataProviderType {
        None,           // Node does not provide any data (referencing it with @id is illegal)
        ConstString,    // Node provides constant string data
        DynamicFloats,  // Mode provides list of dynamically changing floats
        Image
    };
    
    NXTNodeRangeParent parents() {
        return NXTNodeRangeParent(this);
    }
    
    NXTNodeRangeChildren children() {
        return NXTNodeRangeChildren(this,false);
    }
    
    NXTNodeRangeChildren children_recursive() {
        return NXTNodeRangeChildren(this,true);
    }
    
    NXTNode* findNodeById( const std::string& node_id );
    NXTNode* findNodeByRef( const std::string& node_ref );
    NXTNode* findNodeByRef( const std::string& node_ref, DataProviderType providerType );

    NXTNodeEffect* getEffect();
    NXTNodeTheme* getTheme();
    
    NXTNode* getParent() const {
        return parent;
    }
    
    template<typename T>
    bool is() {
        return(*T::CLASS_NAME() == nodeClassName());
    }
    
    void setId( std::string nodeId ) {
        this->nodeId = nodeId;
    }
    
    std::string getId() {
        return nodeId;
    }
    
    virtual ~NXTNode() {
        if( child ) {
            delete child;
            child = nullptr;
        }
        if( next ) {
            delete next;
            next = nullptr;
        }
    }
    
    
    // === Data provider support ===
    //
    // Data providers are nodes that provide data when referenced by ID.
    //
    // getDataProviderType() returns one of:
    //    ConstString    ... use getDataProviderStringData()
    //    DynamicFloats  ... use getDataProviderElementCount() and addUpdateTarget()
    //    Image          ... use getDataProviderImageData()
    //
    
    virtual DataProviderType getDataProviderType() {
        return DataProviderType::None;
    }
    
    virtual std::string getDataProviderStringData() {
        return "";
    }
    
    virtual NXTImageAndSettings getDataProviderImageData() {
        return NXTImageAndSettings();
    }
    
    virtual int getDataProviderElementCount() {
        return 0;
    }
    
    virtual void addUpdateTarget(NXTUpdateTarget& updateTarget,
                                 std::string* params,
                                 NXTNode* paramContext )
    {

    }
    
    virtual void updateNotify( int what ) {
        
    }
    
    // === XML parsing support ===
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) {
        
    }
    virtual void childrenDone() {
        
    }
    
    void add_child( NXTNode* new_child ) {
        if( !new_child ) {
            throw std::invalid_argument("new_child is null");
        } else if(  new_child->parent || new_child->next ) {
            throw std::invalid_argument("new_child is already added");
        }
        if( child ) {
            NXTNode *last_child = child;
            while( last_child->next )
                last_child = last_child->next;
            last_child->next = new_child;
        } else {
            child = new_child;
        }
        new_child->parent = this;
    }

    void add_sibling( NXTNode* new_sibling ) {
        if( !new_sibling ) {
            throw std::invalid_argument("new_sibling is null");
        } else if( new_sibling->parent || new_sibling->next ) {
            throw std::invalid_argument("new_sibling is already added");
        } else if( !parent ) {
            throw std::invalid_argument("no parent; cannot add sibling");
        }
        NXTNode *last_sibling = this;
        while( last_sibling->next )
            last_sibling = last_sibling->next;
        last_sibling->next = new_sibling;
        new_sibling->parent = parent;
    }

    // === Renderer support ===
    
    virtual void freeTexture() {
    }
    
    
    virtual void renderBegin( IRenderContext& renderer ) {
    }
    
    virtual Action renderEnd( IRenderContext& renderer ) {
        return Action::Next;
    }
    
    virtual void precache( IRenderContext& renderer, bool openGLContextAvailable ) {
    }
    
    virtual void preRender( IRenderContext& renderer, NXT_RandSeed& rseed ) {
    }
    
public:
    
    int animFloatVectorFromString( std::string attrValue, float* vector, int numElements ) {
        // TODO
        // Return value is # of elements
        return 0;
    }
    
    template<int NUM_ELEMENTS>
    int animFloatVectorFromString( std::string attrValue, float (&vector)[NUM_ELEMENTS] ) {
        return animFloatVectorFromString(attrValue, vector, NUM_ELEMENTS);
    }
    
    int animFloatVectorFromString( std::string attrValue, NXT_Vector4f& vec ) {
        return animFloatVectorFromString(attrValue, vec.e, sizeof(vec.e)/sizeof(*(vec.e)));
    }
    
    int animFloatVectorFromString( std::string attrValue, NXTVector& vec ) {
        return animFloatVectorFromString(attrValue, &vec[0], 4);
    }
    
    int animFloatVectorFromString( std::string attrValue, float& el ) {
        return animFloatVectorFromString(attrValue, &el, 1);
    }
    
    int floatVectorFromString( const std::string& attrValue, float* vector, int numElements ) const {
        // TODO
        // Return value is # of elements
        // (should support CONST and TIMINGFUNCTION nodes)
        return 0;
    }
    
    void animFloatVectorFromString( const std::string& attrValue, std::vector<float> &vector, bool *hasUpdateTargets = nullptr ) const {
        // TODO
    }
    
    
    template<int NUM_ELEMENTS>
    int floatVectorFromString( const std::string& attrValue, float (&vector)[NUM_ELEMENTS]) const {
        return floatVectorFromString(attrValue, vector, NUM_ELEMENTS);
    }
    
    int floatVectorFromString( const std::string& attrValue, NXT_Vector4f& vec ) const {
        return floatVectorFromString(attrValue, vec.e, sizeof(vec.e)/sizeof(*(vec.e)));
    }
    
    int floatVectorFromString( const std::string& attrValue, NXTVector& vec ) const {
        return floatVectorFromString(attrValue, &vec[0], 4);
    }
    
    int floatVectorFromString( const std::string& attrValue, float& el ) const {
        return floatVectorFromString(attrValue, &el, 1);
    }
    
    void deleteChildNodes() {
        // TODO
    }
    
    void intFromString( std::string attrValue, int& output,
                       int minvalue=INT_MIN, int maxvalue=INT_MAX ) {
        float el;
        animFloatVectorFromString(attrValue, el);
        if( el < minvalue )
            el = minvalue;
        if( el > maxvalue )
            el = maxvalue;
        output = (int)el;
    }
    
    void intFromString( std::string attrValue, long long int& output,
                       long long minvalue=LONG_LONG_MIN, long long maxvalue=LONG_LONG_MAX ) {
        float el;
        animFloatVectorFromString(attrValue, el);
        if( el < minvalue )
            el = minvalue;
        if( el > maxvalue )
            el = maxvalue;
        output = (long long int)el;
    }
    
    
    void intFromString( std::string attrValue, unsigned int& output,
                       int minvalue=0, int maxvalue=UINT_MAX ) {
        float el;
        animFloatVectorFromString(attrValue, el);
        if( el < minvalue )
            el = minvalue;
        if( el > maxvalue )
            el = maxvalue;
        output = (unsigned int)el;
    }
    
    
    void parseBool( std::string s, bool& output ) {
        if( s=="" || s=="on" || s=="yes" || s=="true" || s=="1") {
            output = true;
        } else if( s=="no" || s=="off" || s=="0" || s=="false" ) {
            output = false;
        }
    }

    void parseBool( std::string s, unsigned int& flagset, unsigned int flag ) {
        if( s=="" || s=="on" || s=="yes" || s=="true" || s=="1") {
            flagset |= flag;
        } else if( s=="no" || s=="off" || s=="0" || s=="false" ) {
            flagset &= ~flag;
        }
    }
    
    void parseTransformTarget( const std::string& s, NXTTransformTarget& target ) {
        if( s == "vertex" ) {
            target = NXTTransformTarget::Vertex;
        } else if( s == "texture" ) {
            target = NXTTransformTarget::Texture;
        } else if( s == "mask" ) {
            target = NXTTransformTarget::Mask;
        } else if( s == "color" ) {
            target = NXTTransformTarget::Color;
        } else if( s == "masksample" ) {
            target = NXTTransformTarget::MaskSample;
        } else {
            throw NXTNodeParseError("type must be one of: vertex,texture,mask,color,masksample");
        }

    }

};

bool isNodeRef( const std::string& str );

#endif /* defined(__NexVideoEditor__NXTNode__) */
