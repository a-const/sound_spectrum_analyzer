#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <exception>
#include <vector>
#include <map>
#include <Windows.h>
#include "audiofile.h"
#include <dos.h>
#include <iomanip>
#include <conio.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <complex>


#define HEADER_INDEX 0
#define FORMAT_INDEX 12


class sAnalyser {
public:
	sAnalyser() = default;
	bool initialize(const std::string& filename);
	void decode();
	bool computeOneSecSpectrum(int sec);
	void drawSpectrum(int sec);
	int getDurationofAudio();
	void printInfo();
private:

	void fft(std::vector<std::complex<double>>& data);
	std::vector<double> computeFrequency(std::vector<double>& audio_samples, double sample_rate);

	std::vector<byte> file_data;
	std::ifstream input_stream;

	std::string header_chunk_id;
	std::string sound_format;
	short channels;
	int sample_rate;
	short bits_per_sample;
	std::string subchunk_id;
	int data_chunk_size = 0;
	int data_chunk_pos = 0;
	int duration_of_audio;
	bool broken = false;

	std::map<int,std::vector<double>> full_spectrum;
	const std::vector<double> target_frequencies = { 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };

	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	
};