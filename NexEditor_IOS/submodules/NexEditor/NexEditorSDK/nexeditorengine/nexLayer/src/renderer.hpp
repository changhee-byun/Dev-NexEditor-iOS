#pragma once

#include "NexSAL_Internal.h"
#include "NexTheme_Config.h"

class renderer : public RendererInterface{

private:

	template <typename T>
	static T getFirst(T a, T b){

		return a;
	}

private:
	unsigned int			state_stack_point_;
	vector<StateStackEntry>	state_stack_;
	typedef map<unsigned int, SimpleRenderInterface*> bakermap_t;
	bakermap_t 				bakermap_for_preview_;
	bakermap_t 				bakermap_for_export_;
	//reference shaders
	BaseShader				base_shader_;
	ColorConvShader			color_conv_shader_;
	OverlayShader			overlay_shader_;
	UserAlphaShader			useralpha_shader_;
	ChromakeyShader			chromakey_shader_;
	MaskShader				mask_shader_;
	ColorLutShader			colorlut_shader_;
	BlurShaderHorizontal	hblur_shader_;
	BlurShaderVertical		vblur_shader_;
	MosaicShader			mosaic_shader_;
#ifndef __APPLE__
	BaseShaderExternal		base_shader_external_;
	ColorConvShader			color_conv_shader_external_;
	OverlayShader			overlay_shader_external_;
	UserAlphaShader			useralpha_shader_external_;
	ChromakeyShader			chromakey_shader_external_;
	MaskShader				mask_shader_external_;
	ColorLutShader			colorlut_shader_external_;
#endif
	int screen_width_;
	int screen_height_;

	int frame_width_;
	int frame_height_;

	float texture_size_width_;
	float texture_size_height_;
	
	float scale_x_;
	float scale_y_;

	unsigned int	mask_tex_id_;
	unsigned int	lut_tex_id_;

	ChromakeyInfo	chromakey_info_;

	int				current_time_;
	float			matrix_[16];
	float			tex_matrix_[16];
	float 			vertices_[16];
	float 			texcoords_[8];
	ColorMatrix		color_matrix_;
	int 			render_mode_;
	int				render_target_;
	float			alpha_;
	float			alphatest_value_;
	float 			hue_;
	bool			ztest_mode_;
	float			effect_strength_;
	bool			chromakey_viewmask_mode_;
	unsigned int 	attrib_flag_;
	float			color_matrix_for_render_[16];
	ColorMatrix 	fill_rect_color_matrix_;
	float 			identity_matrix_[16];

	void resetTexcoords(float repeat_x, float repeat_y){

		texcoords_[0] = 0.0f;		texcoords_[1] = repeat_y;
		texcoords_[2] = repeat_x;	texcoords_[3] = repeat_y;
		texcoords_[4] = 0.0f;		texcoords_[5] = 0.0f;
		texcoords_[6] = repeat_x;	texcoords_[7] = 0.0f;
	}

public:

