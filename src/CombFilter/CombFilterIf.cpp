
#include "CombFilterIf.h"
#include "CombFilter.h"

CombFilterIf::CombFilterIf()
{
	mParamRanges[gain][0] = -1.0f;
	mParamRanges[gain][1] = 1.0f;
	mParamRanges[delayInSec][0] = 0.0f;
	mParamRanges[delayInSec][1] = 10.0f;
}

CombFilterIf::~CombFilterIf()
{
	reset();
}

Error_t CombFilterIf::init(FilterType_t filterType, float sampleRate)
{
	if (sampleRate <= 0.0f)
		return Error_t::kFunctionInvalidArgsError;

	reset();

	switch (filterType) {
	case fir:
		mCombFilter = new CombFilterFir(sampleRate, mParamRanges[delayInSec][1]);
		break;
	case iir:
		mCombFilter = new CombFilterIir(sampleRate, mParamRanges[delayInSec][1]);
		break;
	default:
		return Error_t::kFunctionInvalidArgsError;
	}
	mIsInitialized = true;
	return Error_t::kNoError;
}

Error_t CombFilterIf::reset()
{
	if (mIsInitialized) {
		delete mCombFilter;
		mCombFilter = nullptr;
		mIsInitialized = false;
	}
	return Error_t::kNoError;
}

Error_t CombFilterIf::setParam(Param_t param, float value)
{
	if (!mIsInitialized)
		return Error_t::kNotInitializedError;
	if (!isInParamRange(param, value))
		return Error_t::kFunctionInvalidArgsError;

	return mCombFilter->setParam(param, value);
}

float CombFilterIf::getParam(Param_t param) const
{
	if (!mIsInitialized)
		return 0.0f;

	return mCombFilter->getParam(param);
}

Error_t CombFilterIf::process(const float* inputBuffer, float* outputBuffer, int numSamples)
{
	if (!mIsInitialized)
		return Error_t::kNotInitializedError;
	if (!inputBuffer || !outputBuffer)
		return Error_t::kMemError;
	if (numSamples < 0)
		return Error_t::kFunctionInvalidArgsError;

	return mCombFilter->process(inputBuffer, outputBuffer, numSamples);
}

bool CombFilterIf::isInParamRange(CombFilterIf::Param_t param, float value) const
{
	return (mParamRanges[param][0] <= value && value <= mParamRanges[param][1]);
}
