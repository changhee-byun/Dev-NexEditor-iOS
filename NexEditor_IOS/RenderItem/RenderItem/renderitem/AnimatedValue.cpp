#include "AnimatedValue.h"


NXT_Node_AnimatedValue::NXT_Node_AnimatedValue(){

	basis = NXT_InterpBasis_Time;
	noiseOctaves = 4;
	noiseType = NXT_NoiseType_None;
	noiseSampleBias = 0.0;
	noiseSampleScale = 1.0;
	noiseScale = 0.5;
	noiseBias = 0.5;
	noiseFunc = NXT_NoiseFunction_Pure;
	startTime = 0.0;
	endTime = 1.0;

	// Default
	timing[0] = 0.25f;
	timing[1] = 0.10f;
	timing[2] = 0.25f;
	timing[3] = 1.0f;

	keyframeSet = NULL;

	bUseOverallTime = 0;
}

NXT_Node_AnimatedValue::~NXT_Node_AnimatedValue(){

	if (keyframeSet){

		for (int i = 0; i < numKeyframeSets; ++i){

			if (keyframeSet[i].keyframes){

				free(keyframeSet[i].keyframes);
				keyframeSet[i].keyframes = NULL;
			}
		}

		free(keyframeSet);
		keyframeSet = NULL;
	}
}

void NXT_Node_AnimatedValue::processNodeKeyFrame(rapidxml::xml_node<>* pnode, NXT_Node_KeyFrame* pkf){

	pkf->timing[0] = 0.0f;
	pkf->timing[1] = 0.0f;
	pkf->timing[2] = 1.0f;
	pkf->timing[3] = 1.0f;

	rapidxml::xml_attribute<>* pattrib = getFirstAttribChain(pnode);
	while (pattrib){

		char* attrName = pattrib->name();
		char* attrValue = pattrib->value();
		processKeyFrameAttr(pkf, attrName, attrValue);
		pattrib = pattrib->next_attribute();
	}
}

void NXT_Node_AnimatedValue::processKeyFrameAttr(NXT_Node_KeyFrame* kf, char* attrName, char* attrValue){

	if (0 == strcasecmp(attrName, "time")) {
		NXT_FloatVectorFromString(attrValue, &kf->t, 1);
	}
	else if (0 == strcasecmp(attrName, "value")) {
		kf->element_count = NXT_FloatVectorFromString(attrValue, kf->e, sizeof(kf->e) / sizeof(*(kf->e)));
	}
	else if (0 == strcasecmp(attrName, "timingfunction")) {

		if (0 == strcasecmp(attrValue, "ease")) {
			kf->timing[0] = 0.25f;
			kf->timing[1] = 0.10f;
			kf->timing[2] = 0.25f;
			kf->timing[3] = 1.0f;
		}
		else if (0 == strcasecmp(attrValue, "ease-in")) {
			kf->timing[0] = 0.42f;
			kf->timing[1] = 0.0f;
			kf->timing[2] = 1.0f;
			kf->timing[3] = 1.0f;
		}
		else if (0 == strcasecmp(attrValue, "ease-out")) {
			kf->timing[0] = 0.0f;
			kf->timing[1] = 0.0f;
			kf->timing[2] = 0.58f;
			kf->timing[3] = 1.0f;
		}
		else if (0 == strcasecmp(attrValue, "ease-in-out")) {
			kf->timing[0] = 0.42f;
			kf->timing[1] = 0.0f;
			kf->timing[2] = 0.58f;
			kf->timing[3] = 1.0f;
		}
		else if (0 == strcasecmp(attrValue, "linear")) {
			kf->timing[0] = 0.0f;
			kf->timing[1] = 0.0f;
			kf->timing[2] = 1.0f;
			kf->timing[3] = 1.0f;
		}
		else if (0 == strcasecmp(attrValue, "default")) {
			kf->timing[0] = 0.25f;
			kf->timing[1] = 0.10f;
			kf->timing[2] = 0.25f;
			kf->timing[3] = 1.0f;
		}
		else {
			NXT_FloatVectorFromString(attrValue, kf->timing, 4);
		}

	}
}

