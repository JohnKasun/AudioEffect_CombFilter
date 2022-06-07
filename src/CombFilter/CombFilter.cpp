#include "CombFilter.h"

CombFilterBase::CombFilterBase(float sampleRate, float maxDelayInSeconds)
{
	mSampleRate = sampleRate;
	mDelayLine.reset(new CRingBuffer<float>(maxDelayInSeconds * mSampleRate));
}

CombFilterBase::~CombFilterBase()
{
	mDelayLine.reset();
	mSampleRate = 0.0f;
}

Error_t CombFilterBase::setParam(CombFilterIf::Param_t param, float value)
{
	if (param == CombFilterIf::Param_t::delayInSec) {
		mDelayLine->setWriteIdx(mDelayLine->getReadIdx() + CUtil::float2int<int>(value * mSampleRate));
	}
	mParamValues[param] = value;
	return Error_t::kNoError;
}

float CombFilterBase::getParam(CombFilterIf::Param_t param) const
{
	return mParamValues[param];
}

Error_t CombFilterFir::process(const float* inputBuffer, float* outputBuffer, int numSamples)
{
	for (int i = 0; i < numSamples; i++) {
		outputBuffer[i] = inputBuffer[i] + mParamValues[CombFilterIf::Param_t::gain] * mDelayLine->getPostInc();
		mDelayLine->putPostInc(inputBuffer[i]);
	}
	return Error_t::kNoError;
}

Error_t CombFilterIir::process(const float* inputBuffer, float* outputBuffer, int numSamples)
{
	for (int i = 0; i < numSamples; i++) {
		outputBuffer[i] = inputBuffer[i] + mParamValues[CombFilterIf::Param_t::gain] * mDelayLine->getPostInc();
		mDelayLine->putPostInc(outputBuffer[i]);
	}
	return Error_t::kNoError;
}
