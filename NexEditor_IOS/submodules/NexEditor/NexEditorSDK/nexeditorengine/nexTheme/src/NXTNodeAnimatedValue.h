#ifndef __NexVideoEditor__NXTNodeAnimatedValue__
#define __NexVideoEditor__NXTNodeAnimatedValue__

#include <iostream>

#include "NXTNode.h"
#include "NXTNodeKeyframe.h"
#include "NXTNodePart.h"
#include "NXTTimingFunction.h"

enum class NXTInterpBasis {
    Time,ClipIndex,ClipIndexPermuted
};

enum class NXTNoiseType {
    None,PerlinSimplex
};

enum class NXTNoiseFunction {
    Pure,           // noise(p)
    Fractal,        // noise(p) + (1/2)noise(2p) + (1/4)noise(4p) ...
    Turbulence,     // abs(noise(p)) + abs((1/2)noise(2p)) + abs((1/4)noise(4p)) ...
    WaveFractal,    // sin( n + abs(noise(p)) + abs((1/2)noise(2p)) + abs((1/4)noise(4p)) ... )
};


class NXTKeyframe {
public:
    NXTKeyframe(){};
    NXTKeyframe(NXTVector value, float t, NXTTimingFunction timing){
        this->value     = value;
        this->t         = t;
        this->timing    = timing;
    };
    NXTVector value;
    float t;
    NXTTimingFunction timing;
};

class NXTKeyframeSet {
public:
    std::vector<NXTKeyframe> keyframes;
    NXTPart             partType;
    NXTClipType         clipType;
    bool                bDefault;
};

class NXTNodeAnimatedValue : public NXTNode {
    
private:
    int                 element_count = 0;
    bool                bUseOverallTime = false;
    NXTInterpBasis      basis = NXTInterpBasis::Time;
    int                 noiseOctaves = 4;
    NXTNoiseType        noiseType = NXTNoiseType::None;
    float               noiseBias = 0.5;
    float               noiseScale = 0.5;
    float               noiseSampleBias = 0.0;
    float               noiseSampleScale = 1.0;
    NXTNoiseFunction    noiseFunc = NXTNoiseFunction::Pure;
    NXTTimingFunction   timing;
    float               startTime = 0.0;
    float               endTime = 1.0;
    NXTNode*            altChildNode = nullptr;
    std::vector<NXTKeyframeSet> keyframeSets;
    std::vector<NXTUpdateTarget> updateTargets;

public:
    
    static const std::string* CLASS_NAME() {
        static const std::string s("AnimatedValue");
        return &s;
    }
    
    virtual const char* nodeClassName() override  {
        return "AnimatedValue";
    }
    
    NXTNodeAnimatedValue() : timing(0.25, 0.10, 0.25, 1.0) {
        
    }
    
    virtual void setAttr( const std::string& attrName, const std::string& attrValue ) override  {
        if( attrName=="timingfunction" ) {
            timing.setFromString(*this, attrValue);
        } else if( attrName=="src" ) {
            if(isNodeRef(attrValue)) {
                altChildNode = findNodeById(attrValue.substr(1));
            }
        } else if( attrName=="scope" ) {
            if( attrValue=="part" ) {
                bUseOverallTime = false;
            } else if( attrValue=="whole" ) {
                bUseOverallTime = true;
            } else {
                throw NXTNodeParseError("scope must be one of: part,whole");
            }
            
        } else if( attrName=="basis" ) {
            if( attrValue=="time" ) {
                basis = NXTInterpBasis::Time;
            } else if( attrValue=="clipindex" ) {
                basis = NXTInterpBasis::ClipIndex;
            } else if( attrValue=="clipindexshuffle" ) {
                basis = NXTInterpBasis::ClipIndexPermuted;
            } else {
                throw NXTNodeParseError("basis must be one of: time,clipindex,clipindexshuffle");
            }
        } else if( attrName=="noise" ) {
            if( attrValue=="none" ) {
                noiseType = NXTNoiseType::None;
            } else if( attrValue=="perlin-simplex" ) {
                noiseType = NXTNoiseType::PerlinSimplex;
            } else {
                throw NXTNodeParseError("noise must be one of: none,perlin-simplex");
            }
        } else if( attrName == "noisefunction") {
            if( attrValue == "pure") {
                noiseFunc = NXTNoiseFunction::Pure;
            } else if( attrValue == "fractal") {
                noiseFunc = NXTNoiseFunction::Fractal;
            } else if( attrValue == "turbulence") {
                noiseFunc = NXTNoiseFunction::Turbulence;
            } else if( attrValue == "wavefractal") {
                noiseFunc = NXTNoiseFunction::WaveFractal;
            } else {
                throw NXTNodeParseError("noisefunction must be one of: pure,fractal,turbulence,wavefractal");
            }
        } else if( attrName == "start") {
            floatVectorFromString(attrValue, startTime);
        } else if( attrName == "end") {
            floatVectorFromString(attrValue, endTime);
        } else if( attrName == "noisebias") {
            floatVectorFromString(attrValue, noiseBias);
        } else if( attrName == "noisescale") {
            floatVectorFromString(attrValue, noiseScale);
        } else if( attrName == "noisesamplebias") {
            floatVectorFromString(attrValue, noiseSampleBias);
        } else if( attrName == "noisesamplescale") {
            floatVectorFromString(attrValue, noiseSampleScale);
        } else if( attrName == "noiseoctaves") {
            intFromString(attrValue, noiseOctaves,1,32);
        }
    }
    
private:
    
