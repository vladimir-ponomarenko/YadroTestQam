#include <iostream>
#include <vector>
#include <complex>
#include <random>
#include <stdexcept>
#include <iomanip>
#include <fstream>
#include <string>
#include <cmath>

#include "channel/awgn_channel.h"
#include "signal_processing/qam_processor.h"

using namespace SignalProcessing;
using namespace Channel;

std::vector<int> generateRandomBits(size_t num_bits, std::mt19937& generator) {
    std::uniform_int_distribution<int> distribution(0, 1);
    std::vector<int> bits;
    bits.reserve(num_bits);
    for (size_t i = 0; i < num_bits; ++i) {
        bits.push_back(distribution(generator));
    }
    return bits;
}

size_t countBitErrors(const std::vector<int>& original_bits,
                      const std::vector<int>& demodulated_bits) {
    if (original_bits.size() != demodulated_bits.size()) {
        throw std::runtime_error("countBitErrors: Input vectors have different sizes ("
                                 + std::to_string(original_bits.size()) + " vs "
                                 + std::to_string(demodulated_bits.size()) + ")");
    }
    size_t errors = 0;
    for (size_t i = 0; i < original_bits.size(); ++i) {
        if (original_bits[i] != demodulated_bits[i]) {
            errors++;
        }
    }
    return errors;
}

std::string modulationTypeToString(ModulationType type) {
    switch (type) {
        case ModulationType::QPSK:  return "QPSK";
        case ModulationType::QAM16: return "QAM16";
        case ModulationType::QAM64: return "QAM64";
        default:                    return "Unknown";
    }
}

int main() {
    const std::vector<ModulationType> modulationTypes = {
        ModulationType::QPSK,
        ModulationType::QAM16,
        ModulationType::QAM64
    };


    const size_t numBitsToSimulate = 240000;


    const std::vector<double> snr_db_values = {
        -20.0, -18.0, -16.0, -14.0, -12.0, -10.0, -8.0, -6.0, -4.0, -2.0,
        0.0, 2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0
    };

    const std::string output_filename = "simulation_results_snr.csv";

    std::ofstream outputFile;
    try {
        outputFile.open(output_filename, std::ios::out | std::ios::trunc);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Failed to open output file: " + output_filename);
        }

        outputFile << "Modulation,SNR_dB,BER\n";

        std::cout << "Starting simulation (BER vs SNR)...\n";
        std::cout << "Saving results to: " << output_filename << "\n\n";
        std::cout << std::fixed;
        std::cout << "Modulation | SNR (dB) | BER\n";
        std::cout << "-----------|----------|---------------------\n";


        AwgnChannel channel;
        std::random_device rd;
        std::mt19937 bitGenerator(rd());

        for (const auto& modType : modulationTypes) {
            QamModulator modulator(modType);
            QamDemodulator demodulator(modType);
            const int bitsPerSymbol = modulator.getBitsPerSymbol();

            if (numBitsToSimulate == 0 || numBitsToSimulate % bitsPerSymbol != 0) {
                 std::cerr << "Warning: numBitsToSimulate (" << numBitsToSimulate
                           << ") not a non-zero multiple of bitsPerSymbol (" << bitsPerSymbol
                           << ") for " << modulationTypeToString(modType) << ". Skipping." << std::endl;
                 continue;
            }

            std::cout << std::setw(10) << std::left << modulationTypeToString(modType) << " |";

            for (double snr_db : snr_db_values) {
                double eb_n0_linear = std::pow(10.0, snr_db / 10.0);

                double noise_variance_per_dimension = 1.0 / (2.0 * static_cast<double>(bitsPerSymbol) * eb_n0_linear);

                if (noise_variance_per_dimension < 1e-20) {
                     noise_variance_per_dimension = 1e-20;
                }

                std::vector<int> originalBits = generateRandomBits(numBitsToSimulate, bitGenerator);

                std::vector<std::complex<double>> modulatedSymbols = modulator.modulate(originalBits);

                std::vector<std::complex<double>> noisySymbols = channel.addNoise(modulatedSymbols, noise_variance_per_dimension);

                std::vector<int> demodulatedBits = demodulator.demodulate(noisySymbols);

                size_t errors = countBitErrors(originalBits, demodulatedBits);

                double ber = static_cast<double>(errors) / numBitsToSimulate;

                std::cout << std::setprecision(2) << std::setw(9) << std::right << snr_db << " | ";
                std::cout << std::setprecision(8) << std::setw(19) << ber << std::endl;
                if (&snr_db != &snr_db_values.back()) {
                     std::cout << std::setw(10) << std::left << "" << " |";
                }


                outputFile << modulationTypeToString(modType) << ","
                           << std::setprecision(2) << snr_db << ","
                           << std::setprecision(8) << ber << "\n";

            }
            std::cout << "-----------|----------|---------------------\n";
        }
        outputFile.close();
        std::cout << "\nSimulation finished successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Simulation failed with exception: " << e.what() << std::endl;
        if (outputFile.is_open()) {
            outputFile.close();
        }
        return 1;
    }

    return 0;
}