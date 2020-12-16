//
//  NXTEffect.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/26/14.
//
//

#ifndef __NexVideoEditor__NXTEffect__
#define __NexVideoEditor__NXTEffect__

#include <iostream>
#include "IEffect.h"
#include "NXTNode.h"
#include "rapidxml-1.13/rapidxml.hpp"

class NXTEffect : public IEffect {
    
public:
    
    // From IEffect
    virtual NXT_Error loadFromString( const std::string& effect_data ) override;
    virtual ~NXTEffect();
    virtual const std::string& get_parse_error() const override;
    virtual void render(IRenderContext &renderer) override;
    virtual void update_time(IRenderContext &renderer) override;
    
private:
    
    bool is_effect_loaded_ = false;
    std::vector<char> effect_data_;
    std::string parse_error_;
    
    NXTNode* root_node_ = nullptr;

    NXTNode* process_node( rapidxml::xml_node<>* node );
    
};

#endif /* defined(__NexVideoEditor__NXTEffect__) */
