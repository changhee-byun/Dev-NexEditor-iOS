#include "RenderTarget.h"
#include "util.h"
#include "message_processor.h"
#include "rootframe.h"
#include "RenderItem.h"
#include "streamingProject.h"
#include <algorithm>
#include <vector>

#include "maya/material.h"
#include "maya/mesh.h"
#include "Matrices.h"
#include "maya/Vectors.h"

#define PROGRAM_NAME "RenderItem Simulator"

void testGPXDraw(float distance, float rotateX, float rotateY, int flag);

void checkSDLError(int line){

#ifndef NDEBUG
	const char *error = SDL_GetError();
	if (*error != '\0')
	{
		printf("SDL Error: %s\n", error);
		if (line != -1)
			printf(" + line: %i\n", line);
		SDL_ClearError();
	}
#endif
}

class RenderItemApp{

	SDL_GLContext		maincontext_;
	SDL_Window			*mainwindow_;
	MessageProcessor	message_processor_;
	int					total_play_tick_;
	StreamingProject	project_;

	int					hold_;
	int					frame_;
	DWORD				prev_tick_;
	int					pre_frame_;
	int					tick_for_start_;
	int					user_advance_;
	NXT_HThemeRenderer	theme_renderer_;

	int test_distance_;
	int test_rotate_y_;
	int test_limit_;

	std::vector<ManagerInterface*> manager_;

	void sdldie(const char *msg)
	{
		printf("%s: %s\n", msg, SDL_GetError());
		SDL_Quit();
		exit(1);
	}

	int procEvent(){

		SDL_Event event;
		user_advance_ = 0;
		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_KEYDOWN){

				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){

					if (project_.getItemCount() > 0){

						project_.freeResource();
					}
					else{

						total_play_tick_ = project_.reload();
						pre_frame_ = getTickCount();
						tick_for_start_ = getTickCount();
					}
				}

				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE){

					hold_ = (hold_ == 0) ? 1 : 0;
				}
				/*if (event.key.keysym.scancode == SDL_SCANCODE_LEFT){
					user_advance_ = -10;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
					user_advance_ = 10;
				}*/
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT){
					test_rotate_y_++;
					printf("test_rotate_y_:%d\n", test_rotate_y_);
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
					test_rotate_y_--;
					printf("test_rotate_y_:%d\n", test_rotate_y_);
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_PAGEUP){
					test_limit_++;
					printf("test_limit:%d\n", test_limit_);
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_PAGEDOWN){
					test_limit_--;// = test_limit_ > 0 ? test_limit_ - 1 : test_limit_;
					printf("test_limit:%d\n", test_limit_);
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_UP){
					test_distance_+= 10;
					printf("test_distance:%d\n", test_distance_);
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_DOWN){
					test_distance_ -= 10;
					printf("test_distance:%d\n", test_distance_);
				}
			}
			if (message_processor_.doMessageProcess(event)){

				if (event.type == SDL_QUIT) {

					project_.freeResource();

					for (int i = 0; i < 5; ++i){

						if (cleanUpManager())
							break;
					}
					
					return 0;
				}
			}
		}

		return 1;
	}

	int getProjectTime(){

		int frame_gap = getTickCount() - pre_frame_;
		int elapsed_tick = getTickCount() - tick_for_start_;

		pre_frame_ = getTickCount();
		if (hold_){

			tick_for_start_ += frame_gap;
			elapsed_tick -= frame_gap;
			elapsed_tick += user_advance_;
			tick_for_start_ -= user_advance_;
		}

		elapsed_tick %= total_play_tick_;

		return elapsed_tick;
	}

	void mainLoop(NXT_HThemeRenderer renderer){

		project_.doPlay(getProjectTime());
		setRenderTargetAsDefault(renderer, NULL);
		project_.doRender(renderer);
	}

	void setRenderer(NXT_HThemeRenderer renderer){

		theme_renderer_ = renderer;
	}

	void resetDefaultGLSetting(){

		glDisable(GL_DEPTH_TEST);                       CHECK_GL_ERROR();
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); CHECK_GL_ERROR();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CHECK_GL_ERROR();
		glEnable(GL_BLEND);                             CHECK_GL_ERROR();
		glDisable(GL_CULL_FACE);                        CHECK_GL_ERROR();
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);    CHECK_GL_ERROR();
	}

	void doUI(){

		glDisable(GL_CULL_FACE);                       CHECK_GL_ERROR();
		glDisable(GL_DEPTH_TEST);                       CHECK_GL_ERROR();

		message_processor_.doDraw();
	}

	void updateManagerGC(){

		for (std::vector<ManagerInterface*>::iterator itor = manager_.begin(); itor != manager_.end(); ++itor)
			(*itor)->gc();
	}

	bool cleanUpManager(){

		for (std::vector<ManagerInterface*>::iterator itor = manager_.begin(); itor != manager_.end();){

			if ((*itor)->forceGC()){

				itor = manager_.erase(itor);
			}
			else
				++itor;
		}

		if (manager_.empty())
			return true;

		return false;
	}

	void doSwap(){

		SDL_GL_SwapWindow(mainwindow_);
		RenderTargetStack_t& rendertarget_stack = *(Singleton<RenderTargetStack_t>::getInstance());
		for (RenderTargetStack_t::iterator itor = rendertarget_stack.begin(); itor != rendertarget_stack.end(); ++itor)
			theme_renderer_->releaseRenderTarget(*itor);
		rendertarget_stack.clear();

		frame_++;
		DWORD cur_tick = getTickCount();
		DWORD tick_gap = cur_tick - prev_tick_;
		if (tick_gap >= 1000){

			printf("fps:%f\r", float(frame_) / double(tick_gap) * 1000.0);
			prev_tick_ = cur_tick;
			frame_ = 0;
			theme_renderer_->updateRenderTargetManager();
			updateManagerGC();
		}
		if (project_.getItemCount() <= 0){

			Sleep(100);
		}
		Sleep(10);
	}

	void reset(){

		hold_ = 0;
		frame_ = 0;
		prev_tick_ = getTickCount();
		pre_frame_ = getTickCount();
		tick_for_start_ = getTickCount();
	}

