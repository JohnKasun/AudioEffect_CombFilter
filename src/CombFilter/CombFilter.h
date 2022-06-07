#pragma once

#include "ErrorDef.h"
#include "CombFilterIf.h"
#include "RingBuffer.h"

class CombFilterBase {
public:
	CombFilterBase(float sampleRate, float maxDelayInSeconds);
	virtual ~CombFilterBase();

	void setParam(CombFilterIf::Param_t param, float value);
	float getParam(CombFilterIf::Param_t param) const;

	virtual void process(const float* inputBuffer, float* outputBuffer, int numSamples) = 0;
protected:

	float mParamValues[CombFilterIf::Param_t::numParams]{};
	float mSampleRate = 44100.0f;
};

class CombFilterFir : public CombFilterBase{
public:
	CombFilterFir(float sampleRate, float maxDelayInSeconds);
	~CombFilterFir();

	void process(const float* inputBuffer, float* outputBuffer, int numSamples) override;
};

class CombFilterIir : public CombFilterBase{
public:
	CombFilterIir(float sampleRate, float maxDelayInSeconds);
	~CombFilterIir();

	void process(const float* inputBuffer, float* outputBuffer, int numSamples) override;
};