    void processKeyframeSet(NXTNode& node, NXTKeyframeSet& kfset ) {
        
        for( NXTNode* pNode: node.children() ) {
            if( pNode->is<NXTNodeKeyframe>() ) {
                NXTNodeKeyframe* pkf = static_cast<NXTNodeKeyframe*>(pNode);
                kfset.keyframes.push_back(NXTKeyframe(pkf->value,
                                                      startTime + (pkf->time*(endTime-startTime)),
                                                      pkf->timingFunction));
                if( pkf->element_count > element_count ) {
                    element_count = pkf->element_count;
                }
            }
        }
        
        // Interpolate unspecified values
        int numzeroes = 0;
        float nzval = 0.0;
        int i;
        for( i=0; i<kfset.keyframes.size(); i++ ) {
            float wkval = kfset.keyframes[i].t;
            
            // Limit to 0...1
            if( wkval > 1.0 )
                wkval = 1.0;
            else if( wkval < 0.0 )
                wkval = 0.0;
            
            if( wkval == 0.0 && i==kfset.keyframes.size()-1 )
                wkval = 1.0;
            if( wkval == 0.0 && i>0 ) {
                numzeroes++;
            } else {
                if( numzeroes ) {
                    float step = (wkval - nzval)/((float)(numzeroes+1));
                    int j;
                    for( j=i-numzeroes; j<i; j++ ) {
                        kfset.keyframes[j].t = step * (float)(j-(i-numzeroes)+1);
                    }
                }
                if( wkval > nzval ) {
                    nzval = wkval;
                }
                kfset.keyframes[i].t = nzval;
            }
        }

        
    }
    
public:
    
    virtual void childrenDone() {
        
        bool keyFrameFound = false;
        
        for( NXTNode* pNode: children() ) {
            if( pNode->is<NXTNodePart>() ) {
                NXTNodePart* pPartNode = static_cast<NXTNodePart*>(pNode);
                keyframeSets.resize(keyframeSets.size()+1);
                keyframeSets.back().partType = pPartNode->getType();
                keyframeSets.back().clipType = pPartNode->getClipType();
                keyframeSets.back().bDefault = false;
                processKeyframeSet( *this, keyframeSets.back() );
            } else if( pNode->is<NXTNodeKeyframe>() && !keyFrameFound ) {
                keyFrameFound=true;
                keyframeSets.resize(keyframeSets.size()+1);
                keyframeSets.back().partType = NXTPart::All;
                keyframeSets.back().clipType = NXTClipType::All;
                keyframeSets.back().bDefault = true;
                processKeyframeSet( *this, keyframeSets.back() );
            }
        }
        
        deleteChildNodes();
        
    }
    
    virtual DataProviderType getDataProviderType() {
        return DataProviderType::DynamicFloats;
    }
    
    virtual int getDataProviderElementCount() {
        return element_count;
    }
    
    virtual void preRender( IRenderContext& renderer, NXT_RandSeed& rseed );
    
    virtual void addUpdateTarget(NXTUpdateTarget& updateTarget,
                                 std::string* params,
                                 NXTNode* paramContext )
    {
        updateTargets.push_back(updateTarget);
    }


};
#endif /* defined(__NexVideoEditor__NXTNodeAnimatedValue__) */