public:
	RenderItemApp(NXT_HThemeRenderer renderer) :maincontext_(NULL), mainwindow_(NULL), total_play_tick_(0), theme_renderer_(renderer){

		test_limit_ = 0;
		test_rotate_y_ = 0;
		test_distance_ = -500;

		manager_.push_back(Singleton<ResourceManager<RenderItemBin> >::getInstance());
		manager_.push_back(Singleton<ResourceManager<TextureBin> >::getInstance());
		manager_.push_back(Singleton<ResourceManager<ShaderBin> >::getInstance());
		manager_.push_back(Singleton<ResourceManager<ProgramBin> >::getInstance());
		manager_.push_back(Singleton<ResourceManager<LuaBin> >::getInstance());
		manager_.push_back(Singleton<ResourceManager<AnimationBin> >::getInstance());
	}

	~RenderItemApp(){

		SDL_GL_DeleteContext(maincontext_);
		SDL_DestroyWindow(mainwindow_);
		SDL_Quit();
	}

	void doLoop(){

		reset();

		while (procEvent()){
			
			resetDefaultGLSetting();

			mainLoop(theme_renderer_);

			glEnable(GL_DEPTH_TEST);                       CHECK_GL_ERROR();
			//glEnable(GL_CULL_FACE); CHECK_GL_ERROR();

			testGPXDraw(float(test_distance_), float(test_limit_), float(test_rotate_y_), hold_);
			//drawTestMeshGroup(test_limit_);

			doUI();

			doSwap();
		}
	}

	

	void init(int w, int h){

		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			sdldie("Unable to initialize SDL");

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		mainwindow_ = SDL_CreateWindow(PROGRAM_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (!mainwindow_)
			sdldie("Unable to create window");

		theme_renderer_->view_width = w;
		theme_renderer_->view_height = h;
		theme_renderer_->current_program_id_ = 0xFFFFFFFF;
		checkSDLError(__LINE__);

		maincontext_ = SDL_GL_CreateContext(mainwindow_);
		checkSDLError(__LINE__);
		LoadGLExtFuncs();
		SDL_GL_SetSwapInterval(0);

		message_processor_.setTopFrame(new RootFrame());
	}

	void load(const char* filename){

		int perf = getTickCount();
		total_play_tick_ = project_.loadProject(filename);
		perf = getTickCount() - perf;
		printf("Load Perf:%d\n", perf);
	}
};

#include <time.h>
#include <iomanip>
#include <sstream>

extern "C" char* strptime(const char* s,
	const char* f,
struct tm* tm) {

	return NULL;
	
	//// Isn't the C++ standard lib nice? std::get_time is defined such that its
	//// format parameters are the exact same as strptime. Of course, we have to
	//// create a string stream first, and imbue it with the current C locale, and
	//// we also have to make sure we return the right things if it fails, or
	//// if it succeeds, but this is still far simpler an implementation than any
	//// of the versions in any of the C standard libraries.
	//std::istringstream input(s);
	//input.imbue(std::locale(setlocale(LC_ALL, NULL)));
	//input >> std::get_time(tm, f);
	//if (input.fail()) {
	//	return nullptr;
	//}
	//return (char*)(s + input.tellg());
}

#define Deg2Rad(x) (M_PI * (x) / 180.0)

void glhLookAtf222(Matrix4& matrix, Vector3& eyePosition3D,
	Vector3& center3D, Vector3& upVector3D)
{
	Vector3 forward, side, up;
	Matrix4 matrix2, resultMatrix;
	//------------------
	forward.x = center3D.x - eyePosition3D.x;
	forward.y = center3D.y - eyePosition3D.y;
	forward.z = center3D.z - eyePosition3D.z;

	forward.normalize();
	//------------------
	//Side = forward x up
	side = forward.cross(upVector3D);
	side.normalize();
	up = side.cross(forward);
	//------------------
	matrix2[0] = side.x;
	matrix2[4] = side.y;
	matrix2[8] = side.z;
	matrix2[12] = 0.0;
	//------------------
	matrix2[1] = up.x;
	matrix2[5] = up.y;
	matrix2[9] = up.z;
	matrix2[13] = 0.0;
	//------------------
	matrix2[2] = -forward.x;
	matrix2[6] = -forward.y;
	matrix2[10] = -forward.z;
	matrix2[14] = 0.0;
	//------------------
	matrix2[3] = matrix2[7] = matrix2[11] = 0.0;
	matrix2[15] = 1.0;
	//------------------
	
	matrix *= matrix2;
	matrix.translate(-eyePosition3D);
}

