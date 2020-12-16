#include "NexTheme_Config.h"
#include <string.h>

#if defined(ANDROID)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_OS_IOS > 0
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif
#endif

#include "NexTheme.h"
#include "NexThemeRenderer.h"
#include "NexThemeRenderer_Internal.h"

#define  LOG_TAG    "NexThemeRenderer_Shaders"


static void registerShaderProgram( NXT_ShaderProgram_Base* base,
                                    const char *programName,
                                    const char *vertexSource,
                                    const char *fragmentSource,
                                    const NXT_Shader_LightingInfo *lightingInfo,
                                    const char *macroDefs,
                                    NXT_ShaderField* uniforms, // this fields was copied to new memory. should call unregister
                                    int uniformsCnt,
                                    NXT_ShaderField* attributes, // // this fields was copied to new memory. should call unregister
                                    int attributesCnt
                                    );
static void unregisterShaderProgram(NXT_ShaderProgram_Base* base);
static NXT_Error useShaderProgram( void* programBase,
                                   int programSize);

static NXT_Error createShaderProgram(GLuint *program,
                                     GLuint *shaderV,
                                     GLuint *shaderF,
                                     const char *programName,
                                     void* programBase,
                                     int programSize,
                                     const char *vertexSource,
                                     const char *fragmentSource,
                                     NXT_Shader_LightingInfo *lightingInfo,
                                     const char *macroDefs,
                                     NXT_ShaderField* uniforms,
                                     NXT_ShaderField* attributes);

static NXT_Error deleteShaderProgram(GLuint *program,
                                     GLuint *shaderV,
                                     GLuint *shaderF );

static GLuint loadShader(GLenum shaderType, const char* pMainCode, const char* pMacroDef);
static int validateShaderProgram( GLuint program );
static unsigned int shaderCompileStatus( GLuint hShader );
static unsigned int shaderProgramLinkStatus( GLuint hProgram );
static char* getShaderInfoLog( GLuint hShader );
static char* getProgramInfoLog( GLuint hProgram );
static void freeShaderInfoLog( char *s );
static void freeProgramInfoLog( char *s );
static void processUniforms( GLuint hShaderProgram, const char *programName, NXT_ShaderField *uniforms );
static void processAttributes( GLuint hShaderProgram, const char *programName, NXT_ShaderField *attributes );

/*static const char gVertexShader[] =
 "attribute vec4 a_position;\n"
 "attribute vec2 a_texCoord;\n"
 "varying highp vec2 v_texCoord;\n"
 "\n"
 "void main()\n"
 "{\n"
 "    gl_Position = a_position;\n"
 "    v_texCoord = a_texCoord;\n"
 "}\n";
 
 static const char gFragmentShader[] =
 "varying highp vec2 v_texCoord;\n"
 "uniform sampler2D s_texture;\n"
 "void main() {\n"
 "  gl_FragColor = texture2D(s_texture, v_texCoord);\n"
 "}\n";*/

#define GLSL(shader) #shader

static const char gStdVertexShaderPassThroughExternal_Textured[] = GLSL(

	attribute vec4 a_position;
	attribute vec2 a_texCoord;
	uniform mat4 u_tex_matrix;
	varying highp vec2 v_texCoord;

	void main() {
		v_texCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
		gl_Position = a_position;
	}

);

static const char gStdVertexShaderPassThrough_Textured[] = GLSL(

	attribute vec4 a_position;
	attribute vec2 a_texCoord;
	varying highp vec2 v_texCoord;

	void main() {
		v_texCoord = a_texCoord.st;
		gl_Position = a_position;
	}

);

static const char gFragmentShaderPassthrough_Textured[] = GLSL(

	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color)
    {
        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }

    void main()
    {
        highp vec4 color = (texture2D(u_textureSampler, v_texCoord)).rgba;
        color = applyHue(color);
		gl_FragColor = color * vec4(color.a, color.a, color.a, 1.0);
	}


);

static const char gFragmentShaderPassthroughWithColorConv_Textured[] = GLSL(

	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;
	uniform highp mat4 u_colorconv;
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }


    highp vec4 applyHue(highp vec4 color) 
    {
        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }

	void main() {
		highp vec4 color = (texture2D(u_textureSampler, v_texCoord)).rgba;
		color = color * u_colorconv;
        color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		gl_FragColor = color * vec4(color.a, color.a, color.a, 1.0);
	}

);

// Laplacian of Gaussian sharpening.
static const char gFragmentShaderSharpness_Textured[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;
    uniform highp float u_sharpness;
    uniform highp vec2 u_resolution;

    highp vec3 blurSample(in highp vec2 uv, in highp vec2 xoff, in highp vec2 yoff) {
        highp vec3 v11 = texture2D(u_textureSampler, uv + xoff).rgb;
        highp vec3 v12 = texture2D(u_textureSampler, uv + yoff).rgb;
        highp vec3 v21 = texture2D(u_textureSampler, uv - xoff).rgb;
        highp vec3 v22 = texture2D(u_textureSampler, uv - yoff).rgb;
        return (v11 + v12 + v21 + v22 + 2.0 * texture2D(u_textureSampler, uv).rgb) * 0.166667;
    }

    highp vec3 edgeStrength(in highp vec2 uv) {
        const highp float spread = 0.5;
        highp vec2 offset = 1.0 / u_resolution.xy;
        highp vec2 up = vec2(0.0, offset.y) * spread;
        highp vec2 right = vec2(offset.x, 0.0) * spread;
        
        highp vec3 v11 = blurSample(uv + up - right, right, up);
        highp vec3 v12 = blurSample(uv + up, right, up);
        highp vec3 v13 = blurSample(uv + up + right, right, up);

        highp vec3 v21 = blurSample(uv - right, right, up);
        highp vec3 v22 = blurSample(uv, right, up);
        highp vec3 v23 = blurSample(uv + right, right, up);

        highp vec3 v31 = blurSample(uv - up - right, right, up);
        highp vec3 v32 = blurSample(uv - up, right, up);
        highp vec3 v33 = blurSample(uv - up + right, right, up);

        highp vec3 laplacian_of_g = ( v11 * 0.0 + v12 *  1.0 + v13 * 0.0 )
                                  + ( v21 * 1.0 + v22 * -4.0 + v23 * 1.0 )
                                  + ( v31 * 0.0 + v32 *  1.0 + v33 * 0.0 );
        return laplacian_of_g.rgb;
    }

    void main() {
        gl_FragColor = vec4(texture2D(u_textureSampler, v_texCoord).rgb - edgeStrength(v_texCoord.xy) * u_sharpness, 1.0);
    }
);

static const char gFragmentShaderUserVignette_Textured[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;
    uniform highp float u_vignette; // Position for the Outer vignette
    uniform highp float u_vignetteRange; // Position for the inner Vignette Ring

    void main() {
        highp vec4 color = texture2D(u_textureSampler, v_texCoord);

        highp vec2 center = vec2(0.5, .5);
        highp float dist = distance(center, v_texCoord.xy) * 1.414213;
        highp float vig = clamp((u_vignette - dist) / u_vignetteRange, 0.0, 1.0);     
        gl_FragColor = vec4(vig * color.rgb, 1);
    }
);

static const char gFragmentShaderLUT_Textured[] = GLSL(
	const highp float block_factor = 64.0;
	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;
	uniform sampler2D u_textureLUT;
	uniform highp float u_realX;
	uniform highp float u_realY;
	uniform highp float u_alpha;

	void main() {
		
		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		if(s > 0.0)
		{
			highp vec4 color = (texture2D(u_textureSampler, v_texCoord)).rgba;
			highp float block = (block_factor - 1.0) * color.b;

			highp float low = floor(block);
			highp float high = ceil(block);
			highp float y0 = mod(low, 8.0);
			highp float x0 = (low - y0) / 8.0;
			highp float y1 = mod(high, 8.0);
			highp float x1 = (high - y1) / 8.0;

			x0 /= 8.0;
			y0 /= 8.0;
			x1 /= 8.0;
			y1 /= 8.0;
			
			color.rg = (63.0 / 512.0) * color.rg + 0.5 / 512.0;
			highp vec4 color0 = texture2D(u_textureLUT, vec2(x0, y0) + color.gr).rgba;
			highp vec4 color1 = texture2D(u_textureLUT, vec2(x1, y1) + color.gr).rgba;
			highp vec4 finalcolor = vec4(mix(color0, color1, fract(block)).rgb, color.a);
			gl_FragColor = finalcolor * u_alpha;
		}
		else
			discard;
	}
);

#ifdef USE_FRAGMENT_SHADER_LUT_64x4096
static const char gFragmentShaderLUT_64x4096_Textured[] = GLSL(
    const highp float block_factor = 64.0;
    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;
    uniform sampler2D u_textureLUT;
    uniform highp float u_realX;
    uniform highp float u_realY;
    uniform highp float u_alpha;
    void main() {
        highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
        if(s > 0.0)
        {
            highp vec4 color = (texture2D(u_textureSampler, v_texCoord)).rgba;
            highp float block = (block_factor - 1.0) * color.b;
            color.r = (62.0 / 64.0) * color.r + 1.0 / 64.0;
            highp float t0 = 1.0 - (floor(block) + color.r) / block_factor;
            highp float t1 = 1.0 - (ceil(block) + color.r) / block_factor;
            highp vec4 color0 = texture2D(u_textureLUT, vec2(color.g, t0)).rgba;
            highp vec4 color1 = texture2D(u_textureLUT, vec2(color.g, t1)).rgba;
            gl_FragColor = vec4(mix(color0, color1, fract(block)).rgb, color.a);
        }
        else
            gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
);
#endif

// static const char gFragmentShaderLGLUT_Textured[] = GLSL(
// 	const highp float block_factor = 64.0;
// 	varying highp vec2 v_texCoord;
// 	uniform sampler2D u_textureSampler;
// 	uniform sampler2D u_textureLUT;
// 	uniform highp float u_realX;
// 	uniform highp float u_realY;

// 	void main() {
		
// 		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
// 		if(s > 0.0)
// 		{
// 			const highp mat4 rgbyuv = mat4(
//                          0.300,  0.587,  0.114,  0.000,
//                         -0.169, -0.331,  0.500,  0.500,
//                          0.500, -0.419, -0.081,  0.500,
//                          0.000,  0.000,  0.000,  1.000);
// 	        highp vec4 color = (texture2D(u_textureSampler, v_texCoord)).rgba * rgbyuv;

// 			highp float block = (block_factor - 1.0) * color.b;

// 			highp float low = floor(block);
// 			highp float high = ceil(block);
// 			highp float y0 = mod(low, 8.0);
// 			highp float x0 = (low - y0) / 8.0;
// 			highp float y1 = mod(high, 8.0);
// 			highp float x1 = (high - y1) / 8.0;

// 			x0 /= 8.0;
// 			y0 /= 8.0;
// 			x1 /= 8.0;
// 			y1 /= 8.0;
			
// 			color.rg = (63.0 / 512.0) * color.rg + 0.5 / 512.0;
// 			highp vec4 color0 = texture2D(u_textureLUT, vec2(x0, y0) + color.rg).rgba;
// 			highp vec4 color1 = texture2D(u_textureLUT, vec2(x1, y1) + color.rg).rgba;
// 			gl_FragColor = vec4(mix(color0, color1, fract(block)).rgb, color.a);
// 		}
// 		else
// 			discard;
// 	}
// );
//For test normal refraction
//static const char gFragmentShaderLUT_Textured[] = GLSL(
//const highp float block_factor = 64.0;
//varying highp vec2 v_texCoord;
//uniform sampler2D u_textureSampler;
//uniform sampler2D u_textureLUT;
//uniform highp float u_strength;
//
//void main() {
//	highp vec3 normal = normalize(texture2D(u_textureLUT, v_texCoord).xyz);
//	highp vec2 uv = v_texCoord + (2.0 * normal.xy - 1.0) * 0.05;
//	gl_FragColor = texture2D(u_textureSampler, uv).rgba;;
//}
//);
////Mosaic
//static const char gFragmentShaderForFilter_Textured[] = GLSL(
//	varying highp vec2 v_texCoord;
//	uniform sampler2D u_textureSampler;
//	uniform highp vec2 u_texture_size;
//	
//	void main(void)
//	{
//		vec2 uv = v_texCoord;
//		if (uv.x <= 0.01f){
//			gl_FragColor = vec4(1, 0, 0, 1);
//		}
//		else if (uv.x >= 0.99f){
//			gl_FragColor = vec4(1, 0, 0, 1);
//		}
//		else if (uv.y <= 0.01f){
//			gl_FragColor = vec4(1, 0, 0, 1);
//		}
//		else if (uv.y >= 0.99f){
//			gl_FragColor = vec4(1, 0, 0, 1);
//		}
//		else{
//			float dx = 15.0*(1. / u_texture_size.x);
//			float dy = 10.0*(1. / u_texture_size.y);
//			vec2 coord = vec2(dx*floor(uv.x / dx),
//				dy*floor(uv.y / dy));
//			gl_FragColor = texture2D(u_textureSampler, coord);
//		}
//	}
//);

//Shockwave
// static const char gFragmentShaderForFilter_Textured[] = GLSL(
// 	varying highp vec2 v_texCoord;
// 	uniform sampler2D u_textureSampler;
// 	uniform highp vec2 u_texture_size;
// 	uniform float u_time; // effect elapsed time
// 	const vec3 shockParams = vec3(10.0, 0.8, 0.1);
// 	void main()
// 	{
// 		vec2 uv = v_texCoord.xy;
// 		vec2 texCoord = uv;
// 		float time = sin(u_time);
// 		float distance = distance(uv, u_texture_size);
// 		if ((distance <= (time + shockParams.z)) &&
// 			(distance >= (time - shockParams.z)))
// 		{
// 			float diff = (distance - time);
// 			float powDiff = 1.0 - pow(abs(diff*shockParams.x),
// 				shockParams.y);
// 			float diffTime = diff  * powDiff;
// 			vec2 diffUV = normalize(uv - u_texture_size);
// 			texCoord = uv + (diffUV * diffTime);
// 		}
// 		gl_FragColor = texture2D(u_textureSampler, texCoord);
// 	}
// );

// //Ripple
// static const char gFragmentShaderForFilter_Textured[] = GLSL(
// 	varying highp vec2 v_texCoord;
// 	uniform sampler2D u_textureSampler;
// 	uniform highp vec2 u_texture_size;
// 	uniform float u_time; // effect elapsed time
// 	const vec3 shockParams = vec3(10.0, 0.8, 0.1);
// 	void main()
// 	{
// 		vec2 cPos = -1.0 + 2.0 * v_texCoord.xy / u_texture_size.xy;
// 		float cLength = length(cPos);

// 		vec2 uv = v_texCoord.xy/u_texture_size.xy+(cPos/cLength)*cos((cLength*12.0-u_time*4.0)*2.0)*0.03;
// 		vec3 col = texture2D(u_textureSampler,uv).xyz;

// 		gl_FragColor = vec4(col,1.0);
// 	}
// );

////radial blur
//static const char gFragmentShaderForFilter_Textured[] = GLSL(
//	varying highp vec2 v_texCoord;
//	uniform sampler2D u_textureSampler;
//	uniform highp vec2 u_texture_size;
//	
//	void main() 
//	{
//	  vec3 p = vec3(v_texCoord.xy/u_texture_size-.5, 0.0);
//	  vec3 o = texture2D(u_textureSampler,.5+(p.xy*=.992)).rgb;
//	  for (float i=0.;i<100.;i++) 
//	    p.z += pow(max(0.,.5-length(texture2D(u_textureSampler,.5+(p.xy*=.992)).rg)),2.)*exp(-i*.08);
//	  gl_FragColor=vec4(o*o+p.z,1);
//	}
//);

////chroma keying - first attemp
//static const char gFragmentShaderForFilter_Textured[] = GLSL(
//	varying highp vec2 v_texCoord;
//	uniform sampler2D u_textureSampler;
//	
//	void main() 
//	{
//		highp vec4 col = vec4(texture2D(u_textureSampler, v_texCoord).xyz, 1.0);
//		float rb_length = length(vec2(col.rb));
//		if (rb_length < 0.7){
//			col.a = smoothstep(0.2, 0.4, 1.0 - col.g);
//			if (col.a < 0.2)
//				col.g = 0.0;
//		}
//		gl_FragColor = col;
//	}
//);

//chroma keying - second attemp
static const char gFragmentShaderForFilter_Textured[] = GLSL(
	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;
	const highp vec3 concentric = vec3(0.0, 1.0, 0.0);
	const highp vec4 division = vec4(0.3, 0.6, 0.9, 1.2);

	void main()
	{
		highp vec4 col = vec4(texture2D(u_textureSampler, v_texCoord).xyz, 1.0);
		float radius = length(vec3(col.rgb) - concentric);
		col.a = smoothstep(division.x, division.z, radius);
		col.g = col.g * col.a;
		gl_FragColor = col;
	}
);

//------ TEXTURED+RGBA->YUVA8888 ----------------------------------------------------------
static const char gFragmentShaderPassthrough_TexturedRGBAtoYUVA8888[] = GLSL(

	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;

	void main() {
		const highp mat4 rgbyuv = mat4(
			0.300, 0.589, 0.111, -0.003,
			-0.169, -0.332, 0.502, 0.502,
			0.499, -0.420, -0.079, 0.502,
			0.000, 0.000, 0.000, 1.000);
		highp vec4 color;
		color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		gl_FragColor = CHANNEL_FILTER(color * rgbyuv);
	}

);

//------ TEXTURED+YUV ----------------------------------------------------------
static const char gFragmentShaderPassthrough_TexturedYUV[] = GLSL(

	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSamplerY;
	uniform sampler2D u_textureSamplerU;
	uniform sampler2D u_textureSamplerV;
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }

	void main() {

		const highp mat4 yuvrgb = mat4(
			1.000, 0.000, 1.402, -0.701,
			1.000, -0.334, -0.714, 0.529,
			1.000, 1.772, 0.000, -0.886,
			0.000, 0.000, 0.000, 1.000);
		highp vec4 color = vec4(texture2D(u_textureSamplerY, v_texCoord).r,
			texture2D(u_textureSamplerU, v_texCoord).r,
			texture2D(u_textureSamplerV, v_texCoord).r,
			1.0)*yuvrgb;
        
        color = applyHue(color);
		gl_FragColor = color * vec4(color.a, color.a, color.a, 1.0);
	}
);
//------ TEXTURED+NV12 ----------------------------------------------------------
// Needs lighting
static const char gFragmentShaderPassthrough_TexturedNV12[] = GLSL(
	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSamplerY;
	uniform sampler2D u_textureSamplerUV;
	uniform highp mat4 u_colorconv;
	uniform highp float u_realY;
	uniform highp float u_realX;
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }

	void main() {
		highp vec4 color_pre;
		highp vec4 color;
		color_pre.r = texture2D(u_textureSamplerY, v_texCoord).r;
		color_pre.g = texture2D(u_textureSamplerUV, v_texCoord).r;
#ifdef __APPLE__
        color_pre.b = texture2D(u_textureSamplerUV, v_texCoord).g; // RG_EXT for iOS
#else
        color_pre.b = texture2D(u_textureSamplerUV, v_texCoord).a;
#endif
		color_pre.a = 1.0;
		color_pre = color_pre*u_colorconv;
		color = color_pre.rgba;
        color = applyHue(color);
		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		gl_FragColor = color * vec4(color.a * s, color.a * s, color.a * s, 1.0);
	}

);

//------ TEXTURED+EXTERNAL --------------------------------------------------------------
static const char gFragmentShaderPassthrough_TexturedExternal[] =
	"#extension GL_OES_EGL_image_external : require\n"
	GLSL(

	varying highp vec2			v_texCoord;
	uniform samplerExternalOES	u_textureSampler;
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color)
    {
        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	
	void main() {
		highp vec4 color = texture2D(u_textureSampler, v_texCoord).rgba;
        color = applyHue(color);
		gl_FragColor = color * vec4(color.a, color.a, color.a, 1.0);
	}


);

//------ TEXTURED+EXTERNAL+HDR10 --------------------------------------------------------------
static const char gFragmentShaderPassthrough_TexturedExternalHDR10[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;
        uniform samplerExternalOES u_textureSampler;
        
        uniform highp float u_MaximumContentLightLevel;   //Maximum Content Light Level
        uniform highp float u_DeviceLightLevel;  //Maximum Frame Average Light Level
        uniform highp float u_Gamma;     //device gamma

        highp float Gamma(highp float V, highp float gamma) {

            return pow(V, 1.0 / gamma);
        }

        highp float iEOTF_PQ(highp float V) {

            const highp float c1 = 0.8359375;
            const highp float c2 = 18.8515625;
            const highp float c3 = 18.6875;
            const highp float m = 78.84375;
            const highp float n = 0.1593017578125;

            return pow(max(pow(V, 1.0 / m) - c1, 0.0) / (c2 - c3 * pow(V, 1.0 / m)), 1.0 / n) * u_MaximumContentLightLevel;
        }

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }

        void main() {
            highp vec4 color = texture2D(u_textureSampler, v_texCoord).rgba;

            color.r = Gamma(iEOTF_PQ(color.r) / u_DeviceLightLevel, u_Gamma);
            color.g = Gamma(iEOTF_PQ(color.g) / u_DeviceLightLevel, u_Gamma);
            color.b = Gamma(iEOTF_PQ(color.b) / u_DeviceLightLevel, u_Gamma);

            const highp mat4 conv = mat4(
                1.660511, -0.587711, -0.072801, 0.0,
                -0.124561, 1.132961, -0.008399, 0.0,
                -0.018168, -0.100561, 1.118728, 0.0,
                0.000, 0.000, 0.000, 1.000);

            color = color * conv;
            color = applyHue(color);
            gl_FragColor = color * vec4(color.a, color.a, color.a, 1.0);

        });

//For Layers....
static const char gStdVertexShader_Textured_For_Layers[] = GLSL(

	attribute vec4 a_position;
	attribute vec2 a_texCoord;
	attribute vec4 a_normal;
	uniform mat4 u_mvp_matrix;
	uniform mat4 u_tex_matrix;
	varying highp vec2 v_texCoord;
	varying highp vec2 v_texCoord_for_mask;

	// Lighting
	uniform highp vec3 u_lightpos;
	uniform highp float u_light_f0;
	uniform highp float u_light_f1;
	uniform highp float u_specexp;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	void main() {
		highp vec3 light_direction;
		highp vec3 half_plane;

		v_texCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
		gl_Position = a_position * u_mvp_matrix;
		v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;

		// Lighting
		//highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
		//light_direction = normalize(u_lightpos.xyz/* - gl_Position.xyz*/);
		//half_plane = normalize(light_direction + vec3(0.0, 0.0, 1.0));
		//highp float ndl = dot(normal, light_direction);
		//v_diffuse_intensity = max(0.0, ndl*u_light_f0) + max(0.0, ndl*u_light_f1);
		//v_specular_intensity = pow(max(0.0, dot(normal, half_plane)), u_specexp);
	}

);
static const char gFragmentShader_Textured_For_Layers[] = GLSL(

	varying highp vec2 v_texCoord;
	varying highp vec2 v_texCoord_for_mask;
	uniform sampler2D u_textureSampler;
	uniform sampler2D u_textureSampler_for_mask;
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;

	// ---- Lighting ----
	uniform highp vec4 u_diffuse_light;
	uniform highp vec4 u_ambient_light;
	uniform highp vec4 u_specular_light;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
	}
	// -------------------

	void main() {
		highp vec4 color;
		color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);		
		gl_FragColor = u_alpha * color * mask.r;
	}
);

static const char gStdVertexShader_Textured[] = GLSL(

	attribute vec4 a_position;
	attribute vec2 a_texCoord;
	attribute vec4 a_normal;
	uniform mat4 u_mvp_matrix;
	uniform mat4 u_tex_matrix;
	varying highp vec2 v_texCoord;

	// Lighting
	uniform highp vec3 u_lightpos;
	uniform highp float u_light_f0;
	uniform highp float u_light_f1;
	uniform highp float u_specexp;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	void main() {

		highp vec3 light_direction;
		highp vec3 half_plane;

		v_texCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
		gl_Position = a_position * u_mvp_matrix;

		// Lighting
		highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
		light_direction = normalize(u_lightpos.xyz/* - gl_Position.xyz*/);
		half_plane = normalize(light_direction + vec3(0.0, 0.0, 1.0));
		highp float ndl = dot(normal, light_direction);
		v_diffuse_intensity = max(0.0, ndl*u_light_f0) + max(0.0, ndl*u_light_f1);
		v_specular_intensity = pow(max(0.0, dot(normal, half_plane)), u_specexp);
	}

);
static const char gStdVertexShaderForScreenMasking_Textured[] = GLSL(

    attribute vec4 a_position;
    attribute vec2 a_texCoord;
    attribute vec4 a_normal;
    uniform mat4 u_mvp_matrix;
    uniform mat4 u_tex_matrix;
    varying highp vec2 v_texCoord;
    //ScreenMasking
    varying highp vec2 v_texCoord_for_mask;

    // Lighting
    uniform highp vec3 u_lightpos;
    uniform highp float u_light_f0;
    uniform highp float u_light_f1;
    uniform highp float u_specexp;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    void main() {

        highp vec3 light_direction;
        highp vec3 half_plane;

        v_texCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
        gl_Position = a_position * u_mvp_matrix;
        //ScreenMasking
        v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;
        

        // Lighting
        highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
        light_direction = normalize(u_lightpos.xyz /* - gl_Position.xyz*/);
        half_plane = normalize(light_direction + vec3(0.0, 0.0, 1.0));
        highp float ndl = dot(normal, light_direction);
        v_diffuse_intensity = max(0.0, ndl * u_light_f0) + max(0.0, ndl * u_light_f1);
        v_specular_intensity = pow(max(0.0, dot(normal, half_plane)), u_specexp);
    }

);

static const char gStdVertexShader_CombinedTextured[] = GLSL(

	attribute vec4 a_position;
	attribute vec2 a_texCoord;
	attribute vec4 a_normal;
	uniform mat4 u_mvp_matrix;
	uniform mat4 u_tex_matrix;
	uniform mat4 u_back_tex_matrix;
	varying highp vec2 v_texCoord;
	varying highp vec2 v_texCoord_for_back;

	// Lighting
	uniform highp vec3 u_lightpos;
	uniform highp float u_light_f0;
	uniform highp float u_light_f1;
	uniform highp float u_specexp;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	void main() {

		highp vec3 light_direction;
		highp vec3 half_plane;

		v_texCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
		v_texCoord_for_back = (vec4(a_texCoord, 1.0, 1.0) * u_back_tex_matrix).st;
		gl_Position = a_position * u_mvp_matrix;

		// Lighting
		highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
		light_direction = normalize(u_lightpos.xyz/* - gl_Position.xyz*/);
		half_plane = normalize(light_direction + vec3(0.0, 0.0, 1.0));
		highp float ndl = dot(normal, light_direction);
		v_diffuse_intensity = max(0.0, ndl*u_light_f0) + max(0.0, ndl*u_light_f1);
		v_specular_intensity = pow(max(0.0, dot(normal, half_plane)), u_specexp);
	}

);

static const char gStdVertexShaderForScreenMasking_CombinedTextured[] = GLSL(

    attribute vec4 a_position;
    attribute vec2 a_texCoord;
    attribute vec4 a_normal;
    uniform mat4 u_mvp_matrix;
    uniform mat4 u_tex_matrix;
    uniform mat4 u_back_tex_matrix;
    varying highp vec2 v_texCoord;
    varying highp vec2 v_texCoord_for_back;

    varying highp vec2 v_texCoord_for_mask;

    // Lighting
    uniform highp vec3 u_lightpos;
    uniform highp float u_light_f0;
    uniform highp float u_light_f1;
    uniform highp float u_specexp;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    void main() {

		highp vec3 light_direction;
		highp vec3 half_plane;

		v_texCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
		v_texCoord_for_back = (vec4(a_texCoord, 1.0, 1.0) * u_back_tex_matrix).st;
		gl_Position = a_position * u_mvp_matrix;
        v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;
      
        // Lighting
		highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
		light_direction = normalize(u_lightpos.xyz/* - gl_Position.xyz*/);
		half_plane = normalize(light_direction + vec3(0.0, 0.0, 1.0));
		highp float ndl = dot(normal, light_direction);
		v_diffuse_intensity = max(0.0, ndl*u_light_f0) + max(0.0, ndl*u_light_f1);
		v_specular_intensity = pow(max(0.0, dot(normal, half_plane)), u_specexp);
    }

);

static const char gHomoVertexShader[] = GLSL(

	attribute vec4 a_position;
	uniform mat4 u_mvp_matrix;
	varying highp vec2 v_homopos;

	void main() {

		gl_Position = a_position * u_mvp_matrix;
		v_homopos = gl_Position.xy;
	}
);

static const char gHomoFragmentShader_Textured[] = GLSL(

	varying highp vec2 v_homopos;
	uniform sampler2D u_textureSampler;
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;
	uniform highp mat4 u_tex_matrix;
	uniform highp mat3 u_homo_matrix;
	uniform highp float u_realX;
	uniform highp float u_realY;

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	void main() {
		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;
		highp float s = step(0.0, texCoord.y) * step(0.0, texCoord.x) * step(texCoord.y, u_realY) * step(texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0){
			
			color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;			
		}
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		gl_FragColor = u_alpha * color;
	}
);

static const char gHomoVertexShaderForScreenMasking[] = GLSL(

    attribute vec4 a_position;
    uniform mat4 u_mvp_matrix;
    varying highp vec2 v_homopos;
    varying highp vec2 v_texCoord_for_mask;

    void main() {

		gl_Position = a_position * u_mvp_matrix;
		v_homopos = gl_Position.xy;
        v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;
    });

static const char gHomoFragmentShaderForScreenMasking_Textured[] = GLSL(

	varying highp vec2 v_homopos;
	uniform sampler2D u_textureSampler;
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;
	uniform highp mat4 u_tex_matrix;
	uniform highp mat3 u_homo_matrix;
	uniform highp float u_realX;
	uniform highp float u_realY;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;
    
    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	void main() {
		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;
		highp float s = step(0.0, texCoord.y) * step(0.0, texCoord.x) * step(texCoord.y, u_realY) * step(texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0){
			
			color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;			
		}
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
        gl_FragColor = applyMask(u_alpha * color);
	}
);

static const char gHomoFragmentShader_CombinedTextured[] = GLSL(

	varying highp vec2 v_homopos;
	uniform sampler2D u_textureSampler;
	uniform sampler2D u_back_textureSampler;
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;
	uniform highp mat4 u_tex_matrix;
	uniform highp mat4 u_back_tex_matrix;
	uniform highp mat3 u_homo_matrix;
	uniform highp float u_realX;
	uniform highp float u_realY;
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }

	void main() {
		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;
		highp float s = step(0.0, texCoord.y) * step(0.0, texCoord.x) * step(texCoord.y, u_realY) * step(texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color;
		if(s > 0.0){
			
			color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;
		}
		else{

			texCoord = (vec4(st.xy, 1.0, 1.0) * u_back_tex_matrix).st;
			color.rgba = (texture2D(u_back_textureSampler, texCoord)).rgba;
		}
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		gl_FragColor = u_alpha * color;
	}
);

static const char gHomoFragmentShaderForScreenMasking_CombinedTextured[] = GLSL(

    varying highp vec2 v_homopos;
    uniform sampler2D u_textureSampler;
    uniform sampler2D u_back_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    uniform highp mat4 u_tex_matrix;
    uniform highp mat4 u_back_tex_matrix;
    uniform highp mat3 u_homo_matrix;
    uniform highp float u_realX;
    uniform highp float u_realY;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

	highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    void main() {
		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;
		highp float s = step(0.0, texCoord.y) * step(0.0, texCoord.x) * step(texCoord.y, u_realY) * step(texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color;
		if(s > 0.0){
			
			color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;
		}
		else{

			texCoord = (vec4(st.xy, 1.0, 1.0) * u_back_tex_matrix).st;
			color.rgba = (texture2D(u_back_textureSampler, texCoord)).rgba;
		}
		color = applyHue(color) * u_colorconv;
		color = clamp(color, 0.0, 1.0);
		gl_FragColor = applyMask(u_alpha * color);
    }
);

static const char gHomoFragmentShader_Textured_vignette[] = GLSL(

	varying highp vec2 v_homopos;
	uniform sampler2D u_textureSampler;	
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;
	uniform highp mat4 u_tex_matrix;
	uniform highp mat3 u_homo_matrix;
	varying highp vec2 v_texCoord_for_vignette;
	uniform sampler2D u_textureSampler_vignette;
	uniform highp float u_realX;
	uniform highp float u_realY;

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }

	void main() {

		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;

		highp float s = step(0.0, texCoord.y) * step(0.0, texCoord.x) * step(texCoord.y, u_realY) * step(texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0){
			
			color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;			
		}

		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		color = u_alpha * color;

		highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, st.xy)).rgba;
		color.rgb = color.rgb * vignette.r;

		gl_FragColor = color;
	}

);

static const char gHomoFragmentShaderForScreenMasking_Textured_vignette[] = GLSL(

	varying highp vec2 v_homopos;
	uniform sampler2D u_textureSampler;	
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;
	uniform highp mat4 u_tex_matrix;
	uniform highp mat3 u_homo_matrix;
	varying highp vec2 v_texCoord_for_vignette;
	uniform sampler2D u_textureSampler_vignette;
	uniform highp float u_realX;
	uniform highp float u_realY;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;
    
    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	void main() {

		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;

		highp float s = step(0.0, texCoord.y) * step(0.0, texCoord.x) * step(texCoord.y, u_realY) * step(texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0){
			
			color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;			
		}

		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		color = u_alpha * color;

		highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, st.xy)).rgba;
		color.rgb = color.rgb * vignette.r;

		gl_FragColor = applyMask(color);
	}

);

