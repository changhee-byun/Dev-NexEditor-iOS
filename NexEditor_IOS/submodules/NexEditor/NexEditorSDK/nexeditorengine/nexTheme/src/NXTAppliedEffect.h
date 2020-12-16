//
//  NXTAppliedEffect.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 7/25/14.
//
//

#ifndef __NexVideoEditor__NXTAppliedEffect__
#define __NexVideoEditor__NXTAppliedEffect__

#include <iostream>
#include <map>
#define NXT_ALLOW_DIRECT_THEME_ACCESS
#include "NexTheme.h"
#undef NXT_ALLOW_DIRECT_THEME_ACCESS

class NXTAppliedEffect {
  
public:
    std::string effectId;
	NXT_EffectType effectType   = NXT_EffectType_NONE;
    int clip_index              = 0;
    int clip_count              = 0;
	int bEffectChanged          = 0;
    int clipStartTime           = 0;
    int clipEndTime             = 0;
    int effectStartTime         = 0;
    int effectEndTime           = 0;
    int effectUserStartTime     = 0;
    int effectUserEndTime       = 0;
    int changeSerial            = 0;
    
    
    // Effect options
    std::map<std::string,std::string>   effectOptions;

};

#endif /* defined(__NexVideoEditor__NXTAppliedEffect__) */
