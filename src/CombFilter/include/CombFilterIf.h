#pragma once

#include "ErrorDef.h";

class CombFilterBase;

class CombFilterIf {
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

	CombFilterIf();
	~CombFilterIf();

	Error_t init(FilterType_t filterType, float sampleRate);
	Error_t reset();

	Error_t setParam(Param_t param, float value);
	float getParam(Param_t param) const;

	Error_t process(const float* inputBuffer, float* outputBuffer, int numSamples);
private:
	bool isInParamRange(CombFilterIf::Param_t param, float value) const;
	float mParamRanges[CombFilterIf::Param_t::numParams][2]{};

	CombFilterBase* mCombFilter = nullptr;
	bool mIsInitialized = false;
};