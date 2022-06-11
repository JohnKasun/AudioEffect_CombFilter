#include "CombFilter.h"

CombFilter::CombFilter()
{
	mParamRanges[gain][0] = -1.0f;
	mParamRanges[gain][1] = 1.0f;
	mParamRanges[delayInSec][0] = 0.0f;
	mParamRanges[delayInSec][1] = 10.0f;
}

CombFilter::~CombFilter()
{
	reset();
}

Error_t CombFilter::init(float sampleRate)
{
	if (sampleRate <= 0.0f)
		return Error_t::kFunctionInvalidArgsError;

	reset();

	mSampleRate = sampleRate;
	mDelayLine.reset(new CRingBuffer<float>(CUtil::float2int<int>(mParamRanges[delayInSec][1] * mSampleRate)));
	mIsInitialized = true;
	return Error_t::kNoError;
}

Error_t CombFilter::reset()
{
	if (mIsInitialized) {
		for (int i = 0; i < numParams; i++) {
			mParamValues[i] = 0.0f;
		}
		mFilterType = fir;
		mSampleRate = 44100.0f;
		mDelayLine.reset();
		mIsInitialized = false;
	}
	return Error_t::kNoError;
}

Error_t CombFilter::setParam(CombFilter::Param_t param, float value)
{
	if (!mIsInitialized)
		return Error_t::kNotInitializedError;
	if (!isInParamRange(param, value))
		return Error_t::kFunctionInvalidArgsError;

	if (param == CombFilter::Param_t::delayInSec) {
		mDelayLine->setWriteIdx(mDelayLine->getReadIdx() + CUtil::float2int<int>(value * mSampleRate));
	}
	mParamValues[param] = value;
	return Error_t::kNoError;
}

float CombFilter::getParam(CombFilter::Param_t param) const
{
	return mParamValues[param];
}

Error_t CombFilter::setFilterType(CombFilter::FilterType_t filterType)
{
	if (filterType == numFilterTypes)
		return Error_t::kFunctionInvalidArgsError;

	mFilterType = filterType;
	return Error_t::kNoError;
}

CombFilter::FilterType_t CombFilter::getFiltertType() const
{
	return mFilterType;
}

Error_t CombFilter::process(const float* inputBuffer, float* outputBuffer, int numSamples)
{
	if (!mIsInitialized)
		return Error_t::kNotInitializedError;
	if (!inputBuffer || !outputBuffer)
		return Error_t::kMemError;
	if (numSamples < 0)
		return Error_t::kFunctionInvalidArgsError;

	for (int i = 0; i < numSamples; i++) {
		outputBuffer[i] = inputBuffer[i] + mParamValues[CombFilter::Param_t::gain] * mDelayLine->getPostInc();
		(mFilterType == fir) ? mDelayLine->putPostInc(inputBuffer[i]) : mDelayLine->putPostInc(outputBuffer[i]);
	}
	return Error_t::kNoError;
}

bool CombFilter::isInParamRange(CombFilter::Param_t param, float value) const
{
	return (mParamRanges[param][0] <= value && value <= mParamRanges[param][1]);
}
