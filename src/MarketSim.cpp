/**
 * @author aleksandar panich
 * @version 1.0
 */
#include "../include/MarketSim.hpp"
#include <random>
#include <cmath>
#include <iostream>

namespace titan::sim
{
    std::vector<double> generateStockPath(
        double initialPrice,
        double drift,
        double volatility,
        double timeHorizon,
        double timeStep
    )
    {
        std::vector<double> prices;
        
        // Reserve memory to prevent re-allocations (Optimization)
        size_t steps = static_cast<size_t>(timeHorizon / timeStep);
        prices.reserve(steps + 1);
        prices.push_back(initialPrice);

        // Random Number Generator
        // hardware_entropy (rd) seeds the Mersenne Twister (gen)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> d(0.0, 1.0); // Standard Normal (Mean=0, StdDev=1)

        double currentPrice = initialPrice;

        for (size_t i = 0; i < steps; ++i)
        {
            // 1. The Shock (Wiener Process)
            // Random shock scales with the square root of time
            double dW = d(gen) * std::sqrt(timeStep);

            // 2. The Drift (Trend)
            double driftComponent = (drift - 0.5 * volatility * volatility) * timeStep;

            // 3. The Diffusion (Volatility)
            double diffusionComponent = volatility * dW;

            // Combine them: S(t+1) = S(t) * e^(drift + diffusion)
            currentPrice = currentPrice * std::exp(driftComponent + diffusionComponent);

            prices.push_back(currentPrice);
        }

        return prices;
    }

}