static const char gHomoFragmentShader_CombinedTextured_vignette[] = GLSL(

	varying highp vec2 v_homopos;
	uniform sampler2D u_textureSampler;	
	uniform sampler2D u_back_textureSampler;	
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;
	uniform highp mat4 u_tex_matrix;
	uniform highp mat4 u_back_tex_matrix;
	uniform highp mat3 u_homo_matrix;
	varying highp vec2 v_texCoord_for_vignette;
	uniform sampler2D u_textureSampler_vignette;
	uniform highp float u_realX;
	uniform highp float u_realY;


	void main() {

		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;

		highp float s = step(0.0, texCoord.y) * step(0.0, texCoord.x) * step(texCoord.y, u_realY) * step(texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color;
		if(s > 0.0){
			
			color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;
		}
		else{

			texCoord = (vec4(st.xy, 1.0, 1.0) * u_back_tex_matrix).st;
			color.rgba = (texture2D(u_back_textureSampler, texCoord)).rgba;			
		}
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
		color = u_alpha * color;

		highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, st.xy)).rgba;
		color.rgb = color.rgb * vignette.r;

		gl_FragColor = color;
	}
);

static const char gHomoFragmentShaderForScreenMasking_CombinedTextured_vignette[] = GLSL(

    varying highp vec2 v_homopos;
    uniform sampler2D u_textureSampler;
    uniform sampler2D u_back_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    uniform highp mat4 u_tex_matrix;
    uniform highp mat4 u_back_tex_matrix;
    uniform highp mat3 u_homo_matrix;
    varying highp vec2 v_texCoord_for_vignette;
    uniform sampler2D u_textureSampler_vignette;
    uniform highp float u_realX;
    uniform highp float u_realY;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;
    
    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    void main() {

        highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
        st.xy /= st.z;
        highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;

        highp float s = step(0.0, texCoord.y) * step(0.0, texCoord.x) * step(texCoord.y, u_realY) * step(texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
        highp vec4 color;
        if (s > 0.0)
        {

            color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;
        }
        else
        {

            texCoord = (vec4(st.xy, 1.0, 1.0) * u_back_tex_matrix).st;
            color.rgba = (texture2D(u_back_textureSampler, texCoord)).rgba;
        }
        color = color * u_colorconv;
        color = clamp(color, 0.0, 1.0);
        color = u_alpha * color;

        highp vec4 vignette;
        vignette = (texture2D(u_textureSampler_vignette, st.xy)).rgba;
        color.rgb = color.rgb * vignette.r;

        gl_FragColor = applyMask(color);
    });

static const char gHomoFragmentShader_MaskedStencil[] = GLSL(
	
	varying highp vec2 v_homopos;
	uniform sampler2D u_textureSampler;
	uniform sampler2D u_maskSampler;
	uniform highp float u_alpha;
	uniform highp vec4 u_color;
	uniform highp mat4 u_maskxfm;
	uniform highp mat4 u_tex_matrix;
	uniform highp mat4 u_mask_matrix;
	uniform highp mat3 u_homo_matrix;
	uniform highp mat3 u_mask_homo_matrix;
	void main() {

		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;

		highp vec3 mask_st = vec3(v_homopos.xy, 1.0) * u_mask_homo_matrix;
		mask_st.xy /= mask_st.z;
		highp vec2 maskCoord = mask_st.xy;

	    highp vec4 color;
	    color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;
	    color = color*(1.0-u_color.a)+u_color;
	    color = (texture2D(u_maskSampler, maskCoord)*u_maskxfm).r*u_alpha*color;
	    if( color.a < 0.5 )
	      discard;
	    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	}
);

static const char gHomoFragmentShaderForScreenMasking_MaskedStencil[] = GLSL(

    varying highp vec2 v_homopos;
    uniform sampler2D u_textureSampler;
    uniform sampler2D u_maskSampler;
    uniform highp float u_alpha;
    uniform highp vec4 u_color;
    uniform highp mat4 u_maskxfm;
    uniform highp mat4 u_tex_matrix;
    uniform highp mat4 u_mask_matrix;
    uniform highp mat3 u_homo_matrix;
    uniform highp mat3 u_mask_homo_matrix;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    void main() {

        highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
        st.xy /= st.z;
        highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;

        highp vec3 mask_st = vec3(v_homopos.xy, 1.0) * u_mask_homo_matrix;
        mask_st.xy /= mask_st.z;
        highp vec2 maskCoord = mask_st.xy;

        highp vec4 color;
        color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;
        color = color * (1.0 - u_color.a) + u_color;
        color = applyMask( (texture2D(u_maskSampler, maskCoord) * u_maskxfm).r * u_alpha * color);
        if (color.a < 0.5)
            discard;
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    });

static const char gHomoFragmentShader_Masked[] = GLSL(

	varying highp vec2 v_homopos;
	uniform sampler2D u_textureSampler;
	uniform sampler2D u_maskSampler;
	uniform highp float u_alpha;
	uniform highp vec4 u_color;
	uniform highp mat4 u_colorconv;
	uniform highp mat4 u_maskxfm;
	uniform highp mat4 u_tex_matrix;
	uniform highp mat3 u_homo_matrix;
	
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	void main() {

		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;

		highp vec3 mask_st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		mask_st.xy /= mask_st.z;
		highp vec2 maskCoord = mask_st.xy;

	    highp vec4 color;
	    highp float s = step(0.0,texCoord.y) * step(0.0,texCoord.x) * step(texCoord.y,1.0) * step(texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside
	    if(s <= 0.0)
	    	color = vec4(0, 0, 0, 1);
	    else
	    	color.rgba = texture2D(u_textureSampler, texCoord).rgba;
	    color = color*u_colorconv;
        color = applyHue(color);
	    color = u_color*(1.0-color.a)+color;
	    color = (texture2D(u_maskSampler, maskCoord)*u_maskxfm).r*u_alpha*color;
	    gl_FragColor = color;
	}
);

static const char gHomoFragmentShaderForScreenMasking_Masked[] = GLSL(

    varying highp vec2 v_homopos;
    uniform sampler2D u_textureSampler;
    uniform sampler2D u_maskSampler;
    uniform highp float u_alpha;
    uniform highp vec4 u_color;
    uniform highp mat4 u_colorconv;
    uniform highp mat4 u_maskxfm;
    uniform highp mat4 u_tex_matrix;
    uniform highp mat3 u_homo_matrix;
    uniform highp mat3 u_mask_homo_matrix;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {

		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;

		highp vec3 mask_st = vec3(v_homopos.xy, 1.0) * u_mask_homo_matrix;
		mask_st.xy /= mask_st.z;
		highp vec2 maskCoord = mask_st.xy;

	    highp vec4 color;
	    highp float s = step(0.0,texCoord.y) * step(0.0,texCoord.x) * step(texCoord.y,1.0) * step(texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside
	    if(s <= 0.0)
	    	color = vec4(0, 0, 0, 1);
	    else
	    	color.rgba = texture2D(u_textureSampler, texCoord).rgba;
	    color = color*u_colorconv;
        color = applyHue(color);
	    color = u_color*(1.0-color.a)+color;
	    color = (texture2D(u_maskSampler, maskCoord)*u_maskxfm).r*u_alpha*color;
	    gl_FragColor = applyMask( color);
    });

static const char gHomoFragmentShader_CombinedMasked[] = GLSL(

	varying highp vec2 v_homopos;
	uniform sampler2D u_textureSampler;
	uniform sampler2D u_back_textureSampler;
	uniform sampler2D u_maskSampler;
	uniform highp float u_alpha;
	uniform highp vec4 u_color;
	uniform highp mat4 u_colorconv;
	uniform highp mat4 u_maskxfm;
	uniform highp mat4 u_tex_matrix;
	uniform highp mat3 u_homo_matrix;
	uniform highp mat3 u_mask_homo_matrix;
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	void main() {

		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;

		highp vec3 mask_st = vec3(v_homopos.xy, 1.0) * u_mask_homo_matrix;
		mask_st.xy /= mask_st.z;
		highp vec2 maskCoord = mask_st.xy;

	    highp vec4 color;
	    highp float s = step(0.0,texCoord.y) * step(0.0,texCoord.x) * step(texCoord.y,1.0) * step(texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside
	    if(s <= 0.0){

	    	texCoord = (vec4(st.xy, 1.0, 1.0) * u_back_tex_matrix).st;
	    	color.rgba = texture2D(u_back_textureSampler, texCoord).rgba;	    	
	    }
	    else{

	    	color.rgba = texture2D(u_textureSampler, texCoord).rgba;	    	
	    }
	    
	    color = color*u_colorconv;
        color = applyHue(color);
	    color = u_color*(1.0-color.a)+color;
	    color = (texture2D(u_maskSampler, maskCoord)*u_maskxfm).r*u_alpha*color;
	    gl_FragColor = color;
	}
);

static const char gHomoFragmentShaderForScreenMasking_CombinedMasked[] = GLSL(

    varying highp vec2 v_homopos;
    uniform sampler2D u_textureSampler;
    uniform sampler2D u_back_textureSampler;
    uniform sampler2D u_maskSampler;
    uniform highp float u_alpha;
    uniform highp vec4 u_color;
    uniform highp mat4 u_colorconv;
    uniform highp mat4 u_maskxfm;
    uniform highp mat4 u_tex_matrix;
    uniform highp mat3 u_homo_matrix;
    uniform highp mat3 u_mask_homo_matrix;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    void main() {

        highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
        st.xy /= st.z;
        highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;

        highp vec3 mask_st = vec3(v_homopos.xy, 1.0) * u_mask_homo_matrix;
        mask_st.xy /= mask_st.z;
        highp vec2 maskCoord = mask_st.xy;

        highp vec4 color;
        highp float s = step(0.0, texCoord.y) * step(0.0, texCoord.x) * step(texCoord.y, 1.0) * step(texCoord.x, 1.0); // s == 1.0 if inside, 0.0 if outside
        if (s <= 0.0)
        {

            texCoord = (vec4(st.xy, 1.0, 1.0) * u_back_tex_matrix).st;
            color.rgba = texture2D(u_back_textureSampler, texCoord).rgba;
        }
        else
        {

            color.rgba = texture2D(u_textureSampler, texCoord).rgba;
        }

        color = color * u_colorconv;
        color = u_color * (1.0 - color.a) + color;
        color = (texture2D(u_maskSampler, maskCoord) * u_maskxfm).r * u_alpha * color;
        gl_FragColor = applyMask(color);
    });

static const char gHomoFragmentShader_TexturedStencil[] = GLSL(

	varying highp vec2 v_homopos;
	uniform sampler2D u_textureSampler;
	uniform highp float u_alpha;
	uniform highp mat4 u_tex_matrix;
	uniform highp mat3 u_homo_matrix;
	void main() {

		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;
	    highp vec4 color;
	    color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;
	    if( (color.a*u_alpha) < 0.5 )
	      discard;
	    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	}
);

static const char gHomoFragmentShaderForScreenMasking_TexturedStencil[] = GLSL(

    varying highp vec2 v_homopos;
    uniform sampler2D u_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_tex_matrix;
    uniform highp mat3 u_homo_matrix;
    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    void main() {

		highp vec3 st = vec3(v_homopos.xy, 1.0) * u_homo_matrix;
		st.xy /= st.z;
		highp vec2 texCoord = (vec4(st.xy, 1.0, 1.0) * u_tex_matrix).st;
	    highp vec4 color;
	    color.rgba = (texture2D(u_textureSampler, texCoord)).rgba;
        color = applyMask(color);
	    if( (color.a*u_alpha) < 0.5 )
            discard;
	    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    });

static const char gStdVertexShader_Textured_vignette[] = GLSL(

	attribute vec4 a_position;
	attribute vec2 a_texCoord;
	attribute vec4 a_normal;
	uniform mat4 u_mvp_matrix;
	uniform mat4 u_tex_matrix;
	varying highp vec2 v_texCoord;
	varying highp vec2 v_texCoord_for_vignette;

	// Lighting
	uniform highp vec3 u_lightpos;
	uniform highp float u_light_f0;
	uniform highp float u_light_f1;
	uniform highp float u_specexp;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	void main() {

		highp vec3 light_direction;
		highp vec3 half_plane;

		v_texCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
		v_texCoord_for_vignette = a_texCoord;
		gl_Position = a_position * u_mvp_matrix;

		// Lighting
		highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
		light_direction = normalize(u_lightpos.xyz/* - gl_Position.xyz*/);
		half_plane = normalize(light_direction + vec3(0.0, 0.0, 1.0));
		highp float ndl = dot(normal, light_direction);
		v_diffuse_intensity = max(0.0, ndl*u_light_f0) + max(0.0, ndl*u_light_f1);
		v_specular_intensity = pow(max(0.0, dot(normal, half_plane)), u_specexp);
	}

);

static const char gStdVertexShaderForScreenMasking_Textured_vignette[] = GLSL(

    attribute vec4 a_position;
    attribute vec2 a_texCoord;
    attribute vec4 a_normal;
    uniform mat4 u_mvp_matrix;
    uniform mat4 u_tex_matrix;
    varying highp vec2 v_texCoord;
    varying highp vec2 v_texCoord_for_vignette;
    varying highp vec2 v_texCoord_for_mask;

    // Lighting
    uniform highp vec3 u_lightpos;
    uniform highp float u_light_f0;
    uniform highp float u_light_f1;
    uniform highp float u_specexp;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    void main() {

        highp vec3 light_direction;
        highp vec3 half_plane;

        v_texCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
        v_texCoord_for_vignette = a_texCoord;
        gl_Position = a_position * u_mvp_matrix;
        v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;

        // Lighting
        highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
        light_direction = normalize(u_lightpos.xyz /* - gl_Position.xyz*/);
        half_plane = normalize(light_direction + vec3(0.0, 0.0, 1.0));
        highp float ndl = dot(normal, light_direction);
        v_diffuse_intensity = max(0.0, ndl * u_light_f0) + max(0.0, ndl * u_light_f1);
        v_specular_intensity = pow(max(0.0, dot(normal, half_plane)), u_specexp);
    }

);

static const char gStdVertexShader_CombinedTextured_vignette[] = GLSL(

	attribute vec4 a_position;
	attribute vec2 a_texCoord;
	attribute vec4 a_normal;
	uniform mat4 u_mvp_matrix;
	uniform mat4 u_tex_matrix;
	uniform mat4 u_back_tex_matrix;
	varying highp vec2 v_texCoord;
	varying highp vec2 v_texCoord_for_back;
	varying highp vec2 v_texCoord_for_vignette;

	// Lighting
	uniform highp vec3 u_lightpos;
	uniform highp float u_light_f0;
	uniform highp float u_light_f1;
	uniform highp float u_specexp;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	void main() {

		highp vec3 light_direction;
		highp vec3 half_plane;

		v_texCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
		v_texCoord_for_back = (vec4(a_texCoord, 1.0, 1.0) * u_back_tex_matrix).st;
		v_texCoord_for_vignette = a_texCoord;
		gl_Position = a_position * u_mvp_matrix;

		// Lighting
		highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
		light_direction = normalize(u_lightpos.xyz/* - gl_Position.xyz*/);
		half_plane = normalize(light_direction + vec3(0.0, 0.0, 1.0));
		highp float ndl = dot(normal, light_direction);
		v_diffuse_intensity = max(0.0, ndl*u_light_f0) + max(0.0, ndl*u_light_f1);
		v_specular_intensity = pow(max(0.0, dot(normal, half_plane)), u_specexp);
	}

);

static const char gStdVertexShaderForScreenMasking_CombinedTextured_vignette[] = GLSL(

    attribute vec4 a_position;
    attribute vec2 a_texCoord;
    attribute vec4 a_normal;
    uniform mat4 u_mvp_matrix;
    uniform mat4 u_tex_matrix;
    uniform mat4 u_back_tex_matrix;
    varying highp vec2 v_texCoord;
    varying highp vec2 v_texCoord_for_back;
    varying highp vec2 v_texCoord_for_vignette;
    varying highp vec2 v_texCoord_for_mask;

    // Lighting
    uniform highp vec3 u_lightpos;
    uniform highp float u_light_f0;
    uniform highp float u_light_f1;
    uniform highp float u_specexp;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    void main() {

		highp vec3 light_direction;
		highp vec3 half_plane;

		v_texCoord = (vec4(a_texCoord, 1.0, 1.0) * u_tex_matrix).st;
		v_texCoord_for_back = (vec4(a_texCoord, 1.0, 1.0) * u_back_tex_matrix).st;
		v_texCoord_for_vignette = a_texCoord;
		gl_Position = a_position * u_mvp_matrix;
        v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;

        // Lighting
		highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
		light_direction = normalize(u_lightpos.xyz/* - gl_Position.xyz*/);
		half_plane = normalize(light_direction + vec3(0.0, 0.0, 1.0));
		highp float ndl = dot(normal, light_direction);
		v_diffuse_intensity = max(0.0, ndl*u_light_f0) + max(0.0, ndl*u_light_f1);
		v_specular_intensity = pow(max(0.0, dot(normal, half_plane)), u_specexp);
    }

);

static const char gStdVertexShader_Solid[] = GLSL(

    attribute vec4 a_position;
    attribute vec4 a_color;
    attribute vec4 a_normal;
    uniform mat4 u_mvp_matrix;
    varying highp vec4 v_color;
    varying highp vec4 v_normal;
    
    // Lighting
    uniform highp vec3 u_lightpos;
    uniform highp float u_light_f0;
    uniform highp float u_light_f1;
    uniform highp float u_specexp;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    void main() {
    
        highp vec3 light_direction;
        highp vec3 half_plane;
    
        v_color = a_color;
        v_normal = a_normal * u_mvp_matrix;
        gl_Position = a_position * u_mvp_matrix;
        
        // Lighting
        highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
        light_direction = normalize(u_lightpos.xyz/* - gl_Position.xyz*/);
        half_plane = normalize(light_direction + vec3(0.0,0.0,1.0));
        highp float ndl = dot(normal,light_direction);
        v_diffuse_intensity = max(0.0, ndl*u_light_f0) + max(0.0, ndl*u_light_f1);
//        v_diffuse_intensity = max(0.0, dot(normal,light_direction));
        v_specular_intensity = pow(max(0.0, dot(normal,half_plane)),u_specexp);
        
    }
    
);

static const char gStdVertexShaderForScreenMasking_Solid[] = GLSL(

    attribute vec4 a_position;
    attribute vec4 a_color;
    attribute vec4 a_normal;
    uniform mat4 u_mvp_matrix;
    varying highp vec4 v_color;
    varying highp vec4 v_normal;
    varying highp vec2 v_texCoord_for_mask;

    // Lighting
    uniform highp vec3 u_lightpos;
    uniform highp float u_light_f0;
    uniform highp float u_light_f1;
    uniform highp float u_specexp;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    void main() {

        highp vec3 light_direction;
        highp vec3 half_plane;

        v_color = a_color;
        v_normal = a_normal * u_mvp_matrix;
        gl_Position = a_position * u_mvp_matrix;
        v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;

        // Lighting
        highp vec3 normal = normalize((a_normal*u_mvp_matrix).xyz);
        light_direction = normalize(u_lightpos.xyz /* - gl_Position.xyz*/);
        half_plane = normalize(light_direction + vec3(0.0, 0.0, 1.0));
        highp float ndl = dot(normal, light_direction);
        v_diffuse_intensity = max(0.0, ndl * u_light_f0) + max(0.0, ndl * u_light_f1);
        //        v_diffuse_intensity = max(0.0, dot(normal,light_direction));
        v_specular_intensity = pow(max(0.0, dot(normal, half_plane)), u_specexp);

    }

);

static const char gStdVertexShader_Masked[] =
"attribute vec4 a_position;\n"
"attribute vec2 a_texCoord;\n"
"attribute vec2 a_maskCoord;\n"
"uniform mat4 u_mvp_matrix;\n"
"uniform mat4 u_tex_matrix;\n"
"uniform mat4 u_mask_matrix;\n"
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_maskCoord;\n"
"void main() {\n"
"  v_texCoord = (vec4(a_texCoord,1.0,1.0) * u_tex_matrix).st;\n"
"  v_maskCoord = (vec4(a_maskCoord,1.0,1.0) * u_mask_matrix).st;\n"
"  gl_Position = a_position * u_mvp_matrix;\n"
"}\n";

static const char gStdVertexShaderForScreenMasking_Masked[] =
"attribute vec4 a_position;\n"
"attribute vec2 a_texCoord;\n"
"attribute vec2 a_maskCoord;\n"
"uniform mat4 u_mvp_matrix;\n"
"uniform mat4 u_tex_matrix;\n"
"uniform mat4 u_mask_matrix;\n"
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_maskCoord;\n"
"varying highp vec2 v_texCoord_for_mask;\n"
"void main() {\n"
"  v_texCoord = (vec4(a_texCoord,1.0,1.0) * u_tex_matrix).st;\n"
"  v_maskCoord = (vec4(a_maskCoord,1.0,1.0) * u_mask_matrix).st;\n"
"  gl_Position = a_position * u_mvp_matrix;\n"
"  v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;\n"
"}\n";

static const char gStdVertexShader_CombinedMasked[] =
"attribute vec4 a_position;\n"
"attribute vec2 a_texCoord;\n"
"attribute vec2 a_maskCoord;\n"
"uniform mat4 u_mvp_matrix;\n"
"uniform mat4 u_tex_matrix;\n"
"uniform mat4 u_back_tex_matrix;\n"
"uniform mat4 u_mask_matrix;\n"
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_maskCoord;\n"
"varying highp vec2 v_texCoord_for_back;\n"
"void main() {\n"
"  v_texCoord = (vec4(a_texCoord,1.0,1.0) * u_tex_matrix).st;\n"
"  v_texCoord_for_back = (vec4(a_texCoord,1.0,1.0) * u_back_tex_matrix).st;\n"
"  v_maskCoord = (vec4(a_maskCoord,1.0,1.0) * u_mask_matrix).st;\n"
"  gl_Position = a_position * u_mvp_matrix;\n"
"}\n";

static const char gStdVertexShaderForScreenMasking_CombinedMasked[] =
"attribute vec4 a_position;\n"
"attribute vec2 a_texCoord;\n"
"attribute vec2 a_maskCoord;\n"
"uniform mat4 u_mvp_matrix;\n"
"uniform mat4 u_tex_matrix;\n"
"uniform mat4 u_back_tex_matrix;\n"
"uniform mat4 u_mask_matrix;\n"
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_maskCoord;\n"
"varying highp vec2 v_texCoord_for_back;\n"
"varying highp vec2 v_texCoord_for_mask;\n"

"void main() {\n"
"  v_texCoord = (vec4(a_texCoord,1.0,1.0) * u_tex_matrix).st;\n"
"  v_texCoord_for_back = (vec4(a_texCoord,1.0,1.0) * u_back_tex_matrix).st;\n"
"  v_maskCoord = (vec4(a_maskCoord,1.0,1.0) * u_mask_matrix).st;\n"
"  gl_Position = a_position * u_mvp_matrix;\n"
"  v_texCoord_for_mask = gl_Position.xy / gl_Position.w * 0.5 + 0.5;\n"
"}\n";

static const char gStdVertexShader_Masked_vignette[] =
"attribute vec4 a_position;\n"
"attribute vec2 a_texCoord;\n"
"attribute vec2 a_maskCoord;\n"
"uniform mat4 u_mvp_matrix;\n"
"uniform mat4 u_tex_matrix;\n"
"uniform mat4 u_mask_matrix;\n"
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_texCoord_for_vignette;\n"
"varying highp vec2 v_maskCoord;\n"
"void main() {\n"
"  v_texCoord = (vec4(a_texCoord,1.0,1.0) * u_tex_matrix).st;\n"
"  v_texCoord_for_vignette = a_texCoord;\n"
"  v_maskCoord = (vec4(a_maskCoord,1.0,1.0) * u_mask_matrix).st;\n"
"  gl_Position = a_position * u_mvp_matrix;\n"
"}\n";


//------ SOLID -----------------------------------------------------------------
// Needs lighting
static const char gFragmentShader_Solid[] = GLSL(

    varying highp vec4 v_color;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    
    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {
    	highp vec4 color = v_color * u_colorconv;
        color = applyHue(color);
    	gl_FragColor = u_alpha*applyLighting(color);
    }
    
);

// Needs lighting
static const char gFragmentShaderForScreenMasking_Solid[] = GLSL(

    varying highp vec4 v_color;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

	highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {
    	highp vec4 color = applyHue(v_color) * u_colorconv;
    	gl_FragColor = applyMask( u_alpha*applyLighting(color));
    }

);

//------ TEXTURED --------------------------------------------------------------
// Needs lighting
//static const char gFragmentShader_Textured[] = GLSL(
//
//    varying highp vec2 v_texCoord;
//    uniform sampler2D u_textureSampler;
//    uniform highp float u_alpha;
//    uniform highp mat4 u_colorconv;
//    
//    // ---- Lighting ----
//    uniform highp vec4 u_diffuse_light;
//    uniform highp vec4 u_ambient_light;
//    uniform highp vec4 u_specular_light;
//    varying highp float v_diffuse_intensity;
//    varying highp float v_specular_intensity;
//    
//    highp vec4 applyLighting( highp vec4 color ) {
//    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
//        result.a = color.a;
//        return result;
//    }
//    // -------------------
//
//    
//    void main() {
//        highp vec4 color;
//        color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
//        color = color * u_colorconv;
//        color = clamp(color, 0.0, 1.0);
//        gl_FragColor = u_alpha * applyLighting(color); // * vec4(color.a, color.a, color.a, 1.0);
//		gl_FragColor = color;
//    }
//    
//);

static const char gFragmentShader_Textured[] = GLSL(

	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;

	// ---- Lighting ----
	uniform highp vec4 u_diffuse_light;
	uniform highp vec4 u_ambient_light;
	uniform highp vec4 u_specular_light;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
	}
	// -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	void main() {
		highp vec4 color;
		color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		// highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, 1.0) * step(v_texCoord.x, 1.0); // s == 1.0 if inside, 0.0 if outside
		// color.rgb *= s;
		gl_FragColor = u_alpha * color; // * applyLighting(color);
	}

);

static const char gFragmentShaderForScreenMasking_Textured[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
    }
    // -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {
		highp vec4 color;
		color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		// highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, 1.0) * step(v_texCoord.x, 1.0); // s == 1.0 if inside, 0.0 if outside
		// color.rgb *= s;
		gl_FragColor = applyMask(u_alpha * color); // * applyLighting(color);
    }

);

static const char gFragmentShader_Textured_vignette[] = GLSL(

	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;	
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;
	varying highp vec2 v_texCoord_for_vignette;
	uniform sampler2D u_textureSampler_vignette;

	// ---- Lighting ----
	uniform highp vec4 u_diffuse_light;
	uniform highp vec4 u_ambient_light;
	uniform highp vec4 u_specular_light;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
	}
	// -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	void main() {
		highp vec4 color;
		color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		color = u_alpha * color;

		highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;
		color.rgb = color.rgb * vignette.r;

		gl_FragColor = color;
	}

);

static const char gFragmentShaderForScreenMasking_Textured_vignette[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    varying highp vec2 v_texCoord_for_vignette;
    uniform sampler2D u_textureSampler_vignette;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
        highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

	highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {
        highp vec4 color;
        color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
        color = applyHue(color) * u_colorconv;
        color = clamp(color, 0.0, 1.0);
        color = u_alpha * color;

        highp vec4 vignette;
        vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;
        color.rgb = color.rgb * vignette.r;

        gl_FragColor = applyMask(color);
    }

);

static const char gFragmentShader_Textured_ForVideo[] = GLSL(

	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;

	// ---- Lighting ----
	uniform highp vec4 u_diffuse_light;
	uniform highp vec4 u_ambient_light;
	uniform highp vec4 u_specular_light;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
	}
	// -------------------

	uniform highp float u_realX;
	uniform highp float u_realY;

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	void main() {
		
		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0)
		{
			color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		}
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		gl_FragColor = u_alpha * color;
	}
);

static const char gFragmentShaderForScreenMasking_Textured_ForVideo[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
    }
    // -------------------

    uniform highp float u_realX;
    uniform highp float u_realY;

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {
		
		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0)
		{
			color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		}
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		gl_FragColor = applyMask( u_alpha * color);
    }
);

static const char gFragmentShader_CombinedTextured_ForVideo[] = GLSL(

	varying highp vec2 v_texCoord;
	varying highp vec2 v_texCoord_for_back;
	uniform sampler2D u_textureSampler;
	uniform sampler2D u_back_textureSampler;
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;

	// ---- Lighting ----
	uniform highp vec4 u_diffuse_light;
	uniform highp vec4 u_ambient_light;
	uniform highp vec4 u_specular_light;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
	}
	// -------------------

	uniform highp float u_realX;
	uniform highp float u_realY;

	void main() {
		
		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0)
		{
			color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		}
		else{

			color.rgba = texture2D(u_back_textureSampler, v_texCoord_for_back).rgba;
		}
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
		gl_FragColor = u_alpha * color;
	}
);

static const char gFragmentShaderForScreenMasking_CombinedTextured_ForVideo[] = GLSL(

    varying highp vec2 v_texCoord;
    varying highp vec2 v_texCoord_for_back;
    uniform sampler2D u_textureSampler;
    uniform sampler2D u_back_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
    }
    // -------------------

    uniform highp float u_realX;
    uniform highp float u_realY;

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {
		
		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0)
		{
			color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		}
		else{

			color.rgba = texture2D(u_back_textureSampler, v_texCoord_for_back).rgba;
		}
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		gl_FragColor = applyMask( u_alpha * color);
    }
);

static const char gFragmentShader_Textured_ForVideo360[] = GLSL(

	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;

	// ---- Lighting ----
	uniform highp vec4 u_diffuse_light;
	uniform highp vec4 u_ambient_light;
	uniform highp vec4 u_specular_light;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
	}
	// -------------------

	uniform highp float u_realX;
	uniform highp float u_realY;
	uniform highp float u_moveX_value;
    uniform highp float u_moveY_value;
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }

	void main() {
		
		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0)
		{
			// for 360 video ------------------------------------------------------
			highp float pi = 3.1415926535;
			highp float X = sin(4.0 / 9.0 * pi) * (2.0 *v_texCoord.x - 1.0);
			highp float Y = sin(1.0 / 8.0 * pi) * (2.0 *v_texCoord.y - 1.0);
			highp vec3 ray = normalize(vec3(-X, Y, -1.0));
			highp float moveX = u_moveX_value * (pi * 18.0 / 9.0);
			highp float moveY = (u_moveY_value - 0.5) * (pi * 27.0 / 36.0);
			highp float rot_y = ray.y * cos(moveY) + ray.z * sin(moveY);
			highp float rot_z = -ray.y * sin(moveY) + ray.z * cos(moveY);
			ray.y = rot_y;
			ray.z = rot_z;
			highp float thetaX = atan(ray.x / ray.z);
			highp float thetaY = asin(ray.y) / pi;
			highp vec2 uv = vec2( (thetaX + moveX) / (2.0*pi), thetaY + 0.5);
			if(uv.x > 1.0){
			   uv.x -= 1.0;
			}
			if(uv.x < 0.0){
			   uv.x += 1.0;
			}
			if(uv.y > 1.0){
			   uv.y -= 1.0;
			}
			color.rgba = (texture2D(u_textureSampler, uv)).rgba;
			// end ----------------------------------------------------------------
			// color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
			
		}
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		gl_FragColor = u_alpha * color;
	}
);

static const char gFragmentShaderForScreenMasking_Textured_ForVideo360[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;
    
    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
    }
    // -------------------

    uniform highp float u_realX;
    uniform highp float u_realY;
    uniform highp float u_moveX_value;
    uniform highp float u_moveY_value;

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {
		
		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0)
		{
			// for 360 video ------------------------------------------------------
			highp float pi = 3.1415926535;
			highp float X = sin(4.0 / 9.0 * pi) * (2.0 *v_texCoord.x - 1.0);
			highp float Y = sin(1.0 / 8.0 * pi) * (2.0 *v_texCoord.y - 1.0);
			highp vec3 ray = normalize(vec3(-X, Y, -1.0));
			highp float moveX = u_moveX_value * (pi * 18.0 / 9.0);
			highp float moveY = (u_moveY_value - 0.5) * (pi * 27.0 / 36.0);
			highp float rot_y = ray.y * cos(moveY) + ray.z * sin(moveY);
			highp float rot_z = -ray.y * sin(moveY) + ray.z * cos(moveY);
			ray.y = rot_y;
			ray.z = rot_z;
			highp float thetaX = atan(ray.x / ray.z);
			highp float thetaY = asin(ray.y) / pi;
			highp vec2 uv = vec2( (thetaX + moveX) / (2.0*pi), thetaY + 0.5);
			if(uv.x > 1.0)
            {
                uv.x -= 1.0;
			}
			if(uv.x < 0.0)
            {
                uv.x += 1.0;
			}
			if(uv.y > 1.0)
            {
                uv.y -= 1.0;
			}
			color.rgba = (texture2D(u_textureSampler, uv)).rgba;
			// end ----------------------------------------------------------------
			// color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
			
		}
		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		gl_FragColor = applyMask( u_alpha * color);
    });

static const char gFragmentShader_Textured_ForVideo_vignette[] = GLSL(

	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;
	varying highp vec2 v_texCoord_for_vignette;
	uniform sampler2D u_textureSampler_vignette;

	// ---- Lighting ----
	uniform highp vec4 u_diffuse_light;
	uniform highp vec4 u_ambient_light;
	uniform highp vec4 u_specular_light;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
	}
	// -------------------

	uniform highp float u_realX;
	uniform highp float u_realY;

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	void main() {
		
		highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0)
		{
			color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		}

		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		color = u_alpha * color;
		color.rgb = color.rgb * vignette.r;
		gl_FragColor = color;
	}
);

static const char gFragmentShaderForScreenMasking_Textured_ForVideo_vignette[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    varying highp vec2 v_texCoord_for_vignette;
    uniform sampler2D u_textureSampler_vignette;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
    }
    // -------------------

    uniform highp float u_realX;
    uniform highp float u_realY;

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {
		
		highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0)
		{
			color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		}

		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		color = u_alpha * color;
		color.rgb = color.rgb * vignette.r;
		gl_FragColor = applyMask( color);
    }
);