void ComputeNormalOfPlane(float* out, float* vec1, float* vec2)
{
	out[0] = vec1[1] * vec2[2] - vec1[2] * vec2[1];
	out[1] = vec1[2] * vec2[0] - vec1[0] * vec2[2];
	out[2] = vec1[0] * vec2[1] - vec1[1] * vec2[0];
}

void NormalizeVector(float* vec)
{
	float d = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	if (d == 0)
		return;
	vec[0] /= d;
	vec[1] /= d;
	vec[2] /= d;
}

void MultiplyMatrices4by4OpenGL_FLOAT(float *result, const float *matrix1, const float *matrix2)
{
	result[0] = matrix1[0] * matrix2[0] +
		matrix1[4] * matrix2[1] +
		matrix1[8] * matrix2[2] +
		matrix1[12] * matrix2[3];
	result[4] = matrix1[0] * matrix2[4] +
		matrix1[4] * matrix2[5] +
		matrix1[8] * matrix2[6] +
		matrix1[12] * matrix2[7];
	result[8] = matrix1[0] * matrix2[8] +
		matrix1[4] * matrix2[9] +
		matrix1[8] * matrix2[10] +
		matrix1[12] * matrix2[11];
	result[12] = matrix1[0] * matrix2[12] +
		matrix1[4] * matrix2[13] +
		matrix1[8] * matrix2[14] +
		matrix1[12] * matrix2[15];
	result[1] = matrix1[1] * matrix2[0] +
		matrix1[5] * matrix2[1] +
		matrix1[9] * matrix2[2] +
		matrix1[13] * matrix2[3];
	result[5] = matrix1[1] * matrix2[4] +
		matrix1[5] * matrix2[5] +
		matrix1[9] * matrix2[6] +
		matrix1[13] * matrix2[7];
	result[9] = matrix1[1] * matrix2[8] +
		matrix1[5] * matrix2[9] +
		matrix1[9] * matrix2[10] +
		matrix1[13] * matrix2[11];
	result[13] = matrix1[1] * matrix2[12] +
		matrix1[5] * matrix2[13] +
		matrix1[9] * matrix2[14] +
		matrix1[13] * matrix2[15];
	result[2] = matrix1[2] * matrix2[0] +
		matrix1[6] * matrix2[1] +
		matrix1[10] * matrix2[2] +
		matrix1[14] * matrix2[3];
	result[6] = matrix1[2] * matrix2[4] +
		matrix1[6] * matrix2[5] +
		matrix1[10] * matrix2[6] +
		matrix1[14] * matrix2[7];
	result[10] = matrix1[2] * matrix2[8] +
		matrix1[6] * matrix2[9] +
		matrix1[10] * matrix2[10] +
		matrix1[14] * matrix2[11];
	result[14] = matrix1[2] * matrix2[12] +
		matrix1[6] * matrix2[13] +
		matrix1[10] * matrix2[14] +
		matrix1[14] * matrix2[15];
	result[3] = matrix1[3] * matrix2[0] +
		matrix1[7] * matrix2[1] +
		matrix1[11] * matrix2[2] +
		matrix1[15] * matrix2[3];
	result[7] = matrix1[3] * matrix2[4] +
		matrix1[7] * matrix2[5] +
		matrix1[11] * matrix2[6] +
		matrix1[15] * matrix2[7];
	result[11] = matrix1[3] * matrix2[8] +
		matrix1[7] * matrix2[9] +
		matrix1[11] * matrix2[10] +
		matrix1[15] * matrix2[11];
	result[15] = matrix1[3] * matrix2[12] +
		matrix1[7] * matrix2[13] +
		matrix1[11] * matrix2[14] +
		matrix1[15] * matrix2[15];
}

void glhTranslatef2(float *matrix, float x, float y, float z)
{
	matrix[12] = matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12];
	matrix[13] = matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13];
	matrix[14] = matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14];
	matrix[15] = matrix[3] * x + matrix[7] * y + matrix[11] * z + matrix[15];
}

