#pragma once
#include "Vectors.h"
#include "glext_loader.h"
#include <vector>
#include <list>

class RenderItem;



namespace RI{

    enum
    {
        MAX_RENDER_TARGET_RESOURCE = 15,
    };

    struct RI_ThemeRenderer_;

    struct NXT_RenderTarget
    {   
        GLuint fbo_;
        GLuint depth_buffer_;
        GLuint target_texture_;
        int width_;
        int height_;
        int alpha_;
        int depth_;
        int stencil_;
        int inited_;

        NXT_RenderTarget():fbo_(0), depth_buffer_(0), target_texture_(0), width_(0), height_(0), alpha_(0), depth_(0), stencil_(0), inited_(0){

        }

        ~NXT_RenderTarget(){

        }

        bool init(int w, int h, int alpha_request, int depth_request, int stencil_request);

        void destroy(unsigned int isDetachedConext);
    };

    struct RenderTargetResource{

        RenderTargetResource():prender_target_(NULL), unused_counter_(0){

        }

        ~RenderTargetResource(){

        }

        NXT_RenderTarget* prender_target_;
        int unused_counter_;
    };

    class RenderTargetManager{

    public:
        RenderTargetManager(){

        }
        ~RenderTargetManager(){};
        void destroy();
        NXT_RenderTarget* getRenderTarget(int width, int height, int alpha_request, int depth_request, int stencil_request);
        void releaseRenderTarget(NXT_RenderTarget* prendertarget);
        void update();
    private:
        typedef std::list<RenderTargetResource> RenderTargetResourceList_t;
        RenderTargetResourceList_t resource_;
    };

    typedef std::vector<NXT_RenderTarget*> RenderTargetStack_t;

    struct RI_ThemeRenderer_ {

        //Jeff RTT------------------------------------------------------------------------------------------
        
        //--------------------------------------------------------------------------------------------------
        unsigned int view_width;
        unsigned int view_height;
        int max_texture_size;
        GLboolean colorMask[4];


        void drawPRS(NXT_RenderTarget* psrc, const Vector3& pos, const Vector3& scale, const Vector3& rotate_axis, float angle, float scr_ratio_w, float scr_ratio_h);
        void drawMATRIX(NXT_RenderTarget* psrc, float* pmatrix, float left, float top, float right, float bottom, float alpha);
        void drawSolid(float* pmatrix, float left, float top, float right, float bottom);
        void drawTexBox(int tex, bool mask_enabled, float* pmatrix, float left, float top, float right, float bottom, float alpha, int no_filter = 0, int reverse = 0);
        void drawTexBoxWTexCoord(int tex, bool mask_enabled, float* pmatrix, float* ltrb, float* txcoord, float alpha, int no_filter = 0, int reverse = 0);
        RI_ThemeRenderer_();
        ~RI_ThemeRenderer_();

        NXT_RenderTarget* getRenderTarget(int width, int height, int alpha_request, int depth_request, int stencil_request);
        void releaseRenderTarget(NXT_RenderTarget* prender_target);
        void releaseRenderTarget(NXT_RenderTarget* prender_target, unsigned int isDetachedConext);
        void updateRenderTargetManager();
        int initRenderTargetElement(NXT_RenderTarget* prender_target, int w, int h, int alpha_request, int depth_request, int stencil_request);
        void setRenderTargetAsDefault(NXT_RenderTarget* prender_target, bool set_immediately = true);
        void setRenderTarget(NXT_RenderTarget* prender_target);
        void useProgram(GLuint program_id);
        int getRenderTargetStackPoint();
        void pushRenderTarget(NXT_RenderTarget* prender_target);
        void releaseRenderTargetStackToThePoint(int point);
        void setCurrentRenderItem(RenderItem* prenderitem);
        RenderItem* getCurrentRenderItem();
        NXT_RenderTarget* getDefaultRenderTarget();
        void resetToDefaultRenderTarget();
        void resetCurrentProgramID();
        void updateDefaultTexID(GLuint mask, GLuint blend, GLuint white);
        GLuint getWhiteTexID();
        GLuint getMaskTexID();
        GLuint getBlendTexID();
        void setDefaultFramebufferNum(unsigned int default_framebuffer_num);
    private:
        RenderTargetManager render_target_manager_;
        RenderTargetStack_t render_target_stack_;

        RenderItem* pcurrent_render_item_;
        NXT_RenderTarget* pdefault_render_target_;

        GLuint current_program_id_;
        GLuint current_mask_id_;
        GLuint current_blend_id_;
        GLuint white_tex_id_;
        
        unsigned int default_framebuffer_num_;

        struct TexShader{

            GLuint program_;
            GLuint mvp_matrix_;
            GLuint tex_sampler_;
            GLuint mask_sampler_;
            GLuint reverse_;
            GLuint alpha_;
            GLuint a_pos_;
            GLuint a_tex_;

            TexShader():program_(0), mvp_matrix_(0), tex_sampler_(0), mask_sampler_(0), alpha_(0), a_pos_(0), a_tex_(0), reverse_(0){

            }

            ~TexShader();

            void init();
        };

        struct SolidShader{

            GLuint program_;
            GLuint mvp_matrix_;
            GLuint a_pos_;

            SolidShader():program_(0), mvp_matrix_(0), a_pos_(0){

            }

            ~SolidShader();

            void init();
        };

        TexShader tex_;
        SolidShader solid_;
    };

    typedef struct RI_ThemeRenderer_ NXT_ThemeRenderer;
    typedef struct RI_ThemeRenderer_ *NXT_HThemeRenderer;
};

