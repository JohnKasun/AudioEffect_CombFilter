#include "catch.hpp"

#include <memory>

#include "CatchUtil.h"
#include "Synthesis.h"
#include "CombFilterIf.h"
#include "Vector.h"

TEST_CASE("Error Checking") {
	std::unique_ptr<CombFilterIf> combFilter;
	combFilter.reset(new CombFilterIf());
	SECTION("FIR") {
		REQUIRE(combFilter->init(CombFilterIf::FilterType_t::fir, -1.0f) == Error_t::kFunctionInvalidArgsError);
		REQUIRE(combFilter->init(CombFilterIf::FilterType_t::fir, 44100.0f) == Error_t::kNoError);
	}
	SECTION("IIR") {
		REQUIRE(combFilter->init(CombFilterIf::FilterType_t::iir, -1.0f) == Error_t::kFunctionInvalidArgsError);
		REQUIRE(combFilter->init(CombFilterIf::FilterType_t::iir, 44100.0f) == Error_t::kNoError);
	}
	REQUIRE(combFilter->setParam(CombFilterIf::Param_t::gain, -1.1f) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->setParam(CombFilterIf::Param_t::gain, 1.1f) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->setParam(CombFilterIf::Param_t::gain, 0.5f) == Error_t::kNoError);
	REQUIRE(combFilter->setParam(CombFilterIf::Param_t::delayInSec, 11.0f) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->setParam(CombFilterIf::Param_t::delayInSec, -4.0f) == Error_t::kFunctionInvalidArgsError);
	REQUIRE(combFilter->setParam(CombFilterIf::Param_t::delayInSec, 0.2f) == Error_t::kNoError);
	REQUIRE(combFilter->getParam(CombFilterIf::Param_t::delayInSec) == 0.2f);
	REQUIRE(combFilter->getParam(CombFilterIf::Param_t::gain) == 0.5f);
	
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
	std::unique_ptr<CombFilterIf> combFilter;
	float* inputBuffer;
	float* outputBuffer;
	float* groundBuffer;
	const int numSamples = 1000;

	combFilter.reset(new CombFilterIf());
	inputBuffer = new float[numSamples] {};
	outputBuffer = new float[numSamples] {};
	groundBuffer = new float[numSamples] {};

	SECTION("Feedforward") {
		const float sampleRate = 44100.0f;
		const float freq = 100.0f;

		CSynthesis::generateSine(inputBuffer, freq, sampleRate, numSamples);
		float delayInSec = 1.0f / freq;
		combFilter->init(CombFilterIf::FilterType_t::fir, sampleRate);
		combFilter->setParam(CombFilterIf::Param_t::gain, -1.0f);
		combFilter->setParam(CombFilterIf::Param_t::delayInSec, delayInSec);
		combFilter->process(inputBuffer, outputBuffer, numSamples);

		int delayInSamp = CUtil::float2int<int>(delayInSec * sampleRate);
		CatchUtil::compare(outputBuffer + delayInSamp, groundBuffer, numSamples - delayInSamp);
	}

	SECTION("Magnitude") {
		const float sampleRate = 44100.0f;
		const float freq = 100.0f;
		const float delayInSec = 1.0f / freq;
		const int delayInSamp = CUtil::float2int<int>(delayInSec * sampleRate);

		CSynthesis::generateSine(inputBuffer, freq, sampleRate, numSamples, 1);
		CVectorFloat::addC_I(inputBuffer, 2, numSamples);
		combFilter->init(CombFilterIf::FilterType_t::iir, sampleRate);
		combFilter->setParam(CombFilterIf::Param_t::delayInSec, delayInSec);
		combFilter->setParam(CombFilterIf::Param_t::gain, 1.0f);
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
		const float sampleRate = 44100.0f;

		CVectorFloat::setZero(inputBuffer, numSamples);
		SECTION("FIR") {
			combFilter->init(CombFilterIf::FilterType_t::fir, sampleRate);
			combFilter->setParam(CombFilterIf::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilterIf::Param_t::gain, 0.5);
			combFilter->process(inputBuffer, outputBuffer, numSamples);
			CatchUtil::compare(inputBuffer, outputBuffer, numSamples);
		}
		SECTION("IIR") {
			combFilter->init(CombFilterIf::FilterType_t::iir, sampleRate);
			combFilter->setParam(CombFilterIf::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilterIf::Param_t::gain, 0.5);
			combFilter->process(inputBuffer, outputBuffer, numSamples);
			CatchUtil::compare(inputBuffer, outputBuffer, numSamples);
		}
	}

	SECTION("Varying Input Block Sizes") {
		const float sampleRate = 44100.0f;

		CSynthesis::generateSine(inputBuffer, 100, sampleRate, numSamples);
		SECTION("FIR") {
			combFilter->init(CombFilterIf::FilterType_t::fir, sampleRate);
			combFilter->setParam(CombFilterIf::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilterIf::Param_t::gain, 0.5);
			combFilter->process(inputBuffer, groundBuffer, numSamples);
			combFilter->reset();

			combFilter->init(CombFilterIf::FilterType_t::fir, sampleRate);
			combFilter->setParam(CombFilterIf::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilterIf::Param_t::gain, 0.5);

			std::vector<int> blockSizes{ 1, 10, 100, 546, 23, 320 };
			int offset = 0;
			for (int& blockSize : blockSizes) {
				combFilter->process(&inputBuffer[offset], &outputBuffer[offset], blockSize);
				offset += blockSize;
			}
			CatchUtil::compare(outputBuffer, groundBuffer, numSamples);
		}
		SECTION("IIR") {
			combFilter->init(CombFilterIf::FilterType_t::iir, sampleRate);
			combFilter->setParam(CombFilterIf::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilterIf::Param_t::gain, 0.5);
			combFilter->process(inputBuffer, groundBuffer, numSamples);
			combFilter->reset();

			combFilter->init(CombFilterIf::FilterType_t::iir, sampleRate);
			combFilter->setParam(CombFilterIf::Param_t::delayInSec, 0.002);
			combFilter->setParam(CombFilterIf::Param_t::gain, 0.5);

			std::vector<int> blockSizes{ 1, 10, 100, 546, 23, 320 };
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
