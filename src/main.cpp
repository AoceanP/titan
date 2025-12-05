/**
 * @author aleksandar panich
 * @version 1.0
 */
#include <iostream>
#include <vector>
#include "../include/OrderBook.hpp"
#include "../include/MarketSim.hpp"

int main()
{
    std::cout << "Titan Trading System Starting..." << std::endl;

    // 1. Setup Simulation Parameters
    double S0 = 100.0;      // Start at $100
    double mu = 0.05;       // 5% drift
    double sigma = 0.2;     // 20% volatility
    double T = 1.0;         // 1 year
    double dt = 1.0/252.0;  // Daily steps

    // 2. Run Simulation
    std::cout << "Running Market Simulator..." << std::endl;
    std::vector<double> prices = titan::sim::generateStockPath(S0, mu, sigma, T, dt);

    // 3. Output Results
    std::cout << "Generated " << prices.size() << " price points." << std::endl;
    std::cout << "Start Price: $" << prices.front() << std::endl;
    std::cout << "End Price:   $" << prices.back() << std::endl;

    return 0;
}