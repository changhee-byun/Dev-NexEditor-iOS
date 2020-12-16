#pragma once

#include "util.h"

#define NXT_EFFECT_FLAG_SETOFFSET   0x00000001
#define NXT_EFFECT_FLAG_SETOVERLAP  0x00000002

typedef enum NXT_NoiseType_ {
	NXT_NoiseType_None = 0,
	NXT_NoiseType_PerlinSimplex = 1
} NXT_NoiseType;

typedef enum NXT_NoiseFunction_ {
	NXT_NoiseFunction_Pure = 0,    // noise(p)
	NXT_NoiseFunction_Fractal = 1,    // noise(p) + (1/2)noise(2p) + (1/4)noise(4p) ...
	NXT_NoiseFunction_Turbulence = 2,    // abs(noise(p)) + abs((1/2)noise(2p)) + abs((1/4)noise(4p)) ...
	NXT_NoiseFunction_WaveFractal = 3     // sin( n + abs(noise(p)) + abs((1/2)noise(2p)) + abs((1/4)noise(4p)) ... )
} NXT_NoiseFunction;

typedef enum NXT_Function_ {
	NXT_Function_SimplexPure = 0,
	NXT_Function_SimplexFractal = 1,
	NXT_Function_SimplexTurbulence = 2,
	NXT_Function_SimplexWaveFractal = 3
} NXT_Function;

typedef enum NXT_ClipType_ {
	NXT_ClipType_All = 0,
	NXT_ClipType_First = 1,
	NXT_ClipType_Last = 2,
	NXT_ClipType_Middle = 3,
	NXT_ClipType_Even = 4,
	NXT_ClipType_Odd = 5,
	NXT_ClipType_NotFirst = 6,
	NXT_ClipType_NotLast = 7,
	NXT_ClipType_NotMiddle = 8,
	NXT_ClipType_NotEven = 9,
	NXT_ClipType_NotOdd = 10
} NXT_ClipType;

typedef enum NXT_PartType_ {
	NXT_PartType_All = 0,
	NXT_PartType_In = 1,
	NXT_PartType_Out = 2,
	NXT_PartType_Mid = 3
} NXT_PartType;

typedef enum NXT_EffectType_ {
	NXT_EffectType_NONE = 0,
	NXT_EffectType_Transition = 1,
	NXT_EffectType_Title = 2
} NXT_EffectType;

typedef enum NXT_InterpBasis_ {
	NXT_InterpBasis_Time = 0,
	NXT_InterpBasis_ClipIndex = 1,
	NXT_InterpBasis_ClipIndexPermuted = 2
} NXT_InterpBasis;

typedef enum NXT_RepeatType_ {
	NXT_RepeatType_None = 0,
	NXT_RepeatType_Repeat = 1,
	NXT_RepeatType_Sawtooth = 2,
	NXT_RepeatType_Sine = 3
} NXT_RepeatType;

struct NXT_KeyFrame {
	float       e[4];
	float       t;
	float       timing[4];
};

struct NXT_KeyFrameSet {

	NXT_KeyFrame        *keyframes;
	int                 numKeyFrames;
	NXT_PartType        partType;
	NXT_ClipType        clipType;
	unsigned int        bDefault;

	void operator=(NXT_KeyFrameSet& item);
};

struct NXT_Node_KeyFrame {
	float   t;
	float   e[4];
	int     element_count;
	float   timing[4];
};

struct NXT_Node_AnimatedValue{

	NXT_Node_AnimatedValue();

	~NXT_Node_AnimatedValue();

	NXT_Node_AnimatedValue(NXT_Node_AnimatedValue& item);

	void processNodeKeyFrame(rapidxml::xml_node<>* pnode, NXT_Node_KeyFrame* pkf);

	void processKeyFrameAttr(NXT_Node_KeyFrame* kf, char* attrName, char* attrValue);

	void processKeyframeSetAttr(rapidxml::xml_node<>* pnode, NXT_KeyFrameSet *kfset);

	void processKeyframeSet(rapidxml::xml_node<>* pstart_node, NXT_KeyFrameSet *kfset);

	void processChildNode(rapidxml::xml_node<>* pstart_node);

	void setAttr(char* attrName, char* attrValue);

	std::string			id_;
	int                 numUpdateTargets;
	int                 allocUpdateTargets;
	NXT_KeyFrameSet     *keyframeSet;
	int                 numKeyframeSets;
	int                 element_count;
	float               timing[4];
	unsigned int        bUseOverallTime;
	NXT_NoiseType       noiseType;
	NXT_NoiseFunction   noiseFunc;
	float               noiseBias;
	float               noiseScale;
	float               noiseSampleBias;
	float               noiseSampleScale;
	float               startTime;
	float               endTime;
	int                 noiseOctaves;
	NXT_InterpBasis     basis;
};