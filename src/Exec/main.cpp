#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>

#include "AudioFileIf.h"
#include "ErrorDef.h"
#include "CombFilter.h"
#include "Exception.h"

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

	try {
		if (argc == 1) {
			cout << "-- CombFilter Effect -- " << endl;
			cout << endl;

			///////////////////
			/// Open Input File
			const int labelFormat = 30;
			cout << std::setw(labelFormat) << std::right << "Input WAV file: ";
			std::cin >> inputFilePath;
			CAudioFileIf::create(audioFileIn);
			audioFileIn->openFile(inputFilePath, CAudioFileIf::FileIoType_t::kFileRead);
			if (!audioFileIn->isOpen()) {
				throw Exception("Couldn't open input file...");
			}
			audioFileIn->getFileSpec(fileSpec);


			////////////////////
			/// Open Output File
			cout << std::setw(labelFormat) << "Output WAV file: ";
			std::cin >> outputFilePath;
			CAudioFileIf::create(audioFileOut);
			audioFileOut->openFile(outputFilePath, CAudioFileIf::FileIoType_t::kFileWrite, &fileSpec);
			if (!audioFileOut->isOpen()) {
				throw Exception("Couldn't open output file...");
			}

			////////////////////
			/// Create Instances
			for (int c = 0; c < fileSpec.iNumChannels; c++) {
				combFilter.emplace_back(new CombFilter(fileSpec.fSampleRateInHz));
			}

			///////////////////
			/// Set Filter Type
			cout << std::setw(labelFormat) << "Filter Type (fir or iir): ";
			string filterInput;
			std::cin >> filterInput;
			CUtil::toLower(filterInput);
			if (filterInput == "fir") {
				filterType = CombFilter::FilterType_t::fir;
			}
			else if (filterInput == "iir") {
				filterType = CombFilter::FilterType_t::iir;
			}
			else {
				throw Exception("Invalid Filter Type");
			}
			for (int c = 0; c < fileSpec.iNumChannels; c++) {
				combFilter[c]->setFilterType(filterType);
			}

			////////////
			/// Set Gain
			cout << std::setw(labelFormat) << "Gain (-1.0 - 1.0): ";
			std::cin >> gain;
			for (int c = 0; c < fileSpec.iNumChannels; c++) {
				if (combFilter[c]->setParam(CombFilter::gain, gain) != Error_t::kNoError) {
					throw Exception("Invalid gain value...");
				}
			}

			/////////////
			/// SET Delay
			cout << std::setw(labelFormat) << "Delay (0.01 - 10.0): ";
			std::cin >> delayInSec;
			for (int c = 0; c < fileSpec.iNumChannels; c++) {
				if (combFilter[c]->setParam(CombFilter::delayInSec, delayInSec) != Error_t::kNoError) {
					throw Exception("Invalid delay value...");
				}
			}

		}
		else if (argc == 6) {
			//////////////////
			/// Get Parameters
			inputFilePath = argv[1];
			outputFilePath = argv[2];
			string filterInput = argv[3];
			CUtil::toLower(filterInput);
			if (filterInput == "fir") {
				filterType = CombFilter::FilterType_t::fir;
			}
			else if (filterInput == "iir") {
				filterType = CombFilter::FilterType_t::iir;
			}
			else {
				throw Exception("Invalid filter type...");
			}
			gain = atof(argv[4]);
			delayInSec = atof(argv[5]);

			///////////////////
			/// Open Input File
			CAudioFileIf::create(audioFileIn);
			audioFileIn->openFile(inputFilePath, CAudioFileIf::FileIoType_t::kFileRead);
			if (!audioFileIn->isOpen()) {
				cout << "Couldn't open input file..." << endl;
				CAudioFileIf::destroy(audioFileIn);
				return -1;
			}
			audioFileIn->getFileSpec(fileSpec);

			////////////////////
			/// Open Output File
			CAudioFileIf::create(audioFileOut);
			audioFileOut->openFile(outputFilePath, CAudioFileIf::FileIoType_t::kFileWrite, &fileSpec);
			if (!audioFileOut->isOpen()) {
				cout << "Couldn't open output file..." << endl;
				CAudioFileIf::destroy(audioFileIn);
				CAudioFileIf::destroy(audioFileOut);
				return -1;
			}

			////////////////////
			/// Create Instances
			for (int c = 0; c < fileSpec.iNumChannels; c++) {
				combFilter.emplace_back(new CombFilter(fileSpec.fSampleRateInHz));
			}

			//////////////////
			/// Set Parameters
			for (int c = 0; c < combFilter.size(); c++) {
				if (combFilter[c]->setFilterType(filterType) != Error_t::kNoError) {
					throw Exception("Invalid filter type...");
				}
				if (combFilter[c]->setParam(CombFilter::Param_t::gain, gain) != Error_t::kNoError) {
					throw Exception("Invalid gain value...");
				}
				if (combFilter[c]->setParam(CombFilter::Param_t::delayInSec, delayInSec) != Error_t::kNoError) {
					throw Exception("Invalid delay value...");
				}
			}
		}
		else {
			throw Exception("Incorrect number of arguments");
		}
	}
	catch (Exception& ex) {
		cout << "\n--------------------------" << endl;
		cout << ex.what() << endl;
		cout << "--------------------------" << endl;
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

	cout << endl;
	cout << "Done" << endl;

	// Clean-up
	for (int c = 0; c < fileSpec.iNumChannels; c++) {
		delete[] inputAudioData[c];
		delete[] outputAudioData[c];
	}
	delete[] inputAudioData;
	delete[] outputAudioData;

	CAudioFileIf::destroy(audioFileOut);
	CAudioFileIf::destroy(audioFileOut);

	return 0;
}