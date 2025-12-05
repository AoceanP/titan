# Titan Mobile

**A High-Performance C++ Trading Simulator & Visualization Engine.**

<img width="441" height="881" alt="Screenshot 2025-12-05 072527" src="https://github.com/user-attachments/assets/b9d58885-9a0b-4f52-9efa-2c85d08505fe" />

## Overview

**Titan Mobile** is a financial trading simulation built from scratch using **Modern C++20**. Unlike standard charting libraries that just plot static points, Titan features a custom-built **Real-Time Physics Engine** that simulates market volatility using **Geometric Brownian Motion (GBM)**.

The goal was to engineer a fintech-grade user experience—60 FPS, smooth scrolling, and pixel-perfect rendering—running natively on the GPU via OpenGL.

## Key Features

* **Physics-Based Volatility:** Prices are generated using a physics sub-stepping loop to simulate organic market drift and shock, rather than random noise.
* **Custom Candlestick Renderer:** Implements a custom plotting pipeline that draws "Ghost Candles" (active price formation) and historical data with pixel-snapping to prevent anti-aliasing bleed.
* **Smart Camera System:** Features a custom linear-interpolation (Lerp) camera that smooths out Y-axis jitters, ensuring the chart glides rather than snaps during high volatility.
* **Mobile-First Architecture:** Designed with a vertical layout, touch-friendly targets, and a high-contrast dark mode theme suitable for mobile displays.
* **Interactive Trading:** Buy and sell assets (Doughnuts) in real-time with instant portfolio updates and P&L tracking.

## Tech Stack

* **Language:** C++20 (Modern Standard)
* **Build System:** CMake
* **Rendering:** OpenGL 3.0 + GLFW
* **UI Framework:** Dear ImGui (Docking Branch)
* **Plotting:** ImPlot (Customized)
* **Math:** Standard Library Random & CMath (GBM Algorithms)

## Architecture

The project follows a strict **Model-View-Controller (MVC)** separation to ensure scalability:

* **TitanEngine (The Brain):** Handles the math, money logic, circular buffers for time-series data, and the physics update loop. It has zero knowledge of the UI.
* **TitanGUI (The Face):** Handles the OpenGL context, texture loading, style definitions, and ImGui render calls.
* **main.cpp (The Conductor):** Connects the Engine to the GUI and manages the main application loop.

## Getting Started

### Prerequisites
* **CMake** (3.20 or higher)
* **C++ Compiler** (GCC, Clang, or MSVC) supporting C++20.

### Installation

1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/your-username/titan.git](https://github.com/your-username/titan.git)
    cd titan
    ```

2.  **Build using CMake:**
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

3.  **Run the Simulation:**
    * *Note: Ensure rocket.jpg is in the same directory as the executable.*
    ```bash
    ./Titan
    ```

## Controls

* **Day / Month Tabs:** Switch between timeframes. The engine automatically resamples the volatility physics to match the timeframe.
* **Buy / Sell Buttons:** Instantly execute market orders.
* **Interactive Chart:** The chart automatically scrolls to follow live price action.

---
*Built with C++ by Aleksandar Panich*
