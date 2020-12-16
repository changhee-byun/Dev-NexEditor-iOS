#ifndef __NexVideoEditor__NXTNodeUserField__
#define __NexVideoEditor__NXTNodeUserField__

#include <vector>
#include <iostream>
#include "NXTNode.h"
#include "NXTRenderer.h"
#include "NexTheme_Util.h"

class NXTNodeUserField : public NXTNode {
    
private:
    NXT_UserFieldType type = NXT_UserFieldType_UNDEFINED;
    int element_count = 0;
    std::string fieldDefault;
    std::vector<NXTUpdateTarget> updateTargets;

public:
    
    virtual const char* nodeClassName() override  {
        return "UserField";
    }

    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="default" ) {
            fieldDefault = attrValue;
        } else if( attrName=="type" ) {
            if( attrValue=="text" ) {
                type = NXT_UserFieldType_Text;
                element_count = 0;
            } else if( attrValue=="color" ) {
                type = NXT_UserFieldType_Color;
                element_count = 4;
            } else if( attrValue=="selection" ) {
                type = NXT_UserFieldType_Selection;
                element_count = 4;
            } else if( attrValue=="overlay" ) {
                type = NXT_UserFieldType_Overlay;
                element_count = 4;
            } else {
                throw NXTNodeParseError("type must be one of: text,color,selection,overlay");
            }
        }
    }
    
    int getElementCount() {
        return element_count;
    }
    
    void processUpdateTargets( IRenderContext &renderer ) {
        if( element_count < 1 ) {
            return;
        }
        
//        std::string option = renderer->getEffectOption(getId(), fieldDefault);
        std::string option;
        if( renderer.getActiveEffect().effectOptions.count(getId()) < 1 ) {
            option = fieldDefault;
        } else {
            option = renderer.getActiveEffect().effectOptions[getId()];
        }
        
        if( option == "" ) {
            return;
        }

        const char *pOption = option.c_str();
        float e[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        NXT_PartialFloatVectorFromString(&pOption, e, 4);
        
        for( auto target: updateTargets ) {
            target.publishUpdate(e);
        }

    }
    
    virtual void addUpdateTarget(NXTUpdateTarget& updateTarget,
                                 std::string* params,
                                 NXTNode* paramContext ) override 
    {
        updateTargets.push_back(updateTarget);
    }
    
};

#endif /* defined(__NexVideoEditor__NXTNodeUserField__) */
