#pragma once
#include <vector>
#include <string>
#include <random>
#include "imgui.h"

enum Timeframe { TF_DAY = 0, TF_MONTH = 1 };

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

    CandleBuffer(int size=150);
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
    double startEquity = 1000.0;
    double currentPrice = 100.0;

    Timeframe activeTimeframe = TF_DAY;

    std::mt19937 gen;
    std::normal_distribution<> dist;

    CandleBuffer buffers[2];
    double durations[2] = { 2.0, 5.0 };

    NotificationSystem notifs;

    // Camera
    double camMinY = 90.0, camMaxY = 110.0;
    double camMinX = 0.0, camMaxX = 10.0;

    // Assets
    unsigned int rocketTexture = 0;
    int rocketW = 0, rocketH = 0;

    GameState();
    void generateHistory();
    void update(float realDt);
    void buy();
    void sell();
};