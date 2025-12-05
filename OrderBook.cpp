/**
 * @author aleksandar panich
 * @version 1.0
 */

#include "OrderBook.hpp"
#include <iostream>

namespace titan
{
//public interface

    void OrderBook::addOrder(const Order& incomingOrder)
    {
        // We create a mutable copy because we might modify its quantity during matching.
        Order order = incomingOrder;

        if (order.side == Side::Buy)
        {
            matchBuyOrder(order);
        }
        else
        {
            matchSellOrder(order);
        }
    }

    bool OrderBook::isEmpty() const
    {
        return bids.empty() && asks.empty();
    }

    size_t OrderBook::getBidDepth() const
    {
      return bids.size();
    }

    std::optional<Order> OrderBook::getBestBid() const
    {
        if (bids.empty())
        {
            return std::nullopt;
        }
        // bids.begin() is the highest price because of std::greater comparator
        return bids.begin()->second.front();
    }

    std::optional<Order> OrderBook::getBestAsk() const
    {
        if (asks.empty())
        {
            return std::nullopt;
        }
        // asks.begin() is the lowest price because of std::less comparator
        return asks.begin()->second.front();
    }
    void OrderBook::matchBuyOrder(Order& order)
    {
        // Continuous Matching Loop
        // We keep trying to match as long as:
        // 1. The incoming order still needs to buy something (quantity > 0)
        // 2. There are sellers available (asks is not empty)
        while (order.quantity > 0 && !asks.empty())
        {
            // Access the "Best Ask" (Lowest Price)
            // 'bestAskIter' is an iterator to the map entry: [Price, OrderList]
            auto bestAskIter = asks.begin();
            Price bestAskPrice = bestAskIter->first;
            OrderList& ordersAtLevel = bestAskIter->second;

            // Price Check:
            // If the seller wants MORE than we are willing to pay, no trade happens.
            if (bestAskPrice > order.price)
            {
                break;
            }

            // We have a match! Look at the oldest order in the queue (FIFO)
            Order& sellOrder = ordersAtLevel.front();

            // Calculate fill quantity (min of what I want vs what they have)
            Quantity fillQty = std::min(order.quantity, sellOrder.quantity);

            // "Execute" the trade (Updating state)
            order.quantity -= fillQty;
            sellOrder.quantity -= fillQty;

            // Log the trade (Crucial for debugging later)
            std::cout << "[MATCH] Buy Order " << order.id
                      << " matched " << fillQty
                      << " @ " << bestAskPrice << std::endl;

            // Cleanup: If the seller is empty, remove them from the list
            if (sellOrder.quantity == 0)
            {
                ordersAtLevel.pop_front();

                // If no more sellers at this specific price level, remove the price level entirely
                if (ordersAtLevel.empty())
                {
                    asks.erase(bestAskIter);
                }
            }
        }

        // If we still have quantity left after checking all valid sellers,
        // we must park this order in the Book (Liquidity Provider).
        if (order.quantity > 0)
        {
            bids[order.price].push_back(order);
        }
    }

    void OrderBook::matchSellOrder(Order& order)
    {
        // Continuous Matching Loop for Sellers
        // Looking for Buyers (Bids)
        while (order.quantity > 0 && !bids.empty())
        {
            // Access the "Best Bid" (Highest Price)
            // Remember: BidsMap uses std::greater, so begin() is the HIGHEST price.
            auto bestBidIter = bids.begin();
            Price bestBidPrice = bestBidIter->first;
            OrderList& ordersAtLevel = bestBidIter->second;

            // Price Check:
            // If the buyer is offering LESS than I am willing to sell for, no trade.
            if (bestBidPrice < order.price)
            {
                break;
            }

            // Match logic (FIFO)
            Order& buyOrder = ordersAtLevel.front();
            Quantity fillQty = std::min(order.quantity, buyOrder.quantity);

            order.quantity -= fillQty;
            buyOrder.quantity -= fillQty;

            std::cout << "[MATCH] Sell Order " << order.id
                      << " matched " << fillQty
                      << " @ " << bestBidPrice << std::endl;

            if (buyOrder.quantity == 0)
            {
                ordersAtLevel.pop_front();
                if (ordersAtLevel.empty())
                {
                    bids.erase(bestBidIter);
                }
            }
        }

        // Park remaining quantity
        if (order.quantity > 0)
        {
            asks[order.price].push_back(order);
        }
    }
}