static const char gFragmentShader_CombinedTextured_ForVideo_vignette[] = GLSL(

	varying highp vec2 v_texCoord;
	uniform sampler2D u_textureSampler;
	uniform sampler2D u_back_textureSampler;
	uniform highp float u_alpha;
	uniform highp mat4 u_colorconv;
	varying highp vec2 v_texCoord_for_vignette;
	varying highp vec2 v_texCoord_for_back;
	uniform sampler2D u_textureSampler_vignette;

	// ---- Lighting ----
	uniform highp vec4 u_diffuse_light;
	uniform highp vec4 u_ambient_light;
	uniform highp vec4 u_specular_light;
	varying highp float v_diffuse_intensity;
	varying highp float v_specular_intensity;

	highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
	}
	// -------------------

	uniform highp float u_realX;
	uniform highp float u_realY;

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
	void main() {
		
		highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0)
		{
			color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		}
		else{

			color.rgba = (texture2D(u_back_textureSampler, v_texCoord_for_back)).rgba;
		}

		color = color * u_colorconv;
		color = clamp(color, 0.0, 1.0);
        color = applyHue(color);
		color = u_alpha * color;
		color.rgb = color.rgb * vignette.r;
		gl_FragColor = color;
	}
);

static const char gFragmentShaderForScreenMasking_CombinedTextured_ForVideo_vignette[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;
    uniform sampler2D u_back_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    varying highp vec2 v_texCoord_for_vignette;
    varying highp vec2 v_texCoord_for_back;
    uniform sampler2D u_textureSampler_vignette;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
		highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
		result.a = color.a;
		return result;
    }
    // -------------------
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

	highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }

    uniform highp float u_realX;
    uniform highp float u_realY;

    void main() {
		
		highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

		highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
		highp vec4 color = vec4(0, 0, 0, 1);
		if(s > 0.0)
		{
			color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
		}
		else{

			color.rgba = (texture2D(u_back_textureSampler, v_texCoord_for_back)).rgba;
		}

		color = applyHue(color) * u_colorconv;
		color = clamp(color, 0.0, 1.0);
		color = u_alpha * color;
		color.rgb = color.rgb * vignette.r;
		gl_FragColor = applyMask(color);
    }
);


//------ TEXTURED+STENCIL --------------------------------------------------------------
static const char gFragmentShader_TexturedStencil[] =
"varying highp vec2 v_texCoord;\n"
"uniform sampler2D u_textureSampler;\n"
"uniform highp float u_alpha;\n"
"void main() {\n"
"  highp vec4 color;\n"
"  color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;\n"
"  if( (color.a*u_alpha) < 0.5 )\n"
"    discard;\n"
"  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

static const char gFragmentShaderForScreenMasking_TexturedStencil[] =
    "varying highp vec2 v_texCoord;\n"
    "uniform sampler2D u_textureSampler;\n"
    "uniform highp float u_alpha;\n"

    "varying highp vec2 v_texCoord_for_mask;\n"
    "uniform sampler2D u_textureSampler_for_mask;\n"
    "uniform highp float u_inverse_Mask;\n"
    "highp vec4 applyMask(highp vec4 color) {\n"
    "  highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;\n"
    "  if (u_inverse_Mask > 0.5) {\n"
    "    color *= (1.0 - mask.r);\n"
    "  }\n"
    "  else {\n"
    "    color *= mask.r;\n"
    "  }\n"
    "  return color;\n"
    "}\n"

    "void main() {\n"
    "  highp vec4 color;\n"
    "  color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;\n"
    "  color = applyMask(color);\n"
    "  if( (color.a*u_alpha) < 0.5 )\n"
    "    discard;\n"
    "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\n";

//------ MASKED+STENCIL ----------------------------------------------------------------
static const char gFragmentShader_MaskedStencil[] =
    "varying highp vec2 v_texCoord;\n"
    "varying highp vec2 v_maskCoord;\n"
    "uniform sampler2D u_textureSampler;\n"
    "uniform sampler2D u_maskSampler;\n"
    "uniform highp float u_alpha;\n"
    "uniform highp vec4 u_color;\n"
    "uniform highp mat4 u_maskxfm;\n"
    "void main() {\n"
    "  highp vec4 color;\n"
    "  color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;\n"
    //"  color = u_color*(1.0-color.a)+color;\n"
    "  color = color*(1.0-u_color.a)+u_color;\n"
    "  color = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r*u_alpha*color;\n"
    "  if( color.a < 0.5 )\n"
    "    discard;\n"
    "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\n";

static const char gFragmentShaderForScreenMasking_MaskedStencil[] =
    "varying highp vec2 v_texCoord;\n"
    "varying highp vec2 v_maskCoord;\n"
    "uniform sampler2D u_textureSampler;\n"
    "uniform sampler2D u_maskSampler;\n"
    "uniform highp float u_alpha;\n"
    "uniform highp vec4 u_color;\n"
    "uniform highp mat4 u_maskxfm;\n"
    "varying highp vec2 v_texCoord_for_mask;\n"
    "uniform sampler2D u_textureSampler_for_mask;\n"
    "uniform highp float u_inverse_Mask;\n"
    "highp vec4 applyMask(highp vec4 color) {\n"
    "  highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;\n"
    "  if (u_inverse_Mask > 0.5) {\n"
    "    color *= (1.0 - mask.r);\n"
    "  }\n"
    "  else {\n"
    "    color *= mask.r;\n"
    "  }\n"
    "  return color;\n"
    "}\n"
    "void main() {\n"
    "  highp vec4 color;\n"
    "  color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;\n"
    //"  color = u_color*(1.0-color.a)+color;\n"
    "  color = color*(1.0-u_color.a)+u_color;\n"
    "  color = applyMask( (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r*u_alpha*color );\n"
    "  if( color.a < 0.5 )\n"
    "    discard;\n"
    "  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\n";

//------ TEXTURED+MASKED+EXTERNAL --------------------------------------------------------------
static const char gFragmentShader_MaskedExternal[] =
    "#extension GL_OES_EGL_image_external : require\n" 
    GLSL(
        varying highp vec2 v_texCoord;
        varying highp vec2 v_maskCoord;
        uniform samplerExternalOES u_textureSampler;
        uniform sampler2D u_maskSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_maskxfm;
        uniform highp vec4 u_color;
        uniform highp mat4 u_colorconv;

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {
            highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside
            highp vec4 color = vec4(0, 0, 0, 1);
            if(s > 0.0){
                color = texture2D(u_textureSampler, v_texCoord) * u_colorconv;
                color = applyHue(color);
            }                
            highp float mask = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r;
            color = ( u_alpha * color) + u_color;
            gl_FragColor = mask *color;
        }

    );

static const char gFragmentShaderForScreenMasking_MaskedExternal[] =
    "#extension GL_OES_EGL_image_external : require\n"
    GLSL(
        varying highp vec2 v_texCoord;
        varying highp vec2 v_maskCoord;
        uniform samplerExternalOES u_textureSampler;
        uniform sampler2D u_maskSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_maskxfm;
        uniform highp vec4 u_color;
        uniform highp mat4 u_colorconv;

        varying highp vec2 v_texCoord_for_mask;
        uniform sampler2D u_textureSampler_for_mask;
        uniform highp float u_inverse_Mask;

        highp vec4 applyMask(highp vec4 color) {
            highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
            if (u_inverse_Mask > 0.5)
            {
                color *= (1.0 - mask.r);
            }
            else
            {
                color *= mask.r;
            }
            return color;
        }

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {
            highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside
            highp vec4 color = vec4(0, 0, 0, 1);
            if(s > 0.0){
                color = texture2D(u_textureSampler, v_texCoord) * u_colorconv;
                color = applyHue(color);
            }
            highp float mask = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r;
            color = ( u_alpha * color) + u_color;
            gl_FragColor = applyMask(mask *color);
        }

    );

//------ Combined TEXTURED+MASKED+EXTERNAL --------------------------------------------------------------
static const char gFragmentShader_CombinedMaskedExternal[] =
    "#extension GL_OES_EGL_image_external : require\n"
    GLSL(
        varying highp vec2 v_texCoord;
        varying highp vec2 v_texCoord_for_back;
        varying highp vec2 v_maskCoord;
        uniform samplerExternalOES u_textureSampler;
        uniform sampler2D u_maskSampler;
        uniform sampler2D u_back_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_maskxfm;
        uniform highp vec4 u_color;
        uniform highp mat4 u_colorconv;

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {
            highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside
            highp vec4 color = vec4(0, 0, 0, 1);
            if(s > 0.0)
                color = texture2D(u_textureSampler, v_texCoord);
            else
                color = texture2D(u_back_textureSampler, v_texCoord_for_back);
            color = color * u_colorconv;
            color = applyHue(color);
            highp float mask = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r;
            color = (u_alpha * color) + u_color;
            gl_FragColor = mask * color;
        }

    );

static const char gFragmentShaderForScreenMasking_CombinedMaskedExternal[] =
    "#extension GL_OES_EGL_image_external : require\n"
    GLSL(
        varying highp vec2 v_texCoord;
        varying highp vec2 v_texCoord_for_back;
        varying highp vec2 v_maskCoord;
        uniform samplerExternalOES u_textureSampler;
        uniform sampler2D u_maskSampler;
        uniform sampler2D u_back_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_maskxfm;
        uniform highp vec4 u_color;
        uniform highp mat4 u_colorconv;

        varying highp vec2 v_texCoord_for_mask;
        uniform sampler2D u_textureSampler_for_mask;
        uniform highp float u_inverse_Mask;

        highp vec4 applyMask(highp vec4 color) {
            highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
            if (u_inverse_Mask > 0.5)
            {
                color *= (1.0 - mask.r);
            }
            else
            {
                color *= mask.r;
            }
            return color;
        }

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {
            highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, 1.0) * step(v_texCoord.x, 1.0); // s == 1.0 if inside, 0.0 if outside
            highp vec4 color = vec4(0, 0, 0, 1);
            if (s > 0.0)
                color = texture2D(u_textureSampler, v_texCoord);
            else
                color = texture2D(u_back_textureSampler, v_texCoord_for_back);
            color = color * u_colorconv;
            color = applyHue(color);
            highp float mask = (texture2D(u_maskSampler, v_maskCoord) * u_maskxfm).r;
            color = (u_alpha * color) + u_color;
            gl_FragColor = applyMask(mask * color);
        }

    );

//------ TEXTURED+MASKED+EXTERNAL->YUVA8888 --------------------------------------------------------------
static const char gFragmentShader_MaskedExternaltoYUVA8888[] =
    "#extension GL_OES_EGL_image_external : require\n"
    "varying highp vec2 v_texCoord;\n"
    "varying highp vec2 v_maskCoord;\n"
    "uniform samplerExternalOES u_textureSampler;\n"
    "uniform sampler2D u_maskSampler;\n"
    "uniform highp float u_alpha;\n"
    "uniform highp mat4 u_maskxfm;\n"
    "uniform highp vec4 u_color;\n"
    "uniform highp mat4 u_colorconv;\n"
    "void main() {\n"
    "    highp float m = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r;\n"
    "    highp vec4 color = texture2D(u_textureSampler, v_texCoord) * u_colorconv;\n"
    //"    gl_FragColor = CHANNEL_FILTER((step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0) * u_alpha) * color);\n"
    "    highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0);\n" // s == 1.0 if inside, 0.0 if outside
    "    gl_FragColor = CHANNEL_FILTER(m * u_alpha * (s * color + (1.0-s) * vec4(0.0,0.5,0.5,1.0) ));\n"

    "}\n";

//------ TEXTURED+EXTERNAL --------------------------------------------------------------
// Needs lighting
static const char gFragmentShader_TexturedExternal[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;

        uniform samplerExternalOES u_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_colorconv;
        uniform highp float u_realX;
        uniform highp float u_realY;

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {
            highp vec4 color = texture2D(u_textureSampler, v_texCoord) * u_colorconv;
            color = applyHue(color);
            highp float f = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX);
            gl_FragColor = u_alpha * applyLighting(color * vec4(color.a * f, color.a * f, color.a * f, 1.0));
        }

    );

static const char gFragmentShaderForScreenMasking_TexturedExternal[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;

        uniform samplerExternalOES u_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_colorconv;
        uniform highp float u_realX;
        uniform highp float u_realY;

        varying highp vec2 v_texCoord_for_mask;
        uniform sampler2D u_textureSampler_for_mask;
        uniform highp float u_inverse_Mask;

        highp vec4 applyMask(highp vec4 color) {
            highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
            if (u_inverse_Mask > 0.5)
            {
                color *= (1.0 - mask.r);
            }
            else
            {
                color *= mask.r;
            }
            return color;
        }

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {
            highp vec4 color = texture2D(u_textureSampler, v_texCoord) * u_colorconv;
            color = applyHue(color);
            highp float f = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX);
            gl_FragColor = applyMask(u_alpha * applyLighting(color * vec4(color.a * f, color.a * f, color.a * f, 1.0)));
        }

    );

//------ Combined TEXTURED+EXTERNAL --------------------------------------------------------------
// Needs lighting
static const char gFragmentShader_CombinedTexturedExternal[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;
        varying highp vec2 v_texCoord_for_back;

        uniform samplerExternalOES u_textureSampler;
        uniform samplerExternalOES u_back_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_colorconv;
        uniform highp float u_realX;
        uniform highp float u_realY;

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        void main() {

            highp float f = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX);
            highp vec4 color;
            if (f > 0.0)
            {

                color = texture2D(u_textureSampler, v_texCoord);
            }
            else
            {

                color = texture2D(u_back_textureSampler, v_texCoord_for_back);
            }

            color = color * u_colorconv;

            gl_FragColor = u_alpha * applyLighting(color * vec4(color.a, color.a, color.a, 1.0));
        }

    );

static const char gFragmentShaderForScreenMasking_CombinedTexturedExternal[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;
        varying highp vec2 v_texCoord_for_back;

        uniform samplerExternalOES u_textureSampler;
        uniform samplerExternalOES u_back_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_colorconv;
        uniform highp float u_realX;
        uniform highp float u_realY;

        varying highp vec2 v_texCoord_for_mask;
        uniform sampler2D u_textureSampler_for_mask;
        uniform highp float u_inverse_Mask;

        highp vec4 applyMask(highp vec4 color) {
            highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
            if (u_inverse_Mask > 0.5)
            {
                color *= (1.0 - mask.r);
            }
            else
            {
                color *= mask.r;
            }
            return color;
        }

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {

            highp float f = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX);
            highp vec4 color;
            if (f > 0.0)
            {

                color = texture2D(u_textureSampler, v_texCoord);
            }
            else
            {

                color = texture2D(u_back_textureSampler, v_texCoord_for_back);
            }

            color = color * u_colorconv;
            color = applyHue(color);

            gl_FragColor = applyMask(u_alpha * applyLighting(color * vec4(color.a, color.a, color.a, 1.0)));
        }

    );

static const char gFragmentShader_TexturedExternal360[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;

        uniform samplerExternalOES u_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_colorconv;

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        uniform highp float u_moveX_value;
        uniform highp float u_moveY_value;
        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }

        void main() {
            highp vec4 color;
            // highp vec4 color = texture2D(u_textureSampler, v_texCoord) * u_colorconv;
            highp float f = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, 1.0) * step(v_texCoord.x, 1.0);
            // for 360 video ------------------------------------------------------
            highp float pi = 3.1415926535;
            highp float X = sin(4.0 / 9.0 * pi) * (2.0 * v_texCoord.x - 1.0);
            highp float Y = sin(1.0 / 8.0 * pi) * (2.0 * v_texCoord.y - 1.0);
            highp vec3 ray = normalize(vec3(-X, Y, -1.0));
            highp float moveX = u_moveX_value * (pi * 18.0 / 9.0);
            highp float moveY = (u_moveY_value - 0.5) * (pi * 27.0 / 36.0);
            highp float rot_y = ray.y * cos(moveY) + ray.z * sin(moveY);
            highp float rot_z = -ray.y * sin(moveY) + ray.z * cos(moveY);
            ray.y = rot_y;
            ray.z = rot_z;
            highp float thetaX = atan(ray.x / ray.z);
            highp float thetaY = asin(ray.y) / pi;
            highp vec2 uv = vec2((thetaX + moveX) / (2.0 * pi), thetaY + 0.5);
            if (uv.x > 1.0)
            {
                uv.x -= 1.0;
            }
            if (uv.x < 0.0)
            {
                uv.x += 1.0;
            }
            if (uv.y > 1.0)
            {
                uv.y -= 1.0;
            }
            color.rgba = (texture2D(u_textureSampler, uv)) * u_colorconv;
            color = applyHue(color);
            // end ----------------------------------------------------------------
            gl_FragColor = u_alpha * applyLighting(color * vec4(color.a * f, color.a * f, color.a * f, 1.0));
        }

    );

static const char gFragmentShaderForScreenMasking_TexturedExternal360[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;

        uniform samplerExternalOES u_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_colorconv;

        varying highp vec2 v_texCoord_for_mask;
        uniform sampler2D u_textureSampler_for_mask;
        uniform highp float u_inverse_Mask;

        highp vec4 applyMask(highp vec4 color) {
            highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
            if (u_inverse_Mask > 0.5)
            {
                color *= (1.0 - mask.r);
            }
            else
            {
                color *= mask.r;
            }
            return color;
        }

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        uniform highp float u_moveX_value;
        uniform highp float u_moveY_value;

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {
            highp vec4 color;
            // highp vec4 color = texture2D(u_textureSampler, v_texCoord) * u_colorconv;
            highp float f = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, 1.0) * step(v_texCoord.x, 1.0);
            // for 360 video ------------------------------------------------------
            highp float pi = 3.1415926535;
            highp float X = sin(4.0 / 9.0 * pi) * (2.0 * v_texCoord.x - 1.0);
            highp float Y = sin(1.0 / 8.0 * pi) * (2.0 * v_texCoord.y - 1.0);
            highp vec3 ray = normalize(vec3(-X, Y, -1.0));
            highp float moveX = u_moveX_value * (pi * 18.0 / 9.0);
            highp float moveY = (u_moveY_value - 0.5) * (pi * 27.0 / 36.0);
            highp float rot_y = ray.y * cos(moveY) + ray.z * sin(moveY);
            highp float rot_z = -ray.y * sin(moveY) + ray.z * cos(moveY);
            ray.y = rot_y;
            ray.z = rot_z;
            highp float thetaX = atan(ray.x / ray.z);
            highp float thetaY = asin(ray.y) / pi;
            highp vec2 uv = vec2((thetaX + moveX) / (2.0 * pi), thetaY + 0.5);
            if (uv.x > 1.0)
            {
                uv.x -= 1.0;
            }
            if (uv.x < 0.0)
            {
                uv.x += 1.0;
            }
            if (uv.y > 1.0)
            {
                uv.y -= 1.0;
            }
            color.rgba = (texture2D(u_textureSampler, uv)) * u_colorconv;
            color = applyHue(color);
            // end ----------------------------------------------------------------
            gl_FragColor = applyMask(u_alpha * applyLighting(color * vec4(color.a * f, color.a * f, color.a * f, 1.0)));
        }

    );

//------ TEXTURED+EXTERNAL+VIGNETTE --------------------------------------------------------------
// Needs lighting
static const char gFragmentShader_TexturedExternal_vignette[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;

        uniform samplerExternalOES u_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_colorconv;
        uniform highp float u_realX;
        uniform highp float u_realY;

        varying highp vec2 v_texCoord_for_vignette;
        uniform sampler2D u_textureSampler_vignette;

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {
            highp vec4 color = texture2D(u_textureSampler, v_texCoord) * u_colorconv;
            color = applyHue(color);
            highp float f = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX);
            color = u_alpha * applyLighting(color * vec4(color.a * f, color.a * f, color.a * f, 1.0));

            highp vec4 vignette;
            vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

            color.rgb = color.rgb * vignette.r;
            gl_FragColor = color;

        }

    );

static const char gFragmentShaderForScreenMasking_TexturedExternal_vignette[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;

        uniform samplerExternalOES u_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_colorconv;
        uniform highp float u_realX;
        uniform highp float u_realY;

        varying highp vec2 v_texCoord_for_vignette;
        uniform sampler2D u_textureSampler_vignette;

        varying highp vec2 v_texCoord_for_mask;
        uniform sampler2D u_textureSampler_for_mask;
        uniform highp float u_inverse_Mask;

        highp vec4 applyMask(highp vec4 color) {
            highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
            if (u_inverse_Mask > 0.5)
            {
                color *= (1.0 - mask.r);
            }
            else
            {
                color *= mask.r;
            }
            return color;
        }

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {
            highp vec4 color = texture2D(u_textureSampler, v_texCoord) * u_colorconv;
            color = applyHue(color);
            highp float f = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX);
            color = u_alpha * applyLighting(color * vec4(color.a * f, color.a * f, color.a * f, 1.0));

            highp vec4 vignette;
            vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

            color.rgb = color.rgb * vignette.r;
            gl_FragColor = applyMask(color);

        }

    );

//------ COMBINED TEXTURED+EXTERNAL+VIGNETTE --------------------------------------------------------------
// Needs lighting
static const char gFragmentShader_CombinedTexturedExternal_vignette[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;

        uniform samplerExternalOES u_textureSampler;
        uniform sampler2D u_back_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_colorconv;
        uniform highp float u_realX;
        uniform highp float u_realY;

        varying highp vec2 v_texCoord_for_vignette;
        varying highp vec2 v_texCoord_for_back;
        uniform sampler2D u_textureSampler_vignette;

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {

            highp float f = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX);
            highp vec4 color;

            if (f > 0.0)
                color = texture2D(u_textureSampler, v_texCoord);
            else
                color = texture2D(u_back_textureSampler, v_texCoord_for_back);
            color = color * u_colorconv;
            color = applyHue(color);
            color = u_alpha * applyLighting(color * vec4(color.a * f, color.a * f, color.a * f, 1.0));

            highp vec4 vignette;
            vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

            color.rgb = color.rgb * vignette.r;
            gl_FragColor = color;

        }

    );

// Needs lighting
static const char gFragmentShaderForScreenMasking_CombinedTexturedExternal_vignette[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(

        varying highp vec2 v_texCoord;

        uniform samplerExternalOES u_textureSampler;
        uniform sampler2D u_back_textureSampler;
        uniform highp float u_alpha;
        uniform highp mat4 u_colorconv;
        uniform highp float u_realX;
        uniform highp float u_realY;

        varying highp vec2 v_texCoord_for_vignette;
        varying highp vec2 v_texCoord_for_back;
        uniform sampler2D u_textureSampler_vignette;
        varying highp vec2 v_texCoord_for_mask;
        uniform sampler2D u_textureSampler_for_mask;
        uniform highp float u_inverse_Mask;

        highp vec4 applyMask(highp vec4 color) {
            highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
            if (u_inverse_Mask > 0.5)
            {
                color *= (1.0 - mask.r);
            }
            else
            {
                color *= mask.r;
            }
            return color;
        }

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        uniform highp float u_hue;

        highp vec3 rgb2hsv(highp vec3 c)
        {
            highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
            highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            highp float d = q.x - min(q.w, q.y);
            highp float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
        }
        

        highp vec3 hsv2rgb(highp vec3 c)
        {
            highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
            return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
        }

        highp vec4 applyHue(highp vec4 color) {

            highp vec3 hsv = rgb2hsv(color.rgb);
            hsv.r += u_hue;
            color = vec4(hsv2rgb(hsv).rgb, color.a);
            return color;
        }
        void main() {

            highp float f = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX);
            highp vec4 color;

            if (f > 0.0)
                color = texture2D(u_textureSampler, v_texCoord);
            else
                color = texture2D(u_back_textureSampler, v_texCoord_for_back);
            color = applyHue(color) * u_colorconv;
            color = u_alpha * applyLighting(color * vec4(color.a * f, color.a * f, color.a * f, 1.0));

            highp vec4 vignette;
            vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

            color.rgb = color.rgb * vignette.r;
            gl_FragColor = applyMask(color);

        }

    );

//------ TEXTURED+EXTERNAL->YUVA8888 ----------------------------------------------------------
// Needs lighting
static const char gFragmentShader_TexturedExternaltoYUVA8888[] =
    "#extension GL_OES_EGL_image_external : require\n" GLSL(
        varying highp vec2 v_texCoord;
        uniform samplerExternalOES u_textureSampler;
        uniform highp float u_alpha;
        uniform highp float u_realY;
        uniform highp float u_realX;
        uniform highp mat4 u_colorconv;

        // ---- Lighting ----
        uniform highp vec4 u_diffuse_light;
        uniform highp vec4 u_ambient_light;
        uniform highp vec4 u_specular_light;
        varying highp float v_diffuse_intensity;
        varying highp float v_specular_intensity;

        highp vec4 applyLighting(highp vec4 color) {
            highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
            result.a = color.a;
            return result;
        }
        // -------------------

        void main() {

            const highp mat4 rgb_biasedyuv = mat4(0.257, 0.504, 0.098, 0.0625,
                                                  -0.148, -0.291, 0.439, 0.500,
                                                  0.439, -0.368, -0.071, 0.500,
                                                  0.000, 0.000, 0.000, 1.000);

            highp vec4 color = texture2D(u_textureSampler, v_texCoord) * u_colorconv;
            color = clamp(color, 0.0, 1.0);
            highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside
            gl_FragColor = CHANNEL_FILTER((u_alpha * applyLighting(color * vec4(color.a * s, color.a * s, color.a * s, 1.0)) * rgb_biasedyuv));

        });

//------ TEXTURED+YUV ----------------------------------------------------------
// Needs lighting
static const char gFragmentShader_TexturedYUV[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerU;
    uniform sampler2D u_textureSamplerV;
    uniform highp float u_alpha;

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
        highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {

        const highp mat4 yuvrgb = mat4(1.000, 0.000, 1.402, -0.701,
                                       1.000, -0.334, -0.714, 0.529,
                                       1.000, 1.772, 0.000, -0.886,
                                       0.000, 0.000, 0.000, 1.000);
        highp vec4 color = vec4(texture2D(u_textureSamplerY, v_texCoord).r,
                                texture2D(u_textureSamplerU, v_texCoord).r,
                                texture2D(u_textureSamplerV, v_texCoord).r,
                                1.0) *
                           yuvrgb;
        color = applyHue(color);
        gl_FragColor = u_alpha * applyLighting(color * vec4(color.a, color.a, color.a, 1.0));

    }

);

// Needs lighting
static const char gFragmentShaderForScreenMasking_TexturedYUV[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerU;
    uniform sampler2D u_textureSamplerV;
    uniform highp float u_alpha;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
        highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {

        const highp mat4 yuvrgb = mat4(1.000, 0.000, 1.402, -0.701,
                                       1.000, -0.334, -0.714, 0.529,
                                       1.000, 1.772, 0.000, -0.886,
                                       0.000, 0.000, 0.000, 1.000);
        highp vec4 color = vec4(texture2D(u_textureSamplerY, v_texCoord).r,
                                texture2D(u_textureSamplerU, v_texCoord).r,
                                texture2D(u_textureSamplerV, v_texCoord).r,
                                1.0) *
                           yuvrgb;
        color = applyHue(color);
        gl_FragColor = applyMask(u_alpha * applyLighting(color * vec4(color.a, color.a, color.a, 1.0)));

    }

);

//------ TEXTURED+YUV+VIGNETTE ----------------------------------------------------------
// Needs lighting
static const char gFragmentShader_TexturedYUV_vignette[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerU;
    uniform sampler2D u_textureSamplerV;
    uniform highp float u_alpha;

    varying highp vec2 v_texCoord_for_vignette;
    uniform sampler2D u_textureSampler_vignette;

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
        highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {

        const highp mat4 yuvrgb = mat4(1.000, 0.000, 1.402, -0.701,
                                       1.000, -0.334, -0.714, 0.529,
                                       1.000, 1.772, 0.000, -0.886,
                                       0.000, 0.000, 0.000, 1.000);
        highp vec4 color = vec4(texture2D(u_textureSamplerY, v_texCoord).r,
                                texture2D(u_textureSamplerU, v_texCoord).r,
                                texture2D(u_textureSamplerV, v_texCoord).r,
                                1.0) *
                           yuvrgb;
        color = applyHue(color);
        color = u_alpha * applyLighting(color * vec4(color.a, color.a, color.a, 1.0));

        highp vec4 vignette;
        vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;
        color.rgb = color.rgb * vignette.r;

        gl_FragColor = color;

    }

);

// Needs lighting
static const char gFragmentShaderForScreenMasking_TexturedYUV_vignette[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerU;
    uniform sampler2D u_textureSamplerV;
    uniform highp float u_alpha;

    varying highp vec2 v_texCoord_for_vignette;
    uniform sampler2D u_textureSampler_vignette;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
        highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

	highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {

        const highp mat4 yuvrgb = mat4(1.000, 0.000, 1.402, -0.701,
                                       1.000, -0.334, -0.714, 0.529,
                                       1.000, 1.772, 0.000, -0.886,
                                       0.000, 0.000, 0.000, 1.000);
        highp vec4 color = vec4(texture2D(u_textureSamplerY, v_texCoord).r,
                                texture2D(u_textureSamplerU, v_texCoord).r,
                                texture2D(u_textureSamplerV, v_texCoord).r,
                                1.0) *
                           yuvrgb;
        color = applyHue(color);
        color = u_alpha * applyLighting(color * vec4(color.a, color.a, color.a, 1.0));

        highp vec4 vignette;
        vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;
        color.rgb = color.rgb * vignette.r;

        gl_FragColor = applyMask(color);

    }

);

//------ TEXTURED+Y2UVA ----------------------------------------------------------
// Needs lighting
static const char gFragmentShader_TexturedY2UVA[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;

    void main() {

        highp vec2 v_texCoord2;

        highp float y;
        highp float u;
        highp float v;

        const highp mat4 yuvrgb = mat4(1.000, 0.000, 1.402, -0.701,
                                       1.000, -0.334, -0.714, 0.529,
                                       1.000, 1.772, 0.000, -0.886,
                                       0.000, 0.000, 0.000, 1.000);

        v_texCoord2 = v_texCoord;
        v_texCoord2.y = 1.0 - v_texCoord2.y;
        if (v_texCoord2.y > 0.5)
        {
            v_texCoord2.y = (v_texCoord2.y - 0.5) * 2.0;
            y = texture2D(u_textureSampler, v_texCoord2).g;
        }
        else
        {
            v_texCoord2.y = v_texCoord2.y * 2.0;
            y = texture2D(u_textureSampler, v_texCoord2).r;
        }

        v_texCoord2 = v_texCoord;
        v_texCoord2.y = 1.0 - v_texCoord2.y;

        v_texCoord2.x *= 0.5;
        u = texture2D(u_textureSampler, v_texCoord2).b;

        v_texCoord2.x += 0.5;
        v = texture2D(u_textureSampler, v_texCoord2).b;

        gl_FragColor = vec4(y, u, v, 1.0) * yuvrgb;
    });

//------ MASKED+YUV ------------------------------------------------------------
static const char gFragmentShader_MaskedYUV[] =
    "varying highp vec2 v_texCoord;\n"
    "varying highp vec2 v_maskCoord;\n"
    "uniform sampler2D u_textureSamplerY;\n"
    "uniform sampler2D u_textureSamplerU;\n"
    "uniform sampler2D u_textureSamplerV;\n"
    "uniform sampler2D u_maskSampler;\n"
    "uniform highp float u_alpha;\n"
    "uniform highp vec4 u_color;\n"
    "uniform highp mat4 u_maskxfm;\n"
    "uniform highp float u_hue;\n"

    "highp vec3 rgb2hsv(highp vec3 c)\n"
    "{\n"
    "    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
    "    highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
    "    highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"

    "    highp float d = q.x - min(q.w, q.y);\n"
    "    highp float e = 1.0e-10;\n"
    "    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
    "}\n"
    

    "highp vec3 hsv2rgb(highp vec3 c)\n"
    "{\n"
    "    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
    "    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
    "    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
    "}\n"

    "highp vec4 applyHue(highp vec4 color) {\n"

    "    highp vec3 hsv = rgb2hsv(color.rgb);\n"
    "    hsv.r += u_hue;\n"
    "    color = vec4(hsv2rgb(hsv).rgb, color.a);\n"
    "    return color;\n"
    "}\n"
    "void main() {\n"
    "   const highp mat4 yuvrgb = mat4(1.000,  0.000,  1.402,-0.701,\n"
    "                                   1.000, -0.334, -0.714, 0.529,\n"
    "                                   1.000,  1.772,  0.000, -0.886,\n"
    "                                   0.000,  0.000,  0.000, 1.000);\n"
    "   highp vec4 color = vec4(texture2D(u_textureSamplerY,v_texCoord).r,\n"
    "                         texture2D(u_textureSamplerU,v_texCoord).r,\n"
    "                         texture2D(u_textureSamplerV,v_texCoord).r,\n"
    "                         1.0)*yuvrgb;\n"
    "  color = applyHue(color)*(1.0-u_color.a)+u_color;\n"
    "   color = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r * u_alpha * color;\n"
    "   gl_FragColor = color * vec4(color.a, color.a, color.a, 1.0);\n"
    "}\n";

