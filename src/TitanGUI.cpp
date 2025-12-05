#include "TitanGUI.hpp"

// THIS DEFINES THE IMAGE LOADER LOGIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void SetupFintechStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f; style.FrameRounding = 8.0f;
    style.GrabRounding = 8.0f; style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 10.0f; style.WindowPadding = ImVec2(16, 16);
    style.ItemSpacing = ImVec2(10, 12); style.FramePadding = ImVec2(10, 10);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]       = ImVec4(0.04f, 0.04f, 0.05f, 1.00f);
    colors[ImGuiCol_ChildBg]        = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_Text]           = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]         = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
    colors[ImGuiCol_ButtonHovered]  = ImVec4(0.18f, 0.18f, 0.20f, 1.0f);
    colors[ImGuiCol_ButtonActive]   = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
    colors[ImGuiCol_Separator]      = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
}

bool LoadTextureFromFile(const char* filename, unsigned int* out_texture, int* out_width, int* out_height) {
    int image_width = 0; int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL) return false;

    unsigned int image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);

    stbi_image_free(image_data);
    *out_texture = image_texture; *out_width = image_width; *out_height = image_height;
    return true;
}

void DrawToggle(const char* label1, const char* label2, int* modeVar) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
    bool isMode0 = (*modeVar == 0);

    ImGui::PushStyleColor(ImGuiCol_Button, isMode0 ? ImVec4(0.2f,0.2f,0.2f,1) : ImVec4(0.08f,0.08f,0.08f,1));
    if (ImGui::Button(label1, ImVec2(50, 25))) *modeVar = 0;
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, !isMode0 ? ImVec4(0.2f,0.2f,0.2f,1) : ImVec4(0.08f,0.08f,0.08f,1));
    if (ImGui::Button(label2, ImVec2(50, 25))) *modeVar = 1;
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}