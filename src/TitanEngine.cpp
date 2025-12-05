#include "TitanEngine.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

// --- Notification Logic ---
void NotificationSystem::trigger(const std::string& msg, bool success) {
    message = msg;
    timer = 2.0f;
    color = success ? ImVec4(0.0f, 0.85f, 0.35f, 1.0f) : ImVec4(1.0f, 0.23f, 0.19f, 1.0f);
}
void NotificationSystem::update(float dt) { if (timer > 0) timer -= dt; }
void NotificationSystem::draw(int w) {
    if (timer <= 0) return;
    ImGui::SetNextWindowPos(ImVec2(w/2.0f, 130.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.95f * std::min(1.0f, timer));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 20.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.2f,0.2f,0.2f,1.0f));
    if (ImGui::Begin("##Notif", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoInputs)) {
        ImGui::TextColored(color, "%s", message.c_str());
    }
    ImGui::End();
    ImGui::PopStyleColor(); ImGui::PopStyleVar();
}

// --- Candle Logic ---
CandleBuffer::CandleBuffer(int size) : maxSize(size) { reserve(); curOpen=curHigh=curLow=0; }
void CandleBuffer::reserve() { time.reserve(maxSize); open.reserve(maxSize); high.reserve(maxSize); low.reserve(maxSize); close.reserve(maxSize); }
void CandleBuffer::clear() { time.clear(); open.clear(); high.clear(); low.clear(); close.clear(); }
void CandleBuffer::initCandle(double price) { curOpen=price; curHigh=price; curLow=price; }
void CandleBuffer::updateCandle(double price) { if (price > curHigh) curHigh = price; if (price < curLow) curLow = price; }
void CandleBuffer::push(double t, double c) {
    if (time.size() >= maxSize) {
        time.erase(time.begin()); open.erase(open.begin()); high.erase(high.begin()); low.erase(low.begin()); close.erase(close.begin());
    }
    time.push_back(t); open.push_back(curOpen); high.push_back(curHigh); low.push_back(curLow); close.push_back(c);
    curOpen=c; curHigh=c; curLow=c;
}
void CandleBuffer::getMinMax(double& outMin, double& outMax) const {
    if (low.empty()) { outMin=90.0; outMax=110.0; return; }
    outMin = 9999999.0; outMax = 0.0;
    for(double v : low) if(v < outMin) outMin=v;
    for(double v : high) if(v > outMax) outMax=v;
}

// --- Game Logic ---
GameState::GameState() : gen(std::random_device{}()), dist(0.0, 1.0) {
    for(int i=0; i<3; ++i) buffers[i].initCandle(currentPrice);
    generateHistory();
}

void GameState::generateHistory() {
    double simPrice = 100.0;
    for(int i=0; i<200; ++i) {
        double shock = dist(gen) * 0.005;
        simPrice *= std::exp(0.0001 + shock);
        for(int k=0; k<3; ++k) {
            buffers[k].updateCandle(simPrice);
            buffers[k].timeAccumulator += 1.0;
            if(buffers[k].timeAccumulator >= durations[k]) {
                buffers[k].push((double)buffers[k].time.size(), simPrice);
                buffers[k].timeAccumulator = 0.0;
            }
        }
    }
    currentPrice = simPrice;
}

void GameState::update(float realDt) {
    int subSteps = 10; double dtSub = realDt / subSteps; double vol = 0.15;
    for(int i=0; i<subSteps; ++i) {
        double shock = dist(gen) * std::sqrt(dtSub) * vol * 0.01;
        currentPrice *= std::exp(shock);
        for(int k=0; k<3; ++k) buffers[k].updateCandle(currentPrice);
    }
    for(int k=0; k<3; ++k) {
        buffers[k].timeAccumulator += realDt;
        if(buffers[k].timeAccumulator >= durations[k]) {
            double gTime = buffers[k].time.empty() ? 0.0 : buffers[k].time.back() + 1.0;
            buffers[k].push(gTime, currentPrice);
            buffers[k].timeAccumulator = 0.0;
        }
    }
    notifs.update(realDt);
}

void GameState::buy() {
    if(tradeQty <= 0) return;
    double cost = currentPrice * tradeQty;
    if(cash >= cost) {
        double val = (shares * entryPrice) + cost; shares += tradeQty; entryPrice = val/shares; cash -= cost;
        notifs.trigger("Bought " + std::to_string(tradeQty) + " BTC", true);
        log.insert(log.begin(), {"BUY", tradeQty, currentPrice, "Now"});
    } else notifs.trigger("Insufficient Funds", false);
}

void GameState::sell() { 
    if(tradeQty <= 0) return;
    if(shares >= tradeQty) { 
        cash += currentPrice * tradeQty; shares -= tradeQty; if(shares==0) entryPrice=0;
        notifs.trigger("Sold " + std::to_string(tradeQty) + " BTC", true);
        log.insert(log.begin(), {"SELL", tradeQty, currentPrice}); 
    } else notifs.trigger("No Shares", false);
}