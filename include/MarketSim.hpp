/**
 * @author aleksandar panich
 * @version 1.0
 */
#ifndef MARKET_SIM_HPP
#define MARKET_SIM_HPP

#include <vector>

namespace titan::sim
{
    /**
     * Generates a random price path using Geometric Brownian Motion (GBM).
     *
     * @param initialPrice  (S0) Starting price (e.g., 100.0)
     * @param drift         (mu) Expected annual return (e.g., 0.05 for 5%)
     * @param volatility    (sigma) Annual Volatility (e.g., 0.2 for 20%)
     * @param timeHorizon   (T) Total time in years (e.g., 1.0)
     * @param timeStep      (dt) Time increment (e.g., 1.0/252.0 for daily)
     * @return              Vector of prices
     */
    std::vector<double> generateStockPath(
        double initialPrice,
        double drift,
        double volatility,
        double timeHorizon,
        double timeStep
    );

} // namespace titan::sim

#endif