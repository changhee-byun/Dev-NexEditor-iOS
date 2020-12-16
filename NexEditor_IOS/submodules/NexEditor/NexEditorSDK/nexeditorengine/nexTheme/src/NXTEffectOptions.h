//
//  NXTEffectOptions.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 12/1/14.
//
//

#ifndef __NexVideoEditor__NXTEffectOptions__
#define __NexVideoEditor__NXTEffectOptions__

#include <stdio.h>

class NXTEffectOptions {
    
public:
    
    void parse( const char* options );
    
    NXTEffectOptions(){};
    ~NXTEffectOptions();
    
    NXTEffectOptions(const NXTEffectOptions&) = delete;
    NXTEffectOptions& operator=( NXTEffectOptions const&) = delete;
    
    const char* get(const char* key);
    
private:
    int effect_user_start_time_ = 0;
    int effect_user_end_time_ = 0;
    int effect_option_count_ = 0;
    int transition_option_count_ = 0;
    int clip_effect_option_count_ = 0;
    char *effect_options_buffer_;
    char **effect_option_keys_;
    char **effect_option_values_;
    void free_options();
    
};

#endif /* defined(__NexVideoEditor__NXTEffectOptions__) */
