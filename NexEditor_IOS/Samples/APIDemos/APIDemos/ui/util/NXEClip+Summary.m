//
//  NXEClip+Summary.m
//  APIDemos
//
//  Created by Simon Kim on 11/29/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

#import "NXEClip+Summary.h"
#import "Util.h"
#import "LUTNames.h"
#import "APIDemos-Swift.h"

@implementation NXEClip (Summary)

- (NSString *) envelopSummaryText
{
    NSString *summary = @"[]";
    NXEAudioEnvelop *envelop = self.audioEnvelop;
    
    if (envelop.volumeEnvelopeTime.count > 0) {
        int time = [envelop.volumeEnvelopeTime[0] intValue];
        int value = [envelop.volumeEnvelopeLevel[0] intValue];
        summary = [NSString stringWithFormat:@"%@,%d", [StringTools formatTimeMs:time], value];
        if ( envelop.volumeEnvelopeTime.count > 1 ) {
            summary = [summary stringByAppendingString:@",..."];
        }
        
        summary = [NSString stringWithFormat:@"[%@]", summary];
    }
    return summary;
}

- (NSString *) summaryText
{
    NXEClip *clip = self;
    
    NSString *clipEffect = [self.clipEffectID componentsSeparatedByString:@"."].lastObject;
    NSString *transitionEffect = [self.transitionEffectID componentsSeparatedByString:@"."].lastObject;
    
    int startTrimTime = clip.headTrimDuration;
    int endTrimTime = clip.trimEndTime;
    
    NSString *trimTimeinfo = @"[-]";
    if(startTrimTime > 0 || endTrimTime > 0) {
        trimTimeinfo = [NSString stringWithFormat:@"[%@ ~ %@]",
                        [StringTools formatTimeMs:startTrimTime], [StringTools formatTimeMs:endTrimTime]];
    }
    
    int showStartTime=0; int showEndTime=0;
    [clip getEffectShowTime:&showStartTime endTime:&showEndTime];
    
    NSString *summary = [NSString stringWithFormat:
                         @"%@, Color(B:%d C:%d S:%d)\nFilter(%@),Vignette:%@\nEffect:%@, Transition:%@\nEffect Duratioin:%@",
                         [StringTools formatTimeMs:clip.durationMs],
                         clip.vinfo.brightness,
                         clip.vinfo.contrast,
                         clip.vinfo.saturation,
                         [LUTNames nameForLutId:clip.lutId],
                         clip.vinfo.vignette == 1 ? @"ON" : @"OFF",
                         clipEffect,
                         transitionEffect,
                         [StringTools formatTimeMs:showEndTime]
                         ];
    
    if (clip.clipType == NXE_CLIPTYPE_VIDEO) {
        NSString *videoSummary = [NSString stringWithFormat:
                                  @"\nTrim:%@, Speed:%d, Volume:%d",
                                  trimTimeinfo,
                                  clip.vinfo.speedControl,
                                  clip.vinfo.clipVolume
                                  ];
        summary = [summary stringByAppendingString:videoSummary];
        
        NSString *audioSummary = [NSString stringWithFormat:
                                  @"\nCompressor:%@,Pan:L/%@,R/%@,Pitch:%d\nVoice:%d,Envelop:%@",
                                  clip.vinfo.compressor == 4 ? @"ON" : @"OFF",
                                  [NSNumber numberWithInt:clip.vinfo.panLeft],
                                  [NSNumber numberWithInt:clip.vinfo.panRight],
                                  clip.vinfo.pitch,
                                  clip.vinfo.voiceChanger,
                                  self.envelopSummaryText];
        summary = [summary stringByAppendingString:audioSummary];
        
    }
    return summary;
}
@end