static const char gFragmentShaderForScreenMasking_MaskedYUV[] =
    "varying highp vec2 v_texCoord;\n"
    "varying highp vec2 v_maskCoord;\n"
    "uniform sampler2D u_textureSamplerY;\n"
    "uniform sampler2D u_textureSamplerU;\n"
    "uniform sampler2D u_textureSamplerV;\n"
    "uniform sampler2D u_maskSampler;\n"
    "uniform highp float u_alpha;\n"
    "uniform highp vec4 u_color;\n"
    "uniform highp mat4 u_maskxfm;\n"
    "varying highp vec2 v_texCoord_for_mask;\n"
    "uniform sampler2D u_textureSampler_for_mask;\n"
    "uniform highp float u_inverse_Mask;\n"
    "highp vec4 applyMask(highp vec4 color) {\n"
    "  highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;\n"
    "  if (u_inverse_Mask > 0.5) {\n"
    "    color *= (1.0 - mask.r);\n"
    "  }\n"
    "  else {\n"
    "    color *= mask.r;\n"
    "  }\n"
    "  return color;\n"
    "}\n"
    "uniform highp float u_hue;\n"

    "highp vec3 rgb2hsv(highp vec3 c)\n"
    "{\n"
    "    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
    "    highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
    "    highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"

    "    highp float d = q.x - min(q.w, q.y);\n"
    "    highp float e = 1.0e-10;\n"
    "    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
    "}\n"
    

    "highp vec3 hsv2rgb(highp vec3 c)\n"
    "{\n"
    "    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
    "    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
    "    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
    "}\n"

	"highp vec4 applyHue(highp vec4 color) {\n"

    "    highp vec3 hsv = rgb2hsv(color.rgb);\n"
    "    hsv.r += u_hue;\n"
    "    color = vec4(hsv2rgb(hsv).rgb, color.a);\n"
    "    return color;\n"
    "}\n"
    "void main() {\n"
    "   const highp mat4 yuvrgb = mat4(1.000,  0.000,  1.402,-0.701,\n"
    "                                   1.000, -0.334, -0.714, 0.529,\n"
    "                                   1.000,  1.772,  0.000, -0.886,\n"
    "                                   0.000,  0.000,  0.000, 1.000);\n"
    "   highp vec4 color = vec4(texture2D(u_textureSamplerY,v_texCoord).r,\n"
    "                         texture2D(u_textureSamplerU,v_texCoord).r,\n"
    "                         texture2D(u_textureSamplerV,v_texCoord).r,\n"
    "                         1.0)*yuvrgb;\n"
    "  color = applyHue(color)*(1.0-u_color.a)+u_color;\n"
    "   color = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r * u_alpha * color;\n"
    "   gl_FragColor = applyMask(color * vec4(color.a, color.a, color.a, 1.0));\n"
    "}\n";

//------ TEXTURED+NV12 ----------------------------------------------------------
// Needs lighting
static const char gFragmentShader_TexturedNV12[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerUV;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    uniform highp float u_realY;
    uniform highp float u_realX;

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
        highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {

        highp vec4 color;
        color.r = texture2D(u_textureSamplerY, v_texCoord).r;
        color.g = texture2D(u_textureSamplerUV, v_texCoord).r;
#ifdef __APPLE__
        color.b = texture2D(u_textureSamplerUV, v_texCoord).g;
#else
        color.b = texture2D(u_textureSamplerUV, v_texCoord).a;
#endif
        color.a = 1.0;
        color = color * u_colorconv;
        color = applyHue(color);
        highp float s = step(0.0, v_texCoord.y) * step(0.0, v_texCoord.x) * step(v_texCoord.y, u_realY) * step(v_texCoord.x, u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
        gl_FragColor = u_alpha * applyLighting(color * vec4(color.a * s, color.a * s, color.a * s, 1.0));

    }

);

static const char gFragmentShaderForScreenMasking_TexturedNV12[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerUV;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    uniform highp float u_realY;
    uniform highp float u_realX;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;
    
    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }    
    
    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    
    void main() {
    
        highp vec4 color;
        color.r = texture2D(u_textureSamplerY,v_texCoord).r;
        color.g = texture2D(u_textureSamplerUV,v_texCoord).r;
#ifdef __APPLE__
        color.b = texture2D(u_textureSamplerUV,v_texCoord).g;
#else
        color.b = texture2D(u_textureSamplerUV,v_texCoord).a;
#endif
        color.a = 1.0;
        color = color*u_colorconv;
        color = applyHue(color);
        highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,u_realY) * step(v_texCoord.x,u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
        gl_FragColor = applyMask( u_alpha * applyLighting(color * vec4(color.a * s, color.a * s, color.a * s, 1.0)));
        
    }
    
);

//------ Combined TEXTURED+NV12 ----------------------------------------------------------
// Needs lighting
static const char gFragmentShader_CombinedTexturedNV12[] = GLSL(

    varying highp vec2 v_texCoord;
    varying highp vec2 v_texCoord_for_back;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerUV;
    uniform sampler2D u_back_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    uniform highp float u_realY;
    uniform highp float u_realX;
    
    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    
    void main() {

    	highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,u_realY) * step(v_texCoord.x,u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)    
        highp vec4 color;
        if(s > 0){

        	color.r = texture2D(u_textureSamplerY,v_texCoord).r;
	        color.g = texture2D(u_textureSamplerUV,v_texCoord).r;
	#ifdef __APPLE__
	        color.b = texture2D(u_textureSamplerUV,v_texCoord).g;
	#else
	        color.b = texture2D(u_textureSamplerUV,v_texCoord).a;
	#endif
	        color.a = 1.0;        	
        }
        else{

        	color = texture2D(u_back_textureSampler, v_texCoord_for_back);
        }
        
        color = color*u_colorconv;
        color = applyHue(color);
        
        gl_FragColor = u_alpha * applyLighting(color * vec4(color.a * s, color.a * s, color.a * s, 1.0));
        
    }
    
);

// Needs lighting
static const char gFragmentShaderForScreenMasking_CombinedTexturedNV12[] = GLSL(

    varying highp vec2 v_texCoord;
    varying highp vec2 v_texCoord_for_back;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerUV;
    uniform sampler2D u_back_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    uniform highp float u_realY;
    uniform highp float u_realX;
    
    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;
    
    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }

    
    void main() {

    	highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,u_realY) * step(v_texCoord.x,u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)    
        highp vec4 color;
        if(s > 0){

        	color.r = texture2D(u_textureSamplerY,v_texCoord).r;
	        color.g = texture2D(u_textureSamplerUV,v_texCoord).r;
	#ifdef __APPLE__
	        color.b = texture2D(u_textureSamplerUV,v_texCoord).g;
	#else
	        color.b = texture2D(u_textureSamplerUV,v_texCoord).a;
	#endif
	        color.a = 1.0;        	
        }
        else{

        	color = texture2D(u_back_textureSampler, v_texCoord_for_back);
        }
        
        color = color*u_colorconv;
        color = applyHue(color);
        
        gl_FragColor = applyMask( u_alpha * applyLighting(color * vec4(color.a * s, color.a * s, color.a * s, 1.0)));
        
    }
    
);

//------ TEXTURED+NV12+VIGNETTE ----------------------------------------------------------
// Needs lighting
static const char gFragmentShader_TexturedNV12_vignette[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerUV;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    uniform highp float u_realY;
    uniform highp float u_realX;

    varying highp vec2 v_texCoord_for_vignette;
	uniform sampler2D u_textureSampler_vignette;
    
    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    
    void main() {
    
        highp vec4 color;
        color.r = texture2D(u_textureSamplerY,v_texCoord).r;
        color.g = texture2D(u_textureSamplerUV,v_texCoord).r;
#ifdef __APPLE__
        color.b = texture2D(u_textureSamplerUV,v_texCoord).g;
#else
        color.b = texture2D(u_textureSamplerUV,v_texCoord).a;
#endif
        color.a = 1.0;
        color = color*u_colorconv;
        color = applyHue(color);
        highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,u_realY) * step(v_texCoord.x,u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
        color = u_alpha * applyLighting(color * vec4(color.a * s, color.a * s, color.a * s, 1.0));

        highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

		color.rgb = color.rgb * vignette.r;
		gl_FragColor = color;
        
    }
    
);

static const char gFragmentShaderForScreenMasking_TexturedNV12_vignette[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerUV;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    uniform highp float u_realY;
    uniform highp float u_realX;

    varying highp vec2 v_texCoord_for_vignette;
	uniform sampler2D u_textureSampler_vignette;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;
    
    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }    
    
    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    
    void main() {
    
        highp vec4 color;
        color.r = texture2D(u_textureSamplerY,v_texCoord).r;
        color.g = texture2D(u_textureSamplerUV,v_texCoord).r;
#ifdef __APPLE__
        color.b = texture2D(u_textureSamplerUV,v_texCoord).g;
#else
        color.b = texture2D(u_textureSamplerUV,v_texCoord).a;
#endif
        color.a = 1.0;
        color = color*u_colorconv;
        color = applyHue(color);
        highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,u_realY) * step(v_texCoord.x,u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
        color = u_alpha * applyLighting(color * vec4(color.a * s, color.a * s, color.a * s, 1.0));

        highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

		color.rgb = color.rgb * vignette.r;
		gl_FragColor = applyMask(color);
        
    }
    
);

//------ Combined TEXTURED+NV12+VIGNETTE ----------------------------------------------------------
// Needs lighting
static const char gFragmentShader_CombinedTexturedNV12_vignette[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerUV;
    uniform sampler2D u_back_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    uniform highp float u_realY;
    uniform highp float u_realX;

    varying highp vec2 v_texCoord_for_vignette;
    varying highp vec2 v_texCoord_for_back;
	uniform sampler2D u_textureSampler_vignette;
    
    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------
    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

    highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    
    void main() {
    
        highp vec4 color;
        highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,u_realY) * step(v_texCoord.x,u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)

        if(s > 0){

	        color.r = texture2D(u_textureSamplerY,v_texCoord).r;
	        color.g = texture2D(u_textureSamplerUV,v_texCoord).r;
	#ifdef __APPLE__
	        color.b = texture2D(u_textureSamplerUV,v_texCoord).g;
	#else
	        color.b = texture2D(u_textureSamplerUV,v_texCoord).a;
	#endif
	        color.a = 1.0;        	
        }
        else{

        	color = texture2D(u_back_textureSampler, v_texCoord_for_back);
        }
        
        color = color*u_colorconv;
        color = applyHue(color);
        
        color = u_alpha * applyLighting(color * vec4(color.a, color.a, color.a, 1.0));

        highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

		color.rgb = color.rgb * vignette.r;
		gl_FragColor = color;
        
    }
    
);

// Needs lighting
static const char gFragmentShaderForScreenMasking_CombinedTexturedNV12_vignette[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerUV;
    uniform sampler2D u_back_textureSampler;
    uniform highp float u_alpha;
    uniform highp mat4 u_colorconv;
    uniform highp float u_realY;
    uniform highp float u_realX;

    varying highp vec2 v_texCoord_for_vignette;
    varying highp vec2 v_texCoord_for_back;
    uniform sampler2D u_textureSampler_vignette;

    varying highp vec2 v_texCoord_for_mask;
    uniform sampler2D u_textureSampler_for_mask;
    uniform highp float u_inverse_Mask;

    highp vec4 applyMask(highp vec4 color) {
        highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;
        if (u_inverse_Mask > 0.5)
        {
            color *= (1.0 - mask.r);
        }
        else
        {
            color *= mask.r;
        }
        return color;
    }

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;

    highp vec4 applyLighting(highp vec4 color) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    uniform highp float u_hue;

    highp vec3 rgb2hsv(highp vec3 c)
    {
        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

        highp float d = q.x - min(q.w, q.y);
        highp float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
    }
    

    highp vec3 hsv2rgb(highp vec3 c)
    {
        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
    }

	highp vec4 applyHue(highp vec4 color) {

        highp vec3 hsv = rgb2hsv(color.rgb);
        hsv.r += u_hue;
        color = vec4(hsv2rgb(hsv).rgb, color.a);
        return color;
    }
    void main() {
    
        highp vec4 color;
        highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,u_realY) * step(v_texCoord.x,u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)

        if(s > 0)
        {

	        color.r = texture2D(u_textureSamplerY,v_texCoord).r;
	        color.g = texture2D(u_textureSamplerUV,v_texCoord).r;
#ifdef __APPLE__
	        color.b = texture2D(u_textureSamplerUV,v_texCoord).g;
#else
	        color.b = texture2D(u_textureSamplerUV,v_texCoord).a;
#endif
	        color.a = 1.0;        	
        }
        else
        {

        	color = texture2D(u_back_textureSampler, v_texCoord_for_back);
        }
        
        color = applyHue(color)*u_colorconv;
        
        color = u_alpha * applyLighting(color * vec4(color.a, color.a, color.a, 1.0));

        highp vec4 vignette;
		vignette = (texture2D(u_textureSampler_vignette, v_texCoord_for_vignette)).rgba;

		color.rgb = color.rgb * vignette.r;
		gl_FragColor = applyMask(color);

    }

);

//------ MASKED+NV12 ------------------------------------------------------------
static const char gFragmentShader_MaskedNV12[] =
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_maskCoord;\n"
"uniform sampler2D u_textureSamplerY;\n"
"uniform sampler2D u_textureSamplerUV;\n"
"uniform sampler2D u_maskSampler;\n"
"uniform highp float u_alpha;\n"
"uniform highp vec4 u_color;\n"
"uniform highp mat4 u_maskxfm;\n"
"uniform highp mat4 u_colorconv;\n"
"uniform highp float u_hue;\n"

"highp vec3 rgb2hsv(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
"    highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
"    highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"

"    highp float d = q.x - min(q.w, q.y);\n"
"    highp float e = 1.0e-10;\n"
"    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
"}\n"


"highp vec3 hsv2rgb(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
"    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
"    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
"}\n"

"highp vec4 applyHue(highp vec4 color) {\n"

"    highp vec3 hsv = rgb2hsv(color.rgb);\n"
"    hsv.r += u_hue;\n"
"    color = vec4(hsv2rgb(hsv).rgb, color.a);\n"
"    return color;\n"
"}\n"
"void main() {\n"
"   highp vec4 color = vec4(0, 0, 0, 1);\n"
"   highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside\n"
"   if(s > 0.0){\n"
"       color.r = texture2D(u_textureSamplerY,v_texCoord).r;\n"
#ifdef __APPLE__
"       color.gb = texture2D(u_textureSamplerUV,v_texCoord).rg;\n"
#elif defined(ANDROID)
"       color.gb = texture2D(u_textureSamplerUV,v_texCoord).ra;\n"
#endif
"   }\n"
"   color = color*(1.0-u_color.a)+u_color;\n"
"   color = color*u_colorconv;\n"
"   color = applyHue(color);\n"
"   color = ((texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r * u_alpha) * (color);\n"
"   gl_FragColor = color;\n" // * vec4(color.a, color.a, color.a, 1.0);\n"
"}\n";

static const char gFragmentShaderForScreenMasking_MaskedNV12[] =
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_maskCoord;\n"
"uniform sampler2D u_textureSamplerY;\n"
"uniform sampler2D u_textureSamplerUV;\n"
"uniform sampler2D u_maskSampler;\n"
"uniform highp float u_alpha;\n"
"uniform highp vec4 u_color;\n"
"uniform highp mat4 u_maskxfm;\n"
"uniform highp mat4 u_colorconv;\n"

"varying highp vec2 v_texCoord_for_mask;\n"
"uniform sampler2D u_textureSampler_for_mask;\n"
"uniform highp float u_inverse_Mask;\n"
"highp vec4 applyMask(highp vec4 color) {\n"
"  highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;\n"
"  if (u_inverse_Mask > 0.5) {\n"
"    color *= (1.0 - mask.r);\n"
"  }\n"
"  else {\n"
"    color *= mask.r;\n"
"  }\n"
"  return color;\n"
"}\n"

"uniform highp float u_hue;\n"

"highp vec3 rgb2hsv(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
"    highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
"    highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"

"    highp float d = q.x - min(q.w, q.y);\n"
"    highp float e = 1.0e-10;\n"
"    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
"}\n"


"highp vec3 hsv2rgb(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
"    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
"    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
"}\n"

"highp vec4 applyHue(highp vec4 color) {\n"

"    highp vec3 hsv = rgb2hsv(color.rgb);\n"
"    hsv.r += u_hue;\n"
"    color = vec4(hsv2rgb(hsv).rgb, color.a);\n"
"    return color;\n"
"}\n"
"void main() {\n"
"   highp vec4 color = vec4(0, 0, 0, 1);\n"
"   highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside\n"
"   if(s > 0.0){\n"
"       color.r = texture2D(u_textureSamplerY,v_texCoord).r;\n"
#ifdef __APPLE__
"       color.gb = texture2D(u_textureSamplerUV,v_texCoord).rg;\n"
#elif defined(ANDROID)
"       color.gb = texture2D(u_textureSamplerUV,v_texCoord).ra;\n"
#endif
"   }\n"
"   color = color*(1.0-u_color.a)+u_color;\n"
"   color = color*u_colorconv;\n"
"   color = applyHue(color);\n"
"   color = ((texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r * u_alpha) * (color);\n"
"   gl_FragColor = applyMask(color);\n" // * vec4(color.a, color.a, color.a, 1.0);\n"
"}\n";

//------ Combined MASKED+NV12 ------------------------------------------------------------
static const char gFragmentShader_CombinedMaskedNV12[] =
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_texCoord_for_mask;\n"
"varying highp vec2 v_maskCoord;\n"
"uniform sampler2D u_textureSamplerY;\n"
"uniform sampler2D u_textureSamplerUV;\n"
"uniform sampler2D u_back_textureSampler;\n"
"uniform sampler2D u_maskSampler;\n"
"uniform highp float u_alpha;\n"
"uniform highp vec4 u_color;\n"
"uniform highp mat4 u_maskxfm;\n"
"uniform highp mat4 u_colorconv;\n"
"uniform highp float u_hue;\n"

"highp vec3 rgb2hsv(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
"    highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
"    highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"

"    highp float d = q.x - min(q.w, q.y);\n"
"    highp float e = 1.0e-10;\n"
"    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
"}\n"


"highp vec3 hsv2rgb(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
"    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
"    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
"}\n"

"highp vec4 applyHue(highp vec4 color) {\n"

"    highp vec3 hsv = rgb2hsv(color.rgb);\n"
"    hsv.r += u_hue;\n"
"    color = vec4(hsv2rgb(hsv).rgb, color.a);\n"
"    return color;\n"
"}\n"
"void main() {\n"
"   highp vec4 color = vec4(0, 0, 0, 1);\n"
"   highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside\n"
"   if(s > 0.0){\n"
"       color.r = texture2D(u_textureSamplerY,v_texCoord).r;\n"
#ifdef __APPLE__
"       color.gb = texture2D(u_textureSamplerUV,v_texCoord).rg;\n"
#elif defined(ANDROID)
"       color.gb = texture2D(u_textureSamplerUV,v_texCoord).ra;\n"
#endif
"   }\n"
"	else\n"
"		color = texture2D(u_back_textureSampler, v_texCoord_for_back);\n"
"   color = color*(1.0-u_color.a)+u_color;\n"
"   color = color*u_colorconv;\n"
"   color = applyHue(color);\n"
"   color = ((texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r * u_alpha) * (color);\n"
"   gl_FragColor = color;\n" // * vec4(color.a, color.a, color.a, 1.0);\n"
"}\n";

static const char gFragmentShaderForScreenMasking_CombinedMaskedNV12[] =
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_texCoord_for_mask;\n"
"varying highp vec2 v_maskCoord;\n"
"uniform sampler2D u_textureSamplerY;\n"
"uniform sampler2D u_textureSamplerUV;\n"
"uniform sampler2D u_back_textureSampler;\n"
"uniform sampler2D u_maskSampler;\n"
"uniform highp float u_alpha;\n"
"uniform highp vec4 u_color;\n"
"uniform highp mat4 u_maskxfm;\n"
"uniform highp mat4 u_colorconv;\n"
"varying highp vec2 v_texCoord_for_mask;\n"
"uniform sampler2D u_textureSampler_for_mask;\n"
"uniform highp float u_inverse_Mask;\n"
"highp vec4 applyMask(highp vec4 color) {\n"
"  highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;\n"
"  if (u_inverse_Mask > 0.5) {\n"
"    color *= (1.0 - mask.r);\n"
"  }\n"
"  else {\n"
"    color *= mask.r;\n"
"  }\n"
"  return color;\n"
"}\n"
"uniform highp float u_hue;\n"

"highp vec3 rgb2hsv(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
"    highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
"    highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"

"    highp float d = q.x - min(q.w, q.y);\n"
"    highp float e = 1.0e-10;\n"
"    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
"}\n"


"highp vec3 hsv2rgb(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
"    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
"    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
"}\n"

"highp vec4 applyHue(highp vec4 color) {\n"

"    highp vec3 hsv = rgb2hsv(color.rgb);\n"
"    hsv.r += u_hue;\n"
"    color = vec4(hsv2rgb(hsv).rgb, color.a);\n"
"    return color;\n"
"}\n"
"void main() {\n"
"   highp vec4 color = vec4(0, 0, 0, 1);\n"
"   highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside\n"
"   if(s > 0.0){\n"
"       color.r = texture2D(u_textureSamplerY,v_texCoord).r;\n"
#ifdef __APPLE__
"       color.gb = texture2D(u_textureSamplerUV,v_texCoord).rg;\n"
#elif defined(ANDROID)
"       color.gb = texture2D(u_textureSamplerUV,v_texCoord).ra;\n"
#endif
"   }\n"
"	else\n"
"		color = texture2D(u_back_textureSampler, v_texCoord_for_back);\n"
"   color = applyHue(color)*(1.0-u_color.a)+u_color;\n"
"   color = color*u_colorconv;\n"
"   color = ((texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r * u_alpha) * (color);\n"
"   gl_FragColor = applyMask(color);\n" // * vec4(color.a, color.a, color.a, 1.0);\n"
"}\n";

//------ MASKED ----------------------------------------------------------------
static const char gFragmentShader_Masked[] =
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_maskCoord;\n"
"uniform sampler2D u_textureSampler;\n"
"uniform sampler2D u_maskSampler;\n"
"uniform highp float u_alpha;\n"
"uniform highp vec4 u_color;\n"
"uniform highp mat4 u_colorconv;\n"
"uniform highp mat4 u_maskxfm;\n"
"uniform highp float u_hue;\n"

"highp vec3 rgb2hsv(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
"    highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
"    highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"

"    highp float d = q.x - min(q.w, q.y);\n"
"    highp float e = 1.0e-10;\n"
"    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
"}\n"


"highp vec3 hsv2rgb(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
"    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
"    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
"}\n"

"highp vec4 applyHue(highp vec4 color) {\n"

"    highp vec3 hsv = rgb2hsv(color.rgb);\n"
"    hsv.r += u_hue;\n"
"    color = vec4(hsv2rgb(hsv).rgb, color.a);\n"
"    return color;\n"
"}\n"
"void main() {\n"
"  highp vec4 color = vec4(0, 0, 0, 1);\n"
"  highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside\n"
"  if(s > 0.0)\n"
"    color.rgba = texture2D(u_textureSampler, v_texCoord).rgba;\n"
"  color = color*u_colorconv;\n"
"  color = applyHue(color);\n"
"  color = u_color*(1.0-color.a)+color;\n"
"  color = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r*u_alpha*color;\n"
"  gl_FragColor = color;\n"
"}\n";

//------ MASKED ----------------------------------------------------------------
static const char gFragmentShaderForScreenMasking_Masked[] =
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_maskCoord;\n"
"uniform sampler2D u_textureSampler;\n"
"uniform sampler2D u_maskSampler;\n"
"uniform highp float u_alpha;\n"
"uniform highp vec4 u_color;\n"
"uniform highp mat4 u_colorconv;\n"
"uniform highp mat4 u_maskxfm;\n"
"varying highp vec2 v_texCoord_for_mask;\n"
"uniform sampler2D u_textureSampler_for_mask;\n"
"uniform highp float u_inverse_Mask;\n"
"highp vec4 applyMask(highp vec4 color) {\n"
"  highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;\n"
"  if (u_inverse_Mask > 0.5) {\n"
"    color *= (1.0 - mask.r);\n"
"  }\n"
"  else {\n"
"    color *= mask.r;\n"
"  }\n"
"  return color;\n"
"}\n"
"uniform highp float u_hue;\n"

"    highp vec3 rgb2hsv(highp vec3 c)\n"
"    {\n"
"        highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
"        highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
"        highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"

"        highp float d = q.x - min(q.w, q.y);\n"
"        highp float e = 1.0e-10;\n"
"        return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
"    }\n"
"    \n"

"    highp vec3 hsv2rgb(highp vec3 c)\n"
"    {\n"
"        highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
"        highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
"        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
"    }\n"

"	highp vec4 applyHue(highp vec4 color) {\n"

"        highp vec3 hsv = rgb2hsv(color.rgb);\n"
"        hsv.r += u_hue;\n"
"        color = vec4(hsv2rgb(hsv).rgb, color.a);\n"
"        return color;\n"
"    }\n"
"void main() {\n"
"  highp vec4 color = vec4(0, 0, 0, 1);\n"
"  highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside\n"
"  if(s > 0.0)\n"
"    color.rgba = applyHue(texture2D(u_textureSampler, v_texCoord).rgba);\n"
"  color = color*u_colorconv;\n"
"  color = u_color*(1.0-color.a)+color;\n"
"  color = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r*u_alpha*color;\n"
"  gl_FragColor = applyMask(color);\n"
"}\n";
//------ Combined MASKED ----------------------------------------------------------------
static const char gFragmentShader_CombinedMasked[] =
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_texCoord_for_back;\n"
"varying highp vec2 v_maskCoord;\n"
"uniform sampler2D u_textureSampler;\n"
"uniform sampler2D u_back_textureSampler;\n"
"uniform sampler2D u_maskSampler;\n"
"uniform highp float u_alpha;\n"
"uniform highp vec4 u_color;\n"
"uniform highp mat4 u_colorconv;\n"
"uniform highp mat4 u_maskxfm;\n"
"uniform highp float u_hue;\n"

"highp vec3 rgb2hsv(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
"    highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
"    highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"

"    highp float d = q.x - min(q.w, q.y);\n"
"    highp float e = 1.0e-10;\n"
"    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
"}\n"


"highp vec3 hsv2rgb(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
"    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
"    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
"}\n"

"highp vec4 applyHue(highp vec4 color) {\n"

"    highp vec3 hsv = rgb2hsv(color.rgb);\n"
"    hsv.r += u_hue;\n"
"    color = vec4(hsv2rgb(hsv).rgb, color.a);\n"
"    return color;\n"
"}\n"
"void main() {\n"
"  highp vec4 color = vec4(0, 0, 0, 1);\n"
"  highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside\n"
"  if(s > 0.0)\n"
"    color.rgba = texture2D(u_textureSampler, v_texCoord).rgba;\n"
"  else\n"
"	 color.rgba = texture2D(u_back_textureSampler, v_texCoord_for_back).rgba;\n"
"  color = color*u_colorconv;\n"
"  color = applyHue(color);\n"
"  color = u_color*(1.0-color.a)+color;\n"
"  color = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r*u_alpha*color;\n"
"  gl_FragColor = color;\n"
"}\n";

static const char gFragmentShaderForScreenMasking_CombinedMasked[] =
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_texCoord_for_back;\n"
"varying highp vec2 v_maskCoord;\n"
"uniform sampler2D u_textureSampler;\n"
"uniform sampler2D u_back_textureSampler;\n"
"uniform sampler2D u_maskSampler;\n"
"uniform highp float u_alpha;\n"
"uniform highp vec4 u_color;\n"
"uniform highp mat4 u_colorconv;\n"
"uniform highp mat4 u_maskxfm;\n"

"varying highp vec2 v_texCoord_for_mask;\n"
"uniform sampler2D u_textureSampler_for_mask;\n"
"uniform highp float u_inverse_Mask;\n"
"highp vec4 applyMask(highp vec4 color) {\n"
"  highp vec4 mask = (texture2D(u_textureSampler_for_mask, v_texCoord_for_mask)).rgba;\n"
"  if (u_inverse_Mask > 0.5) {\n"
"    color *= (1.0 - mask.r);\n"
"  }\n"
"  else {\n"
"    color *= mask.r;\n"
"  }\n"
"  return color;\n"
"}\n"

"uniform highp float u_hue;\n"

"highp vec3 rgb2hsv(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
"    highp vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
"    highp vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"

"    highp float d = q.x - min(q.w, q.y);\n"
"    highp float e = 1.0e-10;\n"
"    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
"}\n"


"highp vec3 hsv2rgb(highp vec3 c)\n"
"{\n"
"    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
"    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
"    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
"}\n"

"highp vec4 applyHue(highp vec4 color) {\n"

"    highp vec3 hsv = rgb2hsv(color.rgb);\n"
"    hsv.r += u_hue;\n"
"    color = vec4(hsv2rgb(hsv).rgb, color.a);\n"
"    return color;\n"
"}\n"

"void main() {\n"
"  highp vec4 color = vec4(0, 0, 0, 1);\n"
"  highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,1.0) * step(v_texCoord.x,1.0); // s == 1.0 if inside, 0.0 if outside\n"
"  if(s > 0.0)\n"
"    color.rgba = texture2D(u_textureSampler, v_texCoord).rgba;\n"
"  else\n"
"	 color.rgba = texture2D(u_back_textureSampler, v_texCoord_for_back).rgba;\n"
"  color = color*u_colorconv;\n"
"  color = applyHue(color);\n"
"  color = u_color*(1.0-color.a)+color;\n"
"  color = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r*u_alpha*color;\n"
"  gl_FragColor = color;\n"
"}\n";

//------ SOLID NV12->YUVA8888 --------------------------------------------------
// Needs lighting
static const char gFragmentShader_SolidNV12toYUVA8888[] = GLSL(

    varying highp vec4 v_color;
    uniform highp float u_alpha;
    
    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------
    
    void main() {
        const highp mat4 rgbyuv = mat4(
                             0.300,  0.589,  0.111, -0.003,
                            -0.169, -0.332,  0.502,  0.502,
                             0.499, -0.420, -0.079,  0.502,
                             0.000,  0.000,  0.000,  1.000);
        gl_FragColor = CHANNEL_FILTER(u_alpha*applyLighting(v_color)*rgbyuv);
    }
);

//------ MASKED+RGBA->YUVA8888 ------------------------------------------------------------

static const char gFragmentShader_MaskedRGBAtoYUVA8888[] =
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_maskCoord;\n"
"uniform sampler2D u_textureSampler;\n"
"uniform sampler2D u_maskSampler;\n"
"uniform highp float u_alpha;\n"
"uniform highp vec4 u_color;\n"
"uniform highp mat4 u_maskxfm;\n"
"void main() {\n"
"  const highp mat4 rgbyuv = mat4(\n"
"    0.300,  0.589,  0.111, -0.003,\n"
"    -0.169, -0.332,  0.502,  0.502,\n"
"    0.499, -0.420, -0.079,  0.502,\n"
"    0.000,  0.000,  0.000,  1.000);\n"
"  highp vec4 color;\n"
"  color.rgba = texture2D(u_textureSampler, v_texCoord).rgba;\n"
"  color = u_color*(1.0-color.a)+color;\n"
"  color = (texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r*u_alpha*color;\n"
"  gl_FragColor = CHANNEL_FILTER(color * vec4(color.a, color.a, color.a, 1.0) * rgbyuv);\n"
"}\n";

//------ TEXTURED+RGBA->YUVA8888 ----------------------------------------------------------
// Needs lighting
static const char gFragmentShader_TexturedRGBAtoYUVA8888[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSampler;
    uniform highp float u_alpha;

    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    
    void main() {
        const highp mat4 rgbyuv = mat4(
                         0.300,  0.589,  0.111, -0.003,
                        -0.169, -0.332,  0.502,  0.502,
                         0.499, -0.420, -0.079,  0.502,
                         0.000,  0.000,  0.000,  1.000);
        highp vec4 color;      
        color.rgba = (texture2D(u_textureSampler, v_texCoord)).rgba;
        gl_FragColor = CHANNEL_FILTER(u_alpha * applyLighting(color) * rgbyuv);
    }
    
);


//------ MASKED+NV12->YUVA8888 ------------------------------------------------------------
static const char gFragmentShader_MaskedNV12toYUVA8888[] =
"varying highp vec2 v_texCoord;\n"
"varying highp vec2 v_maskCoord;\n"
"uniform sampler2D u_textureSamplerY;\n"
"uniform sampler2D u_textureSamplerUV;\n"
"uniform sampler2D u_maskSampler;\n"
"uniform highp float u_alpha;\n"
"uniform highp vec4 u_color;\n"
"uniform highp mat4 u_maskxfm;\n"
"void main() {\n"
"    highp vec4 color;\n"
"    color.r = texture2D(u_textureSamplerY,v_texCoord).r;\n"
"    color.gb = texture2D(u_textureSamplerUV,v_texCoord).ra;\n"
"    color.a = 1.0;\n"
"    color = u_color*(1.0-color.a)+color;\n"
//"    color = texture2D(u_maskSampler, v_maskCoord).r * u_alpha * color;\n"
"    gl_FragColor = CHANNEL_FILTER(((texture2D(u_maskSampler, v_maskCoord)*u_maskxfm).r*u_alpha) * color);\n"
//"    gl_FragColor = CHANNEL_FILTER(color * vec4(color.a, color.a, color.a, 1.0));\n"
"}\n";

//------ TEXTURED+NV12->YUVA8888 ----------------------------------------------------------
// Needs lighting
static const char gFragmentShader_TexturedNV12toYUVA8888[] = GLSL(

    varying highp vec2 v_texCoord;
    uniform sampler2D u_textureSamplerY;
    uniform sampler2D u_textureSamplerUV;
    uniform highp float u_alpha;
    uniform highp float u_realY;
    uniform highp float u_realX;
    uniform highp mat4 u_colorconv;
    
    // ---- Lighting ----
    uniform highp vec4 u_diffuse_light;
    uniform highp vec4 u_ambient_light;
    uniform highp vec4 u_specular_light;
    varying highp float v_diffuse_intensity;
    varying highp float v_specular_intensity;
    
    highp vec4 applyLighting( highp vec4 color ) {
    	highp vec4 result = (u_ambient_light * color) + (u_diffuse_light * v_diffuse_intensity * color) + (u_specular_light * v_specular_intensity);
        result.a = color.a;
        return result;
    }
    // -------------------

    void main() {
    
        const highp mat4 rgb_biasedyuv = mat4(
        								 0.257,	 0.504,	 0.098,	0.0625,
                                        -0.148,	-0.291,	 0.439,	0.500,
                                         0.439,	-0.368,	-0.071,	0.500,
                                         0.000,  0.000,  0.000, 1.000);
        highp vec4 color;
        color.r = texture2D(u_textureSamplerY,v_texCoord).r;
        color.g = texture2D(u_textureSamplerUV,v_texCoord).r;
        color.b = texture2D(u_textureSamplerUV,v_texCoord).a;
        color.a = 1.0;
        highp float s = step(0.0,v_texCoord.y) * step(0.0,v_texCoord.x) * step(v_texCoord.y,u_realY) * step(v_texCoord.x,u_realX); // s == 1.0 if inside, 0.0 if outside (edge, value)
        color = color * u_colorconv;
        color = clamp(color, 0.0, 1.0);
        color = color*rgb_biasedyuv;
        gl_FragColor = CHANNEL_FILTER(u_alpha * applyLighting(color*s));
        
    }
    
);

