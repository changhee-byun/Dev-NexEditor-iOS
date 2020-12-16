//
//  NXTNodeAnimatedValue.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/4/14.
//
//

#include "NXTNodeAnimatedValue.h"
#include "simplexnoise1234.h"
#include "NXTBezier.h"

void NXTNodeAnimatedValue::preRender( IRenderContext& renderer, NXT_RandSeed& rseed ) {
 
    float time;
    NXTTimingFunction timing;
    
    switch( basis ) {
        case NXTInterpBasis::ClipIndex:
            timing.setLinear();
            time = (float)renderer.getCurrentClipIndex() / ((float)renderer.getTotalClipCount()-1.0);
            break;
        case NXTInterpBasis::ClipIndexPermuted:
            timing.setLinear();
            time = NXT_FRandom(&rseed);
            break;
        case NXTInterpBasis::Time:
        default:
            timing = this->timing;
            if( bUseOverallTime ) {
                time = renderer.getEffectProgressOverall();
            } else {
                time = renderer.getEffectProgressInPart();
            }
            break;
    }
    
    if( noiseType == NXTNoiseType::PerlinSimplex ) {
        int octave;
        float factor = 1.0;
        float noise = 0.0;
        float maxval = 0.0;
        float p = time*noiseSampleScale + noiseSampleBias;
        switch( noiseFunc ) {
            case NXTNoiseFunction::Fractal:
                for( octave = 0; octave < noiseOctaves; octave++ ) {
                    noise += (1.0/factor)*simplex_noise1(p*factor);
                    maxval += (1.0/factor);
                    factor++;
                }
                time = (noise/maxval)*noiseScale + noiseBias;
                break;
            case NXTNoiseFunction::Turbulence:
                for( octave = 0; octave < noiseOctaves; octave++ ) {
                    float n = simplex_noise1(p*factor);
                    if( n<0 )
                        noise += (1.0/factor)*(-n);
                    else
                        noise += (1.0/factor)*n;
                    maxval += (1.0/factor);
                    factor++;
                }
                time = (noise/maxval)*noiseScale + noiseBias;
                break;
            case NXTNoiseFunction::WaveFractal:
                for( octave = 0; octave < noiseOctaves; octave++ ) {
                    float n = simplex_noise1(p*factor);
                    if( n<0 )
                        noise += (1.0/factor)*(-n);
                    else
                        noise += (1.0/factor)*n;
                    factor++;
                }
                noise = sinf( p + noise );
                time = noise*noiseScale + noiseBias;
                break;
            case NXTNoiseFunction::Pure:
            default:
                time = simplex_noise1(p)*noiseScale + noiseBias;
                break;
        }
        if( time < 0.0 )
            time = 0.0;
        if( time > 1.0 )
            time = 1.0;
    }
    
//    float e[4] = {0};
    
    NXTVector v;

    NXTKeyframeSet *kfset = nullptr;
    NXTKeyframeSet *pDefaultSet = nullptr;
    int clip_index = renderer.getCurrentClipIndex();
    int clip_count = renderer.getTotalClipCount();
    bool skip;
    
    
    
    for( NXTKeyframeSet& s: keyframeSets ) {
        skip = false;
        if( s.bDefault ) {
            pDefaultSet = &s;
            skip = true;
        }
        if( s.partType != NXTPart::All && renderer.getCurrentPart() != s.partType ) {
            skip = true;
        }
        switch (s.clipType) {
            case NXTClipType::All:                                                                                                      break;
            case NXTClipType::First:        skip = skip || ( clip_index != 0 );                                                         break;
            case NXTClipType::Middle:       skip = skip || ( clip_count>0 && clip_index <= 0 && clip_index>=(clip_count-1) );           break;
            case NXTClipType::Last:         skip = skip || ( clip_index<(clip_count-1) );                                               break;
            case NXTClipType::Even:         skip = skip || ( (clip_index % 2)!=0 );                                                     break;
            case NXTClipType::Odd:          skip = skip || ( (clip_index % 2)==0 );                                                     break;
            case NXTClipType::NotFirst:     skip = skip || ( clip_index == 0 );                                                         break;
            case NXTClipType::NotMiddle:    skip = skip || ( !(clip_count>0 && clip_index <= 0 && clip_index>=(clip_count-1)) );        break;
            case NXTClipType::NotLast:      skip = skip || ( clip_index>=(clip_count-1) );                                              break;
            case NXTClipType::NotEven:      skip = skip || ( (clip_index % 2)==0 );                                                     break;
            case NXTClipType::NotOdd:       skip = skip || ( (clip_index % 2)!=0);                                                      break;
            default:
                break;
        }
        if( !skip ) {
            kfset = &s;
            break;
        }
    }
    
    if( !kfset && pDefaultSet )
        kfset = pDefaultSet;
    
    if( !kfset || kfset->keyframes.empty() ) {
        v.set(0, 0, 0, 0);
    } else if( kfset->keyframes.size() == 1 ) {
        v = kfset->keyframes[0].value;
    } else if( kfset->keyframes.size() > 1 ) {
        
        if( time < kfset->keyframes[0].t ) {
            v = kfset->keyframes[0].value;
        } else if( time > kfset->keyframes.back().t ) {
            v = kfset->keyframes.back().value;
        } else {
            int numKeyFrames = kfset->keyframes.size();
            float animTotal = kfset->keyframes.back().t - kfset->keyframes[0].t;
            float animElapsed = time - kfset->keyframes[0].t;
            float progress = animElapsed/animTotal;
            float interp = NXTBezier::cubicBezierAtTime(progress,
                                                 timing[0],timing[1],
                                                 timing[2],timing[3],
                                                 10.0);
            float adjtime = kfset->keyframes[0].t + (animTotal*interp);
            
            for( int i=0; i<numKeyFrames-1; i++ ) {
                if( adjtime >= kfset->keyframes[i].t && adjtime <= kfset->keyframes[i+1].t  ) {
                    float elapsedTime = adjtime - kfset->keyframes[i].t;
                    float totalTime = kfset->keyframes[i+1].t - kfset->keyframes[i].t;
                    float factor;
                    if( totalTime<=0.0 ) {
                        factor = 1.0;
                    } else {
                        factor = elapsedTime/totalTime;
                        factor = NXTBezier::cubicBezierAtTime(factor,
                                                       kfset->keyframes[i+1].timing[0],kfset->keyframes[i+1].timing[1],
                                                       kfset->keyframes[i+1].timing[2],kfset->keyframes[i+1].timing[3],
                                                       10.0);
                    }
                    v.set(kfset->keyframes[i].value[0] + (kfset->keyframes[i+1].value[0] - kfset->keyframes[i].value[0])*factor,
                          kfset->keyframes[i].value[1] + (kfset->keyframes[i+1].value[1] - kfset->keyframes[i].value[1])*factor,
                          kfset->keyframes[i].value[2] + (kfset->keyframes[i+1].value[2] - kfset->keyframes[i].value[2])*factor,
                          kfset->keyframes[i].value[3] + (kfset->keyframes[i+1].value[3] - kfset->keyframes[i].value[3])*factor);
                    break;
                }
            }
        }
        
    }
    
    for( NXTUpdateTarget target: updateTargets ) {
        target.publishUpdate(v);
    }
    
    // TODO: Check if functions are handled properly???
    
    
}
