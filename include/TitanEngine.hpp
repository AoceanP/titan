#pragma once
#include <vector>
#include <string>
#include <random>
#include "imgui.h" // Required for ImVec4

// --- DEFINITIONS (ONLY HERE) ---
enum Timeframe { TF_REALTIME = 0, TF_DAY = 1, TF_MONTH = 2 };
enum ChartMode { MODE_LINE, MODE_CANDLE };

struct Transaction {
    std::string type;
    int qty;
    double price;
    std::string timeStr;
};

struct NotificationSystem {
    std::string message;
    float timer = 0.0f;
    ImVec4 color;

    void trigger(const std::string& msg, bool success);
    void update(float dt);
    void draw(int w);
};

struct CandleBuffer {
    std::vector<double> time, open, high, low, close;
    int maxSize;
    double timeAccumulator = 0.0;
    double curOpen, curHigh, curLow;

    CandleBuffer(int size=200);
    void reserve();
    void clear();
    void initCandle(double price);
    void updateCandle(double price);
    void push(double t, double c);
    void getMinMax(double& outMin, double& outMax) const;
};

class GameState {
public:
    double cash = 1000.0;
    int shares = 0;
    double entryPrice = 0.0;
    double startEquity = 1000.0;
    double currentPrice = 100.0;

    Timeframe activeTimeframe = TF_DAY;
    ChartMode chartMode = MODE_LINE;

    // Internal Math
    std::mt19937 gen;
    std::normal_distribution<> dist;

    // Data Buffers
    CandleBuffer buffers[3];
    double durations[3] = { 1.0, 5.0, 15.0 };

    std::vector<Transaction> log;
    NotificationSystem notifs;

    // Camera / UI State
    double camMinY = 90.0, camMaxY = 110.0;
    double camMinX = 0.0, camMaxX = 10.0;
    double continuousTime = 0.0;
    double displayTime = 0.0;
    double timer = 0.0;

    float splitHeight = 400.0f;
    int tradeQty = 1;

    // Assets
    unsigned int rocketTexture = 0;
    int rocketW = 0, rocketH = 0;

    // Methods
    GameState();
    void generateHistory();
    void update(float realDt);
    void buy();
    void sell();
};