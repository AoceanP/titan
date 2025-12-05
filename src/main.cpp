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

        // --- 1. HEADER (Price on Left) ---
        ImGui::Dummy(ImVec2(0, 20));
        ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,1.0f), "CURRENT PRICE");

        // Share Price (Left Side)
        ImGui::SetWindowFontScale(3.5f);
        ImGui::Text("$%.2f", game.currentPrice);
        ImGui::SetWindowFontScale(1.0f);

        // Portfolio Stats (Right Side - Small)
        ImGui::SameLine(dw - 150);
        ImGui::BeginGroup();
        ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,1.0f), "NET WORTH");
        double netWorth = game.cash + (game.shares * game.currentPrice);
        ImGui::Text("$%.2f", netWorth);
        ImGui::Dummy(ImVec2(0,5));
        ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,1.0f), "CASH");
        ImGui::Text("$%.2f", game.cash);
        ImGui::EndGroup();

        ImGui::Dummy(ImVec2(0, 10));

        // --- 2. TIMEFRAME TABS ---
        float btnW = (dw - 40) / 2.0f;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12);
        auto Tab = [&](const char* l, Timeframe t) {
            if(game.activeTimeframe==t) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f,0.2f,0.25f,1));
            else ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f,0.1f,0.12f,1));
            if(ImGui::Button(l, ImVec2(btnW, 30))) game.activeTimeframe = t;
            ImGui::PopStyleColor();
        };
        ImGui::BeginGroup();
        Tab("Day", TF_DAY);
        ImGui::SameLine();
        Tab("Month", TF_MONTH);
        ImGui::EndGroup();
        ImGui::PopStyleVar();

        ImGui::Dummy(ImVec2(0, 10));

        // --- 3. CHART ---
        // Fill available space until buttons
        float bottomHeight = 100.0f; // Height for buttons
        float chartHeight = dh - ImGui::GetCursorPosY() - bottomHeight;

        CandleBuffer& activeBuf = game.buffers[game.activeTimeframe];
        double dataMinY, dataMaxY;
        activeBuf.getMinMax(dataMinY, dataMaxY);
        if(game.currentPrice > dataMaxY) dataMaxY = game.currentPrice;
        if(game.currentPrice < dataMinY) dataMinY = game.currentPrice;
        double spread = dataMaxY - dataMinY; if(spread < 1.0) spread = 1.0;

        double lerp = 5.0 * ImGui::GetIO().DeltaTime;
        game.camMinY += ((dataMinY - spread*0.2) - game.camMinY) * lerp;
        game.camMaxY += ((dataMaxY + spread*0.2) - game.camMaxY) * lerp;

        double currentX = activeBuf.time.empty() ? 0.0 : activeBuf.time.back();
        double progress = activeBuf.timeAccumulator / game.durations[game.activeTimeframe];
        double continuousX = currentX + progress;
        game.camMaxX = continuousX + 1.5; game.camMinX = game.camMaxX - 15.0;

        if (ImPlot::BeginPlot("##Chart", ImVec2(-1, chartHeight), ImPlotFlags_NoTitle|ImPlotFlags_NoLegend|ImPlotFlags_NoMenus|ImPlotFlags_NoBoxSelect)) {
            ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_NoDecorations);
            ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoDecorations);
            ImPlot::SetupAxisLimits(ImAxis_X1, game.camMinX, game.camMaxX, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, game.camMinY, game.camMaxY, ImGuiCond_Always);

            ImPlot::PushPlotClipRect();

            ImVec4 bull = ImVec4(0.0f,0.85f,0.35f,1.0f);
            ImVec4 bear = ImVec4(1.0f,0.23f,0.19f,1.0f);

            // Draw Candles
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
            // Draw Ghost
            ImU32 col = ImGui::GetColorU32(game.currentPrice >= activeBuf.curOpen ? bull : bear);
            ImVec2 p1 = ImPlot::PlotToPixels(continuousX-0.3, activeBuf.curOpen);
            ImVec2 p2 = ImPlot::PlotToPixels(continuousX+0.3, game.currentPrice);
            p1.x = round(p1.x); p1.y=round(p1.y); p2.x=round(p2.x); p2.y=round(p2.y);
            ImPlot::GetPlotDrawList()->AddRectFilled(ImVec2(p1.x, std::min(p1.y,p2.y)), ImVec2(p2.x, std::max(p1.y,p2.y)), col);
            ImVec2 w1 = ImPlot::PlotToPixels(continuousX, activeBuf.curHigh);
            ImVec2 w2 = ImPlot::PlotToPixels(continuousX, activeBuf.curLow);
            w1.x=round(w1.x); w2.x=round(w2.x);
            ImPlot::GetPlotDrawList()->AddLine(w1, w2, col);

            // ROCKET (Draw on top of candles)
            if (game.rocketTexture != 0) {
                ImVec2 size(1.5, 1.5 * ((double)game.rocketH/game.rocketW));
                ImPlot::PlotImage("Rocket", (ImTextureID)(intptr_t)game.rocketTexture,
                    ImPlotPoint(continuousX, game.currentPrice), ImPlotPoint(size.x, size.y), ImVec2(1,0), ImVec2(0,1));
            }

            ImPlot::PopPlotClipRect();
            ImPlot::EndPlot();
        }

        // --- 4. BOTTOM BUTTONS ---
        ImGui::SetCursorPosY(dh - 80);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 25.0f);

        // Simple Buy Button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.00f, 0.32f, 1.0f, 1.0f));
        if(ImGui::Button("Buy 1 Share", ImVec2(dw*0.46f, 60))) game.buy();
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Simple Sell Button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
        if(ImGui::Button("Sell 1 Share", ImVec2(dw*0.46f, 60))) game.sell();
        ImGui::PopStyleColor();

        ImGui::PopStyleVar();

        ImGui::End(); // End App Window

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