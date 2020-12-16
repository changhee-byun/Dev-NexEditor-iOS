//
//  NXTNodeEffect.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/9/14.
//
//

#ifndef __NexVideoEditor__NXTNodeEffect__
#define __NexVideoEditor__NXTNodeEffect__

#include <iostream>
#include "NXTNode.h"
#include "NXTRenderer.h"

class NXTNodeEffect : public NXTNode {
    
private:
    NXT_EffectType      effectType          = NXT_EffectType_NONE;
    std::string         name                = "";
    std::string         categoryTitle       = "";
    std::string         icon                = "";
    int                 effectOffset        = 0;
    int                 videoOverlap        = 0;
    int                 minDuration         = 500;
    int                 maxDuration         = 2000;
    int                 defaultDuration     = 1000;
    int                 inTime              = 2000;
    int                 outTime             = 2000;
    int                 inTimeFirst         = 0;
    int                 outTimeLast         = 0;
    int                 cycleTime           = 2000;
    int                 maxTitleDuration    = 0;
    NXT_RepeatType      repeatType          = NXT_RepeatType_None;
    bool                bUserDuration       = true;
    bool                bSetOffset          = false;
    bool                bSetOverlap         = false;
    
public:
    
    static const std::string* CLASS_NAME() {
        static const std::string s("Effect");
        return &s;
    }

    virtual const char* nodeClassName() override  {
        return "Effect";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="name" ) {
            name = attrValue;
        } else if( attrName=="icon" ) {
            icon = attrValue;
        } else if( attrName=="categoryTitle" ) {
            categoryTitle = attrValue;
        } else if( attrName=="type" ) {
            if( attrValue=="transition" ) {
                effectType = NXT_EffectType_Transition;
                if( !bSetOffset ) {
                    effectOffset = -100;
                }
                if( !bSetOverlap ) {
                    videoOverlap = 100;
                }
            } else if( attrValue=="title" ) {
                effectType = NXT_EffectType_Title;
                if( !bSetOffset ) {
                    effectOffset = 0;
                }
                if( !bSetOverlap ) {
                    videoOverlap = 0;
                }
            } else {
                throw NXTNodeParseError("type must be one of: transition,title");
            }
        } else if( attrName=="repeat" ) {
            if( attrValue=="no" || attrValue=="none" ) {
                repeatType = NXT_RepeatType_None;
            } else if( attrValue=="yes" || attrValue=="repeat" ) {
                repeatType = NXT_RepeatType_Repeat;
            } else if( attrValue=="sawtooth" ) {
                repeatType = NXT_RepeatType_Sawtooth;
            } else if( attrValue=="sine" ) {
                repeatType = NXT_RepeatType_Sine;
            } else {
                throw NXTNodeParseError("repeat must be one of: no,none,yes,repeat,sawtooth,sine");
            }
        } else if( attrName=="userduration" ) {
            parseBool(attrValue, bUserDuration);
        } else if( attrName=="intime" ) {
            inTime = atoi(attrValue.c_str());
        } else if( attrName=="outtime" ) {
            outTime = atoi(attrValue.c_str());
        } else if( attrName=="intimefirst" ) {
            inTimeFirst = atoi(attrValue.c_str());
        } else if( attrName=="outtimelast" ) {
            outTimeLast = atoi(attrValue.c_str());
        } else if( attrName=="cycleTime" ) {
            cycleTime = atoi(attrValue.c_str());
        } else if( attrName=="minduration" ) {
            minDuration = atoi(attrValue.c_str());
        } else if( attrName=="maxduration" ) {
            maxDuration = atoi(attrValue.c_str());
        } else if( attrName=="maxtitleduration" ) {
            maxTitleDuration = atoi(attrValue.c_str());
        } else if( attrName=="defaultduration" ) {
            defaultDuration = atoi(attrValue.c_str());
        } else if( attrName=="effectoffset" ) {
            effectOffset = atoi(attrValue.c_str());
            bSetOffset = true;
        } else if( attrName=="videooverlap" ) {
            videoOverlap = atoi(attrValue.c_str());
            bSetOverlap = true;
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override {
        
    }
    virtual Action renderEnd( IRenderContext& renderer ) override {
        return Action::Next;
    }
    
    
};
#endif /* defined(__NexVideoEditor__NXTNodeEffect__) */
