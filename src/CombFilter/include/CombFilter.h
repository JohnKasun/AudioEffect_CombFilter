#pragma once

#include <memory>

#include "ErrorDef.h"
#include "RingBuffer.h"
#include "Util.h"

class CombFilter {
public:

	enum FilterType_t {
		fir,
		iir,

		numFilterTypes
	};

	enum Param_t {
		gain,
		delayInSec,

		numParams
	};

	CombFilter();
	~CombFilter();

	Error_t init(float sampleRate);
	Error_t reset();
	
	Error_t setParam(CombFilter::Param_t param, float value);
	float getParam(CombFilter::Param_t param) const;

	Error_t setFilterType(CombFilter::FilterType_t filterType);
	CombFilter::FilterType_t getFiltertType() const;

	Error_t process(const float* inputBuffer, float* outputBuffer, int numSamples);
private:
	bool isInParamRange(CombFilter::Param_t param, float value) const;
	float mParamRanges[CombFilter::Param_t::numParams][2]{};
	float mParamValues[CombFilter::Param_t::numParams]{};

	FilterType_t mFilterType = fir;
	std::unique_ptr<CRingBuffer<float>> mDelayLine = nullptr;
	float mSampleRate = 44100.0f;
	bool mIsInitialized = false;
};