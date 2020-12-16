//
//  NXTNodePart.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/14/14.
//
//

#ifndef __NexVideoEditor__NXTNodePart__
#define __NexVideoEditor__NXTNodePart__

#include <iostream>
#include "NXTNode.h"

enum class NXTClipType {
    All,
    First,
    Last,
    Middle,
    Even,
    Odd,
    NotFirst,
    NotLast,
    NotMiddle,
    NotEven,
    NotOdd
};


class NXTNodePart : public NXTNode {

private:
    NXTPart     type        = NXTPart::All;
    NXTClipType clipType    = NXTClipType::All;

public:
    
    static const std::string* CLASS_NAME() {
        static const std::string s("Part");
        return &s;
    }

    virtual const char* nodeClassName() override  {
        return "Part";
    }
    
    NXTPart getType() {
        return type;
    }
    NXTClipType getClipType() {
        return clipType;
    }

    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="type" ) {
            if( attrValue=="in" ) {
                type = NXTPart::In;
            } else if( attrValue=="out" ) {
                type = NXTPart::Out;
            } else if( attrValue=="mid" ) {
                type = NXTPart::Mid;
            } else if( attrValue=="all" ) {
                type = NXTPart::All;
            } else {
                throw NXTNodeParseError("type must be one of: in,out,mid,all");
            }
        } else  if( attrName=="clip" ) {
            if( attrValue == "first" ) {
                clipType = NXTClipType::First;
            } else if( attrValue == "mid" ) {
                clipType = NXTClipType::Middle;
            } else if( attrValue == "middle" ) {
                clipType = NXTClipType::Middle;
            } else if( attrValue == "last" ) {
                clipType = NXTClipType::Last;
            } else if( attrValue == "even" ) {
                clipType = NXTClipType::Even;
            } else if( attrValue == "odd" ) {
                clipType = NXTClipType::Odd;
            } else if( attrValue == "all" ) {
                clipType = NXTClipType::All;
            } else if( attrValue == "!first" ) {
                clipType = NXTClipType::NotFirst;
            } else if( attrValue == "!mid" ) {
                clipType = NXTClipType::NotMiddle;
            } else if( attrValue == "!middle" ) {
                clipType = NXTClipType::NotMiddle;
            } else if( attrValue == "!last" ) {
                clipType = NXTClipType::NotLast;
            } else if( attrValue == "!even" ) {
                clipType = NXTClipType::NotEven;
            } else if( attrValue == "!odd" ) {
                clipType = NXTClipType::NotOdd;
            } else {
                throw NXTNodeParseError("clip must be one of: first,mid,middle,last,even,odd,all,!first,!mid,!middle,!last,!even,!odd");
            }
        }
    }
    
    virtual bool isMatchCurrentContext(IRenderContext &renderer) {
        int clip_count = renderer.getTotalClipCount();
        int clip_index = renderer.getCurrentClipIndex();
        NXTPart part = renderer.getCurrentPart();
        if( type!=NXTPart::All && part != type ) {
            renderer.skipChildren();
        }
        switch( clipType ) {
            case NXTClipType::All:
                return true;;
            case NXTClipType::First:
                return clip_index == 0;
            case NXTClipType::Middle:
                return clip_index > 0 && clip_index < clip_count-1;
            case NXTClipType::Last:
                return clip_index >= clip_count-1;
            case NXTClipType::NotOdd:
            case NXTClipType::Even:
                return (clip_index % 2)==0;
            case NXTClipType::NotEven:
            case NXTClipType::Odd:
                return (clip_index % 2)!=0;
            case NXTClipType::NotFirst:
                return clip_index > 0;
            case NXTClipType::NotMiddle:
                return clip_index <= 0 || clip_index >= clip_count-1;
            case NXTClipType::NotLast:
                return clip_index < clip_count-1;
            default:
                break;
        }
    }
    
    virtual void renderBegin( IRenderContext &renderer ) override  {
        if( !isMatchCurrentContext(renderer) ) {
            renderer.skipChildren();
        }
    }
    
};
#endif /* defined(__NexVideoEditor__NXTNodePart__) */