void glhLookAtf2(float *matrix, float *eyePosition3D,
	float *center3D, float *upVector3D)
{
	float forward[3], side[3], up[3];
	float matrix2[16], resultMatrix[16];
	//------------------
	forward[0] = center3D[0] - eyePosition3D[0];
	forward[1] = center3D[1] - eyePosition3D[1];
	forward[2] = center3D[2] - eyePosition3D[2];
	NormalizeVector(forward);
	//------------------
	//Side = forward x up
	ComputeNormalOfPlane(side, forward, upVector3D);
	NormalizeVector(side);
	//------------------
	//Recompute up as: up = side x forward
	ComputeNormalOfPlane(up, side, forward);
	//------------------
	matrix2[0] = side[0];
	matrix2[4] = side[1];
	matrix2[8] = side[2];
	matrix2[12] = 0.0;
	//------------------
	matrix2[1] = up[0];
	matrix2[5] = up[1];
	matrix2[9] = up[2];
	matrix2[13] = 0.0;
	//------------------
	matrix2[2] = -forward[0];
	matrix2[6] = -forward[1];
	matrix2[10] = -forward[2];
	matrix2[14] = 0.0;
	//------------------
	matrix2[3] = matrix2[7] = matrix2[11] = 0.0;
	matrix2[15] = 1.0;
	//------------------
	MultiplyMatrices4by4OpenGL_FLOAT(resultMatrix, matrix, matrix2);
	glhTranslatef2(resultMatrix,
		-eyePosition3D[0], -eyePosition3D[1], -eyePosition3D[2]);
	//------------------
	memcpy(matrix, resultMatrix, 16 * sizeof(float));
}


struct Bezier{
	template<typename T>
	T getPt(T& pt0, T& pt1, float t){

		T diff = pt1 - pt0;
		return pt0 + diff * t;
	}

	template<typename T>
	T getBezierPoint(std::vector<T>& point_list, float t){

		std::vector<T> next_point_list;
		for (typename std::vector<T>::iterator itor = point_list.begin(); itor != point_list.end();){

			next_point_list.push_back(getPt(*itor++, *itor++, t));
		}
		if (next_point_list.size() == 1)
			return next_point_list[0];
		return getBezierPoint(next_point_list, t);
	}
};

#include "poly34.h"


class GpxLoader{

	struct Trkpt{

		double latitude_;
		double longitude_;
		double elevation_;
		double watt_;
		time_t time_;
	};

	int path_index_;
	float progress_;
	Vector3 path_points_[3];
	Vector3 user_pos_;
	std::vector<Trkpt> trkptlist_;
	std::vector<ColoredVertex> vertices_;
	std::vector<Vector3> path_;
	std::vector<unsigned short> indices_;

	TextureBin* ptex_;