	renderer(){

		state_stack_point_ = 0;
		screen_width_ = 0;
		screen_height_ = 0;
		frame_width_ = 0;
		frame_height_ = 0;
		texture_size_width_ = 0;
		texture_size_height_ = 0;
		scale_x_ = 1.0f;
		scale_y_ = 1.0f;
		mask_tex_id_ = 0;
		hue_ = 0.0f;
		
		current_time_ = 0;
		render_mode_ = RenderMode::Preview;
		render_target_ = RenderTarget::Normal;
		alpha_ = 1.0f;
		alphatest_value_ = 0.0f;
		ztest_mode_ = false;
		effect_strength_ = 0.0f;
		chromakey_viewmask_mode_ = false;
		attrib_flag_ = 0x0;
		resetTexcoords(1.0f, 1.0f);
		
		makeShaderBaker(bakermap_for_export_);
		makeShaderBaker(bakermap_for_preview_);

		Matrix::setIdentityM(tex_matrix_, 0);
		Matrix::setIdentityM(identity_matrix_, 0);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "%s %d Layer Renderer initialized", __func__, __LINE__);
	}

	virtual ~renderer(){
	}

	INexVideoEditor* getEditor(){
		
		return getNexVideoEditor();
	}

	void makeShaderBaker(bakermap_t& bakermap){
		
		ShaderDecorator* decorators[] = {
			&base_shader_,
			&color_conv_shader_,
			&overlay_shader_,
			&useralpha_shader_,
			&chromakey_shader_,
			&mask_shader_,
			&colorlut_shader_,
			&hblur_shader_,
			&vblur_shader_,
			&mosaic_shader_,
#ifndef __APPLE__
			&base_shader_external_,
			&color_conv_shader_external_,
			&overlay_shader_external_,
			&useralpha_shader_external_,
			&chromakey_shader_external_,
			&mask_shader_external_,
			&colorlut_shader_external_
#endif
		};

		typedef map<unsigned int, ShaderDecorator*> decorator_map_t;
		decorator_map_t decorator_map;

		for (unsigned int i = 0; i < sizeof(decorators) / sizeof(ShaderDecorator*); ++i){

			decorator_map.insert(make_pair(decorators[i]->getAttrib(), decorators[i]->clone()));
		}

		int shader_program[] = {

			BASESHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRender In Java
			BASESHADER, CHROMAKEYSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithChromakey In Java
			BASESHADER, CHROMAKEYSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, COLORLUTSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithChromakeyAndLUT In Java
			BASESHADER, CHROMAKEYSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, MASKSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithMaskAndChromakey In Java
			BASESHADER, CHROMAKEYSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, COLORLUTSHADER, MASKSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithMaskAndChromakeyAndLUT In Java
			BASESHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, MASKSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithMask In Java
			BASESHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, COLORLUTSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithLUT In Java
			BASESHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, COLORLUTSHADER, MASKSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithMaskAndLut In Java
			HBLURSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, static_cast<int>(SHADERSTRMDLMTR),//HBlurSimpleRenderer In Java
			VBLURSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, static_cast<int>(SHADERSTRMDLMTR),//VBlurSimpleRenderer In Java
			MOSAICSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, static_cast<int>(SHADERSTRMDLMTR),//MosaicSimpleRenderer In Java
#ifndef __APPLE__
			BASESHADER_EXTERNAL, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRender_external In Java
			BASESHADER_EXTERNAL, CHROMAKEYSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithChromakey_external In Java
			BASESHADER_EXTERNAL, CHROMAKEYSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, COLORLUTSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithChromakeyAndLUT_external In Java
			BASESHADER_EXTERNAL, CHROMAKEYSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, MASKSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithMaskAndChromakey_external In Java
			BASESHADER_EXTERNAL, CHROMAKEYSHADER, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, COLORLUTSHADER, MASKSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithMaskAndChromakeyAndLUT_external In Java
			BASESHADER_EXTERNAL, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, MASKSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithMask_external In Java
			BASESHADER_EXTERNAL, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, COLORLUTSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithLUT_external In Java
			BASESHADER_EXTERNAL, COLORCONVSHADER, OVERLAYSHADER, USERALPHASHADER, COLORLUTSHADER, MASKSHADER, static_cast<int>(SHADERSTRMDLMTR),//SimpleRendererWithMaskAndLut_external In Java
#endif
		};

		ShaderDecorator* decoratee = NULL;
		SimpleRenderInterface* pshader_renderer = NULL;
		for (unsigned int i = 0; i < sizeof(shader_program) / sizeof(int); ++i){

			if (!pshader_renderer)

				pshader_renderer = new SimpleRenderInterface();

			if (shader_program[i] == SHADERSTRMDLMTR){
				bakermap.insert(make_pair(pshader_renderer->getAttribFlag(), pshader_renderer));
				decoratee = NULL;
				pshader_renderer = NULL;
				continue;
			}

			decorator_map_t::iterator itor = decorator_map.find(shader_program[i]);
			if (itor != decorator_map.end()){

				ShaderDecorator* baker = itor->second->clone();
				baker->decorate(decoratee);
				pshader_renderer->addDecorator(baker);
				decoratee = baker;
			}
		}

		//free decorator map
		for(decorator_map_t::iterator itor = decorator_map.begin(); itor != decorator_map.end(); ++itor){

			ShaderDecorator* baker = itor->second;
			delete baker;
		}
	}

	void freeBakermap(bakermap_t& bakermap){

		for(bakermap_t::iterator itor = bakermap.begin(); itor != bakermap.end(); ++itor){

			SimpleRenderInterface* baker = itor->second;
			delete baker;
		}

		bakermap.clear();
	}

	void releaseResource(){

		freeBakermap(bakermap_for_export_);
		freeBakermap(bakermap_for_preview_);
	}

	unsigned int getAttribFlag(){
		attrib_flag_ |= (USERALPHASHADER|COLORCONVSHADER|OVERLAYSHADER);
		return attrib_flag_;
	}

	
    float* getTexMatrix() {

        return tex_matrix_;
    }
	float* getIdentityMatrix() {
		return identity_matrix_;
	}

	SimpleRenderInterface* getShaderBaker(){

		bakermap_t bakermap_ = bakermap_for_preview_;

		if(getRenderMode() == RenderMode::Export) {
			
			bakermap_ = bakermap_for_export_;
		}

		bakermap_t::iterator itor = bakermap_.find(getAttribFlag());
		
		if (itor == bakermap_.end()){

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "%s %d getAttribFlag:0x%x", __FUNCTION__, __LINE__, getAttribFlag());
			return NULL;
		}
		return itor->second;
	}

	void draw_Internal(unsigned int img_id, float* matrix, float* color_matrix, float* tex_matrix, float alpha, float* pvertexbuffer, float* ptexbuffer){
		
		
		setAlpha(alpha);
		SimpleRenderInterface* pshader_renderer = getShaderBaker();
		if(pshader_renderer){

			RendererResource resource(*this, img_id, matrix, color_matrix, tex_matrix, pvertexbuffer, ptexbuffer);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "%s %d img_id:%d", __func__, __LINE__, img_id);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "%s %d matrix: %.3f %.3f %.3f %.3f", __func__, __LINE__, matrix[ 0], matrix[ 1], matrix[ 2], matrix[ 3]);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "%s %d matrix: %.3f %.3f %.3f %.3f", __func__, __LINE__, matrix[ 4], matrix[ 5], matrix[ 6], matrix[ 7]);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "%s %d matrix: %.3f %.3f %.3f %.3f", __func__, __LINE__, matrix[ 8], matrix[ 9], matrix[10], matrix[11]);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "%s %d matrix: %.3f %.3f %.3f %.3f", __func__, __LINE__, matrix[12], matrix[13], matrix[14], matrix[15]);
			pshader_renderer->draw(resource);
		}
	}

	void drawDirect(unsigned int tex_name, int type, float x, float y, float w, float h, int flipMode){

		if(tex_name <= 0)
			return;
		
		if( (flipMode & 1)==1 ) {//vertival

			h *= -1;
		}
		if( (flipMode & 2) == 2 ) { //horizontal

			w *= -1;
		}

		draw(tex_name, type, x - w / 2.0f, y - h / 2.0f, x + w / 2.0f, y + h / 2.0f);
	}

	void draw(unsigned int tex_name, int type, float left, float top, float right, float bottom) {

		float* tex_matrix = NULL;
		if(type != 0 )
			tex_matrix = getTexMatrix();
		
		else
			tex_matrix = getIdentityMatrix();
		
		presetForDrawing(left, top, right, bottom, type);
		draw_Internal(tex_name, matrix_, color_matrix_for_render_, tex_matrix, getAlpha(), vertices_, texcoords_);
	}

	void drawVideo(int clipid, float x, float y, float w, float h){
	
        INexVideoEditor* editor = getEditor();
		if(NULL != editor){
// #ifdef _ANDROID
//             const float* pmatrix = getTexMatrix();
// 			int ret = editor->getTexNameForClipIDExternal(getRenderMode(), clipid, &pmatrix);
// 			int tex_name =  ret&0x00FFFFFF;
// 			int type =    (ret&0xFF000000) >> 24;

// 			if(tex_name < 0){

//                 editor->Release();
// 				return;
//             }
// #else
			int tex_name = editor->getTexNameForClipID(getRenderMode(), clipid);
			int type = 0;
//#endif
			draw(tex_name, type, x, y, w, h);
            editor->Release();
		}
	}
	
	void drawBitmap(unsigned int img_id, float left, float top, float right, float bottom, int flipMode) {

		if(img_id <= 0)
			return;
		int current_left= left;
		int current_top = top;
		int current_right = right;
		int current_bottom = bottom;

		if( (flipMode & 1)==1 ) {//vertival

			current_top = getFirst(current_bottom, current_bottom = current_top);
		}
		if( (flipMode & 2) == 2 ) { //horizontal

			current_left = getFirst(current_right, current_right = current_left);
		}

		draw(img_id, 0, current_left, current_top, current_right, current_bottom);
	}

	void drawBitmap(unsigned int img_id, float left, float top, float right, float bottom, float repeat_x, float repeat_y) {

		if(img_id <= 0)
			return;

		glActiveTexture(GL_TEXTURE0);															CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, img_id);													CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/);		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/);		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);							CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);							CHECK_GL_ERROR();

		resetTexcoords(repeat_x, repeat_y);
		draw(img_id, 0, left, top, right, bottom);
		resetTexcoords(1.0f, 1.0f);

		glActiveTexture(GL_TEXTURE0);															CHECK_GL_ERROR();
		glBindTexture(GL_TEXTURE_2D, img_id);													CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR /*GL_NEAREST*/);		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /*GL_NEAREST*/);		CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);					CHECK_GL_ERROR();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);					CHECK_GL_ERROR();
	}
	
	void drawRenderItem_Internal(int effect_id, int tex_id_for_second, char* usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, bool mask_enabled, int flipMode){

        INexVideoEditor* editor = getEditor();
		if(NULL != editor){

			int current_left= left;
			int current_top = top;
			int current_right = right;
			int current_bottom = bottom;

			if( (flipMode & 1)==1 ) {//vertival

				current_top = getFirst(current_bottom, current_bottom = current_top);
			}
			if( (flipMode & 2) == 2 ) { //horizontal

				current_left = getFirst(current_right, current_right = current_left);
			}

			float tmp_matrix[16];
			Matrix::transposeM(tmp_matrix, 0, matrix_, 0);
			editor->drawRenderItemOverlay(effect_id, tex_id_for_second, render_mode_, usersettings, current_time, start_time, end_time, tmp_matrix, left, top, right, bottom, alpha, mask_enabled);
            editor->Release();
		}
	}
	
	void drawRenderItem(int effect_id, char* usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, bool mask_enabled, int flipmode){
		
		drawRenderItem_Internal(effect_id, 0, usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled, flipmode);
	}
	
	void drawRenderItemBlend(int effect_id, int tex_id_for_second, int type, char* usersettings, int current_time, int start_time, int end_time, float x, float y, float w, float h, float alpha, bool mask_enabled, int flipMode){
		
		if(tex_id_for_second <= 0)
			return;

        INexVideoEditor* editor = getEditor();
		if(NULL != editor){

			editor->swapBlendMain(render_mode_);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT| GL_STENCIL_BUFFER_BIT);
			drawDirect(tex_id_for_second, type, x, y, w, h, flipMode);
			editor->swapBlendMain(render_mode_);
			glClearColor(0, 0, 0, 1);
			drawRenderItem_Internal(effect_id, editor->getTexNameForBlend(render_mode_), usersettings, current_time, start_time, end_time, x - w / 2, y - h / 2, x + w / 2, y + h / 2, alpha, mask_enabled, flipMode);
            editor->Release();
		}
	}

	void drawRenderItemBlendOverlay(int effect_id_for_blend, int effect_id, char* usersettings_for_blend, char* usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, bool mask_enabled, int flipMode){
		
		if(effect_id <= 0)
			return;

        INexVideoEditor* editor = getEditor();
		if(NULL != editor){
		
			editor->swapBlendMain(render_mode_);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT| GL_STENCIL_BUFFER_BIT);
			drawRenderItem_Internal(effect_id, editor->getTexNameForBlend(render_mode_), usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled, flipMode);
			editor->swapBlendMain(render_mode_);
			glClearColor(0, 0, 0, 1);
			drawRenderItem_Internal(effect_id_for_blend, editor->getTexNameForBlend(render_mode_), usersettings_for_blend, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled, flipMode);
            editor->Release();
		}
	}
	
	void drawRenderItemBitmap(int effect_id, int tex_id_for_second, char* usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, bool mask_enabled, int flipMode){
		
		if(tex_id_for_second <= 0)
			return;

		INexVideoEditor* editor = getEditor();
		if(NULL != editor){

			editor->swapBlendMain(render_mode_);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT| GL_STENCIL_BUFFER_BIT);
			drawBitmap(tex_id_for_second, left, top, right, bottom, flipMode);
			editor->swapBlendMain(render_mode_);
			glClearColor(0, 0, 0, 1);
			drawRenderItem_Internal(effect_id, editor->getTexNameForBlend(render_mode_), usersettings, current_time, start_time, end_time, left, top, right, bottom, alpha, mask_enabled, flipMode);
            editor->Release();
		}
	}
	

	void drawNexEDL(int effect_id, char* usersettings, int current_time, int start_time, int end_time, float left, float top, float right, float bottom, float alpha, int mode){

		INexVideoEditor* editor = getEditor();
		if(NULL != editor){

			float tmp_matrix[16];
			Matrix::transposeM(tmp_matrix, 0, matrix_, 0);
			editor->drawNexEDLOverlay(effect_id, render_mode_, usersettings, current_time, start_time, end_time, tmp_matrix, left, top, right, bottom, alpha, mode);
            editor->Release();
		}
	}
	
	void setLUT(unsigned int tex_id){

		lut_tex_id_ = tex_id;
		if (tex_id <= 0){

			attrib_flag_ &= (~colorlut_shader_.getAttrib());
			return;
		}
		attrib_flag_ |= colorlut_shader_.getAttrib();
	}

	void setLUTEnable(bool enable){

		if (enable)
			attrib_flag_ |= (~colorlut_shader_.getAttrib());
		else
			attrib_flag_ &= (~colorlut_shader_.getAttrib());
	}
	
	unsigned int getLUT(){

		return lut_tex_id_;
	}

	void setChromakeyViewMaskEnabled(bool maskOnOff){

		chromakey_viewmask_mode_ = maskOnOff;
	}

	int getChromakeyViewMaskEnabled(){

		return (chromakey_viewmask_mode_ ? 1 : 0 );
	}

	void setChromakeyColor(int color, float clipFg, float clipBg, float bx0, float by0, float bx1, float by1){

		chromakey_info_.set(color, clipFg, clipBg, bx0, by0, bx1, by1);
	}

	ChromakeyInfo& getChromakeyColor(){

		return chromakey_info_;
	}

	void setDecoratorOnOff(unsigned int attrib, bool enable){

		if(attrib & BASELINEFILTER){

			if(enable)
				attrib_flag_ = (attrib_flag_ & (~BASELINEFILTER))|(attrib & BASELINEFILTER);
			else
				attrib_flag_ = (attrib_flag_ & (~BASELINEFILTER));
		}
		else{

			if (enable)
				attrib_flag_ |= attrib;
			else
				attrib_flag_ &= (~attrib);
		}
	}

	bool getDecoratorOnOff(unsigned int attrib){

		if(attrib & BASELINEFILTER){

			return (attrib_flag_ & (~BASELINEFILTER)) & (attrib & BASELINEFILTER)  ? true : false;
		}
		else{

			return attrib_flag_ & attrib ? true : false;
		}
	}

	void setChromakeyEnabled(bool enable){

		setDecoratorOnOff(chromakey_shader_.getAttrib(), enable);
	}

	void setVBlurEnabled(bool enable){

		setDecoratorOnOff(vblur_shader_.getAttrib(), enable);
	}

	void setHBlurEnabled(bool enable){

		setDecoratorOnOff(hblur_shader_.getAttrib(), enable);
	}

	void setMosaicEnabled(bool enable){

		setDecoratorOnOff(mosaic_shader_.getAttrib(), enable);
	}
