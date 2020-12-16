//
//  NXTEffectOptions.cpp
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 12/1/14.
//
//

#include "NXTEffectOptions.h"

static int parseint( const char* s, const char **tail ) {
    int result = 0;
    while( *s >='0' && *s <='9' ) {
        result *= 10;
        result += (*s - '0');
        s++;
    }
    if( tail ) {
        *tail = s;
    }
    return result;
}

static int isHexDigit( char c ) {
    if( c>='0' && c<='9' ) {
        return 1;
    } else if( c>='A' && c<='F' ) {
        return 1;
    } else if( c>='a' && c<='f' ) {
        return 1;
    } else {
        return 0;
    }
}

static int hexDigitValue( char c ) {
    if( c>='0' && c<='9' ) {
        return c - '0';
    } else if( c>='A' && c<='F' ) {
        return c - 'A' + 10;
    } else if( c>='a' && c<='f' ) {
        return c - 'a' + 10;
    } else {
        return 0;
    }
}

NXTEffectOptions::~NXTEffectOptions() {
    free_options();
}

const char* get(const char* key) {
    // TODO: getEffectOption
    return nullptr;
}

void NXTEffectOptions::parse(const char *options) {
    
    free_options();
    
    int effectOptionsLen = 0;
    const char *p;
    const char *effectOptionsEncoded = options;
    const char *tail = 0;
    
    p = effectOptionsEncoded;

    // Read effect start and end times
    while( *p==' ' )
        p++;
    effect_user_start_time_ = parseint(p, &tail);
    while( *tail==' ' )
        tail++;
    if( tail!=p && *tail==',' ) {
        p = tail+1;
        while( *p==' ' )
            p++;
        effect_user_end_time_ = parseint(p, &tail);
        while( *tail==' ' )
            tail++;
        if( tail != p && *tail=='?' ) {
            p = tail+1;
            while( *p==' ' )
                p++;
        } else {
            effect_user_start_time_ = 0;
            effect_user_end_time_ = 0;
        }
    } else {
        effect_user_start_time_ = 0;
        effect_user_end_time_ = 0;
    }
    
    // Count number of options
    effectOptionsEncoded = p;

    effect_option_count_ = 0;
    transition_option_count_ = 0;
    clip_effect_option_count_ = 0;
    int bTransition = 1;
    for( p = effectOptionsEncoded; *p; p++ ) {
        effectOptionsLen++;
        if( *p=='?' ) {
            bTransition=0;
        } else if( *p=='=' ) {
            effect_option_count_++;
            if( bTransition ) {
                transition_option_count_++;
            } else {
                clip_effect_option_count_++;
            }
        }
    }

    effect_options_buffer_ = new char[effectOptionsLen+1];
    effect_option_keys_ = new char*[effect_option_count_];
    effect_option_values_ = new char*[effect_option_count_];

    if( !effect_option_values_ || !effect_option_keys_ || !effect_options_buffer_ ) {
        free_options();
        return;
    }

    char* buf = effect_options_buffer_;
    char* pkey = nullptr;
    char* pvalue = nullptr;
    char** pkeylist = effect_option_keys_;
    char** pvaluelist = effect_option_values_;

    for( p = effectOptionsEncoded; *p; p++ ) {
        if( pkey==nullptr && pvalue==nullptr ) {
            pkey = buf;
        }
        if( *p == '=' ) {
            *buf++ = 0;
            pvalue = buf;
        } else if( *p == '&' ) {
            if( pvalue && pkey ) {
                *buf++ = 0;
                *pkeylist++ = pkey;
                *pvaluelist++ = pvalue;
            }
            pvalue = nullptr;
            pkey = nullptr;
        } else if( *p == '?' ) {
            if( pvalue && pkey ) {
                *buf++ = 0;
                *pkeylist++ = pkey;
                *pvaluelist++ = pvalue;
            }
            pvalue = nullptr;
            pkey = nullptr;
        } else if( *p == '+' ) {
            *buf++ = ' ';
        } else if( *p == '%' ) {
            if( isHexDigit(*(p+1)) && isHexDigit(*(p+2)) ) {
                *buf++ = ((hexDigitValue(*(p+1))<<4) | hexDigitValue(*(p+2)));
                p += 2;
            }
        } else {
            *buf++ = *p;
        }
    }
    if( pvalue && pkey ) {
        *buf++ = 0;
        *pkeylist++ = pkey;
        *pvaluelist++ = pvalue;
    }

}

void NXTEffectOptions::free_options() {
    if( effect_options_buffer_ ) {
        delete [] effect_options_buffer_;
        effect_options_buffer_ = nullptr;
    }
    if( effect_option_keys_ ) {
        delete [] effect_option_keys_;
        effect_option_keys_ = nullptr;
    }
    if( effect_option_values_ ) {
        delete [] effect_option_values_;
        effect_option_values_ = nullptr;
    }
}

