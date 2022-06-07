#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "AudioFileIf.h"
#include "ErrorDef.h"
#include "CombFilterIf.h"

using std::string;
using std::vector;
using std::unique_ptr;
using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
	static const float maxDelayInSeconds = 10.0f;
	static const int fileBlockSize = 1023;

	CAudioFileIf* audioFileIn  = nullptr;
	CAudioFileIf* audioFileOut = nullptr;
	CAudioFileIf::FileSpec_t fileSpec;

	float** inputAudioData  = nullptr;
	float** outputAudioData = nullptr;
	std::vector<std::unique_ptr<CombFilterIf>> combFilter;

	string inputFilePath{};
	string outputFilePath{};
	CombFilterIf::FilterType_t filterType{};
	float gain{};
	float delayInSec{};

	if (argc < 6){
		cout << "Incorrect number of arguments!" << endl;
		return -1;
	}
	
	inputFilePath  = argv[1];
	outputFilePath = argv[2];
	if (argv[3] == "fir")
		filterType = CombFilterIf::FilterType_t::fir;
	else if (argv[3] == "iir")
		filterType = CombFilterIf::FilterType_t::iir;
	gain = atof(argv[4]);
	delayInSec = atof(argv[5]);

	// Open Audio Files
	CAudioFileIf::create(audioFileIn);
	audioFileIn->openFile(inputFilePath, CAudioFileIf::FileIoType_t::kFileRead);
	if (!audioFileIn->isOpen()) {
		std::cout << "Couldn't open input file..." << std::endl;
		CAudioFileIf::destroy(audioFileIn);
		return -1;
	}
	audioFileIn->getFileSpec(fileSpec);

	CAudioFileIf::create(audioFileOut);
	audioFileOut->openFile(outputFilePath, CAudioFileIf::FileIoType_t::kFileWrite, &fileSpec);
	if (!audioFileOut->isOpen()) {
		std::cout << "Couldn't open output file..." << std::endl;
		CAudioFileIf::destroy(audioFileOut);
		return -1;
	}

	// Create instance
	for (int c = 0; c < fileSpec.iNumChannels; c++) {
		combFilter.emplace_back(new CombFilterIf());
	}

	// Initialize instance
	for (int c = 0; c < combFilter.size(); c++) {
		if (combFilter[c]->init(filterType, fileSpec.fSampleRateInHz, maxDelayInSeconds) != Error_t::kNoError
			|| combFilter[c]->setParam(CombFilterIf::Param_t::gain, gain) != Error_t::kNoError
			|| combFilter[c]->setParam(CombFilterIf::Param_t::delayInSec, delayInSec) != Error_t::kNoError) {
			std::cout << "Invalid Parameters..." << std::endl;
			combFilter[c].reset();
			CAudioFileIf::destroy(audioFileOut);
			CAudioFileIf::destroy(audioFileIn);
			return -1;
		}
	}

	// Allocate memory
	inputAudioData  = new float* [fileSpec.iNumChannels]{};
	outputAudioData = new float* [fileSpec.iNumChannels]{};
	for (int c = 0; c < fileSpec.iNumChannels; c++){
		inputAudioData[c]  = new float[fileBlockSize]{};
		outputAudioData[c] = new float[fileBlockSize]{};
	}

	// Process
	long long iNumFrames = fileBlockSize;
	while (!audioFileIn->isEof()){
		audioFileIn->readData(inputAudioData, iNumFrames);
		for (int c = 0; c < combFilter.size(); c++) {
			combFilter[c]->process(inputAudioData[c], outputAudioData[c], iNumFrames);
		}
		audioFileOut->writeData(outputAudioData, iNumFrames);
	}

	// Clean-up
	for (int c = 0; c < fileSpec.iNumChannels; c++) {
		delete[] inputAudioData[c];
		delete[] outputAudioData[c];
	}
	delete[] inputAudioData;
	delete[] outputAudioData;

	CAudioFileIf::destroy(audioFileOut);
	CAudioFileIf::destroy(audioFileOut);
	combFilter.clear();

	return 0;
}