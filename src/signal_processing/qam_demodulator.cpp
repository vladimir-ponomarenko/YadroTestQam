#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <stdexcept>
#include <vector>

#include "signal_processing/qam_processor.h"

namespace SignalProcessing {

QamDemodulator::QamDemodulator(ModulationType type) : m_type(type) {
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
            throw std::logic_error(
                "Unknown modulation type in QamDemodulator constructor.");
    }
    initializeConstellation();
}

int QamDemodulator::getBitsPerSymbol() const noexcept {
    return m_bitsPerSymbol;
}

void QamDemodulator::initializeConstellation() {
    const int M = 1 << m_bitsPerSymbol;
    m_constellationMap.resize(M);
    const int k_dim = m_bitsPerSymbol / 2;
    const int L = 1 << k_dim;

    std::vector<double> levels(L);
    for (int i = 0; i < L; ++i) {
        levels[i] = static_cast<double>(2 * i - L + 1);
    }

    auto binaryToGray = [](int n) -> int { return n ^ (n >> 1); };

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
    double normalizationFactor = std::sqrt(avgPower);

    if (normalizationFactor > 1e-9) {
        for (int i = 0; i < M; ++i) {
            m_constellationMap[i] /= normalizationFactor;
        }
    }
}

int QamDemodulator::findClosestSymbolIndex(
    const std::complex<double>& received_symbol) const {
    if (m_constellationMap.empty()) {
        throw std::runtime_error(
            "Constellation map is empty. Demodulator not initialized "
            "correctly.");
    }

    int minIndex = 0;

    double minDistanceSq = std::norm(received_symbol - m_constellationMap[0]);
    for (size_t i = 1; i < m_constellationMap.size(); ++i) {
        double currentDistanceSq =
            std::norm(received_symbol - m_constellationMap[i]);
        if (currentDistanceSq < minDistanceSq) {
            minDistanceSq = currentDistanceSq;
            minIndex = static_cast<int>(i);
        }
    }
    return minIndex;
}

std::vector<int> QamDemodulator::indexToBits(int index) const {
    std::vector<int> bits(m_bitsPerSymbol);

    if (index < 0 || index >= (1 << m_bitsPerSymbol)) {
        throw std::out_of_range("Symbol index is out of range in indexToBits.");
    }

    for (int j = 0; j < m_bitsPerSymbol; ++j) {
        bits[j] = (index >> ((m_bitsPerSymbol - 1) - j)) & 1;
    }
    return bits;
}

std::vector<int> QamDemodulator::demodulate(
    const std::vector<std::complex<double>>& symbols) const {
    std::vector<int> output_bits;
    output_bits.reserve(symbols.size() * m_bitsPerSymbol);

    for (const auto& received_symbol : symbols) {
        int closestIndex = findClosestSymbolIndex(received_symbol);
        std::vector<int> demodulated_bits = indexToBits(closestIndex);
        output_bits.insert(output_bits.end(), demodulated_bits.begin(),
                           demodulated_bits.end());
    }
    return output_bits;
}

}  // namespace SignalProcessing