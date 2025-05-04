#ifndef AWGN_CHANNEL_H
#define AWGN_CHANNEL_H

#include <cmath>
#include <complex>
#include <random>
#include <vector>

namespace Channel {

class AwgnChannel {
   public:
    AwgnChannel();
    std::vector<std::complex<double>> addNoise(
        const std::vector<std::complex<double>>& symbols,
        double noise_variance_per_dimension) const;

   private:
    mutable std::mt19937 m_random_generator;
};

}  // namespace Channel

#endif