#ifndef __APPLE__
	void setExternalEnabled(bool enable){

		setDecoratorOnOff(base_shader_external_.getAttrib(), enable);
	}
#endif
	void setEffectStrength(float strength){

		effect_strength_ = strength;
	}

	float getEffectStrength(){

		return effect_strength_;
	}

	void setEffectTextureSize(int w, int h){

		texture_size_width_ = w;
		texture_size_height_ = h;
	}

	int getEffectTextureWidth(){

		return texture_size_width_;
	}

	int getEffectTextureHeight(){

		return texture_size_height_;
	}

	void setEffectTextureSizeW(float w){

		texture_size_width_ = w;
	}

	void setEffectTextureSizeH(float h){

		texture_size_height_ = h;
	}

	void setScreenDimension(int w, int h){

		screen_width_ = w;
		screen_height_ = h;
	}

	void setFrameDimension(int w, int h){

		frame_width_ = w;
		frame_height_ = h;
	}

	int getScreenDimensionWidth(){

		return screen_width_;
	}

	int getScreenDimensionHeight(){

		return screen_height_;
	}

	int getFrameWidth() {

		return frame_width_;
	}

	int getFrameHeight() {

		return frame_height_;
	}

	void setMaskEnabled(bool enable){

		setDecoratorOnOff(mask_shader_.getAttrib(), enable);
	}

	bool getMaskEnabled(){

		return getDecoratorOnOff(mask_shader_.getAttrib());
	}

	void setMaskTexID(int id){

		mask_tex_id_ = id;
	}

	unsigned int getMaskTexID(){

		return mask_tex_id_;
	}

	void setCurrentTime(int currentTime) {

		current_time_ = currentTime;
	}

	int getCurrentTime(){

		return current_time_;
	}

	void setAlpha(float alpha){

		alpha_ = alpha;
	}

	float getAlpha(){

		return alpha_;
	}

	void setAlphatestValue(float alpha){

		alphatest_value_ = alpha;
	}

	float getAlphatestValue(){

		return alphatest_value_;
	}

	void setHue(float hue){

		hue_ = hue;
	}

	float getHue(){

		return hue_;
	}

	void resetOpenGLState() {

		glEnable(GL_BLEND);
		CHECK_GL_ERROR();
		glDisable(GL_CULL_FACE);
		CHECK_GL_ERROR();
		glDisable(GL_DEPTH_TEST);
		CHECK_GL_ERROR();
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);    CHECK_GL_ERROR();
	}

	void resetMatrix(){

		float frameWidth((float)getFrameWidth());
		float frameHeight((float)getFrameHeight());

		float half_w = frameWidth * 0.5f;
		float half_h = frameHeight * 0.5f;

		Matrix::perspectiveM(matrix_, 0, 45, frameWidth / frameHeight, 0.01f, 3000.0f);
		float dist = half_w * matrix_[0];

		Matrix::translateM(matrix_, 0, -half_w, -half_h, -dist);
	}

	void outputMatrix(const char* func, const int line){

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "%s %d matrix: %.3f %.3f %.3f %.3f", func, line, matrix_[ 0], matrix_[ 1], matrix_[ 2], matrix_[ 3]);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "%s %d matrix: %.3f %.3f %.3f %.3f", func, line, matrix_[ 4], matrix_[ 5], matrix_[ 6], matrix_[ 7]);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "%s %d matrix: %.3f %.3f %.3f %.3f", func, line, matrix_[ 8], matrix_[ 9], matrix_[10], matrix_[11]);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "%s %d matrix: %.3f %.3f %.3f %.3f", func, line, matrix_[12], matrix_[13], matrix_[14], matrix_[15]);
	}

	void preRender() {

		CHECK_GL_ERROR();
		float frameWidth = (float)getFrameWidth();
		float frameHeight = (float)getFrameHeight();
		alpha_ = 1.0f;
		Matrix::setIdentityM(matrix_, 0);
		state_stack_point_ = 0;
		color_matrix_.reset();

		float half_w = frameWidth * 0.5f;
		float half_h = frameHeight * 0.5f;

		Matrix::perspectiveM(matrix_, 0, 45, (float)frameWidth / (float)frameHeight, 1.0f, 3000.0f);

		float dist = half_w * matrix_[0];
		Matrix::scaleM(matrix_, 0, 1, -1, 1);
		Matrix::translateM(matrix_, 0, -half_w, -half_h, -dist);

		resetTexcoords(1.0f, 1.0f);
		resetOpenGLState();

	}

	void postRender() {
	}

	void save() {

		if (state_stack_point_ >= state_stack_.size()) {
			
			state_stack_.push_back(StateStackEntry());
		}
		state_stack_[state_stack_point_].copyFrom(matrix_, alpha_, current_time_, color_matrix_, render_target_, getMaskEnabled(), scale_x_, scale_y_, hue_);
		state_stack_point_++;
	}

	void setRenderTarget(int render_target){

		INexVideoEditor* editor = getEditor();
		if(NULL == editor)
			return;
		
		render_target_ = render_target;
		
		switch( render_target_ ) {
				
			case RenderTarget::Mask:
				editor->setRenderToMask(getRenderMode());
				break;
			case RenderTarget::Normal:
				editor->setRenderToDefault(getRenderMode());
				break;
		}

        editor->Release();
	}

	int getRenderTarget(){

		return render_target_;
	}

	void restore() {

		if (state_stack_point_ < 1) {
			return;
		}
		state_stack_point_--;
		memcpy(matrix_, state_stack_[state_stack_point_].getMatrix(), sizeof(matrix_));
		alpha_ = state_stack_[state_stack_point_].getAlpha();
		scale_x_ = state_stack_[state_stack_point_].getScaleX();
		scale_y_ = state_stack_[state_stack_point_].getScaleY();
		current_time_ = state_stack_[state_stack_point_].getCTS();
		color_matrix_.set(state_stack_[state_stack_point_].getColorMatrix());
		if(getRenderTarget() != state_stack_[state_stack_point_].getRenderTarget()) 
			setRenderTarget(state_stack_[state_stack_point_].getRenderTarget());
		setMaskEnabled(state_stack_[state_stack_point_].getMaskEnabled());
		hue_ = state_stack_[state_stack_point_].getHue();
	}

	void setZTestMode(){

		ztest_mode_ = true;
		if (ztest_mode_){
			glClear(GL_DEPTH_BUFFER_BIT);
		}
	}

	void releaseZTestMode(){

		ztest_mode_ = false;
	}

	void setZTest(){

		setAlphatestValue(0.0f);
		if (ztest_mode_ == true){

			if (alpha_ >= 1.0) {

				glDepthFunc(GL_LEQUAL);
				glEnable(GL_DEPTH_TEST);
				setAlphatestValue(0.5f);
			}

		}
	}

	void releaseZTest(){

		if (ztest_mode_ == true){

			glDisable(GL_DEPTH_TEST);
		}
	}

	void translate(float x, float y, float z) {

		Matrix::translateM(matrix_, 0, x, y, z);
	}

	void translate(float x, float y) {

		Matrix::translateM(matrix_, 0, x, y, 0);
	}

	void scale(float x, float y, float z) {

		scale_x_ *= x;
		scale_y_ *= y;
		Matrix::scaleM(matrix_, 0, x, y, z);
	}

	void scale(float x, float y) {

		scale_x_ *= x;
		scale_y_ *= y;
		Matrix::scaleM(matrix_, 0, x, y, 1);
	}

	void scale(float x, float y, float cx, float cy) {

		scale_x_ *= x;
		scale_y_ *= y;
		translate(cx, cy);
		Matrix::scaleM(matrix_, 0, x, y, 1);
		translate(-cx, -cy);
	}

	void rotateAroundAxis(float a, float x, float y, float z) {

		Matrix::rotateM(matrix_, 0, a, x, y, z);
	}

	float* getMatrix() {

		return matrix_;
	}

	void rotate(float a, float cx, float cy) {

		translate(cx, cy);
		rotateAroundAxis(a, 0, 0, 1);
		translate(-cx, -cy);
	}

	void clearMask() {

		int saved_render_target = getRenderTarget();
		if (saved_render_target != RenderTarget::Mask) {

			setRenderTarget(RenderTarget::Mask);
		}
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (getRenderTarget() != saved_render_target) {
			setRenderTarget(saved_render_target);
		}
	}

	void clearMask(int color) {

		int saved_render_target = getRenderTarget();
		if (saved_render_target != RenderTarget::Mask) {

			setRenderTarget(RenderTarget::Mask);
		}
		float a = color & 0xFF000000;
		float r = color & 0x00FF0000;
		float g = color & 0x0000FF00;
		float b = color & 0x000000FF;
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (getRenderTarget() != saved_render_target) {

			setRenderTarget(saved_render_target);
		}
	}

	int getRenderMode(){

		return render_mode_;
	}

	void setRenderMode(bool isExport){

		if(isExport){
			
			render_mode_ = RenderMode::Export;
		}
		else{
			render_mode_ = RenderMode::Preview;
		}
	}

	void setShaderAndParams(bool isExport){

		INexVideoEditor* editor = getEditor();
		if(NULL == editor)
			return;
		setRenderMode(isExport);
		setMaskTexID(editor->getTexNameForMask(getRenderMode()));
        editor->Release();
	}

	void presetForDrawing(float left, float top, float right, float bottom, int type){

		color_matrix_.applyTo44(color_matrix_for_render_);
		
		vertices_[2] = vertices_[6] = vertices_[10] = vertices_[14] = 0;      // z
		vertices_[3] = vertices_[7] = vertices_[11] = vertices_[15] = 1;        // w

		vertices_[0] = vertices_[8] = left;
		vertices_[1] = vertices_[5] = bottom;
		vertices_[4] = vertices_[12] = right;
		vertices_[9] = vertices_[13] = top;
#ifndef __APPLE__
		setExternalEnabled(type);
#endif
	}

	void setColorMatrix(float* color_matrix){

		if( color_matrix == NULL ) {

			color_matrix_.reset();
		} else {

			color_matrix_.set(color_matrix);
		}
	}

	void setColorMatrix(ColorMatrix& color_matrix){

		color_matrix_.set(color_matrix);
	}

	ColorMatrix& getColorMatrix(){

		return color_matrix_;
	}

	void clearMaskRegion(unsigned int fill_rect_tex_id, int color, float left, float top, float right, float bottom){

		save();

		float lefttop[4] = {left, top, 0.0f, 1.0f};
		float righttop[4] = {right, top, 0.0f, 1.0f};
		float leftbottom[4] = {left, bottom, 0.0f, 1.0f};
		float rightbottom[4] = {right, bottom, 0.0f, 1.0f};

		float result[4][4];
		float tmp_matrix[16];
		Matrix::transposeM(tmp_matrix, 0, matrix_, 0);

		Matrix::multiplyMV(result[0], tmp_matrix, 4, lefttop, 4);
		Matrix::multiplyMV(result[1], tmp_matrix, 4, righttop, 4);
		Matrix::multiplyMV(result[2], tmp_matrix, 4, leftbottom, 4);
		Matrix::multiplyMV(result[3], tmp_matrix, 4, rightbottom, 4);

		float min_x = 1000000000.0f;
		float min_y = 1000000000.0f;
		float max_x = -1000000000.0f;
		float max_y = -1000000000.0f;
		float x = 0.0f;
		float y = 0.0f;
		for(int i = 0; i < 4; ++i){

			x = result[i][0] /= result[i][3];
			y = result[i][1] /= result[i][3];
			if(x < min_x)
				min_x = x;
			if(x > max_x)
				max_x = x;

			if(y < min_y)
				min_y = y;
			if(y > max_y)
				max_y = y;
		}

		Matrix::setIdentityM(matrix_, 0);

		fill_rect_color_matrix_.setScale(((float)((color >> 16) & 0xff) / 255.0f),
											((float)((color >> 8) & 0xff) / 255.0f),
											((float)((color >> 0) & 0xff) / 255.0f),
											1.0f);
		setColorMatrix(fill_rect_color_matrix_);
		setAlpha(getAlpha() * ((float)((color >> 24) & 0xff) / 255.0f));

		float margin_w = 2.0f / (float)getScreenDimensionWidth();
		float margin_h = 2.0f / (float)getScreenDimensionHeight();

		draw(fill_rect_tex_id, 0, (min_x - margin_w), (min_y - margin_h), (max_x + margin_w), (max_y + margin_h));

		restore();
	}

	void fillRect(unsigned int fill_rect_tex_id, int color, float left, float top, float right, float bottom){

		save();
		fill_rect_color_matrix_.setScale(((float)((color >> 16) & 0xff) / 255.0f),
										((float)((color >> 8) & 0xff) / 255.0f),
										((float)((color >> 0) & 0xff) / 255.0f),
										1.0f);
		setColorMatrix(fill_rect_color_matrix_);
		setAlpha(getAlpha() * ((float)((color >> 24) & 0xff) / 255.0f));

		draw(fill_rect_tex_id, 0, left, top, right, bottom);
		restore();
	}


	void setColorLookupTable(int img_id, float x, float y){

		INexVideoEditor* editor = getEditor();
		if(NULL != editor){

			editor->setTexNameForLUT(getRenderMode(),img_id, x, y);
            editor->Release();
		}
	}

	void setBlendFuncNormal(){

		glEnable(GL_BLEND);
		CHECK_GL_ERROR();

		glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		CHECK_GL_ERROR();
	}

	void setBlendFuncMultiply(){

		glEnable(GL_BLEND);
		CHECK_GL_ERROR();

		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);

		CHECK_GL_ERROR();
	}

};