void NXT_Node_AnimatedValue::processKeyframeSet(rapidxml::xml_node<>* pstart_node, NXT_KeyFrameSet *kfset) {

	int nKeyFrame = 0;

	rapidxml::xml_node<>* pnode = pstart_node;

	while (pnode) {

		char* name = pnode->name();
		if (0 == strcasecmp(name, "keyframe")){

			nKeyFrame++;
		}

		pnode = pnode->next_sibling();
	}

	kfset->numKeyFrames = nKeyFrame;

	if (kfset->keyframes)
		free(kfset->keyframes);
	kfset->keyframes = (NXT_KeyFrame*)malloc(kfset->numKeyFrames*sizeof(NXT_KeyFrame));

	nKeyFrame = 0;
	pnode = pstart_node;
	while (pnode) {

		char* name = pnode->name();
		if (0 == strcasecmp(name, "keyframe")){

			NXT_Node_KeyFrame node_kf;
			NXT_Node_KeyFrame* pkf = &node_kf;

			memset(pkf, 0, sizeof(NXT_Node_KeyFrame));
			processNodeKeyFrame(pnode, pkf);

			kfset->keyframes[nKeyFrame].t = startTime + (pkf->t*(endTime - startTime));
			kfset->keyframes[nKeyFrame].e[0] = pkf->e[0];
			kfset->keyframes[nKeyFrame].e[1] = pkf->e[1];
			kfset->keyframes[nKeyFrame].e[2] = pkf->e[2];
			kfset->keyframes[nKeyFrame].e[3] = pkf->e[3];
			kfset->keyframes[nKeyFrame].timing[0] = pkf->timing[0];
			kfset->keyframes[nKeyFrame].timing[1] = pkf->timing[1];
			kfset->keyframes[nKeyFrame].timing[2] = pkf->timing[2];
			kfset->keyframes[nKeyFrame].timing[3] = pkf->timing[3];
			if (pkf->element_count > element_count) {
				element_count = pkf->element_count;
			}
			nKeyFrame++;
		}
		pnode = pnode->next_sibling();
	}

	int numzeroes = 0;
	float nzval = 0.0;
	int i;
	for (i = 0; i<kfset->numKeyFrames; i++) {
		float wkval = kfset->keyframes[i].t;

		// Limit to 0...1
		if (wkval > 1.0)
			wkval = 1.0;
		else if (wkval < 0.0)
			wkval = 0.0;

		if (wkval == 0.0 && i == kfset->numKeyFrames - 1)
			wkval = 1.0;
		if (wkval == 0.0 && i>0) {
			numzeroes++;
		}
		else {
			if (numzeroes) {
				float step = (wkval - nzval) / ((float)(numzeroes + 1));
				int j;
				for (j = i - numzeroes; j<i; j++) {
					kfset->keyframes[j].t = step * (float)(j - (i - numzeroes) + 1);
				}
			}
			if (wkval > nzval) {
				nzval = wkval;
			}
			kfset->keyframes[i].t = nzval;
		}
	}
}

void NXT_Node_AnimatedValue::processChildNode(rapidxml::xml_node<>* pstart_node){

	if (NULL == pstart_node){

		keyframeSet = (NXT_KeyFrameSet*)malloc(sizeof(NXT_KeyFrameSet)* 1);
		memset(keyframeSet, 0, sizeof(NXT_KeyFrameSet)* 1);

		return;
	}

	int partCount = 0;
	int keyFrameFound = 0;
	NXT_KeyFrameSet *kfset = NULL;

	rapidxml::xml_node<> *pnode = pstart_node;

	while (pnode){

		char* node_name = pnode->name();

		if (0 == strcasecmp(node_name, "keyframe")){

			keyFrameFound = 1;
		}
		else if (0 == strcasecmp(node_name, "part")){

			partCount++;
		}
		pnode = pnode->next_sibling();
	}

	element_count = 0;
	numKeyframeSets = partCount + keyFrameFound;

	if (numKeyframeSets == 0) {
		numKeyframeSets = 1;
	}

	keyframeSet = (NXT_KeyFrameSet*)malloc(sizeof(NXT_KeyFrameSet)*numKeyframeSets);
	memset(keyframeSet, 0, sizeof(NXT_KeyFrameSet)*numKeyframeSets);

	kfset = keyframeSet;
	pnode = pstart_node;

	if (keyFrameFound || partCount<1) {

		kfset->partType = NXT_PartType_All;
		kfset->clipType = NXT_ClipType_All;
		kfset->bDefault = 1;
		processKeyframeSet(pnode, kfset);
		kfset++;
	}

	pnode = pstart_node;

	while (pnode){

		char* node_name = pnode->name();

		if (0 == strcasecmp(node_name, "part")){

			processKeyframeSetAttr(pnode, kfset);
			processKeyframeSet(pnode->first_node(), kfset);
			kfset++;
		}
		pnode = pnode->next_sibling();
	}
}