	void buildMesh(){

		const double earth_radius = 6371000.0;
		
		vertices_.clear();
		indices_.clear();
		path_.clear();

		double center_x = 0.0;
		double center_y = 0.0;
		double center_z = 0.0;

		for (std::vector<Trkpt>::iterator itor = trkptlist_.begin(); itor != trkptlist_.end(); ++itor){

			const Trkpt& pt = *itor;
			Vector3 vtx;
			
			vtx.x = earth_radius * M_PI * 2.0 * pt.longitude_ / 360.0;
			vtx.y = earth_radius * M_PI * 2.0 * pt.latitude_ / 360.0;
			vtx.z = pt.elevation_;

			center_x += vtx.x;
			center_y += vtx.y;
			center_z += vtx.z;

			path_.push_back(vtx);
		}

		center_x /= (double)path_.size();
		center_y /= (double)path_.size();
		center_z /= (double)path_.size();

		std::vector<ColoredVertex> temp_vertices_for_rect;
		int dbg_point = 0;
		float pre_spd = 0.0f;

		for (std::vector<Vector3>::iterator itor = path_.begin(); itor != path_.end() - 1; ++itor, ++dbg_point){
			
			Vector3& pt = *itor;
			Vector3& pt_next = *(itor + 1);

			Vector3 diff = pt_next - pt;
			Vector2 plane_vec(diff.x, diff.y);

			float gradient = diff.z / plane_vec.length();
			float gravity = 9.8067f;
			float body_weight = 68.0f;
			float bicycle_weight = 8.0f;
			float crr = 0.005f;
			float force_for_grabitational_force = gravity * sin(atan(gradient)) * (body_weight + bicycle_weight);
			float force_for_rolling_resistance = gravity * cos(atan(gradient)) * (body_weight + bicycle_weight) * crr;
			float frontal_area = 0.509f;
			float drag_coefficient = 0.63f;
			float air_density = 1.226f;
			float velocity_per_sec = diff.length();
			float force_for_drag = 0.5f * frontal_area * drag_coefficient * air_density * velocity_per_sec * velocity_per_sec;
			float force_for_acc = (body_weight + bicycle_weight) * (velocity_per_sec - pre_spd);
			float total_resistance = force_for_grabitational_force + force_for_rolling_resistance + force_for_drag + force_for_acc;
			float work = total_resistance * diff.length();
			float wheel_power = total_resistance * velocity_per_sec;
			float loss_of_drivetrain = 0.03f;
			float leg_power = wheel_power / (1.0f - loss_of_drivetrain);
			printf("speed:%f gradient:%f total_resistance:%f power:%f dev_power:%f\n", diff.length(), gradient, total_resistance, leg_power, trkptlist_[dbg_point].watt_);
			float drag_factor = 0.5f * frontal_area * drag_coefficient * air_density;
			float non_drag_resistance = force_for_grabitational_force + force_for_rolling_resistance;
			
			float cubic_a = 0.0f;
			float cubic_b = non_drag_resistance / drag_factor;
			float cubic_c = -wheel_power / drag_factor;

			double result_value[3];
			int solution_count = SolveP3(result_value, cubic_a, cubic_b, cubic_c);
			for (int i = 0; i < solution_count; ++i){
				
				if (result_value[i] < 0.0)
					continue;
				printf("wheel_power:%f vel:%f diff:%f..........\n", wheel_power, result_value[i], result_value[i] / velocity_per_sec * 100.0);
			}

			pre_spd = velocity_per_sec;

			gradient += 0.1f;

			Vector3 normal(0, 0, 1);
			Vector3 tangent(diff.x, diff.y, 0.0f);
			Vector3 binormal = tangent.cross(normal);

			binormal.normalize();
			binormal *= 5.0f;

			pt.x -= center_x;
			pt.y -= center_y;
			pt.z -= center_z;

			ColoredVertex vtx0, vtx1;
			vtx0.pos_ = pt;
			vtx1.pos_ = pt;
			vtx0.color_ = Vector3(gradient, gradient, gradient);
			vtx1.color_ = Vector3(gradient, gradient, gradient);
			vtx0.pos_ += binormal;
			vtx1.pos_ -= binormal;
			temp_vertices_for_rect.push_back(vtx0);
			temp_vertices_for_rect.push_back(vtx1);
			
		}

		int tri_idx = 0;

		for (std::vector<ColoredVertex>::iterator itor = temp_vertices_for_rect.begin(); itor != temp_vertices_for_rect.end() - 2; itor += 2){

			ColoredVertex& pt0 = *itor;
			ColoredVertex& pt1 = *(itor + 1);
			ColoredVertex& pt2 = *(itor + 2);
			ColoredVertex& pt3 = *(itor + 3);

			ColoredVertex vtx0, vtx1, vtx2, vtx3;

			vtx0 = pt0;
			vtx1 = pt1;
			vtx2 = pt2;
			vtx3 = pt3;

			vtx0.uv_ = Vector2(0, 0);
			vtx1.uv_ = Vector2(1, 0);
			vtx2.uv_ = Vector2(0, 1);
			vtx3.uv_ = Vector2(1, 1);

			vertices_.push_back(vtx0);
			vertices_.push_back(vtx1);
			vertices_.push_back(vtx2);
			vertices_.push_back(vtx3);

			indices_.push_back(tri_idx + 0);
			indices_.push_back(tri_idx + 1);
			indices_.push_back(tri_idx + 2);
			indices_.push_back(tri_idx + 2);
			indices_.push_back(tri_idx + 1);
			indices_.push_back(tri_idx + 3);

			tri_idx += 4;
		}
	}

public:
	GpxLoader(){

		ptex_ = NULL;
	}

	~GpxLoader(){

		SAFE_RELEASE(ptex_);
	}

	rapidxml::xml_node<>* getTrainingCenterDatabase(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* ret = getNode(node, "TrainingCenterDatabase", NULL, NULL);

		return ret;
	}

	rapidxml::xml_node<>* getActivities(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* ret = getNode(node, "Activities", NULL, NULL);

		return ret;
	}

	void getActivity(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* ret = getNode(node, "Activity", NULL, NULL);

		while (ret){

			getLap(ret);
			ret = getNextNode(ret, "Activity", NULL, NULL);
		}
	}

	void getLap(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* ret = getNode(node, "Lap", NULL, NULL);
		while (ret){

			getTrack(ret);
			ret = getNextNode(ret, "Lap", NULL, NULL);
		}
	}

	void getTrack(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* ret = getNode(node, "Track", NULL, NULL);

		while (ret){

			getTrackpoint(ret);
			ret = getNextNode(ret, "Track", NULL, NULL);
		}
	}

	rapidxml::xml_node<>* getAltitudeMeters(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* ele = getNode(node, "AltitudeMeters", NULL, NULL);

		return ele;
	}

	void getPosition(rapidxml::xml_node<>* node, Trkpt& pt){

		rapidxml::xml_node<>* pos_node = node->first_node();
		while (pos_node){

			if (0 == strcasecmp(pos_node->name(), "LatitudeDegrees")){

				pt.latitude_ = atof(pos_node->value());
			}
			else if (0 == strcasecmp(pos_node->name(), "LongitudeDegrees")){

				pt.longitude_ = atof(pos_node->value());
			}

			pos_node = pos_node->next_sibling();
		}

	}

	void getTrackpoint(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* trkpt = getNode(node, "Trackpoint", NULL, NULL);

		while (trkpt){

			Trkpt pt;

			rapidxml::xml_node<>* child_node = trkpt->first_node();
			while (child_node){

				if (0 == strcasecmp(child_node->name(), "Position")){

					getPosition(child_node, pt);
				}
				else if (0 == strcasecmp(child_node->name(), "AltitudeMeters")){

					pt.elevation_ = atof(child_node->value());
				}
				else if (0 == strcasecmp(child_node->name(), "Extensions")){

					getExtensions(child_node, pt);
				}
				child_node = child_node->next_sibling();
			}
			trkptlist_.push_back(pt);
			trkpt = getNextNode(trkpt, "Trackpoint", NULL, NULL);
		}
	}