//------------------------------------------------------------------------------


#ifdef LOAD_SHADER_ON_TIME

static void registerShaderProgram( NXT_ShaderProgram_Base* base,
                                    const char *programName,
                                    const char *vertexSource,
                                    const char *fragmentSource,
                                    const NXT_Shader_LightingInfo *lightingInfo,
                                    const char *macroDefs,
                                    NXT_ShaderField* uniforms, // this fields was copied to new memory. should call unregister
                                    int uniformsCnt,
                                    NXT_ShaderField* attributes, // // this fields was copied to new memory. should call unregister
                                    int attributesCnt
                                    )
{
    base->loaded = 0;

    base->programName = (char*)malloc(strlen(programName)+1);
    strcpy(base->programName, (char*)programName);

    base->vertexSource = (char*)vertexSource;
    base->fragmentSource = (char*)fragmentSource;
    base->lightingInfo = (NXT_Shader_LightingInfo*)lightingInfo;
    base->macroDefs = (char*)macroDefs;

    base->uniforms = (NXT_ShaderField*)malloc(sizeof(NXT_ShaderField)*(uniformsCnt+1));
    memcpy( base->uniforms, uniforms, sizeof(NXT_ShaderField)*(uniformsCnt) );
    base->uniforms[uniformsCnt].fieldId = NULL;
    base->uniforms[uniformsCnt].fieldName = NULL;

    base->attributes = (NXT_ShaderField*)malloc(sizeof(NXT_ShaderField)*(attributesCnt+1));
    memcpy( base->attributes, attributes, sizeof(NXT_ShaderField)*(attributesCnt+1) );
    base->attributes[attributesCnt].fieldId = NULL;
    base->attributes[attributesCnt].fieldName = NULL;
}

static void unregisterShaderProgram(NXT_ShaderProgram_Base* base)
{
    if ( base->uniforms )
        free(base->uniforms);
    
    if ( base->attributes )
        free(base->attributes);

    if ( base->programName )
        free(base->programName);

    memset(base, 0x0, sizeof(NXT_ShaderProgram_Base));
}


NXT_Error NexThemeRenderer_UseShaderProgram( void* programBase,
                                   int programSize)
{
    NXT_ShaderProgram_Base* base = (NXT_ShaderProgram_Base*)programBase;
    if ( !base->loaded ) {

    LOGI("Begin createShaderProgram:%s,%d,%d", base->programName,programSize,sizeof(NXT_ShaderProgram_Base));

    GLuint *pbase = (GLuint*)(&base[1]);
    LOGE("(Eric)%p,%p", programBase, pbase);
    int numentries = (programSize-sizeof(NXT_ShaderProgram_Base))/sizeof(GLuint);
    if( numentries > 0 && pbase ) {
        int i;
        for( i=0; i<numentries; i++ ) {
            pbase[i] = INVALID_UNIFORM;
        }
    }
    LOGE("(Eric)numentries %d", numentries);

    base->shaderProgram = 0;
    base->shaderVertex = 0;
    base->shaderFragment = 0;

    // Load the shaders
    GLuint hVertexShader = loadShader(GL_VERTEX_SHADER, base->vertexSource, base->macroDefs);
    GLuint hFragmentShader = loadShader(GL_FRAGMENT_SHADER, base->fragmentSource, base->macroDefs);
    if( !hVertexShader ) {
        LOGE("[%s %d] Failed to load vertex shader (%s)", __func__, __LINE__, base->programName);
    }
    if( !hFragmentShader ) {
        LOGE("[%s %d] Failed to load fragment shader (%s)", __func__, __LINE__, base->programName);
    }
    if( !hVertexShader || !hFragmentShader )
    {
        if ( hVertexShader ) glDeleteShader(hVertexShader);
        if ( hFragmentShader ) glDeleteShader( hFragmentShader );
        
        return NXT_Error_ShaderFailure;
    }

    GLuint hShaderProgram = glCreateProgram();
    if( !hShaderProgram ) {
        LOGE("[%s %d] Failed to create shader program (%s)", __func__, __LINE__, base->programName);
        if ( hVertexShader ) glDeleteShader(hVertexShader);
        if ( hFragmentShader ) glDeleteShader( hFragmentShader );
        return NXT_Error_ShaderFailure;
    }
    
    glAttachShader(hShaderProgram, hVertexShader);
    CHECK_GL_ERROR();
    glAttachShader(hShaderProgram, hFragmentShader);
    CHECK_GL_ERROR();

    glLinkProgram(hShaderProgram);
    if( shaderProgramLinkStatus(hShaderProgram)!=GL_TRUE ) {
        char *compileErrors = getProgramInfoLog(hShaderProgram);
        glDeleteProgram(hShaderProgram);
        if ( hVertexShader ) glDeleteShader(hVertexShader);
        if ( hFragmentShader ) glDeleteShader( hFragmentShader );
        
        LOGE("----------- Error(s) detected linking shader program --------------\n"
             "Program name: %s\n"
             "%s\n"
             "-------------------------------------------------------------------\n",
             base->programName, compileErrors);
        
        freeProgramInfoLog(compileErrors);
        return NXT_Error_ShaderFailure;
    }

	processUniforms(hShaderProgram, base->programName, base->uniforms);
	processAttributes(hShaderProgram, base->programName, base->attributes);
    

    if( base->lightingInfo ) {
        NXT_Shader_LightingInfo *lightingInfo = base->lightingInfo;
        NXT_ShaderField uniforms[] = {  
        	{&lightingInfo->u_ambient_light, 	"u_ambient_light"},
        	{&lightingInfo->u_diffuse_light, 	"u_diffuse_light"},
        	{&lightingInfo->u_lightpos, 		"u_lightpos"},
        	{&lightingInfo->u_light_f0, 		"u_light_f0"},
        	{&lightingInfo->u_light_f1, 		"u_light_f1"},
        	{&lightingInfo->u_specexp, 			"u_specexp"},
        	{&lightingInfo->u_specular_light, 	"u_specular_light"},
        	{0}
        };
        NXT_ShaderField attributes[]= { 
        	{&lightingInfo->a_normal, 			"a_normal"},
        	{0}
        };

        processUniforms(hShaderProgram, base->programName, uniforms);

        processAttributes(hShaderProgram, base->programName, attributes);
    }
    
    base->shaderProgram = hShaderProgram;
    base->shaderVertex = hVertexShader;
    base->shaderFragment = hFragmentShader;
    
    LOGE("(Eric)End createShaderProgram:%s (hShaderProgram=%d)", base->programName, hShaderProgram);
    base->loaded = 1;
    }

    if ( base->loaded ) {
        glUseProgram(base->shaderProgram);
        CHECK_GL_ERROR();
    }

    return NXT_Error_None;
}


#define CHECK_AND_DELETE_SHADER_UNREGISTER(a) if ( a.base.loaded ) \
deleteShaderProgram( &a.base.shaderProgram, &a.base.shaderVertex, &a.base.shaderFragment ); \
unregisterShaderProgram( &a.base );



NXT_Error NexThemeRenderer_UnloadShaders2(NXT_ThemeRenderer_Context* pctx)
{
    NXT_Error err = NXT_Error_None;
    int i;
    
    // --- SHADER PROGRAM: Solid Color
    //if ( pctx->solidShaderRGB.base.loaded ) deleteShaderProgram( SHADER_SERIES(pctx->solidShaderRGB) );
    //unregisterShaderProgram( &pctx->solidShaderRGB.base )
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->solidShaderRGB);

    // --- SHADER PROGRAM: Screen Masking Type Solid Color
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_solidShaderRGB);

    // --- SHADER PROGRAM: Textured Pass Through ...
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderPassthroughRGB);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderPassthroughRGBWithColorconv);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderLUT);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderPassthroughYUV);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderPassthroughNV12);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderUserVignette);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderSharpness);
#ifndef __APPLE__
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderPassthroughExternal);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderPassthroughExternalHDR10);
#endif
    
    // --- SHADER PROGRAM: Textured+RGBA -> YUVA8888 Pass Through
    for (i = 0; i<NXT_ChannelFilter_MAX; i++) {
        CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderPassThroughRGBAtoYUVA8888[i]);
    }
    
    // --- SHADER PROGRAM: Textured
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB);

    // --- SHADER PROGRAM: Screen Masking Type Textured
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB);

    // --- SHADER PROGRAM: Textured Homo
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_Homo);

    // --- SHADER PROGRAM: Screen Masking Type Textured Homo
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB_Homo);

    // --- SHADER PROGRAM: Combined Textured Homo
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_Combined_Homo);

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured Homo
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB_Combined_Homo);

    // --- SHADER PROGRAM: Textured+VIGNETTE --------------------------------------------
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_vignette);

    // --- SHADER PROGRAM: Screen Masking Type Textured+VIGNETTE --------------------------------------------
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB_vignette);

    // --- SHADER PROGRAM: Textured+VIGNETTE Homo--------------------------------------------
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_vignette_Homo);

    // --- SHADER PROGRAM: Screen Masking Type Textured+VIGNETTE Homo--------------------------------------------
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB_vignette_Homo);

    // --- SHADER PROGRAM: Combined+Textured+VIGNETTE Homo--------------------------------------------
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_Combined_vignette_Homo);

    // --- SHADER PROGRAM: ScreenMaskingType Combined+Textured+VIGNETTE Homo--------------------------------------------
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB_Combined_vignette_Homo);


    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_For_Videos);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_Combined_For_Videos);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_For_Videos360);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_For_Videos_vignette);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_Combined_For_Videos_vignette);

    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB_For_Videos);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB_Combined_For_Videos);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB_For_Videos360);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB_For_Videos_vignette);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderRGB_Combined_For_Videos_vignette);

    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGB_for_layers);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderStencil);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderStencil_Homo);

    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderStencil);

    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderStencil_Homo);
    // --- SHADER PROGRAM: Masked Stencil
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderStencil);
    // --- SHADER PROGRAM: Screen Masking Type Masked Stencil
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderStencil);

    // --- SHADER PROGRAM: Masked Stencil Homo
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderStencil_Homo);
    // --- SHADER PROGRAM: Screen Masking Type Masked Stencil Homo
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderStencil_Homo);

#ifndef __APPLE__
    // --- SHADER PROGRAM: Masked+Textured External
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderExternal);

    // --- SHADER PROGRAM: Screen Masking Type Masked+Textured External
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderExternal);

    // --- SHADER PROGRAM: Combined Masked+Textured External
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderExternal_Combined);

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked+Textured External
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderExternal_Combined);

    // --- SHADER PROGRAM: Textured External
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderExternal);

    // --- SHADER PROGRAM: Screen  Masking Type Textured External
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderExternal);

    // --- SHADER PROGRAM: Combined Textured External
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderExternal_Combined);

    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderExternal360);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderExternal_vignette);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderExternal_Combined_vignette);

    // --- SHADER PROGRAM: Combined Textured External
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderExternal_Combined);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderExternal360);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderExternal_vignette);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderExternal_Combined_vignette);

    // --- SHADER PROGRAM: Textured External->YUVA8888
    for( i=0; i<NXT_ChannelFilter_MAX; i++ ) {
        CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderEXTtoYUVA8888[i]);
    }
#endif

    // --- SHADER PROGRAM: Masked
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderRGB);
    
    // --- SHADER PROGRAM: Screen Masking Type Masked
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderRGB);

    // --- SHADER PROGRAM: Combined Masked
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderRGB_Combined);

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderRGB_Combined);

    // --- SHADER PROGRAM: Masked Homo
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderRGB_Homo);

    // --- SHADER PROGRAM: Screen Masking Type Masked Homo
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderRGB_Homo);

    // --- SHADER PROGRAM: Combined Masked Homo
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderRGB_Combined_Homo);
    
    // --- SHADER PROGRAM: ScreenMaskingtype Combined Masked Homo
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderRGB_Combined_Homo);
    
    // --- SHADER PROGRAM: Textured+YUV
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderYUV);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderYUV_vignette);

    // --- SHADER PROGRAM: Screen Masking Type Textured+YUV
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderYUV);
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderYUV_vignette);

    // --- SHADER PROGRAM: Textured+Y2UVA
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderY2UVA);
    
    // --- SHADER PROGRAM: Masked+YUV
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderYUV);

    // --- SHADER PROGRAM: Screen Masking Type Masked+YUV
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderYUV);

    // --- SHADER PROGRAM: Textured+NV12
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderNV12);
    // --- SHADER PROGRAM: Combined Textured+NV12
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderNV12_Combined);
    // --- SHADER PROGRAM: Textured+NV12+Vignette
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderNV12_vignette);
    // --- SHADER PROGRAM: Combined Textured+NV12+Vignette
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderNV12_Combined_vignette);

    // --- SHADER PROGRAM: Screen Masking Type Textured+NV12
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderNV12);
    // --- SHADER PROGRAM: Screen Masking Type Textured+NV12+Vignette
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderNV12_vignette);
    // --- SHADER PROGRAM: Screen Masking Type Combined Textured+NV12
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderNV12_Combined);
    // --- SHADER PROGRAM: Screen Masking Type Combined Textured+NV12+Vignette
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_texturedShaderNV12_Combined_vignette);

    // --- SHADER PROGRAM: Masked+NV12
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderNV12);

    // --- SHADER PROGRAM: Screen Masking Type Masked+NV12
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderNV12);

    // --- SHADER PROGRAM: Combined Masked+NV12
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderNV12_Combined);
    
    // --- SHADER PROGRAM: Screen Masking Type Combined Masked+NV12
    CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->screenMaskingType_maskedShaderNV12_Combined);

    // --- SHADER PROGRAM: Textured+NV12 -> YUVA8888
    for( i = 0 ; i < NXT_ChannelFilter_MAX ; i++ ) {
        CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderNV12toYUVA8888[i]);
    }
    
    // --- SHADER PROGRAM: Masked+NV12 -> YUVA8888
    for( i=0; i < NXT_ChannelFilter_MAX ; i++ ) {
        CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderNV12toYUVA8888[i]);
    }
    
    // --- SHADER PROGRAM: Textured+RGBA -> YUVA8888
    for( i= 0 ; i < NXT_ChannelFilter_MAX ; i++ ) {
        CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->texturedShaderRGBAtoYUVA8888[i]);
    }

#ifndef __APPLE__
    // --- SHADER PROGRAM: Mask+Textured+External -> YUVA8888 --------------------------------------------
    for( i = 0 ; i < NXT_ChannelFilter_MAX ; i++ ) {
        CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderEXTtoYUVA8888[i]);
    }
#endif
    
    // --- SHADER PROGRAM: Masked+RGBA -> YUVA8888 --------------------------------------------
    for( i = 0 ; i < NXT_ChannelFilter_MAX ; i++ ) {
        CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->maskedShaderRGBAtoYUVA8888[i]);
    }

    // --- SHADER PROGRAM: Solid Color NV12 -> YUVA8888 ------------------------
    for( i = 0 ; i < NXT_ChannelFilter_MAX ; i++ ) {
        CHECK_AND_DELETE_SHADER_UNREGISTER(pctx->solidShaderNV12toYUVA8888[i]);
    }

    return err;
}

static const char *g_channelFilterMacro[NXT_ChannelFilter_MAX] = {
"#define CHANNEL_FILTER(xx) (xx)\n",
"#define CHANNEL_FILTER(xx) ((xx).rrra)\n",
"#define CHANNEL_FILTER(xx) ((xx).ggga)\n",
"#define CHANNEL_FILTER(xx) ((xx).bbba)\n" };


