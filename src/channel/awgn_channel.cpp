#include <stdexcept>

#include "channel/awgn_channel.h"

namespace Channel {

AwgnChannel::AwgnChannel() {
    std::random_device rd;
    m_random_generator.seed(rd());
}

std::vector<std::complex<double>> AwgnChannel::addNoise(
    const std::vector<std::complex<double>>& symbols,
    double noise_variance_per_dimension) const {
    if (noise_variance_per_dimension < 0.0) {
        throw std::invalid_argument("Noise variance cannot be negative.");
    }

    std::vector<std::complex<double>> noisy_symbols;

    noisy_symbols.reserve(symbols.size());

    if (noise_variance_per_dimension < 1e-12) {
        noisy_symbols = symbols;
        return noisy_symbols;
    }

    const double std_dev = std::sqrt(noise_variance_per_dimension);

    std::normal_distribution<double> distribution(0.0, std_dev);

    for (const auto& symbol : symbols) {
        double real_noise = distribution(m_random_generator);
        double imag_noise = distribution(m_random_generator);

        std::complex<double> noise(real_noise, imag_noise);
        noisy_symbols.push_back(symbol + noise);
    }
    return noisy_symbols;
}

}  // namespace Channel