	void getExtensions(rapidxml::xml_node<>* node, Trkpt& pt){

		rapidxml::xml_node<>* tpx = getNode(node, "TPX", NULL, NULL);
		if (tpx){
			
			rapidxml::xml_node<>* child = tpx->first_node();
			while (child){

				if (0 == strcasecmp(child->name(), "Watts")){

					pt.watt_ = atof(child->value());
				}
				child = child->next_sibling();
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	rapidxml::xml_node<>* getGpx(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* gpx = getNode(node, "gpx", NULL, NULL);

		return gpx;
	}

	rapidxml::xml_node<>* getTrk(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* trk = getNode(node, "trk", NULL, NULL);

		return trk;
	}

	rapidxml::xml_node<>* getTrkSeg(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* trkseg = getNode(node, "trkseg", NULL, NULL);

		return trkseg;
	}

	void getTrkpt(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* trkpt = getNode(node, "trkpt", NULL, NULL);

		while (trkpt){

			Trkpt pt;
			pt.latitude_ = atof(getAttribStr(trkpt, "lat"));
			pt.longitude_ = atof(getAttribStr(trkpt, "lon"));
			
			rapidxml::xml_node<>* ele = getElevation(trkpt);
			if (ele){

				pt.elevation_ = atof(ele->value());
			}

			rapidxml::xml_node<>* time = getTime(trkpt);
			if (time){

				struct tm tm;

				memset(&tm, 0, sizeof(struct tm));
				strptime(time->value(), "%Y-%m-%dT%H:%M:%SZ", &tm);
				pt.time_ = mktime(&tm);
			}
			trkptlist_.push_back(pt);
			trkpt = getNextNode(trkpt, "trkpt", NULL, NULL);
		}
	}

	rapidxml::xml_node<>* getElevation(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* ele = getNode(node, "ele", NULL, NULL);

		return ele;
	}

	rapidxml::xml_node<>* getTime(rapidxml::xml_node<>* node){

		rapidxml::xml_node<>* time = getNode(node, "time", NULL, NULL);

		return time;
	}

	void load_tcx(const char* filename){

		std::string cwd(get_current_dir_name_mine());
		Chdir(dirname(std::string(filename)).c_str());

		std::string buf;

		readFromFile(basename(std::string(filename)).c_str(), buf, true);

		rapidxml::xml_document<> doc;
		char* src_text = new char[buf.length()];
		memcpy(src_text, buf.c_str(), sizeof(char)* buf.length());
		doc.parse<0>(src_text);

		getActivity(getActivities(getTrainingCenterDatabase(&doc)));

		Chdir(cwd.c_str());

		buildMesh();

		path_index_ = 0;
		user_pos_ = path_points_[0] = path_[path_index_];
		path_points_[1] = (path_[path_index_] + path_[path_index_ + 1]) * 0.5f;
		path_points_[2] = path_[path_index_ + 1];
		progress_ = 0.0f;

		SAFE_RELEASE(ptex_);
		ResourceManager<TextureBin>& manager = *(Singleton<ResourceManager<TextureBin> >::getInstance());
		std::string roadfilename("sample/image/11730.jpg");
		ptex_ = manager.get(roadfilename);
		if (NULL == ptex_){

			float w, h;
			ptex_ = new TextureBin();
			ptex_->setTexID(Img::loadImage(roadfilename.c_str(), &w, &h));
			ptex_->setWidth(w);
			ptex_->setHeight(h);
			manager.insert(roadfilename, ptex_);
		}
		SAFE_ADDREF(ptex_);
		return;
	}

	void load(const char* filename){

		std::string cwd(get_current_dir_name_mine());
		Chdir(dirname(std::string(filename)).c_str());

		std::string buf;

		readFromFile(basename(std::string(filename)).c_str(), buf, true);

		rapidxml::xml_document<> doc;
		char* src_text = new char[buf.length()];
		memcpy(src_text, buf.c_str(), sizeof(char)* buf.length());
		doc.parse<0>(src_text);

		getTrkpt(getTrkSeg(getTrk(getGpx(&doc))));

		Chdir(cwd.c_str());

		buildMesh();

		path_index_ = 0;
		user_pos_ = path_points_[0] = path_[path_index_];
		path_points_[1] = (path_[path_index_] + path_[path_index_ + 1]) * 0.5f;
		path_points_[2] = path_[path_index_ + 1];
		progress_ = 0.0f;

		SAFE_RELEASE(ptex_);
		ResourceManager<TextureBin>& manager = *(Singleton<ResourceManager<TextureBin> >::getInstance());
		std::string roadfilename("sample/image/11730.jpg");
		ptex_ = manager.get(roadfilename);
		if (NULL == ptex_){

			float w, h;
			ptex_ = new TextureBin();
			ptex_->setTexID(Img::loadImage(roadfilename.c_str(), &w, &h));
			ptex_->setWidth(w);
			ptex_->setHeight(h);
			manager.insert(roadfilename, ptex_);
		}
		SAFE_ADDREF(ptex_);
		return;
	}

	void drive(float distance, float rotateX, float rotateY){

		static int roro = 0;

		MatSolidColor mat;
		mat.setTex(ptex_);
		Matrix4 view;

		
		float t = progress_;
		
		
		Vector3 eye0 = user_pos_ = path_points_[0] * (1.0f - t) * (1.0f - t) + path_points_[1] * (1.0f - t) * t * 2.0f + t * t * path_points_[2];
		Vector3 bezdt = path_points_[0] * (-2.0f + 2.0f * t) + path_points_[1] * (1.0f - 2.0f * t) * 2.0f + 2.0f * t * path_points_[2];
		bezdt.normalize();
		
		float dist = 0.0f;
		while (dist < rotateX * 0.01f){

			progress_ += 0.01f;
			if (progress_ >= 0.5f){

				
				path_index_ = (path_index_ + 1) % path_.size();
				if (path_index_ >= path_.size() - 30){

					path_index_ = 0;
					eye0 = user_pos_ = path_points_[0] = path_[0];
					path_points_[1] = (path_[0] + path_[1]) * 0.5f;
					path_points_[2] = path_[1];
					t = progress_ = 0.0f;
					dist = 0.0f;
					bezdt = path_points_[0] * (-2.0f + 2.0f * t) + path_points_[1] * (1.0f - 2.0f * t) * 2.0f + 2.0f * t * path_points_[2];
					bezdt.normalize();
				}
				float path_distance = user_pos_.distance(path_[path_index_]);
				path_points_[0] = user_pos_;
				path_points_[1] = user_pos_ + bezdt * path_distance * 0.5f;
				path_points_[2] = path_[path_index_];

				progress_ = 0.0f;
			}

			t = progress_;
			Vector3 next_pos = path_points_[0] * (1.0f - t) * (1.0f - t) + path_points_[1] * (1.0f - t) * t * 2.0f + t * t * path_points_[2];
			float gap = rotateX * 0.01f - (dist + next_pos.distance(user_pos_));

			if (gap < 0){
				
				float right = t;
				float left = progress_ - 0.01f;
				t = (right + left) * 0.5f;
				float min_gap = 10.0f;
				int explode_timer = 10;

				while (1){
					
					next_pos = path_points_[0] * (1.0f - t) * (1.0f - t) + path_points_[1] * (1.0f - t) * t * 2.0f + t * t * path_points_[2];

					float final_distance = dist + next_pos.distance(user_pos_);
					gap = rotateX * 0.01f - final_distance;

					if (abs(gap) < abs(min_gap)){

						min_gap = gap;
						progress_ = t;
						explode_timer = 10;
					}
					else{
						explode_timer--;
						if (explode_timer <= 0){

							break;
						}
					}
					{						
						if (gap < 0.0f){

							right = t;
							t = (right + left) * 0.5f;
						}
						else if (gap > 0.0f){

							left = t;
							t = (right + left) * 0.5f;
						}
						else
							break;
					}
				}

				break;
			}
			else{

				dist += next_pos.distance(user_pos_);
				user_pos_ = next_pos;
			}
		}
		

		Matrix4 rotate;
		rotate.rotateZ(-rotateY);
		Vector3 eye_to = bezdt * rotate;
		
		Vector3 eye_center = eye0 + eye_to * 10.0f;

		float test_view[16] = { 1, 0, 0, 0,
								0, 1, 0, 0,
								0, 0, 1, 0,
								0, 0, 0, 1};
		float eye_pos[3] = { eye0.x, eye0.y, eye0.z + 1.8f};
		float center_pos[3] = { eye_center.x, eye_center.y, eye_center.z };
		float up_vector[3] = { 0, 0, 1 };
		glhLookAtf2(test_view, eye_pos, center_pos, up_vector);

		view.set(test_view);
		int start_point = path_index_ - 5;
		start_point = start_point < 0 ? 0 : start_point;
		mat.setShaderSetting(vertices_.data(), view, 0, 0, 0, 0.0f, 0.0f, 0.0f);
		glDrawElements(GL_TRIANGLES, 25 * 6, GL_UNSIGNED_SHORT, indices_.data() + start_point * 6);	CHECK_GL_ERROR();

		
	}

	void draw(float distance, float rotateX, float rotateY){

		static int roro = 0;

		MatSolidColor mat;
		mat.setTex(ptex_);
		Matrix4 view;
		mat.setShaderSetting(vertices_.data(), view, 0, 0, distance, rotateX, rotateY, 0.0f);
		glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_SHORT, indices_.data());	CHECK_GL_ERROR();
	}
};

GpxLoader loader;

void testGPXDraw(float distance, float rotateX, float rotateY, int flag){

	if (flag)
		loader.drive(distance, rotateX, rotateY);
	else
		loader.draw(distance, rotateX, rotateY);
}

int main(int argc, char *argv[]){

	char* filename = "sample/project.xml";
	if (argc > 1)
		filename = argv[1];

	RenderItemApp app(Singleton<NXT_ThemeRenderer>::getInstance());

	app.init(1280, 720);
	initMeshFactory();

	
	loader.load_tcx("sample/test.tcx");
	
	const char* test_files[] =
	{
		//"sample/nex3d/2001.NSK",
		//"sample/nex3d/4001.NSK",
		"sample/nex3d/3004020011.NSK",
		"sample/nex3d/3004010011.NSK",
		"sample/nex3d/3004060011.NSK",
		"sample/nex3d/3004750011.NSK",
		/*"sample/nex3d/2001010011.NSK",
		"sample/nex3d/2001020011.NSK",
		"sample/nex3d/2001060011.NSK",
		"sample/nex3d/2001650011.NSK",*/
		/*"sample/nex3d/1001060001.NSK",
		"sample/nex3d/1001101013.NSM",
		"sample/nex3d/1001060011.NSK",
		"sample/nex3d/1001101001.NSM",*/
		/*"sample/nex3d/1001101002.NSM",
		"sample/nex3d/1001101003.NSM",
		"sample/nex3d/1001101004.NSM",
		"sample/nex3d/1001101005.NSM",
		 "sample/nex3d/1001101006.NSM",
		"sample/nex3d/1001101007.NSM",
		"sample/nex3d/1001101008.NSM",
		"sample/nex3d/1001101009.NSM",
		"sample/nex3d/1001101010.NSM",
		"sample/nex3d/1001101011.NSM",
		"sample/nex3d/1001101012.NSM",
		"sample/nex3d/1001101013.NSM",
		"sample/nex3d/1001101014.NSM",
		"sample/nex3d/1001101015.NSM",
		"sample/nex3d/1001101016.NSM",
		"sample/nex3d/1001101017.NSM",
		 "sample/nex3d/1001101018.NSM",
		"sample/nex3d/1001101019.NSM",
		"sample/nex3d/1001101020.NSM",*/
		// "sample/nex3d/1001102001.NSM",
		// "sample/nex3d/1001102002.NSM",
		// "sample/nex3d/1001102003.NSM",
		//  "sample/nex3d/1001102004.NSM",
		// "sample/nex3d/1001102005.NSM",
		// "sample/nex3d/1001102006.NSM",
		// "sample/nex3d/1001102007.NSM",
		// "sample/nex3d/1001102008.NSM",
		// "sample/nex3d/1001102009.NSM",
		// "sample/nex3d/1001102010.NSM",
		// "sample/nex3d/1001102011.NSM",
		//  "sample/nex3d/1001102012.NSM",
		// "sample/nex3d/1001102013.NSM",
		// "sample/nex3d/1001102014.NSM",
		//  "sample/nex3d/1001102015.NSM",
		// "sample/nex3d/1001102016.NSM",
		// "sample/nex3d/1001102017.NSM",
		// "sample/nex3d/1001102018.NSM",
		// "sample/nex3d/1001102019.NSM",
		// "sample/nex3d/1001102020.NSM",
		// "sample/nex3d/1001103001.NSM",
		// "sample/nex3d/1001103002.NSM",
		// "sample/nex3d/1001103003.NSM",
		//  "sample/nex3d/1001103004.NSM",
		// "sample/nex3d/1001103005.NSM",
		// "sample/nex3d/1001103006.NSM",
		// "sample/nex3d/1001103007.NSM",
		// "sample/nex3d/1001103008.NSM",
		//  "sample/nex3d/1001103009.NSM",
		// "sample/nex3d/1001103010.NSM",
		// "sample/nex3d/1001103011.NSM",
		// "sample/nex3d/1001103012.NSM",
		//  "sample/nex3d/1001103013.NSM",
		// "sample/nex3d/1001103014.NSM",
		// "sample/nex3d/1001103015.NSM",
		// "sample/nex3d/1001103016.NSM",
		// "sample/nex3d/1001103017.NSM",
		// "sample/nex3d/1001103018.NSM",
		//  "sample/nex3d/1001103019.NSM",
		// "sample/nex3d/1001103020.NSM",
		// "sample/nex3d/1001104001.NSM",
		// "sample/nex3d/1001104002.NSM",
		//  "sample/nex3d/1001104003.NSM",
		// "sample/nex3d/1001104004.NSM",
		// "sample/nex3d/1001104005.NSM",
		// "sample/nex3d/1001104006.NSM",
		// "sample/nex3d/1001104007.NSM",
		// "sample/nex3d/1001104008.NSM",
		// "sample/nex3d/1001104009.NSM",
		// "sample/nex3d/1001104010.NSM",
		// "sample/nex3d/1001104011.NSM",
		// "sample/nex3d/1001104012.NSM",
	};

	for (int i = 0; i < sizeof(test_files) / sizeof(const char*); ++i)
		testMeshGroup(test_files[i]);

	testBone("sample/nex3d/3004.bon");
	testAni("sample/nex3d/3004100029.trk");

	app.load(filename);
	app.doLoop();
	
	return 0;
}