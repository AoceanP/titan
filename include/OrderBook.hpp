/**
 * @author aleksandar panich
 * @version 1.0
 */

#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <map>
#include <deque>
#include <vector>
#include <cstdint>
#include <optional>
#include <functional>

namespace titan
{

    // Type Aliases
    using OrderId = uint64_t;
    using Price = uint64_t;
    using Quantity = uint64_t;

    enum class Side
    {
        Buy,
        Sell
    };

    struct Order
    {
        OrderId id;
        Side side;
        Price price;
        Quantity quantity;
        Quantity initialQuantity;

        Order(OrderId id, Side side, Price price, Quantity qty)
            : id(id)
            , side(side)
            , price(price)
            , quantity(qty)
            , initialQuantity(qty)
        {
        }
    };

    class OrderBook
    {
    public:
        using OrderList = std::deque<Order>;
        using BidsMap = std::map<Price, OrderList, std::greater<Price>>;
        using AsksMap = std::map<Price, OrderList, std::less<Price>>;

        OrderBook() = default;
        ~OrderBook() = default;

        void addOrder(const Order& order);
        bool isEmpty() const;
        size_t getBidDepth() const;
        size_t getAskDepth() const;

        std::optional<Order> getBestBid() const;
        std::optional<Order> getBestAsk() const;

        // These variables (bids, asks) are what your red squiggly lines
        // are complaining about. They are defined here:
    private:
        BidsMap bids;
        AsksMap asks;

        void matchBuyOrder(Order& order);
        void matchSellOrder(Order& order);
    };

} // namespace titan

#endif
