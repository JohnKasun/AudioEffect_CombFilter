#include "catch.hpp"

#include <memory>

#include "CatchUtil.h"
#include "CombFilterIf.h"

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

TEST_CASE("Feedforward") {
	FAIL();
}

TEST_CASE("Magnitude") {
	FAIL();
}

TEST_CASE("Varying blocks") {
	SECTION("FIR") {
		FAIL();
	}
	SECTION("IIR") {
		FAIL();
	}
}

TEST_CASE("Zero Input") {
	SECTION("FIR") {
		FAIL();
	}
	SECTION("IIR") {
		FAIL();
	}
}