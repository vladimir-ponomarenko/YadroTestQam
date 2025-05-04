#include <complex>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

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
        throw std::runtime_error(
            "countBitErrors: Input vectors have different sizes (" +
            std::to_string(original_bits.size()) + " vs " +
            std::to_string(demodulated_bits.size()) + ")");
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
        case ModulationType::QPSK:
            return "QPSK";
        case ModulationType::QAM16:
            return "QAM16";
        case ModulationType::QAM64:
            return "QAM64";
        default:
            return "Unknown";
    }
}

int main() {
    const std::vector<ModulationType> modulationTypes = {
        ModulationType::QPSK, ModulationType::QAM16, ModulationType::QAM64};

    const size_t numBitsToSimulate = 120000;

    const std::vector<double> noiseVariances = {0.001, 0.002, 0.005, 0.01, 0.02,
                                                0.05,  0.1,   0.2,   0.5,  1.0};

    const std::string output_filename = "simulation_results.csv";
    std::ofstream outputFile;
    try {
        outputFile.open(output_filename, std::ios::out | std::ios::trunc);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Failed to open output file: " +
                                     output_filename);
        }

        outputFile << "Modulation,NoiseVariance,BER\n";

        std::cout << "Starting simulation...\n";
        std::cout << "Saving results to: " << output_filename << "\n\n";
        std::cout << std::fixed << std::setprecision(8);
        std::cout << "Modulation | NoiseVar (N0/2) |    BER\n";
        std::cout << "-----------|-----------------|-------------\n";

        AwgnChannel channel;

        std::random_device rd;
        std::mt19937 bitGenerator(rd());

        for (const auto& modType : modulationTypes) {
            std::cout << modulationTypeToString(modType) << ":" << std::endl;

            QamModulator modulator(modType);
            QamDemodulator demodulator(modType);
            const int bitsPerSymbol = modulator.getBitsPerSymbol();

            if (numBitsToSimulate == 0 ||
                numBitsToSimulate % bitsPerSymbol != 0) {
                std::cerr << "Warning: numBitsToSimulate (" << numBitsToSimulate
                          << ") not a non-zero multiple of bitsPerSymbol ("
                          << bitsPerSymbol << ") for "
                          << modulationTypeToString(modType) << ". Skipping."
                          << std::endl;
                continue;
            }

            for (double noiseVar : noiseVariances) {
                std::vector<int> originalBits =
                    generateRandomBits(numBitsToSimulate, bitGenerator);

                std::vector<std::complex<double>> modulatedSymbols =
                    modulator.modulate(originalBits);

                std::vector<std::complex<double>> noisySymbols =
                    channel.addNoise(modulatedSymbols, noiseVar);

                std::vector<int> demodulatedBits =
                    demodulator.demodulate(noisySymbols);

                size_t errors = countBitErrors(originalBits, demodulatedBits);

                double ber = static_cast<double>(errors) / numBitsToSimulate;

                std::cout << "           | " << std::setw(15) << noiseVar
                          << " | " << std::setw(10) << ber << std::endl;

                outputFile << modulationTypeToString(modType) << "," << noiseVar
                           << "," << ber << "\n";
            }
            std::cout << "-----------|-----------------|-------------\n";
        }
        outputFile.close();
        std::cout << "\nSimulation finished successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Simulation failed with exception: " << e.what()
                  << std::endl;
        if (outputFile.is_open()) {
            outputFile.close();
        }
        return 1;
    }

    return 0;
}