NXT_Error NexThemeRenderer_PrepareShaders(NXT_ThemeRenderer_Context* pctx)
{

    int i;

    // --- SHADER PROGRAM: Solid Color -----------------------------------------
    {
        NXT_ShaderProgram_Solid *sp = &pctx->solidShaderRGB;
        NXT_ShaderField uniforms[] = {  { &sp->u_mvp_matrix,        "u_mvp_matrix" },
                                        { &sp->u_alpha,             "u_alpha" } ,
                                        { &sp->u_colorconv,         "u_colorconv" },
                                        {&sp->u_hue,                "u_hue"},
                                    };
        NXT_ShaderField attributes[]= { { &sp->a_position,          "a_position" },
                                        { &sp->a_color,             "a_color" } };

        registerShaderProgram(&sp->base,"solid",
                            gStdVertexShader_Solid, gFragmentShader_Solid,
                            &sp->lightingInfo,
                            NULL,
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
    }

    // --- SHADER PROGRAM: Screen Masking Type Solid Color -----------------------------------------
    {
        NXT_ShaderProgram_Solid *sp = &pctx->screenMaskingType_solidShaderRGB;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},            
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_color, "a_color"},
        };

        registerShaderProgram(&sp->base, "screen masking type solid",
                              gStdVertexShaderForScreenMasking_Solid, gFragmentShaderForScreenMasking_Solid,
                              &sp->lightingInfo,
                              NULL,
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }
    
    // --- SHADER PROGRAM: Textured Pass Through--------------------------------------------
	{
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughRGB;
        NXT_ShaderField uniforms[] = {  
            { &sp->u_textureSampler,    "u_textureSampler" },
            { &sp->u_hue, "u_hue"}, 
            };
        NXT_ShaderField attributes[]= { { &sp->a_position,          "a_position" },
                                        { &sp->a_texCoord,          "a_texCoord" } };
        
        registerShaderProgram(&sp->base,"textured+passthrough",
                            gStdVertexShaderPassThrough_Textured, gFragmentShaderPassthrough_Textured,
                            NULL,		
                            NULL,     
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
    }

    // --- SHADER PROGRAM: Textured Pass Through With Colorconv--------------------------------------------
	{
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughRGBWithColorconv;
        NXT_ShaderField uniforms[] = {  
        	{ &sp->u_textureSampler,    "u_textureSampler" } ,
        	{ &sp->u_colorconv, "u_colorconv"},
            { &sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[]= { { &sp->a_position,          "a_position" },
                                        { &sp->a_texCoord,          "a_texCoord" } };
        
        registerShaderProgram(&sp->base,"textured+passthrough+colorconv",
                            gStdVertexShaderPassThrough_Textured, gFragmentShaderPassthroughWithColorConv_Textured,
                            NULL,		
                            NULL,     
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
    }
    
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderUserVignette;

        NXT_ShaderField uniforms[] = {
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_vignette, "u_vignette"},
            {&sp->u_vignetteRange, "u_vignetteRange"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };

        registerShaderProgram(&sp->base, "textured+UserVignette",
                              gStdVertexShaderPassThrough_Textured, gFragmentShaderUserVignette_Textured,
                              NULL,
                              NULL,
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }
    
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderSharpness;

        NXT_ShaderField uniforms[] = {
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_sharpness, "u_sharpness"},
            {&sp->u_resolution, "u_resolution"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };

        registerShaderProgram(&sp->base, "textured+sharpness",
                              gStdVertexShaderPassThrough_Textured, gFragmentShaderSharpness_Textured,
                              NULL,
                              NULL,
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }
    
    {
	    NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderLUT;
	    NXT_ShaderField uniforms[] = {  
        	{ &sp->u_textureSampler, "u_textureSampler" },
	        { &sp->u_textureSamplerY, "u_textureLUT" },
	        { &sp->u_realY, "u_realY" },
	        { &sp->u_realX, "u_realX" },
	        { &sp->u_alpha, "u_alpha" },
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        const char *fragmentSource = gFragmentShaderLUT_Textured;
#ifdef USE_FRAGMENT_SHADER_LUT_64x4096
        fragmentSource = gFragmentShaderLUT_64x4096_Textured;
#endif
        registerShaderProgram(&sp->base,"textured+passthrough+lut",
                            gStdVertexShaderPassThrough_Textured, fragmentSource,
                            NULL,							// Lighting info
                            NULL,                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
	}

	{
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughYUV;
		NXT_ShaderField uniforms[] = {  
        	{ &sp->u_textureSamplerY, "u_textureSamplerY" },
            { &sp->u_textureSamplerU, "u_textureSamplerU" },
            { &sp->u_textureSamplerV, "u_textureSamplerV" },
            { &sp->u_hue, "u_hue" },
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
		registerShaderProgram(&sp->base, "Textured+passthrough+YUV",
                            gStdVertexShaderPassThrough_Textured, gFragmentShaderPassthrough_TexturedYUV,
                            NULL,									// Lighting info
                            NULL,			                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
	}

	{
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughNV12;
		NXT_ShaderField uniforms[] = {  
        	{ &sp->u_textureSamplerY, "u_textureSamplerY" },
            { &sp->u_textureSamplerUV, "u_textureSamplerUV" },
            { &sp->u_realY, "u_realY" },
            { &sp->u_realX, "u_realX" },
            { &sp->u_colorconv, "u_colorconv" },
            { &sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
		registerShaderProgram(&sp->base, "Textured+passthrough+NV12",
                            gStdVertexShaderPassThrough_Textured, gFragmentShaderPassthrough_TexturedNV12,
                            NULL,			// Lighting info
                            NULL,                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
	}

#ifndef __APPLE__
	{
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughExternal;
		NXT_ShaderField uniforms[] = {  
        	{ &sp->u_tex_matrix, "u_tex_matrix" },
            { &sp->u_textureSampler, "u_textureSampler" },
            { &sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
    		{ &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured+passthrough+external",
                              gStdVertexShaderPassThroughExternal_Textured, gFragmentShaderPassthrough_TexturedExternal,
                              NULL, // Lighting info
                              NULL, // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    {
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughExternalHDR10;
		NXT_ShaderField uniforms[] = {  
        	{ &sp->u_tex_matrix, "u_tex_matrix" },
            { &sp->u_textureSampler, "u_textureSampler" },
            { &sp->u_MaximumContentLightLevel, "u_MaximumContentLightLevel"},
            { &sp->u_DeviceLightLevel, "u_DeviceLightLevel"},
            { &sp->u_Gamma, "u_Gamma"},
            { &sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
    		{ &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured+passthrough+external+HDR10",
                              gStdVertexShaderPassThroughExternal_Textured, gFragmentShaderPassthrough_TexturedExternalHDR10,
                              NULL, // Lighting info
                              NULL, // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }
#endif // not APPLE

	// --- SHADER PROGRAM: Textured+RGBA -> YUVA8888 Pass Through--------------------------------------------
	for (i = 0; i<NXT_ChannelFilter_MAX; i++)
	{
		char programName[] = "Textured+RGBA->YUV[_] PassThrough";
		programName[sizeof(programName)-3] = '0' + i;

		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassThroughRGBAtoYUVA8888[i];
		NXT_ShaderField uniforms[] = {  
        	{ &sp->u_textureSampler, "u_textureSampler" },
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
		registerShaderProgram(&sp->base, programName,
                            gStdVertexShaderPassThrough_Textured, gFragmentShaderPassthrough_TexturedRGBAtoYUVA8888,
                            NULL,							// Lighting info
                            g_channelFilterMacro[i],                        // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
	}

    // --- SHADER PROGRAM: Textured --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
          	{&sp->a_texCoord,         "a_texCoord"},
        };
        registerShaderProgram(&sp->base,"textured",
                            gStdVertexShader_Textured, gFragmentShader_Textured,
                            &sp->lightingInfo,			// Lighting info
                            NULL,                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured",
                              gStdVertexShaderForScreenMasking_Textured, gFragmentShaderForScreenMasking_Textured,
                              &sp->lightingInfo, // Lighting info
                              NULL,              // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured Homo--------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_Homo;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
        	{&sp->u_homo_matrix,      "u_homo_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_realY,			  "u_realY"}, 
            {&sp->u_realX,			  "u_realX"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
        };
        registerShaderProgram(&sp->base,"textured homo",
                            gHomoVertexShader, gHomoFragmentShader_Textured,
                            NULL,			// Lighting info
                            NULL,                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured Homo--------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_Homo;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_homo_matrix, "u_homo_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured homo",
                              gHomoVertexShaderForScreenMasking, gHomoFragmentShaderForScreenMasking_Textured,
                              NULL, // Lighting info
                              NULL, // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Combined Textured Homo--------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_Combined_Homo;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       		"u_mvp_matrix"},
        	{&sp->u_homo_matrix,      		"u_homo_matrix"},
			{&sp->u_tex_matrix,       		"u_tex_matrix"},
			{&sp->u_back_tex_matrix,  		"u_back_tex_matrix"},
			{&sp->u_textureSampler,   		"u_textureSampler"},
			{&sp->u_back_textureSampler,   	"u_back_textureSampler"},
			{&sp->u_alpha,            		"u_alpha"},
			{&sp->u_colorconv,        		"u_colorconv"},
			{&sp->u_realY,			  		"u_realY"}, 
            {&sp->u_realX,			  		"u_realX"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
        };
        registerShaderProgram(&sp->base,"textured homo",
                            gHomoVertexShader, gHomoFragmentShader_CombinedTextured,
                            NULL,			// Lighting info
                            NULL,                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured Homo--------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_Combined_Homo;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_homo_matrix, "u_homo_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured homo",
                              gHomoVertexShaderForScreenMasking, gHomoFragmentShaderForScreenMasking_CombinedTextured,
                              NULL, // Lighting info
                              NULL, // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured+VIGNETTE --------------------------------------------
    {
    	NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_vignette;
    	NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
	        {&sp->u_tex_matrix,       "u_tex_matrix"},
	        {&sp->u_textureSampler,   "u_textureSampler"},
	        {&sp->u_textureSamplerY,   "u_textureSampler_vignette"},
	        {&sp->u_alpha,            "u_alpha"},
	        {&sp->u_colorconv,        "u_colorconv"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured+vignette",
                            gStdVertexShader_Textured_vignette,    // Vertex shader
                            gFragmentShader_Textured_vignette,     // Fragment shader
                            &sp->lightingInfo,			// Lighting info
                            NULL,                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+VIGNETTE --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_vignette;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured+vignette",
                              gStdVertexShaderForScreenMasking_Textured_vignette, // Vertex shader
                              gFragmentShaderForScreenMasking_Textured_vignette,  // Fragment shader
                              &sp->lightingInfo,                                  // Lighting info
                              NULL,                                               // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured+VIGNETTE Homo--------------------------------------------
    {
    	NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_vignette_Homo;
    	NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
        	{&sp->u_homo_matrix,      "u_homo_matrix"},
	        {&sp->u_tex_matrix,       "u_tex_matrix"},
	        {&sp->u_textureSampler,   "u_textureSampler"},
	        {&sp->u_textureSamplerY,  "u_textureSampler_vignette"},
	        {&sp->u_alpha,            "u_alpha"},
	        {&sp->u_colorconv,        "u_colorconv"},
	        {&sp->u_realY,			  "u_realY"}, 
            {&sp->u_realX,			  "u_realX"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
        };
        registerShaderProgram(&sp->base, "textured+vignette homo",
                            gHomoVertexShader,    // Vertex shader
                            gHomoFragmentShader_Textured_vignette,     // Fragment shader
                            &sp->lightingInfo,			// Lighting info
                            NULL,                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+VIGNETTE Homo--------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_vignette_Homo;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_homo_matrix, "u_homo_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured+vignette homo",
                              gHomoVertexShaderForScreenMasking,                     // Vertex shader
                              gHomoFragmentShaderForScreenMasking_Textured_vignette, // Fragment shader
                              &sp->lightingInfo,                                     // Lighting info
                              NULL,                                                  // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Combined Textured+VIGNETTE Homo--------------------------------------------
    {
    	NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_Combined_vignette_Homo;
    	NXT_ShaderField uniforms[] = {
        	{&sp->u_mvp_matrix,       	"u_mvp_matrix"},
			{&sp->u_homo_matrix,      	"u_homo_matrix"},
			{&sp->u_tex_matrix,       	"u_tex_matrix"},
			{&sp->u_back_tex_matrix,  	"u_back_tex_matrix"},
			{&sp->u_textureSampler,   	"u_textureSampler"},
			{&sp->u_back_textureSampler,"u_back_textureSampler"},
			{&sp->u_textureSamplerY,  	"u_textureSampler_vignette"},
			{&sp->u_alpha,            	"u_alpha"},
			{&sp->u_colorconv,        	"u_colorconv"},
			{&sp->u_realY,				"u_realY"}, 
			{&sp->u_realX,				"u_realX"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
        };
        registerShaderProgram(&sp->base, "combined+textured+vignette homo",
                            gHomoVertexShader,    // Vertex shader
                            gHomoFragmentShader_CombinedTextured_vignette,     // Fragment shader
                            &sp->lightingInfo,			// Lighting info
                            NULL,                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured+VIGNETTE Homo--------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_Combined_vignette_Homo;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_homo_matrix, "u_homo_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
        };
        registerShaderProgram(&sp->base, "screen masking type combined+textured+vignette homo",
                              gHomoVertexShaderForScreenMasking,                             // Vertex shader
                              gHomoFragmentShaderForScreenMasking_CombinedTextured_vignette, // Fragment shader
                              &sp->lightingInfo,                                             // Lighting info
                              NULL,                                                          // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_For_Videos;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
            {&sp->u_tex_matrix,       "u_tex_matrix"},
            {&sp->u_textureSampler,   "u_textureSampler"},
            {&sp->u_alpha,            "u_alpha"},
            {&sp->u_colorconv,        "u_colorconv"},
            {&sp->u_realY,			  "u_realY"}, 
            {&sp->u_realX,			  "u_realX"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured video",
                            gStdVertexShader_Textured, gFragmentShader_Textured_ForVideo,
                            &sp->lightingInfo,			// Lighting info
                            NULL,                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
    }

    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_For_Videos;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured video",
                              gStdVertexShaderForScreenMasking_Textured, gFragmentShaderForScreenMasking_Textured_ForVideo,
                              &sp->lightingInfo, // Lighting info
                              NULL,              // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_Combined_For_Videos;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       	"u_mvp_matrix"},
            {&sp->u_tex_matrix,       	"u_tex_matrix"},
            {&sp->u_back_tex_matrix,    "u_back_tex_matrix"},
            {&sp->u_textureSampler,   	"u_textureSampler"},
            {&sp->u_back_textureSampler,"u_back_textureSampler"},
            {&sp->u_alpha,            	"u_alpha"},
            {&sp->u_colorconv,        	"u_colorconv"},
            {&sp->u_realY,				"u_realY"}, 
            {&sp->u_realX,				"u_realX"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "combined textured video",
                            gStdVertexShader_CombinedTextured, gFragmentShader_CombinedTextured_ForVideo,
                            &sp->lightingInfo,			// Lighting info
                            NULL,                         // Macro definitions
                            uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                            );
    }

    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_Combined_For_Videos;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type combined textured video",
                              gStdVertexShaderForScreenMasking_CombinedTextured, gFragmentShaderForScreenMasking_CombinedTextured_ForVideo,
                              &sp->lightingInfo, // Lighting info
                              NULL,              // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_For_Videos360;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_realY,			"u_realY"}, 
			{&sp->u_realX,			"u_realX"},
			{&sp->u_moveX_value,		"u_moveX_value"},
			{&sp->u_moveY_value,		"u_moveY_value"},
            {&sp->u_hue,            "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured+360",
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_Textured_ForVideo360,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            	  attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_For_Videos360;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_moveX_value, "u_moveX_value"},
            {&sp->u_moveY_value, "u_moveY_value"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured+360",
                              gStdVertexShaderForScreenMasking_Textured,            // Vertex shader
                              gFragmentShaderForScreenMasking_Textured_ForVideo360, // Fragment shader
                              &sp->lightingInfo,                                    // Lighting info
                              NULL,                                                 // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_For_Videos_vignette;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_textureSamplerY,  "u_textureSampler_vignette"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_realY,			"u_realY"}, 
			{&sp->u_realX,			"u_realX"},
            {&sp->u_hue,            "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured+vignette",
                                  gStdVertexShader_Textured_vignette,    // Vertex shader
                                  gFragmentShader_Textured_ForVideo_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            		attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_For_Videos_vignette;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured+vignette",
                              gStdVertexShaderForScreenMasking_Textured_vignette,         // Vertex shader
                              gFragmentShaderForScreenMasking_Textured_ForVideo_vignette, // Fragment shader
                              &sp->lightingInfo,                                          // Lighting info
                              NULL,                                                       // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_Combined_For_Videos_vignette;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       		"u_mvp_matrix"},
			{&sp->u_tex_matrix,       		"u_tex_matrix"},
			{&sp->u_back_tex_matrix,       	"u_back_tex_matrix"},
			{&sp->u_textureSampler,   		"u_textureSampler"},
			{&sp->u_back_textureSampler,   	"u_back_textureSampler"},
			{&sp->u_textureSamplerY,  		"u_textureSampler_vignette"},
			{&sp->u_alpha,            		"u_alpha"},
			{&sp->u_colorconv,        		"u_colorconv"},
			{&sp->u_realY,					"u_realY"}, 
			{&sp->u_realX,					"u_realX"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "combined textured+vignette",
                                  gStdVertexShader_CombinedTextured_vignette,    // Vertex shader
                                  gFragmentShader_CombinedTextured_ForVideo_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            		attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_Combined_For_Videos_vignette;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type combined textured+vignette",
                              gStdVertexShaderForScreenMasking_CombinedTextured_vignette,         // Vertex shader
                              gFragmentShaderForScreenMasking_CombinedTextured_ForVideo_vignette, // Fragment shader
                              &sp->lightingInfo,                                                  // Lighting info
                              NULL,                                                               // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    {
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_for_layers;
		NXT_ShaderField uniforms[] = {  
        	{ &sp->u_mvp_matrix, "u_mvp_matrix" },
			{ &sp->u_tex_matrix, "u_tex_matrix" },
			{ &sp->u_textureSampler, "u_textureSampler" },
			{ &sp->u_textureSamplerY, "u_textureSampler_for_mask" },
			{ &sp->u_alpha, "u_alpha" },
			{ &sp->u_colorconv, "u_colorconv" },
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
		registerShaderProgram(&sp->base, "textured_for_layers",
                                  gStdVertexShader_Textured_For_Layers,    // Vertex shader
                                  gFragmentShader_Textured_For_Layers,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])                                  );
	}

    // --- SHADER PROGRAM: Textured Stencil --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderStencil;
        NXT_ShaderField uniforms[] = {  
			{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_alpha,            "u_alpha"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured+stencil",
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_TexturedStencil,// Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured Stencil --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderStencil;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured+stencil",
                              gStdVertexShaderForScreenMasking_Textured,       // Vertex shader
                              gFragmentShaderForScreenMasking_TexturedStencil, // Fragment shader
                              NULL,                                            // Lighting info
                              NULL,                                            // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured Stencil Homo--------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderStencil_Homo;
        NXT_ShaderField uniforms[] = {  
			{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_homo_matrix,      "u_homo_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_alpha,            "u_alpha"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
        };
        registerShaderProgram(&sp->base, "textured+stencil homo",
                                  gHomoVertexShader,    // Vertex shader
                                  gHomoFragmentShader_TexturedStencil,// Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            	  attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured Stencil Homo--------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderStencil_Homo;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_homo_matrix, "u_homo_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured+stencil homo",
                              gHomoVertexShaderForScreenMasking,                   // Vertex shader
                              gHomoFragmentShaderForScreenMasking_TexturedStencil, // Fragment shader
                              NULL,                                                // Lighting info
                              NULL,                                                // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Masked Stencil --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderStencil;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_mask_matrix,      "u_mask_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_maskSampler,      "u_maskSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_color,            "u_color"},
			{&sp->u_maskxfm,          "u_maskxfm"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "masked+stencil",
                                  gStdVertexShader_Masked,      // Vertex shader
                                  gFragmentShader_MaskedStencil,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Masked Stencil --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderStencil;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
            {&sp->a_maskCoord, "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type masked+stencil",
                              gStdVertexShaderForScreenMasking_Masked,       // Vertex shader
                              gFragmentShaderForScreenMasking_MaskedStencil, // Fragment shader
                              NULL,                                          // Lighting info
                              NULL,                                          // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Masked Stencil Homo--------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderStencil_Homo;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
        	{&sp->u_homo_matrix,      "u_homo_matrix"},
        	{&sp->u_mask_homo_matrix, "u_mask_homo_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_mask_matrix,      "u_mask_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_maskSampler,      "u_maskSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_color,            "u_color"},
			{&sp->u_maskxfm,          "u_maskxfm"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
        };
        registerShaderProgram(&sp->base, "masked+stencil homo",
                                  gHomoVertexShader,      // Vertex shader
                                  gHomoFragmentShader_MaskedStencil,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            	  attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Masked Stencil Homo--------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderStencil_Homo;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_homo_matrix, "u_homo_matrix"},
            {&sp->u_mask_homo_matrix, "u_mask_homo_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
        };
        registerShaderProgram(&sp->base, "screen masking type masked+stencil homo",
                              gHomoVertexShaderForScreenMasking,                 // Vertex shader
                              gHomoFragmentShaderForScreenMasking_MaskedStencil, // Fragment shader
                              NULL,                                              // Lighting info
                              NULL,                                              // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }
    
#ifndef __APPLE__

    // --- SHADER PROGRAM: Masked+Textured External --------------------------------------------
    {
        
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderExternal;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_mask_matrix,      "u_mask_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_maskSampler,      "u_maskSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_color,            "u_color"},
			{&sp->u_maskxfm,          "u_maskxfm"},
			{&sp->u_colorconv,        "u_colorconv"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
			{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "masked+external",
                                  gStdVertexShader_Masked,      // Vertex shader
                                  gFragmentShader_MaskedExternal,   // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masknig Type Masked+Textured External --------------------------------------------
    {

        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderExternal;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
            {&sp->a_maskCoord, "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type masked+external",
                              gStdVertexShaderForScreenMasking_Masked,        // Vertex shader
                              gFragmentShaderForScreenMasking_MaskedExternal, // Fragment shader
                              NULL,                                           // Lighting info
                              NULL,                                           // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Combined Masked+Textured External --------------------------------------------
    {
        
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderExternal_Combined;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       		"u_mvp_matrix"},
			{&sp->u_tex_matrix,       		"u_tex_matrix"},
			{&sp->u_back_tex_matrix,       	"u_back_tex_matrix"},
			{&sp->u_mask_matrix,      		"u_mask_matrix"},
			{&sp->u_textureSampler,   		"u_textureSampler"},
			{&sp->u_back_textureSampler,   	"u_back_textureSampler"},
			{&sp->u_maskSampler,      		"u_maskSampler"},
			{&sp->u_alpha,            		"u_alpha"},
			{&sp->u_color,            		"u_color"},
			{&sp->u_maskxfm,          		"u_maskxfm"},
			{&sp->u_colorconv,        		"u_colorconv"},
            {&sp->u_hue,                    "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
			{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "combined masked+external",
                                  gStdVertexShader_CombinedMasked,      // Vertex shader
                                  gFragmentShader_CombinedMaskedExternal,   // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked+Textured External --------------------------------------------
    {

        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderExternal_Combined;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
            {&sp->a_maskCoord, "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type combined masked+external",
                              gStdVertexShaderForScreenMasking_CombinedMasked,        // Vertex shader
                              gFragmentShaderForScreenMasking_CombinedMaskedExternal, // Fragment shader
                              NULL,                                                   // Lighting info
                              NULL,                                                   // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured External --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderExternal;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_realY,			"u_realY"},
			{&sp->u_realX,			"u_realX"},
            {&sp->u_hue,            "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured+external",
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_TexturedExternal,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured External --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderExternal;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured+external",
                              gStdVertexShaderForScreenMasking_Textured,        // Vertex shader
                              gFragmentShaderForScreenMasking_TexturedExternal, // Fragment shader
                              &sp->lightingInfo,                                // Lighting info
                              NULL,                                             // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Combined Textured External --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderExternal_Combined;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       		"u_mvp_matrix"},
			{&sp->u_tex_matrix,       		"u_tex_matrix"},
			{&sp->u_back_tex_matrix,       	"u_back_tex_matrix"},
			{&sp->u_textureSampler,   		"u_textureSampler"},
			{&sp->u_back_textureSampler,   	"u_back_textureSampler"},
			{&sp->u_alpha,            		"u_alpha"},
			{&sp->u_colorconv,        		"u_colorconv"},
			{&sp->u_realY,					"u_realY"},
			{&sp->u_realX,					"u_realX"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "combined textured+external",
                                  gStdVertexShader_CombinedTextured,    // Vertex shader
                                  gFragmentShader_CombinedTexturedExternal,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            	  attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured External --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderExternal_Combined;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type combined textured+external",
                              gStdVertexShaderForScreenMasking_CombinedTextured,        // Vertex shader
                              gFragmentShaderForScreenMasking_CombinedTexturedExternal, // Fragment shader
                              &sp->lightingInfo,                                        // Lighting info
                              NULL,                                                     // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }
    // --- SHADER PROGRAM: Textured External 360--------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderExternal360;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_moveX_value,		"u_moveX_value"},
			{&sp->u_moveY_value,		"u_moveY_value"},
            {&sp->u_hue,                "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured+external+360",
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_TexturedExternal360,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured External --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderExternal360;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_moveX_value, "u_moveX_value"},
            {&sp->u_moveY_value, "u_moveY_value"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured+external+360",
                              gStdVertexShaderForScreenMasking_Textured,           // Vertex shader
                              gFragmentShaderForScreenMasking_TexturedExternal360, // Fragment shader
                              &sp->lightingInfo,                                   // Lighting info
                              NULL,                                                // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured External --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderExternal_vignette;
        NXT_ShaderField uniforms[] = {  
			{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_textureSamplerY,  "u_textureSampler_vignette"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_realY,			"u_realY"},
			{&sp->u_realX,			"u_realX"},
            {&sp->u_hue,            "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured+external",
                                  gStdVertexShader_Textured_vignette,    // Vertex shader
                                  gFragmentShader_TexturedExternal_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured External --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderExternal_vignette;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured+external",
                              gStdVertexShaderForScreenMasking_Textured_vignette,        // Vertex shader
                              gFragmentShaderForScreenMasking_TexturedExternal_vignette, // Fragment shader
                              &sp->lightingInfo,                                         // Lighting info
                              NULL,                                                      // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Combined Textured External --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderExternal_Combined_vignette;
        NXT_ShaderField uniforms[] = {  
			{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_tex_matrix,       "u_back_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_textureSampler,   "u_back_textureSampler"},
			{&sp->u_textureSamplerY,  "u_textureSampler_vignette"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_realY,			"u_realY"},
			{&sp->u_realX,			"u_realX"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "textured+external",
                                  gStdVertexShader_CombinedTextured_vignette,    // Vertex shader
                                  gFragmentShader_CombinedTexturedExternal_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }
    
    // --- SHADER PROGRAM: Screen Masking Type Combined Textured External --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderExternal_Combined_vignette;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_textureSampler, "u_back_textureSampler"},
            {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type textured+external",
                              gStdVertexShaderForScreenMasking_CombinedTextured_vignette,        // Vertex shader
                              gFragmentShaderForScreenMasking_CombinedTexturedExternal_vignette, // Fragment shader
                              &sp->lightingInfo,                                                 // Lighting info
                              NULL,                                                              // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }
    
    // --- SHADER PROGRAM: Textured External->YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        char programName[] = "Textured+external->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
            
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderEXTtoYUVA8888[i];
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_realY,			"u_realY"},
			{&sp->u_realX,			"u_realX"},
			{&sp->u_colorconv,        "u_colorconv"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, programName,
                                      gStdVertexShader_Textured,                    // Vertex shader
                                      gFragmentShader_TexturedExternaltoYUVA8888,       // Fragment shader
	                                  &sp->lightingInfo,			// Lighting info
                                      g_channelFilterMacro[i],                        // Macro definitions
                                      uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                      );
            
    }
        
#endif //NOT APPLE

    // --- SHADER PROGRAM: Masked --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderRGB;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_mask_matrix,      "u_mask_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_maskSampler,      "u_maskSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_color,            "u_color"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_maskxfm,          "u_maskxfm"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "masked",
                                  gStdVertexShader_Masked,      // Vertex shader
                                  gFragmentShader_Masked,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Masked --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderRGB;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
            {&sp->a_maskCoord, "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type masked",
                              gStdVertexShaderForScreenMasking_Masked, // Vertex shader
                              gFragmentShaderForScreenMasking_Masked,  // Fragment shader
                              NULL,                                    // Lighting info
                              NULL,                                    // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Combined Masked --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderRGB_Combined;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       		"u_mvp_matrix"},
			{&sp->u_tex_matrix,       		"u_tex_matrix"},
			{&sp->u_back_tex_matrix,       	"u_back_tex_matrix"},
			{&sp->u_mask_matrix,      		"u_mask_matrix"},
			{&sp->u_textureSampler,   		"u_textureSampler"},
			{&sp->u_back_textureSampler,   	"u_back_textureSampler"},
			{&sp->u_maskSampler,      		"u_maskSampler"},
			{&sp->u_alpha,            		"u_alpha"},
			{&sp->u_color,            		"u_color"},
			{&sp->u_colorconv,        		"u_colorconv"},
			{&sp->u_maskxfm,          		"u_maskxfm"},
            {&sp->u_hue,                    "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "combined masked",
                                  gStdVertexShader_CombinedMasked,      // Vertex shader
                                  gFragmentShader_CombinedMasked,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderRGB_Combined;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
            {&sp->a_maskCoord, "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "screeen masking type combined masked",
                              gStdVertexShaderForScreenMasking_CombinedMasked, // Vertex shader
                              gFragmentShaderForScreenMasking_CombinedMasked,  // Fragment shader
                              NULL,                                            // Lighting info
                              NULL,                                            // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Masked Homo--------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderRGB_Homo;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
        	{&sp->u_homo_matrix,      "u_homo_matrix"},
        	{&sp->u_mask_homo_matrix, "u_mask_homo_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_mask_matrix,      "u_mask_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_maskSampler,      "u_maskSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_color,            "u_color"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_maskxfm,          "u_maskxfm"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
        };
        registerShaderProgram(&sp->base, "masked homo",
                                  gHomoVertexShader,      // Vertex shader
                                  gHomoFragmentShader_Masked,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                                  attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Masked Homo--------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderRGB_Homo;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_homo_matrix, "u_homo_matrix"},
            {&sp->u_mask_homo_matrix, "u_mask_homo_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
        };
        registerShaderProgram(&sp->base, "screen masking type masked homo",
                              gHomoVertexShaderForScreenMasking,          // Vertex shader
                              gHomoFragmentShaderForScreenMasking_Masked, // Fragment shader
                              NULL,                                       // Lighting info
                              NULL,                                       // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Combined Masked Homo--------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderRGB_Combined_Homo;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       		"u_mvp_matrix"},
        	{&sp->u_homo_matrix,      		"u_homo_matrix"},
        	{&sp->u_mask_homo_matrix, 		"u_mask_homo_matrix"},
			{&sp->u_tex_matrix,       		"u_tex_matrix"},
			{&sp->u_back_tex_matrix,       	"u_back_tex_matrix"},
			{&sp->u_mask_matrix,      		"u_mask_matrix"},
			{&sp->u_textureSampler,   		"u_textureSampler"},
			{&sp->u_back_textureSampler,   	"u_back_textureSampler"},
			{&sp->u_maskSampler,      		"u_maskSampler"},
			{&sp->u_alpha,            		"u_alpha"},
			{&sp->u_color,            		"u_color"},
			{&sp->u_colorconv,        		"u_colorconv"},
			{&sp->u_maskxfm,          		"u_maskxfm"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
        };
        registerShaderProgram(&sp->base, "combined masked homo",
                                  gHomoVertexShader,      // Vertex shader
                                  gHomoFragmentShader_CombinedMasked,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                                  attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked Homo--------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderRGB_Combined_Homo;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_homo_matrix, "u_homo_matrix"},
            {&sp->u_mask_homo_matrix, "u_mask_homo_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSampler, "u_textureSampler"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
        };
        registerShaderProgram(&sp->base, "screen masking type combined masked homo",
                              gHomoVertexShaderForScreenMasking,                  // Vertex shader
                              gHomoFragmentShaderForScreenMasking_CombinedMasked, // Fragment shader
                              NULL,                                               // Lighting info
                              NULL,                                               // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured+YUV --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderYUV;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSamplerY,  "u_textureSamplerY"},
			{&sp->u_textureSamplerU,  "u_textureSamplerU"},
			{&sp->u_textureSamplerV,  "u_textureSamplerV"},
			{&sp->u_alpha,            "u_alpha"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "Textured+YUV",
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_TexturedYUV,  // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
        
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+YUV --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderYUV;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSamplerY, "u_textureSamplerY"},
            {&sp->u_textureSamplerU, "u_textureSamplerU"},
            {&sp->u_textureSamplerV, "u_textureSamplerV"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "Screen Masking Type Textured+YUV",
                              gStdVertexShaderForScreenMasking_Textured,   // Vertex shader
                              gFragmentShaderForScreenMasking_TexturedYUV, // Fragment shader
                              &sp->lightingInfo,                           // Lighting info
                              NULL,                                        // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured+YUV --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderYUV_vignette;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSamplerY,  "u_textureSamplerY"},
			{&sp->u_textureSamplerU,  "u_textureSamplerU"},
			{&sp->u_textureSamplerV,  "u_textureSamplerV"},
			{&sp->u_textureSamplerUV,  "u_textureSampler_vignette"},
			{&sp->u_alpha,            "u_alpha"},
            {&sp->u_hue,                "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "Textured+YUV",
                                  gStdVertexShader_Textured_vignette,    // Vertex shader
                                  gFragmentShader_TexturedYUV_vignette,  // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
        
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+YUV --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderYUV_vignette;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSamplerY, "u_textureSamplerY"},
            {&sp->u_textureSamplerU, "u_textureSamplerU"},
            {&sp->u_textureSamplerV, "u_textureSamplerV"},
            {&sp->u_textureSamplerUV, "u_textureSampler_vignette"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type Textured+YUV",
                              gStdVertexShaderForScreenMasking_Textured_vignette, // Vertex shader
                              gFragmentShaderForScreenMasking_TexturedYUV_vignette,  // Fragment shader
                              &sp->lightingInfo,                                  // Lighting info
                              NULL,                                               // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured+Y2UVA --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderY2UVA;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_t ex_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_alpha,            "u_alpha"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "Textured+passthrough+Y2UVA",
                            gStdVertexShaderPassThrough_Textured,    // Vertex shader
                            gFragmentShader_TexturedY2UVA,        // Fragment shader
                            &sp->lightingInfo,			// Lighting info
                            NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );

    }
    // --- SHADER PROGRAM: Masked+YUV --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderYUV;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_mask_matrix,      "u_mask_matrix"},
			{&sp->u_textureSamplerY,  "u_textureSamplerY"},
			{&sp->u_textureSamplerU,  "u_textureSamplerU"},
			{&sp->u_textureSamplerV,  "u_textureSamplerV"},
			{&sp->u_maskSampler,      "u_maskSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_color,            "u_color"},
			{&sp->u_maskxfm,          "u_maskxfm"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "Masked+YUV",
                                  gStdVertexShader_Masked,      // Vertex shader
                                  gFragmentShader_MaskedYUV,    // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Masked+YUV --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderYUV;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSamplerY, "u_textureSamplerY"},
            {&sp->u_textureSamplerU, "u_textureSamplerU"},
            {&sp->u_textureSamplerV, "u_textureSamplerV"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
            {&sp->a_maskCoord, "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type Masked+YUV",
                              gStdVertexShaderForScreenMasking_Masked,   // Vertex shader
                              gFragmentShaderForScreenMasking_MaskedYUV, // Fragment shader
                              NULL,                                      // Lighting info
                              NULL,                                      // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured+NV12 --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderNV12;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSamplerY,  "u_textureSamplerY"},
			{&sp->u_textureSamplerUV, "u_textureSamplerUV"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_realY,			"u_realY"}, 
			{&sp->u_realX,			"u_realX"},
            {&sp->u_hue,            "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "Textured+NV12",
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_TexturedNV12, // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+NV12 --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderNV12;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSamplerY, "u_textureSamplerY"},
            {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "screen masking type Textured+NV12",
                              gStdVertexShaderForScreenMasking_Textured,    // Vertex shader
                              gFragmentShaderForScreenMasking_TexturedNV12, // Fragment shader
                              &sp->lightingInfo,                            // Lighting info
                              NULL,                                         // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Combined Textured+NV12 --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderNV12_Combined;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       		"u_mvp_matrix"},
			{&sp->u_tex_matrix,       		"u_tex_matrix"},
			{&sp->u_back_tex_matrix,  		"u_back_tex_matrix"},
			{&sp->u_textureSamplerY,  		"u_textureSamplerY"},
			{&sp->u_textureSamplerUV, 		"u_textureSamplerUV"},
			{&sp->u_back_textureSampler, 	"u_back_textureSampler"},
			{&sp->u_alpha,            		"u_alpha"},
			{&sp->u_colorconv,        		"u_colorconv"},
			{&sp->u_realY,					"u_realY"}, 
			{&sp->u_realX,					"u_realX"},
            {&sp->u_hue,                    "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "Combined Textured+NV12",
                                  gStdVertexShader_CombinedTextured,    // Vertex shader
                                  gFragmentShader_CombinedTexturedNV12, // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured+NV12 --------------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderNV12_Combined;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_textureSamplerY, "u_textureSamplerY"},
            {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type Combined Textured+NV12",
                              gStdVertexShaderForScreenMasking_CombinedTextured,    // Vertex shader
                              gFragmentShaderForScreenMasking_CombinedTexturedNV12, // Fragment shader
                              &sp->lightingInfo,                                    // Lighting info
                              NULL,                                                 // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured+NV12+VIGNETTE --------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderNV12_vignette;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSamplerY,  "u_textureSamplerY"},
			{&sp->u_textureSamplerUV, "u_textureSamplerUV"},
			{&sp->u_textureSamplerU, 	"u_textureSampler_vignette"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_realY,			"u_realY"}, 
			{&sp->u_realX,			"u_realX"},
            {&sp->u_hue,            "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "Textured+NV12+VIGNETTE",
                                  gStdVertexShader_Textured_vignette,    // Vertex shader
                                  gFragmentShader_TexturedNV12_vignette, // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+NV12+VIGNETTE --------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderNV12_vignette;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_textureSamplerY, "u_textureSamplerY"},
            {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
            {&sp->u_textureSamplerU, "u_textureSampler_vignette"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type Textured+NV12+VIGNETTE",
                              gStdVertexShaderForScreenMasking_Textured_vignette,    // Vertex shader
                              gFragmentShaderForScreenMasking_TexturedNV12_vignette, // Fragment shader
                              &sp->lightingInfo,                                     // Lighting info
                              NULL,                                                  // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Combined Textured+NV12+VIGNETTE --------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderNV12_Combined_vignette;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       		"u_mvp_matrix"},
			{&sp->u_tex_matrix,       		"u_tex_matrix"},
			{&sp->u_tex_matrix,       		"u_back_tex_matrix"},
			{&sp->u_textureSamplerY,  		"u_textureSamplerY"},
			{&sp->u_textureSamplerUV, 		"u_textureSamplerUV"},
			{&sp->u_textureSamplerU, 		"u_textureSampler_vignette"},
			{&sp->u_back_textureSampler, 	"u_back_textureSampler"},
			{&sp->u_alpha,            		"u_alpha"},
			{&sp->u_colorconv,        		"u_colorconv"},
			{&sp->u_realY,					"u_realY"}, 
			{&sp->u_realX,					"u_realX"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, "Combined Textured+NV12+VIGNETTE",
                                  gStdVertexShader_CombinedTextured_vignette,    // Vertex shader
                                  gFragmentShader_CombinedTexturedNV12_vignette, // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured+NV12+VIGNETTE --------------------------------------
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderNV12_Combined_vignette;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_textureSamplerY, "u_textureSamplerY"},
            {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
            {&sp->u_textureSamplerU, "u_textureSampler_vignette"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_realY, "u_realY"},
            {&sp->u_realX, "u_realX"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type Combined Textured+NV12+VIGNETTE",
                              gStdVertexShaderForScreenMasking_CombinedTextured_vignette,    // Vertex shader
                              gFragmentShaderForScreenMasking_CombinedTexturedNV12_vignette, // Fragment shader
                              &sp->lightingInfo,                                             // Lighting info
                              NULL,                                                          // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Masked+NV12 --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderNV12;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_mask_matrix,      "u_mask_matrix"},
			{&sp->u_textureSamplerY,  "u_textureSamplerY"},
			{&sp->u_textureSamplerUV, "u_textureSamplerUV"},
			{&sp->u_maskSampler,      "u_maskSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_color,            "u_color"},
			{&sp->u_maskxfm,          "u_maskxfm"},
			{&sp->u_colorconv,        "u_colorconv"},
            {&sp->u_hue,              "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "Masked+NV12",
                                  gStdVertexShader_Masked,      // Vertex shader
                                  gFragmentShader_MaskedNV12,   // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
        
    }

    // --- SHADER PROGRAM: Screen Masking Type Masked+NV12 --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderNV12;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSamplerY, "u_textureSamplerY"},
            {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
            {&sp->a_maskCoord, "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type Masked+NV12",
                              gStdVertexShaderForScreenMasking_Masked,    // Vertex shader
                              gFragmentShaderForScreenMasking_MaskedNV12, // Fragment shader
                              NULL,                                       // Lighting info
                              NULL,                                       // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Combined Masked+NV12 --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderNV12_Combined;
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       		"u_mvp_matrix"},
			{&sp->u_tex_matrix,       		"u_tex_matrix"},
			{&sp->u_back_tex_matrix,       	"u_back_tex_matrix"},
			{&sp->u_mask_matrix,      		"u_mask_matrix"},
			{&sp->u_textureSamplerY,  		"u_textureSamplerY"},
			{&sp->u_textureSamplerUV, 		"u_textureSamplerUV"},
			{&sp->u_maskSampler,      		"u_maskSampler"},
			{&sp->u_back_textureSampler,    "u_back_textureSampler"},
			{&sp->u_alpha,            		"u_alpha"},
			{&sp->u_color,            		"u_color"},
			{&sp->u_maskxfm,          		"u_maskxfm"},
			{&sp->u_colorconv,        		"u_colorconv"},
            {&sp->u_hue,                    "u_hue"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "Combined Masked+NV12",
                                  gStdVertexShader_CombinedMasked,      // Vertex shader
                                  gFragmentShader_CombinedMaskedNV12,   // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
        
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked+NV12 --------------------------------------------
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderNV12_Combined;
        NXT_ShaderField uniforms[] = {
            {&sp->u_mvp_matrix, "u_mvp_matrix"},
            {&sp->u_tex_matrix, "u_tex_matrix"},
            {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
            {&sp->u_mask_matrix, "u_mask_matrix"},
            {&sp->u_textureSamplerY, "u_textureSamplerY"},
            {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
            {&sp->u_maskSampler, "u_maskSampler"},
            {&sp->u_back_textureSampler, "u_back_textureSampler"},
            {&sp->u_alpha, "u_alpha"},
            {&sp->u_color, "u_color"},
            {&sp->u_maskxfm, "u_maskxfm"},
            {&sp->u_colorconv, "u_colorconv"},
            {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
            {&sp->u_inverse_Mask, "u_inverse_Mask"},
            {&sp->u_hue, "u_hue"},
        };
        NXT_ShaderField attributes[] = {
            {&sp->a_position, "a_position"},
            {&sp->a_texCoord, "a_texCoord"},
            {&sp->a_maskCoord, "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, "screen masking type Combined Masked+NV12",
                              gStdVertexShaderForScreenMasking_CombinedMasked,    // Vertex shader
                              gFragmentShaderForScreenMasking_CombinedMaskedNV12, // Fragment shader
                              NULL,                                               // Lighting info
                              NULL,                                               // Macro definitions
                              uniforms, sizeof(uniforms) / sizeof(uniforms[0]),
                              attributes, sizeof(attributes) / sizeof(attributes[0]));
    }

    // --- SHADER PROGRAM: Textured+NV12 -> YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        char programName[] = "Textured+NV12->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderNV12toYUVA8888[i];
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSamplerY,  "u_textureSamplerY"},
			{&sp->u_textureSamplerUV, "u_textureSamplerUV"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
			{&sp->u_realY,			"u_realY"}, 
			{&sp->u_realX,			"u_realX"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, programName,
                                  gStdVertexShader_Textured,                    // Vertex shader
                                  gFragmentShader_TexturedNV12toYUVA8888,       // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  g_channelFilterMacro[i],                        // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
        
    }
    // --- SHADER PROGRAM: Masked+NV12 -> YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        char programName[] = "Masked+NV12->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderNV12toYUVA8888[i];
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_mask_matrix,      "u_mask_matrix"},
			{&sp->u_textureSamplerY,  "u_textureSamplerY"},
			{&sp->u_textureSamplerUV, "u_textureSamplerUV"},
			{&sp->u_maskSampler,      "u_maskSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_color,            "u_color"},
			{&sp->u_maskxfm,          "u_maskxfm"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, programName,
                                  gStdVertexShader_Masked,                  // Vertex shader
                                  gFragmentShader_MaskedNV12toYUVA8888,     // Fragment shader
                                  NULL,										// Lighting info
                                  g_channelFilterMacro[i],                    // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
        
    }

    // --- SHADER PROGRAM: Textured+RGBA -> YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        
        char programName[] = "Textured+RGBA->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGBAtoYUVA8888[i];
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_textureSamplerY,  "u_textureSamplerY"},
			{&sp->u_textureSamplerUV, "u_textureSamplerUV"},
			{&sp->u_alpha,            "u_alpha"},
        };
        NXT_ShaderField attributes[]= { 
        	{ &sp->a_position, "a_position" },
            { &sp->a_texCoord, "a_texCoord" },
        };
        registerShaderProgram(&sp->base, programName,
                                  gStdVertexShader_Textured,                    // Vertex shader
                                  gFragmentShader_TexturedRGBAtoYUVA8888,       // Fragment shader
                                  &sp->lightingInfo,							// Lighting info
                                  g_channelFilterMacro[i],                        // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }

#ifndef __APPLE__
    // --- SHADER PROGRAM: Mask+Textured+External -> YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        char programName[] = "Mask+Textured+External->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderEXTtoYUVA8888[i];
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_mask_matrix,      "u_mask_matrix"},
			{&sp->u_textureSampler,   "u_textureSampler"},
			{&sp->u_maskSampler,      "u_maskSampler"},
			{&sp->u_color,            "u_color"},
			{&sp->u_maskxfm,          "u_maskxfm"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_colorconv,        "u_colorconv"},
        };
        NXT_ShaderField attributes[]= { 
			{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, programName,
                                  gStdVertexShader_Masked,                    // Vertex shader
                                  gFragmentShader_MaskedExternaltoYUVA8888,       // Fragment shader
                                  NULL,											// Lighting info
                                  g_channelFilterMacro[i],                        // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
    }
#endif//not __APPLE__

    // --- SHADER PROGRAM: Masked+RGBA -> YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        char programName[] = "Masked+RGBA->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderRGBAtoYUVA8888[i];
        NXT_ShaderField uniforms[] = {  
        	{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_tex_matrix,       "u_tex_matrix"},
			{&sp->u_mask_matrix,      "u_mask_matrix"},
			{&sp->u_textureSampler,	"u_textureSampler"},
			{&sp->u_maskSampler,      "u_maskSampler"},
			{&sp->u_alpha,            "u_alpha"},
			{&sp->u_color,            "u_color"},
			{&sp->u_maskxfm,          "u_maskxfm"},
        };
        NXT_ShaderField attributes[]= { 
        	{&sp->a_position,         "a_position"},
			{&sp->a_texCoord,         "a_texCoord"},
			{&sp->a_maskCoord,        "a_maskCoord"},
        };
        registerShaderProgram(&sp->base, programName,
                                  gStdVertexShader_Masked,                  // Vertex shader
                                  gFragmentShader_MaskedRGBAtoYUVA8888,     // Fragment shader
                                  NULL,										// Lighting info
                                  g_channelFilterMacro[i],                    // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            	  attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
        
    }
    // --- SHADER PROGRAM: Solid Color NV12 -> YUVA8888 ------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        char programName[] = "SolidNV12toYUVA8888[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        NXT_ShaderProgram_Solid *sp = &pctx->solidShaderNV12toYUVA8888[i];
        NXT_ShaderField uniforms[] = {  
			{&sp->u_mvp_matrix,       "u_mvp_matrix"},
			{&sp->u_alpha,            "u_alpha"},
        };
        NXT_ShaderField attributes[]= { 
			{&sp->a_position,         "a_position"},
			{&sp->a_color,            "a_color"},
        };
        registerShaderProgram(&sp->base, programName,
                                  gStdVertexShader_Solid,                   // Vertex shader
                                  gFragmentShader_SolidNV12toYUVA8888,      // Fragment shader
                                  &sp->lightingInfo,						// Lighting info
                                  g_channelFilterMacro[i],                    // Macro definitions
                                  uniforms, sizeof(uniforms)/sizeof(uniforms[0]),
                            	  attributes, sizeof(attributes)/sizeof(attributes[0])
                                  );
        
    }
    // -------------------------------------------------------------------------

    return NXT_Error_None;
}

#else

#define SHADER_SERIES(a) &a.shaderProgram, &a.shaderVertex, &a.shaderFragment

NXT_Error NexThemeRenderer_UnloadShaders(NXT_ThemeRenderer_Context* pctx)
{
    NXT_Error err = NXT_Error_None;
    int i;
    
    // --- SHADER PROGRAM: Solid Color
    deleteShaderProgram( SHADER_SERIES(pctx->solidShaderRGB) );
    
    // --- SHADER PROGRAM: Screen Masking Type Solid Color
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_solidShaderRGB) );

    // --- SHADER PROGRAM: Textured Pass Through ...
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderPassthroughRGB) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderPassthroughRGBWithColorconv) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderLUT) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderPassthroughYUV) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderPassthroughNV12) );
    deleteShaderProgram(SHADER_SERIES(pctx->texturedShaderUserVignette));
    deleteShaderProgram(SHADER_SERIES(pctx->texturedShaderSharpness));
#ifndef __APPLE__
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderPassthroughExternal) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderPassthroughExternalHDR10) );
#endif
    
    // --- SHADER PROGRAM: Textured+RGBA -> YUVA8888 Pass Through
    for (i = 0; i<NXT_ChannelFilter_MAX; i++) {
        deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderPassThroughRGBAtoYUVA8888[i]) );
    }
    
    // --- SHADER PROGRAM: Textured
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB) );

    // --- SHADER PROGRAM: Textured
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB));

    // --- SHADER PROGRAM: Textured Homo
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_Homo) );

    // --- SHADER PROGRAM: Screen Masking Type Textured Homo
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB_Homo));

    // --- SHADER PROGRAM: Combined Textured Homo
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_Combined_Homo) );

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured Homo
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB_Combined_Homo));

    // --- SHADER PROGRAM: Textured+VIGNETTE --------------------------------------------
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_vignette) );

    // --- SHADER PROGRAM: Screen Masking Type Textured+VIGNETTE --------------------------------------------
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB_vignette));

    // --- SHADER PROGRAM: Textured+VIGNETTE Homo--------------------------------------------
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_vignette_Homo) );

    // --- SHADER PROGRAM: Textured+VIGNETTE Homo--------------------------------------------
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB_vignette_Homo));

    // --- SHADER PROGRAM: Combined Textured+VIGNETTE Homo--------------------------------------------
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_Combined_vignette_Homo) );

    // --- SHADER PROGRAM: ScreenMaskingType Combined+Textured+VIGNETTE Homo--------------------------------------------
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB_Combined_vignette_Homo) );

    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_For_Videos) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_Combined_For_Videos) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_For_Videos360) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_For_Videos_vignette) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_Combined_For_Videos_vignette) );

    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB_For_Videos));
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB_Combined_For_Videos));
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB_For_Videos360));
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB_For_Videos_vignette));
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderRGB_Combined_For_Videos_vignette));

    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGB_for_layers) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderStencil) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderStencil_Homo) );
    
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderStencil));
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderStencil_Homo));

    // --- SHADER PROGRAM: Masked Stencil
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderStencil) );

    // --- SHADER PROGRAM: Screen Masking Type Masked Stencil
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderStencil));

    // --- SHADER PROGRAM: Masked Stencil Homo
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderStencil_Homo) );

    // --- SHADER PROGRAM: ScreenMaskingType Masked Stencil Homo
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderStencil_Homo));

#ifndef __APPLE__
    // --- SHADER PROGRAM: Masked+Textured External
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderExternal) );

    // --- SHADER PROGRAM: Screen Masking Type Masked+Textured External
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderExternal));

    // --- SHADER PROGRAM: Combined Masked+Textured External
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderExternal_Combined) );

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked+Textured External
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderExternal_Combined));

    // --- SHADER PROGRAM: Textured External
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderExternal) );

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured External
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderExternal));

    // --- SHADER PROGRAM: Combined Textured External
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderExternal_Combined) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderExternal360);
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderExternal_vignette));
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderExternal_Combined_vignette) );

    // --- SHADER PROGRAM: Screen Masking Type CombinedCombined Textured External
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderExternal_Combined));
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderExternal360);
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderExternal_vignette));
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderExternal_Combined_vignette));

    // --- SHADER PROGRAM: Textured External->YUVA8888
    for( i=0; i<NXT_ChannelFilter_MAX; i++ ) {
        deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderEXTtoYUVA8888[i]) );
    }
