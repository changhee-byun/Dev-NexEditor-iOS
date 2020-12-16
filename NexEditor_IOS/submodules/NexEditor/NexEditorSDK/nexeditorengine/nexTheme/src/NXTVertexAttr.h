//
//  NXTVertexAttr.h
//  NexVideoEditor
//
//  Created by Matthew Feinberg on 8/29/14.
//
//

#ifndef __NexVideoEditor__NXTVertexAttr__
#define __NexVideoEditor__NXTVertexAttr__

#include <iostream>
#include <vector>

class NXTVertexAttr {
    
public:
    NXTVertexAttr( const NXTVertexAttr& other ) = delete;
    NXTVertexAttr& operator=(const NXTVertexAttr& other) = delete;
    NXTVertexAttr() = default;
    
    void set(const float* elements,
             int num_elements,
             int elements_per_vertex,
             bool normalized = false)
    {
        element_ptr_ = elements;
        element_count_ = num_elements;
        elements_per_vertex_ = elements_per_vertex;
    }
    
    template<int NUM_ELEMENTS>
    void set(const float (&elements)[NUM_ELEMENTS],
             int elements_per_vertex = 4,
             bool normalized = false)
    {
        set(&elements[0],NUM_ELEMENTS,elements_per_vertex,normalized);
    }

    void set(const std::vector<float>& elements,
             int elements_per_vertex = 4,
             bool normalized = false)
    {
        set(&elements[0],(int)elements.size(),elements_per_vertex,normalized);
    }
    
    void clear() {
        element_count_ = 0;
    }
    
private:
    const float* element_ptr_;
    int element_count_;
    int elements_per_vertex_;
};

#endif /* defined(__NexVideoEditor__NXTVertexAttr__) */