void NXT_Node_AnimatedValue::processKeyframeSetAttr(rapidxml::xml_node<>* pnode, NXT_KeyFrameSet *kfset){

	rapidxml::xml_attribute<>* pattrib = getFirstAttribChain(pnode);
	while (pattrib){

		char* attrName = pattrib->name();
		char* attrValue = pattrib->value();

		if (strcasecmp(attrName, "type") == 0) {
			if (strcasecmp(attrValue, "in") == 0) {
				kfset->partType = NXT_PartType_In;
			}
			else if (strcasecmp(attrValue, "out") == 0) {
				kfset->partType = NXT_PartType_Out;
			}
			else if (strcasecmp(attrValue, "mid") == 0) {
				kfset->partType = NXT_PartType_Mid;
			}
			else if (strcasecmp(attrValue, "all") == 0) {
				kfset->partType = NXT_PartType_All;
			}
		}
		else if (strcasecmp(attrName, "clip") == 0) {
			if (strcasecmp(attrValue, "first") == 0) {
				kfset->clipType = NXT_ClipType_First;
			}
			else if (strcasecmp(attrValue, "mid") == 0) {
				kfset->clipType = NXT_ClipType_Middle;
			}
			else if (strcasecmp(attrValue, "middle") == 0) {
				kfset->clipType = NXT_ClipType_Middle;
			}
			else if (strcasecmp(attrValue, "last") == 0) {
				kfset->clipType = NXT_ClipType_Last;
			}
			else if (strcasecmp(attrValue, "even") == 0) {
				kfset->clipType = NXT_ClipType_Even;
			}
			else if (strcasecmp(attrValue, "odd") == 0) {
				kfset->clipType = NXT_ClipType_Odd;
			}
			else if (strcasecmp(attrValue, "all") == 0) {
				kfset->clipType = NXT_ClipType_All;
			}
			else if (strcasecmp(attrValue, "!first") == 0) {
				kfset->clipType = NXT_ClipType_NotFirst;
			}
			else if (strcasecmp(attrValue, "!mid") == 0) {
				kfset->clipType = NXT_ClipType_NotMiddle;
			}
			else if (strcasecmp(attrValue, "!middle") == 0) {
				kfset->clipType = NXT_ClipType_NotMiddle;
			}
			else if (strcasecmp(attrValue, "!last") == 0) {
				kfset->clipType = NXT_ClipType_NotLast;
			}
			else if (strcasecmp(attrValue, "!even") == 0) {
				kfset->clipType = NXT_ClipType_NotEven;
			}
			else if (strcasecmp(attrValue, "!odd") == 0) {
				kfset->clipType = NXT_ClipType_NotOdd;
			}
		}
		else {
			// TODO: Error
		}
		pattrib = pattrib->next_attribute();
	}
}

