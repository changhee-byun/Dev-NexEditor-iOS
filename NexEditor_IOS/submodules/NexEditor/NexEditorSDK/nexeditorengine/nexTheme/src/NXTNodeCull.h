#ifndef __NexVideoEditor__NXTNodeCull__
#define __NexVideoEditor__NXTNodeCull__

#include <iostream>
#include "NXTNode.h"
#include "IRenderContext.h"

class NXTNodeCull : public NXTNode {

private:
    NXTCullFace    cullFace        = NXTCullFace::None;
    NXTCullFace    prevCullFace    = NXTCullFace::None;
    
public:

    virtual const char* nodeClassName() override {
        return "Cull";
    }

    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override {
        if( attrName == "face" ) {
            if( attrValue == "none" ) {
                cullFace = NXTCullFace::None;
            } else if( attrValue == "back" ) {
                cullFace = NXTCullFace::Back;
            } else if( attrValue == "front" ) {
                cullFace = NXTCullFace::Front;
            } else if( attrValue == "all" ) {
                cullFace = NXTCullFace::All;
            } else {
                throw NXTNodeParseError("face must be one of: none,back,front,all");
            }
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override {
        prevCullFace = renderer.getCullFace();
        renderer.setCullFace(cullFace);
        applyCull(cullFace);
        
    }
    virtual Action renderEnd( IRenderContext& renderer ) override {
        renderer.setCullFace(prevCullFace);
        applyCull(prevCullFace);
        return Action::Next;
    }

private:
    void applyCull(NXTCullFace cull) {
        // TODO: Move inside IRenderContext::setCullFace
        if( cull == NXTCullFace::None ) {
            glDisable(GL_CULL_FACE);
        } else if( cull == NXTCullFace::Back) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        } else if( cull == NXTCullFace::Front) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        } else if( cull == NXTCullFace::All) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT_AND_BACK);
        }
    }

};


#endif /* defined(__NexVideoEditor__NXTNodeCull__) */
