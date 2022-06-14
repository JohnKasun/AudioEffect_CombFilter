#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <vector>

#include "AudioFileIf.h"
#include "ErrorDef.h"
#include "CombFilter.h"

using std::string;
using std::vector;
using std::unique_ptr;
using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
	static const int fileBlockSize = 1023;

	CAudioFileIf* audioFileIn  = nullptr;
	CAudioFileIf* audioFileOut = nullptr;
	CAudioFileIf::FileSpec_t fileSpec;

	float** inputAudioData  = nullptr;
	float** outputAudioData = nullptr;
	std::vector<std::unique_ptr<CombFilter>> combFilter;

	string inputFilePath{};
	string outputFilePath{};
	CombFilter::FilterType_t filterType{};
	float gain{};
	float delayInSec{};

	if (argc == 1) {
		cout << "-- CombFilter Effect -- " << endl;
		cout << endl;

		cout << std::setw(20) << std::right << "Enter a path to a WAV file: ";
		std::cin >> inputFilePath;
		CAudioFileIf::create(audioFileIn);
		audioFileIn->openFile(inputFilePath, CAudioFileIf::FileIoType_t::kFileRead);
		if (!audioFileIn->isOpen()) {
			cout << "Couldn't open input file..." << endl;
			CAudioFileIf::destroy(audioFileIn);
			return -1;
		}
		audioFileIn->getFileSpec(fileSpec);

		cout << std::setw(20) << "Enter a path to a WAV file: ";
		std::cin >> outputFilePath;
		CAudioFileIf::create(audioFileOut);
		audioFileOut->openFile(outputFilePath, CAudioFileIf::FileIoType_t::kFileWrite);
		if (!audioFileOut->isOpen()) {
			cout << "Couldn't open output file..." << endl;
			CAudioFileIf::destroy(audioFileIn);
			CAudioFileIf::destroy(audioFileOut);
			return -1;
		}

		for (int c = 0; c < fileSpec.iNumChannels; c++) {
			combFilter.emplace_back(new CombFilter(fileSpec.fSampleRateInHz));
		}

		cout << std::setw(20) << "Enter a filter type (fir or iir): ";
		string filterInput;
		std::cin >> filterInput;
		if (strcmpi(filterInput.c_str(), "fir"))
			filterType = CombFilter::FilterType_t::fir;
		else if (strcmpi(filterInput.c_str(), "iir"))
			filterType = CombFilter::FilterType_t::iir;
		else {
			cout << "Invalid filter type..." << endl;
			CAudioFileIf::destroy(audioFileIn);
			CAudioFileIf::destroy(audioFileOut);
			return -1;
		}
		for (int c = 0; c < fileSpec.iNumChannels; c++) {
			combFilter[c]->setFilterType(filterType);
		}

		cout << std::setw(20) << "Enter a gain parameter (-1.0 - 1.0): ";
		std::cin >> gain;
		for (int c = 0; c < fileSpec.iNumChannels; c++) {
			if (combFilter[c]->setParam(CombFilter::gain, gain) != Error_t::kNoError) {
				cout << "Invalid parameter value..." << endl;
				CAudioFileIf::destroy(audioFileIn);
				CAudioFileIf::destroy(audioFileOut);
				return -1;
			}
		}

		cout << std::setw(20) << "Enter a delay parameter in seconds (0.01 - 10.0): ";
		std::cin >> delayInSec;
		for (int c = 0; c < fileSpec.iNumChannels; c++) {
			if (combFilter[c]->setParam(CombFilter::delayInSec, gain) != Error_t::kNoError) {
				cout << "Invalid parameter value..." << endl;
				CAudioFileIf::destroy(audioFileIn);
				CAudioFileIf::destroy(audioFileOut);
				return -1;
			}
		}

	}
	else if (argc == 6) {
		inputFilePath = argv[1];
		outputFilePath = argv[2];
		if (strcmpi(argv[3], "fir"))
			filterType = CombFilter::FilterType_t::fir;
		else if (strcmpi(argv[3], "iir"))
			filterType = CombFilter::FilterType_t::iir;
		else {
			cout << "Invalid filter type..." << endl;
			return -1;
		}
		gain = atof(argv[4]);
		delayInSec = atof(argv[5]);

		// Open Audio Files
		CAudioFileIf::create(audioFileIn);
		audioFileIn->openFile(inputFilePath, CAudioFileIf::FileIoType_t::kFileRead);
		if (!audioFileIn->isOpen()) {
			cout << "Couldn't open input file..." << endl;
			CAudioFileIf::destroy(audioFileIn);
			return -1;
		}
		audioFileIn->getFileSpec(fileSpec);

		CAudioFileIf::create(audioFileOut);
		audioFileOut->openFile(outputFilePath, CAudioFileIf::FileIoType_t::kFileWrite, &fileSpec);
		if (!audioFileOut->isOpen()) {
			cout << "Couldn't open output file..." << endl;
			CAudioFileIf::destroy(audioFileOut);
			return -1;
		}

		// Create instance
		for (int c = 0; c < fileSpec.iNumChannels; c++) {
			combFilter.emplace_back(new CombFilter(fileSpec.fSampleRateInHz));
		}

		// Initialize instance
		for (int c = 0; c < combFilter.size(); c++) {
			if (combFilter[c]->setFilterType(filterType) != Error_t::kNoError
				|| combFilter[c]->setParam(CombFilter::Param_t::gain, gain) != Error_t::kNoError
				|| combFilter[c]->setParam(CombFilter::Param_t::delayInSec, delayInSec) != Error_t::kNoError) {
				cout << "Invalid Parameters..." << endl;
				cout << "The Ranges for the parameters are as follows: " << endl;
				cout << "\tFilter Type: 'fir' or 'iir'" << endl;
				cout << "\tGain: -1 to 1" << endl;
				cout << "\tDelay: 0.01 to 10.0" << endl;
				combFilter[c].reset();
				CAudioFileIf::destroy(audioFileOut);
				CAudioFileIf::destroy(audioFileIn);
				return -1;
			}
		}
	}
	else {
		cout << "Incorrect number of arguments!" << endl;
		return -1;
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