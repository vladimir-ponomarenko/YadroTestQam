#include <cmath>
#include <map>
#include <numeric>
#include <stdexcept>

#include "signal_processing/qam_processor.h"

namespace SignalProcessing {

QamModulator::QamModulator(ModulationType type) : m_type(type) {
    switch (m_type) {
        case ModulationType::QPSK:
            m_bitsPerSymbol = 2;
            break;
        case ModulationType::QAM16:
            m_bitsPerSymbol = 4;
            break;
        case ModulationType::QAM64:
            m_bitsPerSymbol = 6;
            break;
        default:
            throw std::logic_error("Unknown modulation type in constructor...");
    }

    initializeConsellation();
}

int QamModulator::getBitsPerSymbol() const noexcept { return m_bitsPerSymbol; }

const std::vector<std::complex<double>>& QamModulator::getConstellationMap()
    const noexcept {
    return m_constellationMap;
}

int QamModulator::binaryToGray(int binary_index) const noexcept {
    return binary_index ^ (binary_index >> 1);
}

void QamModulator::initializeConsellation() {
    const int M = 1 << m_bitsPerSymbol;
    m_constellationMap.resize(M);
    const int k_dim = m_bitsPerSymbol / 2;
    const int L = 1 << k_dim;

    std::vector<double> levels(L);
    for (int i = 0; i < L; ++i) {
        levels[i] = static_cast<double>(2 * i - L + 1);
    }

    std::vector<double> levels_gray_order(L);
    for (int bin_idx = 0; bin_idx < L; ++bin_idx) {
        int gray_idx = binaryToGray(bin_idx);
        levels_gray_order[gray_idx] = levels[bin_idx];
    }

    double totalPower = 0.0;
    for (int i = 0; i < M; ++i) {
        int bits_I = (i >> k_dim);
        int bits_Q = i & ((1 << k_dim) - 1);

        int gray_I = binaryToGray(bits_I);
        int gray_Q = binaryToGray(bits_Q);

        double real_part = levels_gray_order[gray_I];
        double imag_part = levels_gray_order[gray_Q];

        std::complex<double> symbol(real_part, imag_part);
        m_constellationMap[i] = symbol;
        totalPower += std::norm(symbol);
    }
    double avgPower = totalPower / M;

    m_normalizationFactor = std::sqrt(avgPower);
    if (m_normalizationFactor > 1e-9) {
        for (int i = 0; i < M; ++i) {
            m_constellationMap[i] /= m_normalizationFactor;
        }
    }
}

std::vector<std::complex<double>> QamModulator::modulate(
    const std::vector<int>& bits) const {
    if (bits.empty() || bits.size() % m_bitsPerSymbol != 0) {
        throw std::invalid_argument(
            "Number of bits (" + std::to_string(bits.size()) +
            "must be a non-zero multiple of bits per symbol (" +
            std::to_string(m_bitsPerSymbol) + ")");
    }

    const size_t numSymbols = bits.size() / m_bitsPerSymbol;
    std::vector<std::complex<double>> symbols;
    symbols.reserve(numSymbols);

    for (size_t i = 0; i < bits.size(); i += m_bitsPerSymbol) {
        int symbolIndex = 0;

        for (int j = 0; j < m_bitsPerSymbol; ++j) {
            const int current_bit = bits[i + j];
            if (current_bit != 0 && current_bit != 1) {
                throw std::invalid_argument(
                    "Input bits must be 0 or 1. Found invalid value at index " +
                    std::to_string(i + j) + ".");
            }
            symbolIndex <<= 1;
            symbolIndex |= current_bit;
        }
        symbols.push_back(m_constellationMap[symbolIndex]);
    }
    return symbols;
}

}  // namespace SignalProcessing