#endif

    // --- SHADER PROGRAM: Masked
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderRGB) );

    // --- SHADER PROGRAM: Screen Masking Type Masked
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderRGB));

    // --- SHADER PROGRAM: Combined Masked
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderRGB_Combined) );

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderRGB_Combined));

    // --- SHADER PROGRAM: Masked Homo
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderRGB_Homo) );

    // --- SHADER PROGRAM: Screen Masking Type Masked Homo
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderRGB_Homo) );

    // --- SHADER PROGRAM: Combined Masked
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderRGB_Combined_Homo) );

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderRGB_Combined_Homo));

    // --- SHADER PROGRAM: Textured+YUV
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderYUV) );
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderYUV_vignette) );

    // --- SHADER PROGRAM: Textured+YUV
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderYUV));
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderYUV_vignette));

    // --- SHADER PROGRAM: Textured+Y2UVA
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderY2UVA) );
    
    // --- SHADER PROGRAM: Masked+YUV
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderYUV) );
    
    // --- SHADER PROGRAM: Screen Masking Type Masked+YUV
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderYUV) );

    // --- SHADER PROGRAM: Textured+NV12
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderNV12) );
    // --- SHADER PROGRAM: Combined Textured+NV12
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderNV12_Combined) );
    // --- SHADER PROGRAM: Textured+NV12+Vignette
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderNV12_vignette) );
    // --- SHADER PROGRAM: Combined Textured+NV12+Vignette
    deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderNV12_Combined_vignette) );

    // --- SHADER PROGRAM: Screen Masking Type Textured+NV12
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderNV12) );
    // --- SHADER PROGRAM: Screen Masking Type Textured+NV12+Vignette
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderNV12_vignette) );
    // --- SHADER PROGRAM: Screen Masking Type Combined Textured+NV12
    deleteShaderProgram (SHADER_SERIES(pctx->screenMaskingType_texturedShaderNV12_Combined));
    // --- SHADER PROGRAM: Screen Masking Type Combined Textured+NV12+Vignette
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_texturedShaderNV12_Combined_vignette));

    // --- SHADER PROGRAM: Masked+NV12
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderNV12) );

    // --- SHADER PROGRAM: Masked+NV12
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderNV12) );

    // --- SHADER PROGRAM: Combined Masked+NV12
    deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderNV12_Combined) );

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked+NV12
    deleteShaderProgram( SHADER_SERIES(pctx->screenMaskingType_maskedShaderNV12_Combined));

    // --- SHADER PROGRAM: Textured+NV12 -> YUVA8888
    for( i = 0 ; i < NXT_ChannelFilter_MAX ; i++ ) {
        deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderNV12toYUVA8888[i]) );
    }
    
    // --- SHADER PROGRAM: Masked+NV12 -> YUVA8888
    for( i=0; i < NXT_ChannelFilter_MAX ; i++ ) {
        deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderNV12toYUVA8888[i]) );
    }
    
    // --- SHADER PROGRAM: Textured+RGBA -> YUVA8888
    for( i= 0 ; i < NXT_ChannelFilter_MAX ; i++ ) {
        deleteShaderProgram( SHADER_SERIES(pctx->texturedShaderRGBAtoYUVA8888[i]) );
    }
    
#ifndef __APPLE__
    // --- SHADER PROGRAM: Mask+Textured+External -> YUVA8888 --------------------------------------------
    for( i = 0 ; i < NXT_ChannelFilter_MAX ; i++ ) {
        deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderEXTtoYUVA8888[i]) );
    }
#endif
    
    // --- SHADER PROGRAM: Masked+RGBA -> YUVA8888 --------------------------------------------
    for( i = 0 ; i < NXT_ChannelFilter_MAX ; i++ ) {
        deleteShaderProgram( SHADER_SERIES(pctx->maskedShaderRGBAtoYUVA8888[i]) );
    }

    // --- SHADER PROGRAM: Solid Color NV12 -> YUVA8888 ------------------------
    for( i = 0 ; i < NXT_ChannelFilter_MAX ; i++ ) {
        deleteShaderProgram( SHADER_SERIES(pctx->solidShaderNV12toYUVA8888[i]) );
    }

    return err;
}

NXT_Error NexThemeRenderer_LoadShaders(NXT_ThemeRenderer_Context* pctx)
{
    int i;
    NXT_Error err = NXT_Error_None;
    
    const char *channelFilterMacro[NXT_ChannelFilter_MAX] = {0};
    channelFilterMacro[NXT_ChannelFilter_ALL] = "#define CHANNEL_FILTER(xx) (xx)\n";
    channelFilterMacro[NXT_ChannelFilter_Y] = "#define CHANNEL_FILTER(xx) ((xx).rrra)\n";
    channelFilterMacro[NXT_ChannelFilter_U] = "#define CHANNEL_FILTER(xx) ((xx).ggga)\n";
    channelFilterMacro[NXT_ChannelFilter_V] = "#define CHANNEL_FILTER(xx) ((xx).bbba)\n";
    
    // --- SHADER PROGRAM: Solid Color -----------------------------------------
    if( !err ){
        NXT_ShaderProgram_Solid *sp = &pctx->solidShaderRGB;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "solid",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Solid,       // Vertex shader
                                  gFragmentShader_Solid,        // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      { &sp->u_colorconv, "u_colorconv" },
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_color,            "a_color"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Solid Color -----------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Solid *sp = &pctx->screenMaskingType_solidShaderRGB;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type solid",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Solid, // Vertex shader
                                  gFragmentShaderForScreenMasking_Solid,  // Fragment shader
                                  &sp->lightingInfo,                      // Lighting info
                                  NULL,                                   // Macro definitions
                                  (NXT_ShaderField[]){                    // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_color, "a_color"},
                                                      {0}});
    }
    
    // --- SHADER PROGRAM: Textured Pass Through--------------------------------------------
	if (!err){
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughRGB;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+passthrough",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderPassThrough_Textured,    // Vertex shader
                                  gFragmentShaderPassthrough_Textured,     // Fragment shader
                                  NULL,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      { &sp->u_textureSampler, "u_textureSampler" },
                                      { 0 }
                                  },
                                  (NXT_ShaderField[]){          // Attributes
                                      { &sp->a_position, "a_position" },
                                      { &sp->a_texCoord, "a_texCoord" },
                                      { 0 }
                                  }
                                  );
	}
	if (!err){
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughRGBWithColorconv;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+passthrough+colorconv",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderPassThrough_Textured,    // Vertex shader
                                  gFragmentShaderPassthroughWithColorConv_Textured,     // Fragment shader
                                  NULL,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      { &sp->u_textureSampler, "u_textureSampler" },
                                      { &sp->u_colorconv, "u_colorconv" },
                                      { 0 }
                                  },
                                  (NXT_ShaderField[]){          // Attributes
                                      { &sp->a_position, "a_position" },
                                      { &sp->a_texCoord, "a_texCoord" },
                                      { 0 }
                                  }
                                  );
	}
    
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderUserVignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+UserVignette",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderPassThrough_Textured, // Vertex shader
                                  gFragmentShaderUserVignette_Textured, // Fragment shader
                                  NULL,                                 // Lighting info
                                  NULL,                                 // Macro definitions
                                  (NXT_ShaderField[]){                  // Uniforms
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_vignette, "u_vignette"},
                                                      {&sp->u_vignetteRange, "u_vignetteRange"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},

                                                      {0}});
    }
    
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderSharpness;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+sharpness",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderPassThrough_Textured, // Vertex shader
                                  gFragmentShaderSharpness_Textured,    // Fragment shader
                                  NULL,                                 // Lighting info
                                  NULL,                                 // Macro definitions
                                  (NXT_ShaderField[]){                  // Uniforms
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_sharpness, "u_sharpness"},
                                                      {&sp->u_resolution, "u_resolution"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},

                                                      {0}});
    }
    if (!err){
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderLUT;
		err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+passthrough+lut",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderPassThrough_Textured,    // Vertex shader
                                  gFragmentShaderLUT_Textured,     // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      { &sp->u_textureSampler, "u_textureSampler" },
                                      { &sp->u_textureSamplerY, "u_textureLUT" },
                                      { &sp->u_realY, "u_realY" },
                                      { &sp->u_realX, "u_realX" },
                                      { &sp->u_alpha, "u_alpha" },
                                      { 0 }
                                  },
                                  (NXT_ShaderField[]){          // Attributes
                                      { &sp->a_position, "a_position" },
                                      { &sp->a_texCoord, "a_texCoord" },
                                      { 0 }
                                  }
                                  );
	}
	//if (!err){
	//	NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderFilterTest;
	//	err = createShaderProgram(&sp->shaderProgram, "textured+lut",
	//		sp, sizeof(*sp),
	//		gStdVertexShaderPassThrough_Textured,    // Vertex shader
	//		gFragmentShaderForFilter_Textured,     // Fragment shader
	//		NULL,							// Lighting info
	//		NULL,                         // Macro definitions
	//		(NXT_ShaderField[]){          // Uniforms
	//			{ &sp->u_textureSampler, "u_textureSampler" },
	//			{ &sp->u_realX, "u_texture_size" },
	//			{ &sp->u_realY, "u_time" },
	//			{ 0 }
	//		},
	//			(NXT_ShaderField[]){          // Attributes
	//				{ &sp->a_position, "a_position" },
	//				{ &sp->a_texCoord, "a_texCoord" },
	//				{ 0 }
	//		}
	//		);
	//}
	if (!err){
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughYUV;
		err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "Textured+passthrough+YUV",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderPassThrough_Textured,    // Vertex shader
                                  gFragmentShaderPassthrough_TexturedYUV,  // Fragment shader
                                  NULL,									// Lighting info
                                  NULL,			                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      { &sp->u_textureSamplerY, "u_textureSamplerY" },
                                      { &sp->u_textureSamplerU, "u_textureSamplerU" },
                                      { &sp->u_textureSamplerV, "u_textureSamplerV" },
                                      { 0 }
                                  },
                                  (NXT_ShaderField[]){          // Attributes
                                      { &sp->a_position, "a_position" },
                                      { &sp->a_texCoord, "a_texCoord" },
                                      { 0 }
                                  }
                                  );
	}
	if (!err){
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughNV12;
		err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "Textured+passthrough+NV12",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderPassThrough_Textured,    // Vertex shader
                                  gFragmentShaderPassthrough_TexturedNV12, // Fragment shader
                                  NULL,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      { &sp->u_textureSamplerY, "u_textureSamplerY" },
                                      { &sp->u_textureSamplerUV, "u_textureSamplerUV" },
                                      { &sp->u_realY, "u_realY" },
                                      { &sp->u_realX, "u_realX" },
                                      { &sp->u_colorconv, "u_colorconv" },
                                      { 0 }
                                  },
                                  (NXT_ShaderField[]){          // Attributes
                                      { &sp->a_position, "a_position" },
                                      { &sp->a_texCoord, "a_texCoord" },
                                      { 0 }
                                  }
                                  );
	}
#ifndef __APPLE__
	if (!err){
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughExternal;
		err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+passthrough+external",
			sp, sizeof(*sp),
			gStdVertexShaderPassThroughExternal_Textured,    // Vertex shader
			gFragmentShaderPassthrough_TexturedExternal,     // Fragment shader
			NULL,			// Lighting info
			NULL,                         // Macro definitions
			(NXT_ShaderField[]){          // Uniforms
				{ &sp->u_tex_matrix, "u_tex_matrix" },
				{ &sp->u_textureSampler, "u_textureSampler" },
				{ 0 }
			},
				(NXT_ShaderField[]){          // Attributes
					{ &sp->a_position, "a_position" },
					{ &sp->a_texCoord, "a_texCoord" },
					{ 0 }
			}
			);
	}

    if (!err){
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassthroughExternalHDR10;
		err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+passthrough+external+HDR10",
			sp, sizeof(*sp),
			gStdVertexShaderPassThroughExternal_Textured,    // Vertex shader
			gFragmentShaderPassthrough_TexturedExternalHDR10,     // Fragment shader
			NULL,			// Lighting info
			NULL,                         // Macro definitions
			(NXT_ShaderField[]){          // Uniforms
				{ &sp->u_tex_matrix, "u_tex_matrix" },
				{ &sp->u_textureSampler, "u_textureSampler" },
                { &sp->u_MaximumContentLightLevel, "u_MaximumContentLightLevel"},
                { &sp->u_Gamma, "u_Gamma"},
                { &sp->u_DeviceLightLevel, "u_DeviceLightLevel"},
				{ 0 }
			},
				(NXT_ShaderField[]){          // Attributes
					{ &sp->a_position, "a_position" },
					{ &sp->a_texCoord, "a_texCoord" },
					{ 0 }
			}
			);
	}    
