#ifndef QAM_PROCESSOR_H
#define QAM_PROCESSOR_H

#include <cmath>
#include <complex>
#include <stdexcept>
#include <string>
#include <vector>

namespace SignalProcessing {

enum class ModulationType { QPSK, QAM16, QAM64 };

/* ******************************************************* */

class QamModulator {
   public:
    explicit QamModulator(ModulationType type);
    std::vector<std::complex<double>> modulate(
        const std::vector<int>& bits) const;
    int getBitsPerSymbol() const noexcept;
    const std::vector<std::complex<double>>& getConstellationMap()
        const noexcept;

   private:
    void initializeConsellation();
    int binaryToGray(int binary_index) const noexcept;
    ModulationType m_type;
    int m_bitsPerSymbol;
    double m_normalizationFactor;
    std::vector<std::complex<double>> m_constellationMap;
};

/* ******************************************************* */

class QamDemodulator {
   public:
    explicit QamDemodulator(ModulationType type);
    std::vector<int> demodulate(
        const std::vector<std::complex<double>>& symbols) const;
    int getBitsPerSymbol() const noexcept;

   private:
    void initializeConstellation();
    int findClosestSymbolIndex(
        const std::complex<double>& received_symbol) const;
    std::vector<int> indexToBits(int index) const;
    ModulationType m_type;
    int m_bitsPerSymbol;
    std::vector<std::complex<double>> m_constellationMap;
};

}  // namespace SignalProcessing

#endif QAM_PROCESSOR_H