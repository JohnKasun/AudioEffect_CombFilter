#include "CombFilter.h"

CombFilterBase::CombFilterBase(float sampleRate, float maxDelayInSeconds)
{
}

CombFilterBase::~CombFilterBase()
{
}

void CombFilterBase::setParam(CombFilterIf::Param_t param, float value)
{

}

float CombFilterBase::getParam(CombFilterIf::Param_t param) const
{
	return 0.0f;
}

CombFilterFir::CombFilterFir(float sampleRate, float maxDelayInSeconds)
	: CombFilterBase(sampleRate, maxDelayInSeconds)
{
}

CombFilterFir::~CombFilterFir()
{
}

void CombFilterFir::process(const float* inputBuffer, float* outputBuffer, int numSamples)
{

}

CombFilterIir::CombFilterIir(float sampleRate, float maxDelayInSeconds)
	: CombFilterBase(sampleRate, maxDelayInSeconds)
{
}

CombFilterIir::~CombFilterIir()
{
}

void CombFilterIir::process(const float* inputBuffer, float* outputBuffer, int numSamples)
{

}