#endif // not APPLE
	// --- SHADER PROGRAM: Textured+RGBA -> YUVA8888 Pass Through--------------------------------------------
	for (i = 0; i<NXT_ChannelFilter_MAX; i++)
	{

		char programName[] = "Textured+RGBA->YUV[_] PassThrough";
		programName[sizeof(programName)-3] = '0' + i;

		if (err)
			break;
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderPassThroughRGBAtoYUVA8888[i];
		err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  programName,
                                  sp, sizeof(*sp),
                                  gStdVertexShaderPassThrough_Textured,        // Vertex shader
                                  gFragmentShaderPassthrough_TexturedRGBAtoYUVA8888,       // Fragment shader
                                  NULL,							// Lighting info
                                  channelFilterMacro[i],                        // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      { &sp->u_textureSampler, "u_textureSampler" },
                                      { 0 }
                                  },
                                  (NXT_ShaderField[]){          // Attributes
                                      { &sp->a_position, "a_position" },
                                      { &sp->a_texCoord, "a_texCoord" },
                                      { 0 }
                                  }
                                  );
	}
    // --- SHADER PROGRAM: Textured --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_Textured,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
									  {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type textured",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured, // Vertex shader
                                  gFragmentShaderForScreenMasking_Textured,  // Fragment shader
                                  &sp->lightingInfo,                         // Lighting info
                                  NULL,                                      // Macro definitions
                                  (NXT_ShaderField[]){                       // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }
    // --- SHADER PROGRAM: Textured Homo--------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShader,    // Vertex shader
                                  gHomoFragmentShader_Textured,     // Fragment shader
                                  NULL,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_homo_matrix,      "u_homo_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {&sp->u_realY,			"u_realY"}, 
            						  {&sp->u_realX,			"u_realX"},
									  {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {0}}
                                  );
    }
    // --- SHADER PROGRAM: Screen Masking Type Textured Homo--------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingTypetexturedShaderRGB_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type textured homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShaderForScreenMasking,            // Vertex shader
                                  gHomoFragmentShaderForScreenMasking_Textured, // Fragment shader
                                  NULL,                                         // Lighting info
                                  NULL,                                         // Macro definitions
                                  (NXT_ShaderField[]){                          // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_homo_matrix, "u_homo_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {0}});
    }
    // --- SHADER PROGRAM: Combined Textured Homo--------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_Combined_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "combined textured homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShader,    // Vertex shader
                                  gHomoFragmentShader_Textured,     // Fragment shader
                                  NULL,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      	{&sp->u_mvp_matrix,       		"u_mvp_matrix"},
							        	{&sp->u_homo_matrix,      		"u_homo_matrix"},
										{&sp->u_tex_matrix,       		"u_tex_matrix"},
										{&sp->u_back_tex_matrix,  		"u_back_tex_matrix"},
										{&sp->u_textureSampler,   		"u_textureSampler"},
										{&sp->u_back_textureSampler,   	"u_back_textureSampler"},
										{&sp->u_alpha,            		"u_alpha"},
										{&sp->u_colorconv,        		"u_colorconv"},
										{&sp->u_realY,			  		"u_realY"}, 
							            {&sp->u_realX,			  		"u_realX"},
									  	{0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured Homo--------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_Combined_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type combined textured homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShaderForScreenMasking,            // Vertex shader
                                  gHomoFragmentShaderForScreenMasking_Textured, // Fragment shader
                                  NULL,                                         // Lighting info
                                  NULL,                                         // Macro definitions
                                  (NXT_ShaderField[]){                          // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_homo_matrix, "u_homo_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured+VIGNETTE --------------------------------------------
    if(!err){
    	NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+vignette",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured_vignette,    // Vertex shader
                                  gFragmentShader_Textured_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_textureSamplerY,   "u_textureSampler_vignette"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
									  {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+VIGNETTE --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking tpye textured+vignette",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured_vignette, // Vertex shader
                                  gFragmentShaderForScreenMasking_Textured_vignette,  // Fragment shader
                                  &sp->lightingInfo,                                  // Lighting info
                                  NULL,                                               // Macro definitions
                                  (NXT_ShaderField[]){                                // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured+VIGNETTE Homo--------------------------------------------
    if(!err){
    	NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_vignette_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+vignette homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShader,    // Vertex shader
                                  gHomoFragmentShader_Textured_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_homo_matrix,      "u_homo_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_textureSamplerY,  "u_textureSampler_vignette"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {&sp->u_realY,			"u_realY"}, 
            						  {&sp->u_realX,			"u_realX"},
									  {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+VIGNETTE Homo--------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_vignette_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type textured+vignette homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShaderForScreenMasking,                     // Vertex shader
                                  gHomoFragmentShaderForScreenMasking_Textured_vignette, // Fragment shader
                                  &sp->lightingInfo,                                     // Lighting info
                                  NULL,                                                  // Macro definitions
                                  (NXT_ShaderField[]){                                   // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_homo_matrix, "u_homo_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Combined Textured+VIGNETTE Homo--------------------------------------------
    if(!err){
    	NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_Combined_vignette_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "combined+textured+vignette homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShader,    // Vertex shader
                                  gHomoFragmentShader_CombinedTextured_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       	"u_mvp_matrix"},
                                      {&sp->u_homo_matrix,      	"u_homo_matrix"},
                                      {&sp->u_tex_matrix,       	"u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,  	"u_back_tex_matrix"},
                                      {&sp->u_textureSampler,   	"u_textureSampler"},
                                      {&sp->u_back_textureSampler,  "u_back_textureSampler"},
                                      {&sp->u_textureSamplerY,  	"u_textureSampler_vignette"},
                                      {&sp->u_alpha,            	"u_alpha"},
                                      {&sp->u_colorconv,        	"u_colorconv"},
                                      {&sp->u_realY,				"u_realY"}, 
            						  {&sp->u_realX,				"u_realX"},
									  {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         	"a_position"},
                                      {0}}
                                  );
    }
    // --- SHADER PROGRAM: Combined Textured+VIGNETTE Homo for ScreenMasking--------------------------------------------
    if(!err){
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_Combined_vignette_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type combined+textured+vignette homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShaderForScreenMasking,                             // Vertex shader
                                  gHomoFragmentShaderForScreenMasking_CombinedTextured_vignette, // Fragment shader
                                  &sp->lightingInfo,                                             // Lighting info
                                  NULL,                                                          // Macro definitions
                                  (NXT_ShaderField[]){                                           // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_homo_matrix, "u_homo_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {0}});
    }

    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_For_Videos;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured videos",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_Textured_ForVideo,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {&sp->u_realY,			"u_realY"}, 
                                      {&sp->u_realX,			"u_realX"},
									  {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_For_Videos;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type textured videos",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured,         // Vertex shader
                                  gFragmentShaderForScreenMasking_Textured_ForVideo, // Fragment shader
                                  &sp->lightingInfo,                                 // Lighting info
                                  NULL,                                              // Macro definitions
                                  (NXT_ShaderField[]){                               // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_Combined_For_Videos;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "combindd textured videos",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_CombinedTextured,    // Vertex shader
                                  gFragmentShader_CombinedTextured_ForVideo,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       	"u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       	"u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,      "u_back_tex_matrix"},
                                      {&sp->u_textureSampler,   	"u_textureSampler"},
                                      {&sp->u_back_textureSampler,  "u_back_textureSampler"},
                                      {&sp->u_alpha,            	"u_alpha"},
                                      {&sp->u_colorconv,        	"u_colorconv"},
                                      {&sp->u_realY,				"u_realY"}, 
                                      {&sp->u_realX,				"u_realX"},
									  {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_Combined_For_Videos;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type combindd textured videos",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_CombinedTextured,         // Vertex shader
                                  gFragmentShaderForScreenMasking_CombinedTextured_ForVideo, // Fragment shader
                                  &sp->lightingInfo,                                         // Lighting info
                                  NULL,                                                      // Macro definitions
                                  (NXT_ShaderField[]){                                       // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }
    
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_For_Videos360;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
		                          "textured+360",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_Textured_ForVideo360,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {&sp->u_realY,			"u_realY"}, 
                                      {&sp->u_realX,			"u_realX"},
                                      {&sp->u_moveX_value,		"u_moveX_value"},
                                      {&sp->u_moveY_value,		"u_moveY_value"},
									  {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_For_Videos360;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type textured+360",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured,            // Vertex shader
                                  gFragmentShaderForScreenMasking_Textured_ForVideo360, // Fragment shader
                                  &sp->lightingInfo,                                    // Lighting info
                                  NULL,                                                 // Macro definitions
                                  (NXT_ShaderField[]){                                  // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_moveX_value, "u_moveX_value"},
                                                      {&sp->u_moveY_value, "u_moveY_value"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_For_Videos_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
		                          "textured+vignette",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured_vignette,    // Vertex shader
                                  gFragmentShader_Textured_ForVideo_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_textureSamplerY,  "u_textureSampler_vignette"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {&sp->u_realY,			"u_realY"}, 
                                      {&sp->u_realX,			"u_realX"},
									  {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_For_Videos_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type textured+vignette",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured_vignette,         // Vertex shader
                                  gFragmentShaderForScreenMasking_Textured_ForVideo_vignette, // Fragment shader
                                  &sp->lightingInfo,                                          // Lighting info
                                  NULL,                                                       // Macro definitions
                                  (NXT_ShaderField[]){                                        // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_Combined_For_Videos_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
		                          "combined textured+vignette",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_CombinedTextured_vignette,    // Vertex shader
                                  gFragmentShader_CombinedTextured_ForVideo_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       	"u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       	"u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,      "u_back_tex_matrix"},
                                      {&sp->u_textureSampler,   	"u_textureSampler"},
                                      {&sp->u_back_textureSampler,  "u_back_textureSampler"},
                                      {&sp->u_textureSamplerY,  	"u_textureSampler_vignette"},
                                      {&sp->u_alpha,            	"u_alpha"},
                                      {&sp->u_colorconv,        	"u_colorconv"},
                                      {&sp->u_realY,				"u_realY"}, 
                                      {&sp->u_realX,				"u_realX"},
									  {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderRGB_Combined_For_Videos_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type combined textured+vignette",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_CombinedTextured_vignette,         // Vertex shader
                                  gFragmentShaderForScreenMasking_CombinedTextured_ForVideo_vignette, // Fragment shader
                                  &sp->lightingInfo,                                                  // Lighting info
                                  NULL,                                                               // Macro definitions
                                  (NXT_ShaderField[]){                                                // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    if (!err){
		NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGB_for_layers;
		err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured_for_layers",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured_For_Layers,    // Vertex shader
                                  gFragmentShader_Textured_For_Layers,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      { &sp->u_mvp_matrix, "u_mvp_matrix" },
                                      { &sp->u_tex_matrix, "u_tex_matrix" },
                                      { &sp->u_textureSampler, "u_textureSampler" },
                                      { &sp->u_textureSamplerY, "u_textureSampler_for_mask" },
                                      { &sp->u_alpha, "u_alpha" },
                                      { &sp->u_colorconv, "u_colorconv" },
                                      { 0 }
                                  },
                                  (NXT_ShaderField[]){          // Attributes
                                      { &sp->a_position, "a_position" },
                                      { &sp->a_texCoord, "a_texCoord" },
                                      { 0 }
                                  }
                                  );
	}
    // --- SHADER PROGRAM: Textured Stencil --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderStencil;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+stencil",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_TexturedStencil,// Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured Stencil --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderStencil;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "Screen Masking Type textured+stencil",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured,       // Vertex shader
                                  gFragmentShaderForScreenMasking_TexturedStencil, // Fragment shader
                                  NULL,                                            // Lighting info
                                  NULL,                                            // Macro definitions
                                  (NXT_ShaderField[]){                             // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured Stencil Homo--------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderStencil_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+stencil homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShader,    // Vertex shader
                                  gHomoFragmentShader_TexturedStencil,// Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_homo_matrix,      "u_homo_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured Stencil Homo--------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderStencil_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type textured+stencil homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShaderForScreenMasking,                   // Vertex shader
                                  gHomoFragmentShaderForScreenMasking_TexturedStencil, // Fragment shader
                                  NULL,                                                // Lighting info
                                  NULL,                                                // Macro definitions
                                  (NXT_ShaderField[]){                                 // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_homo_matrix, "u_homo_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Masked Stencil --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderStencil;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "masked+stencil",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Masked,      // Vertex shader
                                  gFragmentShader_MaskedStencil,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_mask_matrix,      "u_mask_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_maskSampler,      "u_maskSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_color,            "u_color"},
                                      {&sp->u_maskxfm,          "u_maskxfm"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {&sp->a_maskCoord,        "a_maskCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Masked Stencil --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderStencil;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type masked+stencil",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Masked,       // Vertex shader
                                  gFragmentShaderForScreenMasking_MaskedStencil, // Fragment shader
                                  NULL,                                          // Lighting info
                                  NULL,                                          // Macro definitions
                                  (NXT_ShaderField[]){                           // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_mask_matrix, "u_mask_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_maskSampler, "u_maskSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_color, "u_color"},
                                                      {&sp->u_maskxfm, "u_maskxfm"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {&sp->a_maskCoord, "a_maskCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Masked Stencil Homo--------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderStencil_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "masked+stencil homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShader,      // Vertex shader
                                  gHomoFragmentShader_MaskedStencil,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_homo_matrix,      "u_homo_matrix"},
                                      {&sp->u_mask_homo_matrix, "u_mask_homo_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_mask_matrix,      "u_mask_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_maskSampler,      "u_maskSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_color,            "u_color"},
                                      {&sp->u_maskxfm,          "u_maskxfm"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Masked Stencil Homo--------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderStencil_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type masked+stencil homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShaderForScreenMasking,                 // Vertex shader
                                  gHomoFragmentShaderForScreenMasking_MaskedStencil, // Fragment shader
                                  NULL,                                              // Lighting info
                                  NULL,                                              // Macro definitions
                                  (NXT_ShaderField[]){                               // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_homo_matrix, "u_homo_matrix"},
                                                      {&sp->u_mask_homo_matrix, "u_mask_homo_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_mask_matrix, "u_mask_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_maskSampler, "u_maskSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_color, "u_color"},
                                                      {&sp->u_maskxfm, "u_maskxfm"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {0}});
    }

#ifndef __APPLE__
    // --- SHADER PROGRAM: Masked+Textured External --------------------------------------------
    if( !err ){
        
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderExternal;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "masked+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Masked,      // Vertex shader
                                  gFragmentShader_MaskedExternal,   // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_mask_matrix,      "u_mask_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_maskSampler,      "u_maskSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_color,            "u_color"},
                                      {&sp->u_maskxfm,          "u_maskxfm"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {&sp->a_maskCoord,        "a_maskCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Masked+Textured External --------------------------------------------
    if( !err ){
        
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderExternal;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type masked+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Masked,        // Vertex shader
                                  gFragmentShaderForScreenMasking_MaskedExternal, // Fragment shader
                                  NULL,                                           // Lighting info
                                  NULL,                                           // Macro definitions
                                  (NXT_ShaderField[]){                            // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_mask_matrix, "u_mask_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_maskSampler, "u_maskSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_color, "u_color"},
                                                      {&sp->u_maskxfm, "u_maskxfm"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {&sp->a_maskCoord, "a_maskCoord"},
                                                      {0}});
    }
    
    // --- SHADER PROGRAM: Combined Masked+Textured External --------------------------------------------
    if( !err ){
        
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderExternal_Combined;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "combined masked+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_CombinedMasked,      // Vertex shader
                                  gFragmentShader_CombinedMaskedExternal,   // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,       "u_back_tex_matrix"},
                                      {&sp->u_mask_matrix,      "u_mask_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_back_textureSampler,   "u_back_textureSampler"},
                                      {&sp->u_maskSampler,      "u_maskSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_color,            "u_color"},
                                      {&sp->u_maskxfm,          "u_maskxfm"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {&sp->a_maskCoord,        "a_maskCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked+Textured External --------------------------------------------
    if (!err)
    {

        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderExternal_Combined;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type combined masked+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_CombinedMasked,        // Vertex shader
                                  gFragmentShaderForScreenMasking_CombinedMaskedExternal, // Fragment shader
                                  NULL,                                                   // Lighting info
                                  NULL,                                                   // Macro definitions
                                  (NXT_ShaderField[]){                                    // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_mask_matrix, "u_mask_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_maskSampler, "u_maskSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_color, "u_color"},
                                                      {&sp->u_maskxfm, "u_maskxfm"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {&sp->a_maskCoord, "a_maskCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured External --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderExternal;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "textured+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_TexturedExternal,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {&sp->u_realY,        	"u_realY"},
                                      {&sp->u_realX,        	"u_realX"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured External --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderExternal;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type textured+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured,        // Vertex shader
                                  gFragmentShaderForScreenMasking_TexturedExternal, // Fragment shader
                                  &sp->lightingInfo,                                // Lighting info
                                  NULL,                                             // Macro definitions
                                  (NXT_ShaderField[]){                              // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Combined Textured External --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderExternal_Combined;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "combined textured+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_CombinedTextured,    // Vertex shader
                                  gFragmentShader_CombinedTexturedExternal,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       	"u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       	"u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,      "u_back_tex_matrix"},
                                      {&sp->u_textureSampler,   	"u_textureSampler"},
                                      {&sp->u_back_textureSampler,  "u_back_textureSampler"},
                                      {&sp->u_alpha,            	"u_alpha"},
                                      {&sp->u_colorconv,        	"u_colorconv"},
                                      {&sp->u_realY,        	"u_realY"},
                                      {&sp->u_realX,        	"u_realX"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured External --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderExternal_Combined;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type combined textured+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_CombinedTextured,        // Vertex shader
                                  gFragmentShaderForScreenMasking_CombinedTexturedExternal, // Fragment shader
                                  &sp->lightingInfo,                                        // Lighting info
                                  NULL,                                                     // Macro definitions
                                  (NXT_ShaderField[]){                                      // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured External 360--------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderExternal360;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
		                          "textured+external+360",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_TexturedExternal360,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {&sp->u_moveX_value,		"u_moveX_value"},
                                      {&sp->u_moveY_value,		"u_moveY_value"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured External 360 --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderExternal360;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type textured+external+360",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured,           // Vertex shader
                                  gFragmentShaderForScreenMasking_TexturedExternal360, // Fragment shader
                                  &sp->lightingInfo,                                   // Lighting info
                                  NULL,                                                // Macro definitions
                                  (NXT_ShaderField[]){                                 // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_moveX_value, "u_moveX_value"},
                                                      {&sp->u_moveY_value, "u_moveY_value"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured External --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderExternal_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
		                          "textured+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured_vignette,    // Vertex shader
                                  gFragmentShader_TexturedExternal_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_textureSamplerY,  "u_textureSampler_vignette"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured External --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderExternal_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type textured+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured_vignette,        // Vertex shader
                                  gFragmentShaderForScreenMasking_TexturedExternal_vignette, // Fragment shader
                                  &sp->lightingInfo,                                         // Lighting info
                                  NULL,                                                      // Macro definitions
                                  (NXT_ShaderField[]){                                       // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Combined Textured External --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderExternal_Combined_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
		                          "combined textured+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_CombinedTextured_vignette,    // Vertex shader
                                  gFragmentShader_CombinedTexturedExternal_vignette,     // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       	"u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       	"u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,      "u_back_tex_matrix"},
                                      {&sp->u_textureSampler,   	"u_textureSampler"},
                                      {&sp->u_back_textureSampler,  "u_back_textureSampler"},
                                      {&sp->u_textureSamplerY,  	"u_textureSampler_vignette"},
                                      {&sp->u_alpha,            	"u_alpha"},
                                      {&sp->u_colorconv,        	"u_colorconv"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen MAsking Type Combined Textured External --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderExternal_Combined_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type combined textured+external",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_CombinedTextured_vignette,        // Vertex shader
                                  gFragmentShaderForScreenMasking_CombinedTexturedExternal_vignette, // Fragment shader
                                  &sp->lightingInfo,                                                 // Lighting info
                                  NULL,                                                              // Macro definitions
                                  (NXT_ShaderField[]){                                               // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_textureSamplerY, "u_textureSampler_vignette"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured External->YUVA8888 --------------------------------------------
    if( !err ){
        
        for( i=0; i<NXT_ChannelFilter_MAX; i++ )
        {
            char programName[] = "Textured+external->YUV[_]";
            programName[sizeof(programName)-3]='0'+i;
            
            if( err )
                break;
            NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderEXTtoYUVA8888[i];
            err = createShaderProgram(&sp->shaderProgram,
                                      &sp->shaderVertex,
                                      &sp->shaderFragment,
                                      programName,
                                      sp, sizeof(*sp),
                                      gStdVertexShader_Textured,                    // Vertex shader
                                      gFragmentShader_TexturedExternaltoYUVA8888,       // Fragment shader
	                                  &sp->lightingInfo,			// Lighting info
                                      channelFilterMacro[i],                        // Macro definitions
                                      (NXT_ShaderField[]){          // Uniforms
                                          {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                          {&sp->u_tex_matrix,       "u_tex_matrix"},
                                          {&sp->u_textureSampler,   "u_textureSampler"},
                                          {&sp->u_alpha,            "u_alpha"},
										  {&sp->u_realY,			"u_realY"},
										  {&sp->u_realX,			"u_realX"},
                                          {&sp->u_colorconv,        "u_colorconv"},
                                          {0}},
                                      (NXT_ShaderField[]){          // Attributes
                                          {&sp->a_position,         "a_position"},
                                          {&sp->a_texCoord,         "a_texCoord"},
                                          {0}}
                                      );
            
        }
        
    }
#endif //NOT APPLE
    // --- SHADER PROGRAM: Masked --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderRGB;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "masked",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Masked,      // Vertex shader
                                  gFragmentShader_Masked,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_mask_matrix,      "u_mask_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_maskSampler,      "u_maskSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_color,            "u_color"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {&sp->u_maskxfm,          "u_maskxfm"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {&sp->a_maskCoord,        "a_maskCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Masked --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderRGB;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type masked",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Masked, // Vertex shader
                                  gFragmentShaderForScreenMasking_Masked,  // Fragment shader
                                  NULL,                                    // Lighting info
                                  NULL,                                    // Macro definitions
                                  (NXT_ShaderField[]){                     // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_mask_matrix, "u_mask_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_maskSampler, "u_maskSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_color, "u_color"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_maskxfm, "u_maskxfm"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {&sp->a_maskCoord, "a_maskCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Combined Masked --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderRGB_Combined;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "combined masked",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_CombinedMasked,      // Vertex shader
                                  gFragmentShader_CombinedMasked,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       	"u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       	"u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,      "u_back_tex_matrix"},
                                      {&sp->u_mask_matrix,      	"u_mask_matrix"},
                                      {&sp->u_textureSampler,   	"u_textureSampler"},
                                      {&sp->u_back_textureSampler,  "u_back_textureSampler"},
                                      {&sp->u_maskSampler,      	"u_maskSampler"},
                                      {&sp->u_alpha,            	"u_alpha"},
                                      {&sp->u_color,            	"u_color"},
                                      {&sp->u_colorconv,        	"u_colorconv"},
                                      {&sp->u_maskxfm,          	"u_maskxfm"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {&sp->a_maskCoord,        "a_maskCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderRGB_Combined;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type combined masked",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_CombinedMasked, // Vertex shader
                                  gFragmentShaderForScreenMasking_CombinedMasked,  // Fragment shader
                                  NULL,                                            // Lighting info
                                  NULL,                                            // Macro definitions
                                  (NXT_ShaderField[]){                             // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_mask_matrix, "u_mask_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_maskSampler, "u_maskSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_color, "u_color"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_maskxfm, "u_maskxfm"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {&sp->a_maskCoord, "a_maskCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Masked --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderRGB_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "masked homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShader,      // Vertex shader
                                  gHomoFragmentShader_Masked,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_homo_matrix,      "u_homo_matrix"},
                                      {&sp->u_mask_homo_matrix, "u_mask_homo_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_mask_matrix,      "u_mask_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_maskSampler,      "u_maskSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_color,            "u_color"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {&sp->u_maskxfm,          "u_maskxfm"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Masked --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderRGB_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type masked homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShaderForScreenMasking,          // Vertex shader
                                  gHomoFragmentShaderForScreenMasking_Masked, // Fragment shader
                                  NULL,                                       // Lighting info
                                  NULL,                                       // Macro definitions
                                  (NXT_ShaderField[]){                        // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_homo_matrix, "u_homo_matrix"},
                                                      {&sp->u_mask_homo_matrix, "u_mask_homo_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_mask_matrix, "u_mask_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_maskSampler, "u_maskSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_color, "u_color"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_maskxfm, "u_maskxfm"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {0}});
    }
    // --- SHADER PROGRAM: Combined Masked --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderRGB_Combined_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "combined masked homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShader,      // Vertex shader
                                  gHomoFragmentShader_CombinedMasked,       // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       		"u_mvp_matrix"},
                                      {&sp->u_homo_matrix,      		"u_homo_matrix"},
                                      {&sp->u_mask_homo_matrix, 		"u_mask_homo_matrix"},
                                      {&sp->u_tex_matrix,       		"u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,  		"u_tex_matrix"},
                                      {&sp->u_mask_matrix,      		"u_mask_matrix"},
                                      {&sp->u_textureSampler,   		"u_textureSampler"},
                                      {&sp->u_back_textureSampler,   	"u_back_textureSampler"},
                                      {&sp->u_maskSampler,      		"u_maskSampler"},
                                      {&sp->u_alpha,            		"u_alpha"},
                                      {&sp->u_color,            		"u_color"},
                                      {&sp->u_colorconv,        		"u_colorconv"},
                                      {&sp->u_maskxfm,          		"u_maskxfm"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderRGB_Combined_Homo;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type combined masked homo",
                                  sp, sizeof(*sp),
                                  gHomoVertexShaderForScreenMasking,                  // Vertex shader
                                  gHomoFragmentShaderForScreenMasking_CombinedMasked, // Fragment shader
                                  NULL,                                               // Lighting info
                                  NULL,                                               // Macro definitions
                                  (NXT_ShaderField[]){                                // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_homo_matrix, "u_homo_matrix"},
                                                      {&sp->u_mask_homo_matrix, "u_mask_homo_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_mask_matrix, "u_mask_matrix"},
                                                      {&sp->u_textureSampler, "u_textureSampler"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_maskSampler, "u_maskSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_color, "u_color"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_maskxfm, "u_maskxfm"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured+YUV --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderYUV;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "Textured+YUV",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_TexturedYUV,  // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSamplerY,  "u_textureSamplerY"},
                                      {&sp->u_textureSamplerU,  "u_textureSamplerU"},
                                      {&sp->u_textureSamplerV,  "u_textureSamplerV"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
        
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+YUV --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderYUV;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type Textured+YUV",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured,   // Vertex shader
                                  gFragmentShaderForScreenMasking_TexturedYUV, // Fragment shader
                                  &sp->lightingInfo,                           // Lighting info
                                  NULL,                                        // Macro definitions
                                  (NXT_ShaderField[]){                         // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSamplerY, "u_textureSamplerY"},
                                                      {&sp->u_textureSamplerU, "u_textureSamplerU"},
                                                      {&sp->u_textureSamplerV, "u_textureSamplerV"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured+YUV --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderYUV_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
		                          "Textured+YUV",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured_vignette,    // Vertex shader
                                  gFragmentShader_TexturedYUV_vignette,  // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSamplerY,  "u_textureSamplerY"},
                                      {&sp->u_textureSamplerU,  "u_textureSamplerU"},
                                      {&sp->u_textureSamplerV,  "u_textureSamplerV"},
                                      {&sp->u_textureSamplerUV,  "u_textureSampler_vignette"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
        
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+YUV --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderYUV_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type Textured+YUV",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured_vignette,   // Vertex shader
                                  gFragmentShaderForScreenMasking_TexturedYUV_vignette, // Fragment shader
                                  &sp->lightingInfo,                                    // Lighting info
                                  NULL,                                                 // Macro definitions
                                  (NXT_ShaderField[]){                                  // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSamplerY, "u_textureSamplerY"},
                                                      {&sp->u_textureSamplerU, "u_textureSamplerU"},
                                                      {&sp->u_textureSamplerV, "u_textureSamplerV"},
                                                      {&sp->u_textureSamplerUV, "u_textureSampler_vignette"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured+Y2UVA --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderY2UVA;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "Textured+passthrough+Y2UVA",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderPassThrough_Textured,    // Vertex shader
                                  gFragmentShader_TexturedY2UVA,        // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_t ex_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );

    }
    // --- SHADER PROGRAM: Masked+YUV --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderYUV;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "Masked+YUV",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Masked,      // Vertex shader
                                  gFragmentShader_MaskedYUV,    // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_mask_matrix,      "u_mask_matrix"},
                                      {&sp->u_textureSamplerY,  "u_textureSamplerY"},
                                      {&sp->u_textureSamplerU,  "u_textureSamplerU"},
                                      {&sp->u_textureSamplerV,  "u_textureSamplerV"},
                                      {&sp->u_maskSampler,      "u_maskSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_color,            "u_color"},
                                      {&sp->u_maskxfm,          "u_maskxfm"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {&sp->a_maskCoord,        "a_maskCoord"},
                                      {0}}
                                  );
    }
    // --- SHADER PROGRAM: Screen Masking Type Masked+YUV --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderYUV;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type Masked+YUV",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Masked,   // Vertex shader
                                  gFragmentShaderForScreenMasking_MaskedYUV, // Fragment shader
                                  NULL,                                      // Lighting info
                                  NULL,                                      // Macro definitions
                                  (NXT_ShaderField[]){                       // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_mask_matrix, "u_mask_matrix"},
                                                      {&sp->u_textureSamplerY, "u_textureSamplerY"},
                                                      {&sp->u_textureSamplerU, "u_textureSamplerU"},
                                                      {&sp->u_textureSamplerV, "u_textureSamplerV"},
                                                      {&sp->u_maskSampler, "u_maskSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_color, "u_color"},
                                                      {&sp->u_maskxfm, "u_maskxfm"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {&sp->a_maskCoord, "a_maskCoord"},
                                                      {0}});
    }
    // --- SHADER PROGRAM: Textured+NV12 --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderNV12;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "Textured+NV12",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured,    // Vertex shader
                                  gFragmentShader_TexturedNV12, // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSamplerY,  "u_textureSamplerY"},
                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
									  {&sp->u_realY,			"u_realY"}, 
                                      	{&sp->u_realX,			"u_realX"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+NV12 --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderNV12;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type Textured+NV12",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured,    // Vertex shader
                                  gFragmentShaderForScreenMasking_TexturedNV12, // Fragment shader
                                  &sp->lightingInfo,                            // Lighting info
                                  NULL,                                         // Macro definitions
                                  (NXT_ShaderField[]){                          // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSamplerY, "u_textureSamplerY"},
                                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Combined Textured+NV12 --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderNV12_Combined;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "Combined Textured+NV12",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_CombinedTextured,    // Vertex shader
                                  gFragmentShader_CombinedTexturedNV12, // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       	"u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       	"u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,  	"u_back_tex_matrix"},
                                      {&sp->u_textureSamplerY,  	"u_textureSamplerY"},
                                      {&sp->u_textureSamplerUV, 	"u_textureSamplerUV"},
                                      {&sp->u_back_textureSampler, 	"u_back_textureSampler"},
                                      {&sp->u_alpha,            	"u_alpha"},
                                      {&sp->u_colorconv,        	"u_colorconv"},
									  {&sp->u_realY,				"u_realY"}, 
                                      	{&sp->u_realX,				"u_realX"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Textured+NV12 --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderNV12_Combined;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type Combined Textured+NV12",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_CombinedTextured,    // Vertex shader
                                  gFragmentShaderForScreenMasking_CombinedTexturedNV12, // Fragment shader
                                  &sp->lightingInfo,                                    // Lighting info
                                  NULL,                                                 // Macro definitions
                                  (NXT_ShaderField[]){                                  // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_textureSamplerY, "u_textureSamplerY"},
                                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured+NV12+VIGNETTE --------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderNV12_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
		                          "Textured+NV12+VIGNETTE",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured_vignette,    // Vertex shader
                                  gFragmentShader_TexturedNV12_vignette, // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSamplerY,  "u_textureSamplerY"},
                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                      {&sp->u_textureSamplerU, 	"u_textureSampler_vignette"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
									  {&sp->u_realY,			"u_realY"}, 
                                      	{&sp->u_realX,			"u_realX"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }

    // --- SHADER PROGRAM: Screen Masking Type Textured+NV12+VIGNETTE --------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderNV12_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type Textured+NV12+VIGNETTE",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_Textured_vignette,    // Vertex shader
                                  gFragmentShaderForScreenMasking_TexturedNV12_vignette, // Fragment shader
                                  &sp->lightingInfo,                                     // Lighting info
                                  NULL,                                                  // Macro definitions
                                  (NXT_ShaderField[]){                                   // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_textureSamplerY, "u_textureSamplerY"},
                                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                                      {&sp->u_textureSamplerU, "u_textureSampler_vignette"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Combined+Textured+NV12+VIGNETTE --------------------------------------
    if( !err ){
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderNV12_Combined_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
		                          "Combined Textured+NV12+VIGNETTE",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_CombinedTextured_vignette,    // Vertex shader
                                  gFragmentShader_CombinedTexturedNV12_vignette, // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,  "u_back_tex_matrix"},
                                      {&sp->u_textureSamplerY,  "u_textureSamplerY"},
                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                      {&sp->u_textureSamplerU, 	"u_textureSampler_vignette"},
                                      {&sp->u_back_textureSampler, 	"u_back_textureSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
									  {&sp->u_realY,			"u_realY"}, 
                                      	{&sp->u_realX,			"u_realX"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }
    // --- SHADER PROGRAM: Screen Masking Type Combined+Textured+NV12+VIGNETTE --------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Textured *sp = &pctx->screenMaskingType_texturedShaderNV12_Combined_vignette;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type Combined Textured+NV12+VIGNETTE",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_CombinedTextured_vignette,    // Vertex shader
                                  gFragmentShaderForScreenMasking_CombinedTexturedNV12_vignette, // Fragment shader
                                  &sp->lightingInfo,                                             // Lighting info
                                  NULL,                                                          // Macro definitions
                                  (NXT_ShaderField[]){                                           // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_textureSamplerY, "u_textureSamplerY"},
                                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                                      {&sp->u_textureSamplerU, "u_textureSampler_vignette"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_realY, "u_realY"},
                                                      {&sp->u_realX, "u_realX"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {0}});
    }
    // --- SHADER PROGRAM: Combined Masked+NV12 --------------------------------------------
    if( !err ){
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderNV12_Combined;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "Combined Masked+NV12",
                                  sp, sizeof(*sp),
                                  gStdVertexShader_CombinedMasked,      // Vertex shader
                                  gFragmentShader_CombinedMaskedNV12,   // Fragment shader
                                  NULL,							// Lighting info
                                  NULL,                         // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       	"u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       	"u_tex_matrix"},
                                      {&sp->u_back_tex_matrix,      "u_back_tex_matrix"},
                                      {&sp->u_mask_matrix,      	"u_mask_matrix"},
                                      {&sp->u_textureSamplerY,  	"u_textureSamplerY"},
                                      {&sp->u_textureSamplerUV, 	"u_textureSamplerUV"},
                                      {&sp->u_maskSampler,      	"u_maskSampler"},
                                      {&sp->u_back_textureSampler,  "u_back_textureSampler"},
                                      {&sp->u_alpha,            	"u_alpha"},
                                      {&sp->u_color,            	"u_color"},
                                      {&sp->u_maskxfm,          	"u_maskxfm"},
                                      {&sp->u_colorconv,        	"u_colorconv"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {&sp->a_maskCoord,        "a_maskCoord"},
                                      {0}}
                                  );
        
    }

    // --- SHADER PROGRAM: Screen Masking Type Combined Masked+NV12 --------------------------------------------
    if (!err)
    {
        NXT_ShaderProgram_Masked *sp = &pctx->screenMaskingType_maskedShaderNV12_Combined;
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  "screen masking type Combined Masked+NV12",
                                  sp, sizeof(*sp),
                                  gStdVertexShaderForScreenMasking_CombinedMasked,    // Vertex shader
                                  gFragmentShaderForScreenMasking_CombinedMaskedNV12, // Fragment shader
                                  NULL,                                               // Lighting info
                                  NULL,                                               // Macro definitions
                                  (NXT_ShaderField[]){                                // Uniforms
                                                      {&sp->u_mvp_matrix, "u_mvp_matrix"},
                                                      {&sp->u_tex_matrix, "u_tex_matrix"},
                                                      {&sp->u_back_tex_matrix, "u_back_tex_matrix"},
                                                      {&sp->u_mask_matrix, "u_mask_matrix"},
                                                      {&sp->u_textureSamplerY, "u_textureSamplerY"},
                                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                                      {&sp->u_maskSampler, "u_maskSampler"},
                                                      {&sp->u_back_textureSampler, "u_back_textureSampler"},
                                                      {&sp->u_alpha, "u_alpha"},
                                                      {&sp->u_color, "u_color"},
                                                      {&sp->u_maskxfm, "u_maskxfm"},
                                                      {&sp->u_colorconv, "u_colorconv"},
                                                      {&sp->u_textureSampler_for_mask, "u_textureSampler_for_mask"},
                                                      {&sp->u_inverse_Mask, "u_inverse_Mask"},
                                                      {0}},
                                  (NXT_ShaderField[]){// Attributes
                                                      {&sp->a_position, "a_position"},
                                                      {&sp->a_texCoord, "a_texCoord"},
                                                      {&sp->a_maskCoord, "a_maskCoord"},
                                                      {0}});
    }

    // --- SHADER PROGRAM: Textured+NV12 -> YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        char programName[] = "Textured+NV12->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        if( err )
            break;
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderNV12toYUVA8888[i];
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  programName,
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured,                    // Vertex shader
                                  gFragmentShader_TexturedNV12toYUVA8888,       // Fragment shader
                                  &sp->lightingInfo,			// Lighting info
                                  channelFilterMacro[i],                        // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSamplerY,  "u_textureSamplerY"},
                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {&sp->u_realY,			"u_realY"}, 
                                      {&sp->u_realX,			"u_realX"},									  	
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
        
    }
    // --- SHADER PROGRAM: Masked+NV12 -> YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        char programName[] = "Masked+NV12->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        if( err )
            break;
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderNV12toYUVA8888[i];
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  programName,
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Masked,                  // Vertex shader
                                  gFragmentShader_MaskedNV12toYUVA8888,     // Fragment shader
                                  NULL,										// Lighting info
                                  channelFilterMacro[i],                    // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_mask_matrix,      "u_mask_matrix"},
                                      {&sp->u_textureSamplerY,  "u_textureSamplerY"},
                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                      {&sp->u_maskSampler,      "u_maskSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_color,            "u_color"},
                                      {&sp->u_maskxfm,          "u_maskxfm"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {&sp->a_maskCoord,        "a_maskCoord"},
                                      {0}}
                                  );
        
    }
    // --- SHADER PROGRAM: Textured+RGBA -> YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        
        char programName[] = "Textured+RGBA->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        if( err )
            break;
        NXT_ShaderProgram_Textured *sp = &pctx->texturedShaderRGBAtoYUVA8888[i];
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  programName,
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Textured,                    // Vertex shader
                                  gFragmentShader_TexturedRGBAtoYUVA8888,       // Fragment shader
                                  &sp->lightingInfo,							// Lighting info
                                  channelFilterMacro[i],                        // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_textureSamplerY,  "u_textureSamplerY"},
                                      {&sp->u_textureSamplerUV, "u_textureSamplerUV"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {0}}
                                  );
    }
#ifndef __APPLE__
    // --- SHADER PROGRAM: Mask+Textured+External -> YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        
        char programName[] = "Mask+Textured+External->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        if( err )
            break;
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderEXTtoYUVA8888[i];
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  programName,
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Masked,                    // Vertex shader
                                  gFragmentShader_MaskedExternaltoYUVA8888,       // Fragment shader
                                  NULL,											// Lighting info
                                  channelFilterMacro[i],                        // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_mask_matrix,      "u_mask_matrix"},
                                      {&sp->u_textureSampler,   "u_textureSampler"},
                                      {&sp->u_maskSampler,      "u_maskSampler"},
                                      {&sp->u_color,            "u_color"},
                                      {&sp->u_maskxfm,          "u_maskxfm"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_colorconv,        "u_colorconv"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {&sp->a_maskCoord,        "a_maskCoord"},
                                      {0}}
                                  );
    }
#endif//not __APPLE__
    // --- SHADER PROGRAM: Masked+RGBA -> YUVA8888 --------------------------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        char programName[] = "Masked+RGBA->YUV[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        if( err )
            break;
        NXT_ShaderProgram_Masked *sp = &pctx->maskedShaderRGBAtoYUVA8888[i];
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  programName,
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Masked,                  // Vertex shader
                                  gFragmentShader_MaskedRGBAtoYUVA8888,     // Fragment shader
                                  NULL,										// Lighting info
                                  channelFilterMacro[i],                    // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_tex_matrix,       "u_tex_matrix"},
                                      {&sp->u_mask_matrix,      "u_mask_matrix"},
                                      {&sp->u_textureSampler,	"u_textureSampler"},
                                      {&sp->u_maskSampler,      "u_maskSampler"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {&sp->u_color,            "u_color"},
                                      {&sp->u_maskxfm,          "u_maskxfm"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_texCoord,         "a_texCoord"},
                                      {&sp->a_maskCoord,        "a_maskCoord"},
                                      {0}}
                                  );
        
    }
    // --- SHADER PROGRAM: Solid Color NV12 -> YUVA8888 ------------------------
    for( i=0; i<NXT_ChannelFilter_MAX; i++ )
    {
        char programName[] = "SolidNV12toYUVA8888[_]";
        programName[sizeof(programName)-3]='0'+i;
        
        if( err )
            break;
        NXT_ShaderProgram_Solid *sp = &pctx->solidShaderNV12toYUVA8888[i];
        err = createShaderProgram(&sp->shaderProgram,
                                  &sp->shaderVertex,
                                  &sp->shaderFragment,
                                  programName,
                                  sp, sizeof(*sp),
                                  gStdVertexShader_Solid,                   // Vertex shader
                                  gFragmentShader_SolidNV12toYUVA8888,      // Fragment shader
                                  &sp->lightingInfo,						// Lighting info
                                  channelFilterMacro[i],                    // Macro definitions
                                  (NXT_ShaderField[]){          // Uniforms
                                      {&sp->u_mvp_matrix,       "u_mvp_matrix"},
                                      {&sp->u_alpha,            "u_alpha"},
                                      {0}},
                                  (NXT_ShaderField[]){          // Attributes
                                      {&sp->a_position,         "a_position"},
                                      {&sp->a_color,            "a_color"},
                                      {0}}
                                  );
        
    }
    // -------------------------------------------------------------------------

    return err;
}
#endif


static NXT_Error deleteShaderProgram(GLuint *program,
                                     GLuint *shaderV,
                                     GLuint *shaderF )
{
    if ( *program ) {
        if ( *shaderV ) {
            glDetachShader( *program, *shaderV );
            glDeleteShader( *shaderV );
            *shaderV = 0;
        }
        if ( *shaderF ) {
            glDetachShader( *program, *shaderF );
            glDeleteShader( *shaderF );
            *shaderF = 0;
        }
        glDeleteProgram( *program ); *program = 0;
    }
    return NXT_Error_None;
}


static NXT_Error createShaderProgram(GLuint *program,
                                     GLuint *shaderV,
                                     GLuint *shaderF,
                                     const char *programName,
                                     void* programBase,
                                     int programSize,
                                     const char *vertexSource,
                                     const char *fragmentSource,
                                     NXT_Shader_LightingInfo *lightingInfo,
                                     const char *macroDefs,
                                     NXT_ShaderField* uniforms,
                                     NXT_ShaderField* attributes)
{
    LOGI("[%s %d] begin createShaderProgram:%s", __func__, __LINE__, programName);
    
    
    GLuint *pbase = (GLuint*)programBase;
    int numentries = programSize/sizeof(GLuint);
    if( numentries>0 && pbase ) {
        int i;
        for( i=0; i<numentries; i++ ) {
            pbase[i] = INVALID_UNIFORM;
        }
    }
    
    *program = *shaderV = *shaderF = 0;
    
    // Load the shaders
    GLuint hVertexShader = loadShader(GL_VERTEX_SHADER, vertexSource, macroDefs);
    GLuint hFragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource, macroDefs);
    if( !hVertexShader ) {
        LOGE("[%s %d] Failed to load vertex shader (%s)", __func__, __LINE__, programName);
    }
    if( !hFragmentShader ) {
        LOGE("[%s %d] Failed to load fragment shader (%s)", __func__, __LINE__, programName);
    }
    if( !hVertexShader || !hFragmentShader )
    {
        if ( hVertexShader ) glDeleteShader(hVertexShader);
        if ( hFragmentShader ) glDeleteShader( hFragmentShader );
        
        return NXT_Error_ShaderFailure;
    }
    
    // Create a program from the loaded shaders
    GLuint hShaderProgram = glCreateProgram();
    if( !hShaderProgram ) {
        LOGE("[%s %d] Failed to create shader program (%s)", __func__, __LINE__, programName);
        if ( hVertexShader ) glDeleteShader(hVertexShader);
        if ( hFragmentShader ) glDeleteShader( hFragmentShader );
        return NXT_Error_ShaderFailure;
    }
    
    glAttachShader(hShaderProgram, hVertexShader);
    CHECK_GL_ERROR();
    glAttachShader(hShaderProgram, hFragmentShader);
    CHECK_GL_ERROR();
    
    // Link the program
    glLinkProgram(hShaderProgram);
    if( shaderProgramLinkStatus(hShaderProgram)!=GL_TRUE ) {
        char *compileErrors = getProgramInfoLog(hShaderProgram);
        glDeleteProgram(hShaderProgram);
        if ( hVertexShader ) glDeleteShader(hVertexShader);
        if ( hFragmentShader ) glDeleteShader( hFragmentShader );
        
        LOGE("----------- Error(s) detected linking shader program --------------\n"
             "Program name: %s\n"
             "%s\n"
             "-------------------------------------------------------------------\n",
             programName, compileErrors);
        
        freeProgramInfoLog(compileErrors);
        return NXT_Error_ShaderFailure;
    }
    
//    if( uniforms ) {
//        NXT_ShaderField *pF;
//        for( pF = uniforms; pF->fieldName && pF->fieldId; pF++ ) {
//            *(pF->fieldId) = glGetUniformLocation(hShaderProgram, pF->fieldName);
//            CHECK_GL_ERROR();
//            LOGV("[%s %d] uniform %s->%s=%d", __func__, __LINE__, programName, pF->fieldName, *(pF->fieldId));
//        }
//    }
//    
//    if( attributes ) {
//        NXT_ShaderField *pF;
//        for( pF = attributes; pF->fieldName && pF->fieldId; pF++ ) {
//            *(pF->fieldId) = glGetAttribLocation(hShaderProgram, pF->fieldName);
//            CHECK_GL_ERROR();
//            LOGV("[%s %d] attribute %s->%s=%d", __func__, __LINE__, programName, pF->fieldName, *(pF->fieldId));
//        }
//    }

	processUniforms(hShaderProgram, programName, uniforms);
	processAttributes(hShaderProgram, programName, attributes);
    
    if( lightingInfo ) {
    	NXT_ShaderField light_uniform[] = {
    		{&lightingInfo->u_ambient_light, 	"u_ambient_light"},
        	{&lightingInfo->u_diffuse_light, 	"u_diffuse_light"},
        	{&lightingInfo->u_lightpos, 		"u_lightpos"},
        	{&lightingInfo->u_light_f0, 		"u_light_f0"},
        	{&lightingInfo->u_light_f1, 		"u_light_f1"},
        	{&lightingInfo->u_specexp, 			"u_specexp"},
        	{&lightingInfo->u_specular_light, 	"u_specular_light"},
        	{0}
    	};

    	NXT_ShaderField light_attribute[] = {
    		{&lightingInfo->a_normal, 			"a_normal"},
    		{0}
    	};
        processUniforms(hShaderProgram, programName, light_uniform);
        processAttributes(hShaderProgram, programName, light_attribute);
    }
    
    // Done!
    
    *program = hShaderProgram;
    *shaderV = hVertexShader;
    *shaderF = hFragmentShader;
    
    LOGI("[%s %d] finished createShaderProgram:%s (hShaderProgram=%d)", __func__, __LINE__, programName, hShaderProgram);
    
    return NXT_Error_None;
}

static void processUniforms( GLuint hShaderProgram, const char *programName, NXT_ShaderField *uniforms )
{
	if( !uniforms ) {
    	return;
    }
    NXT_ShaderField *pF;
    for( pF = uniforms; pF->fieldName && pF->fieldId; pF++ ) {
        *(pF->fieldId) = glGetUniformLocation(hShaderProgram, pF->fieldName);
        CHECK_GL_ERROR();
        LOGV("[%s %d] uniform %s->%s=%d", __func__, __LINE__, programName, pF->fieldName, *(pF->fieldId));
    }
}


static void processAttributes( GLuint hShaderProgram, const char *programName, NXT_ShaderField *attributes )
{
	if( !attributes ) {
    	return;
    }
    NXT_ShaderField *pF;
    for( pF = attributes; pF->fieldName && pF->fieldId; pF++ ) {
        *(pF->fieldId) = glGetAttribLocation(hShaderProgram, pF->fieldName);
        CHECK_GL_ERROR();
        LOGV("[%s %d] attribute %s->%s=%d", __func__, __LINE__, programName, pF->fieldName, *(pF->fieldId));
    }
}

static int validateShaderProgram( GLuint program ) {
    GLint logLength, status;
    
    glValidateProgram(program);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        char *log = (char *)malloc(logLength);
        glGetProgramInfoLog(program, logLength, &logLength, log);
        LOGF("Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status == 0)
        return -1;
    
    return 0;
}

static unsigned int shaderCompileStatus( GLuint hShader ) {
    GLint compileStatus = 0;
    glGetShaderiv(hShader, GL_COMPILE_STATUS, &compileStatus);
    return compileStatus;
}

static unsigned int shaderProgramLinkStatus( GLuint hProgram ) {
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(hProgram, GL_LINK_STATUS, &linkStatus);
    return linkStatus;
}

static char* getShaderInfoLog( GLuint hShader ) {
    char *infoLogString;
    GLint infoLogLength = 0;
    glGetShaderiv(hShader, GL_INFO_LOG_LENGTH, &infoLogLength);
    if( infoLogLength < 1 ) {
        infoLogLength = 4096;
        /* //return NULL;
         return strdup("infoLogLength < 1");*/
    }
    infoLogString = (char*)malloc(infoLogLength+1);
    if( !infoLogString ) {
        //return NULL;
        return strdup("infoLogString is null");
    }
    memset(infoLogString,0,infoLogLength+1);
    glGetShaderInfoLog(hShader, infoLogLength, NULL, infoLogString);
    return infoLogString;
}

static char* getProgramInfoLog( GLuint hProgram ) {
    char *infoLogString;
    GLint infoLogLength = 0;
    glGetProgramiv(hProgram, GL_INFO_LOG_LENGTH, &infoLogLength);
    if( infoLogLength < 1 ) {
        return NULL;
    }
    infoLogString = (char*)malloc(infoLogLength);
    if( !infoLogString )
        return NULL;
    glGetProgramInfoLog(hProgram, infoLogLength, NULL, infoLogString);
    return infoLogString;
}

static void freeShaderInfoLog( char *s ) {
    if( s!=NULL )
        free(s);
}

static void freeProgramInfoLog( char *s ) {
    if( s!=NULL )
        free(s);
}

static GLuint loadShader(GLenum shaderType, const char* pMainCode, const char* pMacroDef) {
    
    if( shaderType==GL_FRAGMENT_SHADER ) {
        LOGV("[%s %d] shaderType=GL_FRAGMENT_SHADER", __func__, __LINE__);
    } else if (shaderType==GL_VERTEX_SHADER ) {
        LOGV("[%s %d] shaderType=GL_VERTEX_SHADER", __func__, __LINE__);
    } else {
        LOGE("[%s %d] Unknown shader type: %d", __func__, __LINE__, shaderType);
        return 0;
    }
    
    GLuint hShader = glCreateShader(shaderType);
    if(!hShader)
    {
        CHECK_GL_ERROR();
        LOGE("[%s %d] glCreateShader error", __func__, __LINE__);
        return 0;
    }
    
    if( pMacroDef ) {
        const char *fullSource[2];
        fullSource[0] = pMacroDef;
        fullSource[1] = pMainCode;
        glShaderSource(hShader, 2, fullSource, NULL);
    } else {
        glShaderSource(hShader, 1, &pMainCode, NULL);
    }
    CHECK_GL_ERROR();
    glCompileShader(hShader);
    CHECK_GL_ERROR();
    if( !shaderCompileStatus(hShader) )
    {
        const char *shaderTypeString;
        char *compileErrors = getShaderInfoLog(hShader);
        if( compileErrors ) {
            glDeleteShader(hShader);
            
            if( shaderType==GL_VERTEX_SHADER ) {
                shaderTypeString = "VERTEX shader --";
            } else if( shaderType==GL_FRAGMENT_SHADER ) {
                shaderTypeString = "FRAGMENT shader ";
            } else {
                shaderTypeString = "UNKNOWN shader";
            }
            
            LOGF("------------ Error(s) detected compiling %s------------\n"
                 "%s\n"
                 "-------------------------------------------------------------------\n",
                 shaderTypeString, compileErrors);
            
            freeShaderInfoLog(compileErrors);
            return 0;
        }
        LOGW("[%s %d] OpenGL reports shader failed to compile, but error log is empty; assuming successful compilation", __func__, __LINE__);
    }
    CHECK_GL_ERROR();
    return hShader;
}
