#include "catch.hpp"

#include <memory>

#include "CatchUtil.h"
#include "Synthesis.h"
#include "CombFilter.h"
#include "Vector.h"

TEST_CASE("Error Checking") {
	std::unique_ptr<CombFilter> combFilter;
	combFilter.reset(new CombFilter());
	REQUIRE(combFilter->init(-1.0f) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->init(44100.0f) == Error_t::kNoError);
	REQUIRE(combFilter->setFilterType(CombFilter::FilterType_t::fir) == Error_t::kNoError);
	REQUIRE(combFilter->setFilterType(CombFilter::FilterType_t::iir) == Error_t::kNoError);
	REQUIRE(combFilter->setFilterType(CombFilter::FilterType_t::numFilterTypes) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->setParam(CombFilter::Param_t::gain, -1.1f) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->setParam(CombFilter::Param_t::gain, 1.1f) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->setParam(CombFilter::Param_t::gain, 0.5f) == Error_t::kNoError);
	REQUIRE(combFilter->setParam(CombFilter::Param_t::delayInSec, 11.0f) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->setParam(CombFilter::Param_t::delayInSec, -4.0f) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->setParam(CombFilter::Param_t::delayInSec, 0.2f) == Error_t::kNoError);
	REQUIRE(combFilter->getParam(CombFilter::Param_t::delayInSec) == 0.2f);
	REQUIRE(combFilter->getParam(CombFilter::Param_t::gain) == 0.5f);
	
	int numSamples = 100;
	float* inputBuffer = nullptr;
	float* outputBuffer = new float[numSamples] {};
	REQUIRE(combFilter->process(inputBuffer, outputBuffer, numSamples) == Error_t::kMemError);
	REQUIRE(combFilter->process(outputBuffer, inputBuffer, numSamples) == Error_t::kMemError);
	inputBuffer = new float[numSamples] {};
	REQUIRE(combFilter->process(inputBuffer, outputBuffer, -1.0f) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->process(inputBuffer, outputBuffer, numSamples) == Error_t::kNoError);
	combFilter->reset();
	REQUIRE(combFilter->process(inputBuffer, outputBuffer, numSamples) == Error_t::kNotInitializedError);
	delete[] inputBuffer;
	delete[] outputBuffer;
	inputBuffer = nullptr;
	outputBuffer = nullptr;
	combFilter.reset();
}

TEST_CASE("Correct Output") {
	std::unique_ptr<CombFilter> combFilter;
	float* inputBuffer;
	float* outputBuffer;
	float* groundBuffer;
	const int numSamples = 10000;
	const float sampleRate = 44100.0f;

	combFilter.reset(new CombFilter());
	inputBuffer = new float[numSamples] {};
	outputBuffer = new float[numSamples] {};
	groundBuffer = new float[numSamples] {};

	combFilter->init(sampleRate);

	SECTION("Feedforward") {
		const float freq = 1.0f;
		CSynthesis::generateSine(inputBuffer, freq, sampleRate, numSamples);
		float delayInSec = 1.0f / freq;
		combFilter->setFilterType(CombFilter::FilterType_t::fir);
		combFilter->setParam(CombFilter::Param_t::gain, -1.0f);
		combFilter->setParam(CombFilter::Param_t::delayInSec, delayInSec);
		combFilter->process(inputBuffer, outputBuffer, numSamples);

		int delayInSamp = CUtil::float2int<int>(delayInSec * sampleRate);
		CatchUtil::compare(outputBuffer + delayInSamp, groundBuffer, numSamples - delayInSamp);
	}

	SECTION("Magnitude") {
		const float freq = 1.0f;
		const float delayInSec = 1.0f / freq;
		const int delayInSamp = CUtil::float2int<int>(delayInSec * sampleRate);

		CSynthesis::generateSine(inputBuffer, freq, sampleRate, numSamples, 1);
		CVectorFloat::addC_I(inputBuffer, 2, numSamples);
		combFilter->setFilterType(CombFilter::FilterType_t::iir);
		combFilter->setParam(CombFilter::Param_t::delayInSec, delayInSec);
		combFilter->setParam(CombFilter::Param_t::gain, 1.0f);
		combFilter->process(inputBuffer, outputBuffer, numSamples);

		CVectorFloat::div_I(outputBuffer, inputBuffer, numSamples);
		int value = 0;
		for (int i = 0; i < numSamples; i++) {
			if (i % delayInSamp == 0)
				value++;
			groundBuffer[i] = value;
		}
		CatchUtil::compare(outputBuffer, groundBuffer, numSamples);
	}

	SECTION("Zero Input") {
		CVectorFloat::setZero(inputBuffer, numSamples);
		SECTION("FIR") {
			combFilter->setFilterType(CombFilter::FilterType_t::fir);
			combFilter->setParam(CombFilter::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilter::Param_t::gain, 0.5);
			combFilter->process(inputBuffer, outputBuffer, numSamples);
			CatchUtil::compare(inputBuffer, outputBuffer, numSamples);
		}
		SECTION("IIR") {
			combFilter->setFilterType(CombFilter::FilterType_t::iir);
			combFilter->setParam(CombFilter::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilter::Param_t::gain, 0.5);
			combFilter->process(inputBuffer, outputBuffer, numSamples);
			CatchUtil::compare(inputBuffer, outputBuffer, numSamples);
		}
	}

	SECTION("Varying Input Block Sizes") {
		CSynthesis::generateSine(inputBuffer, 100, sampleRate, numSamples);
		SECTION("FIR") {
			combFilter->setFilterType(CombFilter::FilterType_t::fir);
			combFilter->setParam(CombFilter::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilter::Param_t::gain, 0.5);
			combFilter->process(inputBuffer, groundBuffer, numSamples);
			combFilter->reset();

			combFilter->init(sampleRate);
			combFilter->setFilterType(CombFilter::FilterType_t::fir);
			combFilter->setParam(CombFilter::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilter::Param_t::gain, 0.5);

			std::vector<int> blockSizes{ 10, 100, 1000, 5460, 230, 3200 };
			int offset = 0;
			for (int& blockSize : blockSizes) {
				combFilter->process(&inputBuffer[offset], &outputBuffer[offset], blockSize);
				offset += blockSize;
			}
			CatchUtil::compare(outputBuffer, groundBuffer, numSamples);
		}
		SECTION("IIR") {
			combFilter->setFilterType(CombFilter::FilterType_t::iir);
			combFilter->setParam(CombFilter::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilter::Param_t::gain, 0.5);
			combFilter->process(inputBuffer, groundBuffer, numSamples);
			combFilter->reset();

			combFilter->init(sampleRate);
			combFilter->setFilterType(CombFilter::FilterType_t::iir);
			combFilter->setParam(CombFilter::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilter::Param_t::gain, 0.5);

			std::vector<int> blockSizes{ 10, 100, 1000, 5460, 230, 3200 };
			int offset = 0;
			for (int& blockSize : blockSizes) {
				combFilter->process(&inputBuffer[offset], &outputBuffer[offset], blockSize);
				offset += blockSize;
			}
			CatchUtil::compare(outputBuffer, groundBuffer, numSamples);
		}
	}

	combFilter.reset();
	delete[] inputBuffer;
	delete[] outputBuffer;
	delete[] groundBuffer;
}
