#include "TitanEngine.hpp"
#include "TitanGUI.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include <iostream>

const int WINDOW_WIDTH = 450;
const int WINDOW_HEIGHT = 900;

int main(int, char**) {
    if (!glfwInit()) return 1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Titan Mobile", nullptr, nullptr);
    if (!window) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    SetupFintechStyle();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    GameState game;
    // Ensure rocket.jpg is next to your .exe
    LoadTextureFromFile("rocket.jpg", &game.rocketTexture, &game.rocketW, &game.rocketH);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        game.update(ImGui::GetIO().DeltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int dw, dh; glfwGetFramebufferSize(window, &dw, &dh);
        ImGui::SetNextWindowSize(ImVec2((float)dw, (float)dh));
        ImGui::SetNextWindowPos(ImVec2(0, 0));

        ImGui::Begin("App", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoResize);

        // --- HEADER ---
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,1.0f), "NET WORTH");
        double netWorth = game.cash + (game.shares * game.currentPrice);
        ImGui::SetWindowFontScale(3.0f);
        ImGui::Text("$%.2f", netWorth);
        ImGui::SetWindowFontScale(1.0f);

        // BALANCES
        ImGui::Dummy(ImVec2(0, 5));
        ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,1.0f), "Cash:"); ImGui::SameLine(); ImGui::Text("$%.2f", game.cash);
        ImGui::SameLine(dw/2 + 20);
        ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,1.0f), "Invested:"); ImGui::SameLine(); ImGui::Text("$%.2f", game.shares * game.currentPrice);

        // TOGGLE
        ImGui::SetCursorPos(ImVec2(dw - 120, 20));
        DrawToggle("Line", "Bar", (int*)&game.chartMode);

        ImGui::Dummy(ImVec2(0, 20));

        // --- CHART ---
        CandleBuffer& activeBuf = game.buffers[game.activeTimeframe];
        double dataMinY, dataMaxY;
        activeBuf.getMinMax(dataMinY, dataMaxY);
        if(game.currentPrice > dataMaxY) dataMaxY = game.currentPrice;
        if(game.currentPrice < dataMinY) dataMinY = game.currentPrice;
        double spread = dataMaxY - dataMinY; if(spread < 1.0) spread = 1.0;

        double lerp = 5.0 * ImGui::GetIO().DeltaTime;
        game.camMinY += ((dataMinY - spread*0.15) - game.camMinY) * lerp;
        game.camMaxY += ((dataMaxY + spread*0.15) - game.camMaxY) * lerp;

        // Smooth X
        double currentX = activeBuf.time.empty() ? 0.0 : activeBuf.time.back();
        double progress = activeBuf.timeAccumulator / game.durations[game.activeTimeframe];
        double continuousX = currentX + progress;

        double targetMaxX = continuousX + 1.5;
        double targetMinX = targetMaxX - 20.0;
        game.camMaxX = targetMaxX; game.camMinX = targetMinX;

        if (ImPlot::BeginPlot("##Chart", ImVec2(-1, game.splitHeight), ImPlotFlags_NoTitle|ImPlotFlags_NoLegend|ImPlotFlags_NoMenus|ImPlotFlags_NoBoxSelect)) {
            ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoDecorations);
            ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoDecorations);
            ImPlot::SetupAxisLimits(ImAxis_X1, game.camMinX, game.camMaxX, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, game.camMinY, game.camMaxY, ImGuiCond_Always);

            ImPlot::PushPlotClipRect();

            ImVec4 bull = ImVec4(0.0f,0.85f,0.35f,1.0f);
            ImVec4 bear = ImVec4(1.0f,0.23f,0.19f,1.0f);

            if (game.chartMode == MODE_LINE) {
                ImVec4 lineCol = bull;
                if (!activeBuf.time.empty()) {
                    ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.2f);
                    ImPlot::PlotShaded("Fill", activeBuf.time.data(), activeBuf.close.data(), activeBuf.time.size(), -INFINITY);
                    ImPlot::PopStyleVar();
                    ImPlot::SetNextLineStyle(lineCol, 3.0f);
                    ImPlot::PlotLine("Line", activeBuf.time.data(), activeBuf.close.data(), activeBuf.time.size());
                }
                double t[2] = { activeBuf.time.empty() ? 0 : activeBuf.time.back(), continuousX };
                double p[2] = { activeBuf.close.empty() ? game.currentPrice : activeBuf.close.back(), game.currentPrice };
                ImPlot::SetNextLineStyle(lineCol, 3.0f);
                ImPlot::PlotLine("Active", t, p, 2);

                if (game.rocketTexture != 0) {
                    ImVec2 size(1.5, 1.5 * ((double)game.rocketH/game.rocketW));
                    ImPlot::PlotImage("Rocket", (ImTextureID)(intptr_t)game.rocketTexture,
                        ImPlotPoint(continuousX, game.currentPrice), ImPlotPoint(size.x, size.y), ImVec2(1,0), ImVec2(0,1));
                }
            } else {
                if(!activeBuf.time.empty()) {
                    for(int i=0; i<(int)activeBuf.time.size(); ++i) {
                        ImU32 col = ImGui::GetColorU32(activeBuf.close[i] >= activeBuf.open[i] ? bull : bear);
                        ImVec2 p1 = ImPlot::PlotToPixels(activeBuf.time[i]-0.3, activeBuf.open[i]);
                        ImVec2 p2 = ImPlot::PlotToPixels(activeBuf.time[i]+0.3, activeBuf.close[i]);
                        p1.x = std::round(p1.x); p1.y = std::round(p1.y); p2.x = std::round(p2.x); p2.y = std::round(p2.y);
                        ImPlot::GetPlotDrawList()->AddRectFilled(ImVec2(p1.x, std::min(p1.y,p2.y)), ImVec2(p2.x, std::max(p1.y,p2.y)), col);
                        ImVec2 w1 = ImPlot::PlotToPixels(activeBuf.time[i], activeBuf.high[i]);
                        ImVec2 w2 = ImPlot::PlotToPixels(activeBuf.time[i], activeBuf.low[i]);
                        w1.x = std::round(w1.x); w2.x = std::round(w2.x);
                        ImPlot::GetPlotDrawList()->AddLine(w1, w2, col);
                    }
                }
                ImU32 col = ImGui::GetColorU32(game.currentPrice >= activeBuf.curOpen ? bull : bear);
                ImVec2 p1 = ImPlot::PlotToPixels(continuousX-0.3, activeBuf.curOpen);
                ImVec2 p2 = ImPlot::PlotToPixels(continuousX+0.3, game.currentPrice);
                p1.x = round(p1.x); p1.y=round(p1.y); p2.x=round(p2.x); p2.y=round(p2.y);
                ImPlot::GetPlotDrawList()->AddRectFilled(ImVec2(p1.x, std::min(p1.y,p2.y)), ImVec2(p2.x, std::max(p1.y,p2.y)), col);
                ImVec2 w1 = ImPlot::PlotToPixels(continuousX, activeBuf.curHigh);
                ImVec2 w2 = ImPlot::PlotToPixels(continuousX, activeBuf.curLow);
                w1.x=round(w1.x); w2.x=round(w2.x);
                ImPlot::GetPlotDrawList()->AddLine(w1, w2, col);
            }
            ImPlot::PopPlotClipRect();
            ImPlot::EndPlot();
        }

        // HANDLE
        ImGui::InvisibleButton("Splitter", ImVec2(-1, 8));
        if (ImGui::IsItemActive()) game.splitHeight += ImGui::GetIO().MouseDelta.y;
        ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(30,30,30,255), 4.0f);

        // TIMEFRAMES
        ImGui::Dummy(ImVec2(0, 5));
        float btnW = (dw - 40) / 3.0f;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12);
        auto Tab = [&](const char* l, Timeframe t) {
            if(game.activeTimeframe==t) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.2f,0.25f,1));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.1f,0.12f,1));
            if(ImGui::Button(l, ImVec2(btnW, 25))) game.activeTimeframe = t;
            ImGui::PopStyleColor();
        };
        ImGui::BeginGroup();
        Tab("Realtime", TF_REALTIME); ImGui::SameLine(); Tab("Day", TF_DAY); ImGui::SameLine(); Tab("Month", TF_MONTH);
        ImGui::EndGroup();
        ImGui::PopStyleVar();

        // LOG
        ImGui::BeginChild("Log", ImVec2(0, dh-ImGui::GetCursorPosY()-140));
        for (auto& tx : game.log) {
            ImGui::TextColored(tx.type=="BUY"?ImVec4(0,1,0,1):ImVec4(1,0,0,1), "%s", tx.type.c_str());
            ImGui::SameLine(80); ImGui::Text("%d BTC", tx.qty);
            ImGui::SameLine(200); ImGui::Text("@ $%.2f", tx.price);
        }
        ImGui::EndChild();

        // CONTROLS
        ImGui::SetCursorPosY(dh - 130);
        float center = dw / 2.0f;
        ImGui::SetCursorPosX(center - 80);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15.0f);
        if (ImGui::Button("-", ImVec2(40, 40))) if(game.tradeQty > 1) game.tradeQty--;
        ImGui::SameLine();
        ImGui::PushItemWidth(60);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 10));
        std::string qtyLabel = std::to_string(game.tradeQty);
        ImGui::Button(qtyLabel.c_str(), ImVec2(60, 40));
        ImGui::PopStyleVar();
        ImGui::SameLine();
        if (ImGui::Button("+", ImVec2(40, 40))) game.tradeQty++;
        ImGui::PopItemWidth(); ImGui::PopStyleVar();

        // BUY/SELL
        ImGui::SetCursorPosY(dh - 70);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 25.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.32f, 1.0f, 1.0f));
        if(ImGui::Button("Buy", ImVec2(dw*0.46f, 50))) game.buy();
        ImGui::PopStyleColor(); ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
        if(ImGui::Button("Sell", ImVec2(dw*0.46f, 50))) game.sell();
        ImGui::PopStyleColor(); ImGui::PopStyleVar();

        ImGui::End();

        game.notifs.draw(dw);
        ImGui::Render();
        glViewport(0, 0, dw, dh);
        glClearColor(0.04f, 0.04f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown(); ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext(); ImGui::DestroyContext();
    glfwDestroyWindow(window); glfwTerminate();
    return 0;
}