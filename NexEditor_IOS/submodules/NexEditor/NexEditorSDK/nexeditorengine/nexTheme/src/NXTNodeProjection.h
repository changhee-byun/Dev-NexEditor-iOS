#ifndef __NexVideoEditor__NXTNodeProjection__
#define __NexVideoEditor__NXTNodeProjection__

#include <iostream>
#include "NXTNode.h"

enum class NXTProjectionType {
    NONE,Ortho,Frustum,Perspective,TitleBottom,TitleTop,TitleLeft,TitleRight,TitleMiddle
};

class NXTNodeProjection : public NXTNode {
    
private:
    NXTProjectionType   projectionType = NXTProjectionType::NONE;
    NXTMatrix           saved_proj;
    float               left = 0.0;
    float               right = 0.0;
    float               top = 0.0;
    float               bottom = 0.0;
    float               near = 0.0;
    float               far = 0.0;
    float               fov = 0.0;
    float               aspect = 0.0;

public:
    
    virtual const char* nodeClassName() override  {
        return "Projection";
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="type" ) {
            if( attrValue == "ortho" ) {
                projectionType = NXTProjectionType::Ortho;
            } else if( attrValue == "perspective" ) {
                projectionType = NXTProjectionType::Perspective;
            } else if( attrValue == "frustum" ) {
                projectionType = NXTProjectionType::Frustum;
            } else if( attrValue == "title-bottom" ) {
                projectionType = NXTProjectionType::TitleBottom;
            } else if( attrValue == "title-top" ) {
                projectionType = NXTProjectionType::TitleTop;
            } else if( attrValue == "title-left" ) {
                projectionType = NXTProjectionType::TitleLeft;
            } else if( attrValue == "title-right" ) {
                projectionType = NXTProjectionType::TitleRight;
            } else if( attrValue == "title-middle" ) {
                projectionType = NXTProjectionType::TitleMiddle;
            } else {
                throw NXTNodeParseError("type does not specify a valid projection");
            }
        } else if( attrName == "left" ) {
            animFloatVectorFromString(attrValue, left);
        } else if( attrName == "right" ) {
            animFloatVectorFromString(attrValue, right);
        } else if( attrName == "top" ) {
            animFloatVectorFromString(attrValue, top);
        } else if( attrName == "bottom" ) {
            animFloatVectorFromString(attrValue, bottom);
        } else if( attrName == "near" ) {
            animFloatVectorFromString(attrValue, near);
        } else if( attrName == "far" ) {
            animFloatVectorFromString(attrValue, far);
        } else if( attrName == "fov" ) {
            animFloatVectorFromString(attrValue, fov);
        } else if( attrName == "aspect" ) {
            animFloatVectorFromString(attrValue, aspect);
        }
    }
    
    virtual void renderBegin( IRenderContext& renderer ) override  {
        
        saved_proj = renderer.getProjectionMatrix();
        int view_width = renderer.getViewWidth();
        int view_height = renderer.getViewHeight();
        
        float aspect;
        
        if( aspect==0.0 ) {
            if( view_width > 0 && view_height > 0 ) {
                aspect = (float)view_width / (float)view_height;
            } else {
                aspect = 4.0/3.0;
                
            }
        } else {
            aspect = this->aspect;
        }
        
        switch( projectionType ) {
            case NXTProjectionType::Perspective:
                renderer.getProjectionMatrix() = NXTMatrix::perspective(fov, aspect,
                                                                   near, far);
                break;
            case NXTProjectionType::Frustum:
                renderer.getProjectionMatrix() = NXTMatrix::frustum(left, right,
                                                               bottom, top,
                                                               near, far);
                break;
            case NXTProjectionType::Ortho:
                renderer.getProjectionMatrix() = NXTMatrix::ortho(left, right,
                                                             bottom, top,
                                                             near, far);
                break;
            case NXTProjectionType::TitleTop:
            {
                NXTMatrix proj = NXTMatrix::ortho(-aspect, aspect,
                                                       -1.0, 1.0,
                                                       1.0, 10.0);
                NXTMatrix translate = NXTMatrix::translate( -aspect, 1.0, 0.0 );
                NXTMatrix scale = NXTMatrix::scale( 2.0*aspect, 2.0*aspect, 1.0 );
                NXTMatrix view = scale * translate;
                
                renderer.getProjectionMatrix() =  view * proj;
                break;
            }
            case NXTProjectionType::TitleMiddle:
            {
                NXTMatrix proj = NXTMatrix::ortho(-aspect, aspect,
                                                       -1.0, 1.0,
                                                       1.0, 10.0);
                NXTMatrix translate = NXTMatrix::translate( 0.0, 0.0, 0.0 );
                NXTMatrix scale = NXTMatrix::scale( 2.0*aspect, 2.0*aspect, 1.0 );
                NXTMatrix view = scale * translate;
                
                renderer.getProjectionMatrix() = view * proj;
                break;
            }
            case NXTProjectionType::TitleBottom:
            {
                NXTMatrix proj = NXTMatrix::ortho(-aspect, aspect,
                                                       -1.0, 1.0,
                                                       1.0, 10.0);
                NXTMatrix translate = NXTMatrix::translate( -aspect, -1.0, 0.0 );
                NXTMatrix scale = NXTMatrix::scale( 2.0*aspect, 2.0*aspect, 1.0 );
                NXTMatrix view = scale * translate;
                
                renderer.getProjectionMatrix() = view * proj;
                
                break;
            }
            case NXTProjectionType::TitleLeft:
            {
                NXTMatrix proj = NXTMatrix::ortho(-aspect, aspect,
                                                       -1.0, 1.0,
                                                       1.0, 10.0);
                NXTMatrix translate = NXTMatrix::translate( -aspect, -1.0, 0.0 );
                NXTMatrix scale = NXTMatrix::scale( 2.0, 2.0, 1.0 );
                NXTMatrix view = scale * translate;
                
                renderer.getProjectionMatrix() = view * proj;
                
                break;
            }
            case NXTProjectionType::TitleRight:
            {
                NXTMatrix proj = NXTMatrix::ortho(-aspect, aspect,
                                                       -1.0, 1.0,
                                                       1.0, 10.0);
                NXTMatrix translate = NXTMatrix::translate( aspect, -1.0, 0.0 );
                NXTMatrix scale = NXTMatrix::scale( 2.0, 2.0, 1.0 );
                NXTMatrix view = scale * translate;
                
                renderer.getProjectionMatrix() = view * proj;
                
                break;
            }
        }
        
    }
    virtual Action renderEnd( IRenderContext& renderer ) override  {
        renderer.getProjectionMatrix() = saved_proj;
        return Action::Next;
    }

    
};
#endif /* defined(__NexVideoEditor__NXTNodeProjection__) */
