//
//  EffectOptionEditor.h
//  APIDemos
//
//  Created by Simon Kim on 2/2/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

#ifndef EffectOptionEditor_h
#define EffectOptionEditor_h

@import NexEditorFramework;

typedef void (^EffectOptionConfiguredBlock)(void);

@protocol EffectOptionEditor
- (void)setupWithEffectOption:(NXEEffectOptions *)effectOption
                  selectedIdx:(int)selectedIdx
                   effectType:(NXEEffectType)effectType
                   configured:(EffectOptionConfiguredBlock)configured;
@end


#endif /* EffectOptionEditor_h */
