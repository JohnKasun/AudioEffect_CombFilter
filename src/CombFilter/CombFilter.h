#pragma once

#include <memory>

#include "ErrorDef.h"
#include "CombFilterIf.h"
#include "RingBuffer.h"
#include "Util.h"

class CombFilterBase {
public:
	CombFilterBase(float sampleRate, float maxDelayInSeconds);
	virtual ~CombFilterBase();

	Error_t setParam(CombFilterIf::Param_t param, float value);
	float getParam(CombFilterIf::Param_t param) const;

	virtual Error_t process(const float* inputBuffer, float* outputBuffer, int numSamples) = 0;
protected:
	std::unique_ptr<CRingBuffer<float>> mDelayLine;
	float mSampleRate = 44100.0f;

	float mParamValues[CombFilterIf::Param_t::numParams]{};
};

class CombFilterFir : public CombFilterBase{
public:
	CombFilterFir(float sampleRate, float maxDelayInSeconds)
		: CombFilterBase(sampleRate, maxDelayInSeconds) {};
	~CombFilterFir() = default;

	Error_t process(const float* inputBuffer, float* outputBuffer, int numSamples) override;
};

class CombFilterIir : public CombFilterBase{
public:
	CombFilterIir(float sampleRate, float maxDelayInSeconds)
		: CombFilterBase(sampleRate, maxDelayInSeconds) {};
	~CombFilterIir() = default;

	Error_t process(const float* inputBuffer, float* outputBuffer, int numSamples) override;
};