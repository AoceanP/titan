#pragma once
#include "imgui.h"
#include <GLFW/glfw3.h> // Provides GLuint and OpenGL headers

// Windows Compatibility: Define constants if the system header is old
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_TEXTURE_2D
#define GL_TEXTURE_2D 0x0DE1
#endif

// Helper Declarations
void SetupFintechStyle();
bool LoadTextureFromFile(const char* filename, unsigned int* out_texture, int* out_width, int* out_height);
void DrawToggle(const char* label1, const char* label2, int* modeVar);