void NXT_Node_AnimatedValue::setAttr(char* attrName, char* attrValue){

	if (0 == strcasecmp(attrName, "id")){
		id_ = std::string(attrValue);
	}
	else if (0 == strcasecmp(attrName, "timingfunction")) {

		if (0 == strcasecmp(attrValue, "ease")) {
			timing[0] = 0.25f;
			timing[1] = 0.10f;
			timing[2] = 0.25f;
			timing[3] = 1.0f;
		}
		else if (0 == strcasecmp(attrValue, "ease-in")) {
			timing[0] = 0.42f;
			timing[1] = 0.0f;
			timing[2] = 1.0f;
			timing[3] = 1.0f;
		}
		else if (0 == strcasecmp(attrValue, "ease-out")) {
			timing[0] = 0.0f;
			timing[1] = 0.0f;
			timing[2] = 0.58f;
			timing[3] = 1.0f;
		}
		else if (0 == strcasecmp(attrValue, "ease-in-out")) {
			timing[0] = 0.42f;
			timing[1] = 0.0f;
			timing[2] = 0.58f;
			timing[3] = 1.0f;
		}
		else if (0 == strcasecmp(attrValue, "linear")) {
			timing[0] = 0.0f;
			timing[1] = 0.0f;
			timing[2] = 1.0f;
			timing[3] = 1.0f;
		}
		else if (0 == strcasecmp(attrValue, "default")) {
			timing[0] = 0.25f;
			timing[1] = 0.10f;
			timing[2] = 0.25f;
			timing[3] = 1.0f;
		}
		else {
			NXT_FloatVectorFromString(attrValue, timing, 4);
		}

	}
	else if (0 == strcasecmp(attrName, "scope")) {
		if (0 == strcasecmp(attrValue, "part")) {
			bUseOverallTime = 0;
		}
		else if (0 == strcasecmp(attrValue, "whole")) {
			bUseOverallTime = 1;
		}
	}
	else if (0 == strcasecmp(attrName, "basis")) {
		if (0 == strcasecmp(attrValue, "time")) {
			basis = NXT_InterpBasis_Time;
		}
		else if (0 == strcasecmp(attrValue, "clipindex")) {
			basis = NXT_InterpBasis_ClipIndex;
		}
		else if (0 == strcasecmp(attrValue, "clipindexshuffle")) {
			basis = NXT_InterpBasis_ClipIndexPermuted;
		}
	}
	else if (0 == strcasecmp(attrName, "noise")) {
		if (0 == strcasecmp(attrValue, "none")) {
			noiseType = NXT_NoiseType_None;
		}
		else if (0 == strcasecmp(attrValue, "perlin-simplex")) {
			noiseType = NXT_NoiseType_PerlinSimplex;
		}
	}
	else if (0 == strcasecmp(attrName, "noisefunction")) {
		if (0 == strcasecmp(attrValue, "pure")) {
			noiseFunc = NXT_NoiseFunction_Pure;
		}
		else if (0 == strcasecmp(attrValue, "fractal")) {
			noiseFunc = NXT_NoiseFunction_Fractal;
		}
		else if (0 == strcasecmp(attrValue, "turbulence")) {
			noiseFunc = NXT_NoiseFunction_Turbulence;
		}
		else if (0 == strcasecmp(attrValue, "wavefractal")) {
			noiseFunc = NXT_NoiseFunction_WaveFractal;
		}
	}
	else if (0 == strcasecmp(attrName, "start")) {
		NXT_FloatVectorFromString(attrValue, &startTime, 1);
	}
	else if (0 == strcasecmp(attrName, "end")) {
		NXT_FloatVectorFromString(attrValue, &endTime, 1);
	}
	else if (0 == strcasecmp(attrName, "noisebias")) {
		NXT_FloatVectorFromString(attrValue, &noiseBias, 1);
	}
	else if (0 == strcasecmp(attrName, "noisescale")) {
		NXT_FloatVectorFromString(attrValue, &noiseScale, 1);
	}
	else if (0 == strcasecmp(attrName, "noisesamplebias")) {
		NXT_FloatVectorFromString(attrValue, &noiseSampleBias, 1);
	}
	else if (0 == strcasecmp(attrName, "noisesamplescale")) {
		NXT_FloatVectorFromString(attrValue, &noiseSampleScale, 1);
	}
	else if (0 == strcasecmp(attrName, "noiseoctaves")) {
		float octaves = 0.0;
		NXT_FloatVectorFromString(attrValue, &octaves, 1);
		noiseOctaves = (int)octaves;
		if (noiseOctaves < 1)
			noiseOctaves = 1;
		if (noiseOctaves > 32)
			noiseOctaves = 32;
	}
	else {
		// TODO: Error        
	}
}

NXT_Node_AnimatedValue::NXT_Node_AnimatedValue(NXT_Node_AnimatedValue& item){

	id_ = item.id_;
	numUpdateTargets = item.numUpdateTargets;
	allocUpdateTargets = item.allocUpdateTargets;
	numKeyframeSets = item.numKeyframeSets;

	keyframeSet = NULL;
	if (numKeyframeSets > 0){

		keyframeSet = (NXT_KeyFrameSet*)malloc(sizeof(NXT_KeyFrameSet)*numKeyframeSets);
		memset(keyframeSet, 0, sizeof(NXT_KeyFrameSet)*numKeyframeSets);

		for (int i = 0; i < numKeyframeSets; ++i){

			keyframeSet[i] = item.keyframeSet[i];
		}
	}

	element_count = item.element_count;

	for (int i = 0; i < 4; ++i)
		timing[i] = item.timing[i];
	bUseOverallTime = item.bUseOverallTime;
	noiseType = item.noiseType;
	noiseFunc = item.noiseFunc;
	noiseBias = item.noiseBias;
	noiseScale = item.noiseScale;
	noiseSampleBias = item.noiseSampleBias;
	noiseSampleScale = item.noiseSampleScale;
	startTime = item.startTime;
	endTime = item.endTime;
	noiseOctaves = item.noiseOctaves;
	basis = item.basis;
}

void NXT_KeyFrameSet::operator = (NXT_KeyFrameSet& item){

	keyframes = NULL;
	numKeyFrames = item.numKeyFrames;
	if (numKeyFrames > 0){

		keyframes = (NXT_KeyFrame*)malloc(numKeyFrames*sizeof(NXT_KeyFrame));
		for (int i = 0; i < numKeyFrames; ++i){

			keyframes[i] = item.keyframes[i];
		}
	}
	partType = item.partType;
	clipType = item.clipType;
	bDefault = item.bDefault;
}