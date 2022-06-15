#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <optional>

#include "AudioFileIf.h"
#include "ErrorDef.h"
#include "CombFilter.h"
#include "Exception.h"

using std::string;
using std::vector;
using std::unique_ptr;
using std::cout;
using std::endl;

std::optional<Exception> openInputFile(const string& inputFilePath, CAudioFileIf*& audioFileIn, CAudioFileIf::FileSpec_t& fileSpec);
std::optional<Exception> openOutputFile(const string& outputFilePath, CAudioFileIf*& audioFileOut, CAudioFileIf::FileSpec_t& fileSpec);

int main(int argc, char* argv[])
{
	static const int fileBlockSize = 1023;
	const int labelFormat = 30;

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

	std::optional<Exception> ex;

	cout << "-- CombFilter Effect -- " << endl;

	try {
		// Open Input File
		cout << endl;
		cout << std::setw(labelFormat) << std::right << "Input WAV file: ";
		std::cin >> inputFilePath;
		ex = openInputFile(inputFilePath, audioFileIn, fileSpec);
		if (ex.has_value()) throw ex.value();

		// Open Output File
		cout << std::setw(labelFormat) << "Output WAV file: ";
		std::cin >> outputFilePath;
		ex = openOutputFile(outputFilePath, audioFileOut, fileSpec);
		if (ex.has_value()) throw ex.value();

		// Create Instances
		for (int c = 0; c < fileSpec.iNumChannels; c++) {
			combFilter.emplace_back(new CombFilter(fileSpec.fSampleRateInHz));
		}

		// Set Filter Type
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

		// Set Gain
		cout << std::setw(labelFormat) << "Gain (-1.0 - 1.0): ";
		std::cin >> gain;
		for (int c = 0; c < fileSpec.iNumChannels; c++) {
			if (combFilter[c]->setParam(CombFilter::gain, gain) != Error_t::kNoError) {
				throw Exception("Invalid gain value...");
			}
		}

		// SET Delay
		cout << std::setw(labelFormat) << "Delay (0.01 - 10.0): ";
		std::cin >> delayInSec;
		for (int c = 0; c < fileSpec.iNumChannels; c++) {
			if (combFilter[c]->setParam(CombFilter::delayInSec, delayInSec) != Error_t::kNoError) {
				throw Exception("Invalid delay value...");
			}
		}

		// Allocate memory
		inputAudioData = new float* [fileSpec.iNumChannels]{};
		outputAudioData = new float* [fileSpec.iNumChannels]{};
		for (int c = 0; c < fileSpec.iNumChannels; c++) {
			inputAudioData[c] = new float[fileBlockSize] {};
			outputAudioData[c] = new float[fileBlockSize] {};
		}

		// Process
		long long iNumFrames = fileBlockSize;
		while (!audioFileIn->isEof()) {
			if (audioFileIn->readData(inputAudioData, iNumFrames) != Error_t::kNoError) {
				throw Exception("Data reading error...");
			};
			for (int c = 0; c < combFilter.size(); c++) {
				if (combFilter[c]->process(inputAudioData[c], outputAudioData[c], iNumFrames) != Error_t::kNoError) {
					throw Exception("Processing error...");
				}
			}
			if (audioFileOut->writeData(outputAudioData, iNumFrames) != Error_t::kNoError) {
				throw Exception("Data writing error...");
			};
		}

		cout << endl;
		cout << std::setw(labelFormat) << "Done processing..." << endl;
		cout << "\n-- CombFilter Effect -- " << endl;
		cout << endl;

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
	catch (Exception& ex) {
		if (audioFileIn) {
			CAudioFileIf::destroy(audioFileIn);
		}
		if (audioFileOut) {
			CAudioFileIf::destroy(audioFileOut);
		}
		if (inputAudioData) {
			for (int c = 0; c < fileSpec.iNumChannels; c++) {
				delete[] inputAudioData[c];
			}
			delete[] inputAudioData;
		}
		if (outputAudioData) {
			for (int c = 0; c < fileSpec.iNumChannels; c++) {
				delete[] outputAudioData[c];
			}
			delete[] outputAudioData;
		}
		audioFileIn = nullptr;
		audioFileOut = nullptr;
		inputAudioData = nullptr;
		outputAudioData = nullptr;
		combFilter.clear();

		cout << "\n--------------------------" << endl;
		cout << ex.what() << endl;
		cout << "--------------------------" << endl;
		return -1;
	}
}

std::optional<Exception> openInputFile(const string& inputFilePath, CAudioFileIf*& audioFileIn, CAudioFileIf::FileSpec_t& fileSpec)
{
	CAudioFileIf::create(audioFileIn);
	audioFileIn->openFile(inputFilePath, CAudioFileIf::FileIoType_t::kFileRead);
	if (!audioFileIn->isOpen()) {
		return Exception("Couldn't open input file...");
	}
	audioFileIn->getFileSpec(fileSpec);
	return std::nullopt;
}

std::optional<Exception> openOutputFile(const string& outputFilePath, CAudioFileIf*& audioFileOut, CAudioFileIf::FileSpec_t& fileSpec)
{
	CAudioFileIf::create(audioFileOut);
	audioFileOut->openFile(outputFilePath, CAudioFileIf::FileIoType_t::kFileWrite, &fileSpec);
	if (!audioFileOut->isOpen()) {
		return Exception("Couldn't open output file...");
	}
	return std::nullopt;
}
