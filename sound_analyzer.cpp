#include "sound_analyzer.h"


bool sAnalyser::initialize(const std::string& filename)
{
	input_stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		input_stream.open(filename, std::ifstream::binary);
		input_stream.unsetf(std::ios::skipws);
		input_stream.seekg(0, std::ios::end);
		size_t length = input_stream.tellg();
		input_stream.seekg(0, std::ios::beg);
		file_data.resize(length);
		input_stream.read(reinterpret_cast<char*> (file_data.data()), length);
		input_stream.close();
		return true;
	}
	catch (const std::ios_base::failure& e) {
		std::cerr << "Error! Something wrong with file!";
		return false;
	}
}

void sAnalyser::decode() {
	if (file_data.empty()) return;

	// Getting minimum info for work
	header_chunk_id.insert(0,std::string(file_data.begin(), file_data.begin()+4));
	//
	sound_format.insert(0, std::string(file_data.begin()+8, file_data.begin() + 12));
	//
	channels = file_data.at(FORMAT_INDEX + 11) << 8 | file_data.at(FORMAT_INDEX + 10);
	//
	sample_rate = file_data.at(FORMAT_INDEX + 15) << 24 | file_data.at(FORMAT_INDEX + 14) << 16 | file_data.at(FORMAT_INDEX + 13) << 8 | file_data.at(FORMAT_INDEX + 12);
	//
	bits_per_sample = file_data.at(FORMAT_INDEX + 23) << 8 | file_data.at(FORMAT_INDEX + 22);

	//	By various specifictions next could be chunks like "LIST", "INFO", "chna" etc.
	//	We searching for "data" chunk. If in next 200 bytes we won't get data chunk it means file either broken or kinda special.
	//	This program works with +\- classic .wav files
	for (int i = 36; i < 236; i+=2) {
		subchunk_id.insert(0, std::string(file_data.begin() + i, (file_data.begin() + i + 4)));
		if (subchunk_id == "data") {
			data_chunk_size = file_data.at(i + 7) << 24 | file_data.at(i + 6) << 16 | file_data.at(i + 5) << 8 | file_data.at(i + 4);
			data_chunk_pos = i + 8;
			break;
		}
		subchunk_id.clear();
	}
	if (!data_chunk_size && !data_chunk_pos) {
		std::cout << "Can't find \"data\" chunk of file! Try another file!";
		broken = true;
	}
	duration_of_audio = data_chunk_size / (channels * (bits_per_sample / 8) * sample_rate);
}

bool sAnalyser::computeOneSecSpectrum(int sec)
{
	if (sec < 1 || sec >= duration_of_audio)
		return false;
	try {
		if (!full_spectrum.at(sec).empty())
			return true;
	}
	catch (std::out_of_range& e) {}
	int one_sec = channels * (bits_per_sample / 8) * sample_rate;
	std::cout << "\n..........Processing.............\n" << std::flush;
	std::vector<std::complex<double>> data(file_data.begin() + data_chunk_pos + one_sec * sec, file_data.begin() + data_chunk_pos + one_sec * (sec + 1));
	fft(data);
	std::vector<double> decomplexed;
	for (int i = 0; i < data.size(); i++) {
		decomplexed.push_back(std::abs(data[i]));
	}
	full_spectrum[sec] = computeFrequency(decomplexed, sample_rate);
	return true;
}


void sAnalyser::fft(std::vector<std::complex<double>>& data) {
	int n = data.size();

	if (n == 1) {
		return;
	}

	std::vector<std::complex<double>> even(n / 2), odd(n / 2);
	for (int i = 0; i < n / 2; i++) {
		even[i] = data[2 * i];
		odd[i] = data[2 * i + 1];
	}

	fft(even);
	fft(odd);

	for (int i = 0; i < n / 2; i++) {
		std::complex<double> t = std::polar(1.0, -2 * M_PI * i / n) * odd[i];
		data[i] = even[i] + t;
		data[i + n / 2] = even[i] - t;
	}
}


std::vector<double>  sAnalyser::computeFrequency(std::vector<double>& processed_data, double sample_rate) {
	int size = processed_data.size();
	int num_bins = 400;
	double bin_size = (double)sample_rate / size;
	std::vector<double> magnitudes(target_frequencies.size());

	for (int i = 0; i < target_frequencies.size(); i++)
	{
		int index = std::round(target_frequencies[i] / bin_size);
		if (index < processed_data.size())
		{
			magnitudes[i] = processed_data[index];
			magnitudes[i] /= 1000;
		}
	}
	return magnitudes;
}

int sAnalyser::getDurationofAudio()
{
	return duration_of_audio;
}

void sAnalyser::printInfo()
{
	system("cls");
	std::cout << "Number of channels: " << channels;
	std::cout << "\nSample rate: " << sample_rate;
	std::cout << "\nDuration of audio: " << duration_of_audio / 60 << ':' << duration_of_audio % 60 << "\n\n";
}


void sAnalyser::drawSpectrum(int sec) {
	int size = target_frequencies.size();
	system("cls");
	std::cout << "\n" << sec << " sec out of " << duration_of_audio << "... \n\n";
	for (int i = 0; i < size; i++) {
		std::cout << std::setw(8) << target_frequencies[i] << " Hz: ";
		if (full_spectrum[sec][i] > 50) full_spectrum[sec][i] = 50;
		for (int j = 0; j < 50; j++) {
			if (full_spectrum[sec][i] > j) {
				if (full_spectrum[sec][i] > 49)
					SetConsoleTextAttribute(handle, FOREGROUND_RED);
				else
					SetConsoleTextAttribute(handle, FOREGROUND_GREEN);
				std::cout << '#';
				SetConsoleTextAttribute(handle, 0x000F);
			}
			else
				std::cout << ' ';
		}
		std::cout <<  "|\n";
	}
}
