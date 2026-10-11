#pragma once

#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace cadenza::ui::genos
{
struct Vec3 { float x{}, y{}, z{}; };
struct Point { float x{}, y{}, depth{}; };
struct Face
{
    std::array<Vec3, 4> vertices;
    std::uint32_t colour;
    int control = -1;
    bool top = false;
};
struct Control
{
    enum class Kind { button, key, fader, wheel, knob };
    Kind kind = Kind::button;
    std::string id, label, hint;
    Vec3 centre;
    float width{}, depth{}, height{};
    int value = 0;
    bool black = false;
};
struct Camera
{
    float yaw = -0.06f, pitch = 1.10f, zoom = 1.0f;
    Point project(Vec3 p, float width, float height) const
    {
        const float x = p.x * std::cos(yaw) - p.z * std::sin(yaw);
        const float z = p.x * std::sin(yaw) + p.z * std::cos(yaw);
        const float scale = std::min(width / 13.7f, height / 5.5f) * zoom;
        return {width * 0.5f + x * scale,
                height * 0.52f + (z * std::sin(pitch) - p.y * std::cos(pitch)) * scale,
                z * std::cos(pitch) + p.y * std::sin(pitch)};
    }
};

inline bool isBlack(int note)
{
    const int p = note % 12;
    return p == 1 || p == 3 || p == 6 || p == 8 || p == 10;
}

inline std::vector<Control> makeControls()
{
    std::vector<Control> result;
    auto button = [&](std::string id, std::string label, float x, float z,
                      float width = 0.30f, std::string hint = {}) {
        result.push_back({Control::Kind::button, std::move(id), std::move(label),
                          std::move(hint), {x, 0.39f, z}, width, 0.20f, 0.07f});
    };
    button("style", "STYLE", -5.15f, -1.46f, 0.65f, "Ouvrir un style Cadenza ou Yamaha");
    button("midi", "MIDI", -5.15f, -0.95f, 0.65f, "Choisir le clavier MIDI et ses commandes");
    button("audio", "AUDIO", -5.15f, -0.45f, 0.65f, "Choisir la sortie audio");
    button("transpose-", "−", -4.64f, -1.72f);
    button("transpose+", "+", -4.25f, -1.72f);
    button("tempo-", "−", -4.64f, -1.22f);
    button("tempo+", "+", -4.25f, -1.22f);
    button("acmp", "ACMP", -4.55f, -0.40f, 0.63f);
    button("memory", "HOLD", -3.86f, -0.40f, 0.57f);
    for (int i = 0; i < 9; ++i)
        result.push_back({Control::Kind::fader, "fader" + std::to_string(i),
                          std::to_string(i + 1), "Glisser pour régler le volume de la partie",
                          {-3.54f + i * 0.28f, 0.43f, -1.05f}, 0.14f, 0.65f, 0.05f, i});
    constexpr const char* knobLabels[]={"REV","COMP","VOL","LOW","MID","HIGH"};
    for(int i=0;i<6;++i)
        result.push_back({Control::Kind::knob,"knob"+std::to_string(i),knobLabels[i],
            "Glisser pour régler "+std::string(knobLabels[i]),
            {-3.55f+i*.44f,.42f,-1.60f},.23f,.23f,.13f,i});
    button("creator", "STYLE CREATOR", 0.0f, -0.14f, 1.58f,
           "Ouvrir l'éditeur de styles existant de Cadenza");
    button("soundfont", "SONS", 2.08f, -1.75f, 0.65f);
    button("import", "MIDI → STYLE", 3.12f, -1.75f, 1.20f);
    button("native", "CADENZA", 4.48f, -1.75f, 0.97f);
    button("rec", "REC", 2.22f, -1.25f, 0.55f);
    button("save", "SAVE", 2.90f, -1.25f, 0.60f);
    button("ots0", "1", 2.10f, -0.44f);
    button("ots1", "2", 2.47f, -0.44f);
    button("ots2", "3", 2.84f, -0.44f);
    button("ots3", "4", 3.21f, -0.44f);
    for (int i = 0; i < 3; ++i)
        button("right" + std::to_string(i), "R" + std::to_string(i + 1),
               3.77f + i * 0.46f, -0.44f, 0.38f);
    button("left", "LEFT", 5.18f, -0.44f, 0.53f);
    button("introA", "I", -5.0f, 0.11f);
    button("introB", "II", -4.63f, 0.11f);
    button("introC", "III", -4.26f, 0.11f);
    for (int i = 0; i < 4; ++i)
        button(std::string("main") + char('A' + i), std::string(1, char('A' + i)),
               -3.65f + i * 0.40f, 0.11f);
    button("fill", "FILL", -1.87f, 0.11f, 0.52f);
    button("break", "BREAK", -1.25f, 0.11f, 0.60f);
    button("endingA", "I", 0.12f, 0.30f);
    button("endingB", "II", 0.49f, 0.30f);
    button("endingC", "III", 0.86f, 0.30f);
    button("play", "START / STOP", 2.13f, 0.21f, 1.38f);
    button("autofill", "AUTO FILL", 3.41f, 0.21f, 0.91f);
    button("syncstop", "SYNC STOP", 4.55f, 0.21f, 1.01f);
    result.push_back({Control::Kind::wheel, "pitch", "PITCH", "Glisser ; relâcher pour recentrer",
                      {-5.56f, 0.35f, 1.30f}, 0.26f, 0.77f, 0.19f});
    result.push_back({Control::Kind::wheel, "mod", "MOD", "Glisser pour la modulation",
                      {-5.93f, 0.35f, 1.30f}, 0.23f, 0.77f, 0.19f});

    constexpr int first = 28, last = 103;
    int whiteCount = 0;
    for (int n = first; n <= last; ++n) whiteCount += !isBlack(n);
    const float keyWidth = 10.58f / static_cast<float>(whiteCount);
    const float start = -5.08f;
    int whiteIndex = 0;
    for (int n = first; n <= last; ++n)
    {
        const bool black = isBlack(n);
        const float x = start + (black ? whiteIndex : whiteIndex + 0.5f) * keyWidth;
        result.push_back({Control::Kind::key, "note" + std::to_string(n), "",
                          "Jouer la note " + std::to_string(n),
                          {x, black ? 0.37f : 0.20f, black ? 1.04f : 1.43f},
                          keyWidth * (black ? 0.61f : 0.96f), black ? 0.78f : 1.58f,
                          black ? 0.15f : 0.12f, n, black});
        if (!black) ++whiteIndex;
    }
    return result;
}

inline void box(std::vector<Face>& faces, Vec3 c, float w, float d, float h,
                std::uint32_t colour, int control = -1)
{
    const float l = c.x - w / 2, r = c.x + w / 2;
    const float back = c.z - d / 2, front = c.z + d / 2;
    const float bottom = c.y, top = c.y + h;
    faces.push_back({{{{l,top,back},{r,top,back},{r,top,front},{l,top,front}}}, colour, control, true});
    faces.push_back({{{{l,bottom,front},{l,top,front},{r,top,front},{r,bottom,front}}}, colour, control});
    faces.push_back({{{{l,bottom,back},{r,bottom,back},{r,top,back},{l,top,back}}}, colour, control});
    faces.push_back({{{{l,bottom,back},{l,top,back},{l,top,front},{l,bottom,front}}}, colour, control});
    faces.push_back({{{{r,bottom,back},{r,bottom,front},{r,top,front},{r,top,back}}}, colour, control});
}

inline bool contains(const std::array<Point,4>& polygon, float x, float y)
{
    bool inside = false;
    for (int i = 0, j = 3; i < 4; j = i++)
    {
        const auto& a = polygon[i]; const auto& b = polygon[j];
        if (((a.y > y) != (b.y > y))
            && x < (b.x-a.x)*(y-a.y)/(b.y-a.y)+a.x) inside = !inside;
    }
    return inside;
}
}
