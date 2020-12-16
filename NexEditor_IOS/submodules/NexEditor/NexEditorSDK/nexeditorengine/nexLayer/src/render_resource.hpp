#pragma once

class RendererResource
{
	int tex_target_;
	unsigned int texname_;
	float* mvp_matrix_;
	float* tex_matrix_;
	float* color_matrix_;
	float* pvertices_;
	float* ptexcoords_;
	float overlaycolor_[4];
	RendererInterface& renderer_;
public:
	RendererResource(RendererInterface& renderer) 
		:renderer_(renderer), 
		tex_target_(0),
		texname_(0),
		mvp_matrix_(NULL),
		color_matrix_(NULL),
		tex_matrix_(NULL),
		pvertices_(NULL),
		ptexcoords_(NULL){

		setOverlaycolor(0, 0, 0, 0);
	}

	RendererResource(RendererInterface& renderer, unsigned int texname, float* mvp_matrix, float* color_matrix, float* tex_matrix, float* pvertices, float* ptexcoords)
		:renderer_(renderer),
		tex_target_(0), 
		texname_(texname),
		mvp_matrix_(mvp_matrix),
		color_matrix_(color_matrix),
		tex_matrix_(tex_matrix),
		pvertices_(pvertices),
		ptexcoords_(ptexcoords){

		setOverlaycolor(0, 0, 0, 0);

	}

	RendererInterface& getRenderer(){

		return renderer_;
	}

	int getTexTarget(){

		return ++tex_target_;
	}

	void setMvpMatrix(float* mvp_matrix){

		mvp_matrix_ = mvp_matrix;
	}

	float* getMvpMatrix(){

		return mvp_matrix_;
	}

	void setTexMatrix(float* tex_matrix){

		tex_matrix_ = tex_matrix;
	}

	float* getTexMatrix(){

		return tex_matrix_;
	}

	void setColorMatrix(float* color_matrix){

		color_matrix_ = color_matrix;
	}

	float* getColorMatrix(){

		return color_matrix_;
	}

	void clearOverlayColor(){

		overlaycolor_[0] = 0.0f;
		overlaycolor_[1] = 0.0f;
		overlaycolor_[2] = 0.0f;
		overlaycolor_[3] = 0.0f;
	}

	void setOverlaycolor(float r, float g, float b, float a){

		overlaycolor_[0] = r * a;
		overlaycolor_[1] = g * a;
		overlaycolor_[2] = b * a;
		overlaycolor_[3] = a;
	}

	float* getOverlaycolor(){

		return overlaycolor_;
	}

	void setTexname(unsigned int texname){

		texname_ = texname;
	}

	unsigned int getTexname(){

		return texname_;
	}

	void setVertices(float* pvertices){

		pvertices_ = pvertices;
	}

	float* getVertices(){

		return pvertices_;
	}

	void setTexcoords(float* ptexcoords){

		ptexcoords_ = ptexcoords;
	}

	float* getTexcoords(){

		return ptexcoords_;
	}
};
