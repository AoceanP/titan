/**
 * @author aleksandar panich
 * @version 1.0
 */

#include <gtest/gtest.h>
#include "../include/OrderBook.hpp"

using namespace titan;

// Test 1: Does adding an order increase depth?
TEST(OrderBookTests, OrderAddition)
{
    OrderBook book;
    book.addOrder(Order(1, Side::Buy, 100, 10));
    
    // We expect 1 Bid in the book
    ASSERT_EQ(book.getBidDepth(), 1);
    
    // We expect the best bid to be exactly what we entered
    auto bestBid = book.getBestBid();
    ASSERT_TRUE(bestBid.has_value());
    ASSERT_EQ(bestBid->price, 100);
    ASSERT_EQ(bestBid->quantity, 10);
}

// Test 2: Does a matching order execute?
TEST(OrderBookTests, Execution)
{
    OrderBook book;
    
    // 1. Seller offers 10 units @ $100
    book.addOrder(Order(1, Side::Sell, 100, 10)); 
    ASSERT_EQ(book.getAskDepth(), 1);

    // 2. Buyer takes 10 units @ $100
    book.addOrder(Order(2, Side::Buy, 100, 10));  
    
    // 3. Both should match and disappear (Full Execution)
    ASSERT_EQ(book.getBidDepth(), 0);
    ASSERT_EQ(book.getAskDepth(), 0);
}