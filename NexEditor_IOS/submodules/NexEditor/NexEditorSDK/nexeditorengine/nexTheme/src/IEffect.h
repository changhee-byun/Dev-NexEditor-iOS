//
//  IEffect.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/26/14.
//
//

#ifndef NexVideoEditor_IEffect_h
#define NexVideoEditor_IEffect_h

#include "NexTheme_Errors.h"
#include "IRenderContext.h"

class IEffect {
    
public:
    IEffect(const IEffect& other) = delete;
    IEffect& operator=( const IEffect& other) = delete;
    IEffect() = default;
    virtual ~IEffect(){};
    
    virtual NXT_Error loadFromString( const std::string& effect_data ) = 0;
    virtual const std::string& get_parse_error() const = 0;
    virtual void render(IRenderContext &renderer) = 0;
    virtual void update_time(IRenderContext &renderer) = 0;
    
};

#endif
