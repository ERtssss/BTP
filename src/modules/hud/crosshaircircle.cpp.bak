#include "crosshaircircle.hpp"
#include "../../render/RenderBatch.hpp"
#include "../../render/RenderCommand.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {

// Тот же алгоритм HSV->RGB, что уже используется в keystrokes.cpp и
// breakindicator.cpp для опции "Rainbow" -- локальная копия, чтобы модуль
// оставался независимым файлом.
void crosshairCircleHSVtoRGB(float h, float s, float v, float& outR, float& outG, float& outB) {
    if (s == 0.0f) {
        outR = outG = outB = v;
        return;
    }
    h = std::fmod(h, 1.0f) * 6.0f;
    const int i = static_cast<int>(std::floor(h));
    const float f = h - static_cast<float>(i);
    const float p = v * (1.0f - s);
    const float q = v * (1.0f - s * f);
    const float t = v * (1.0f - s * (1.0f - f));
    switch (i) {
        case 0: outR = v; outG = t; outB = p; break;
        case 1: outR = q; outG = v; outB = p; break;
        case 2: outR = p; outG = v; outB = t; break;
        case 3: outR = p; outG = q; outB = v; break;
        case 4: outR = t; outG = p; outB = v; break;
        default: outR = v; outG = p; outB = q; break;
    }
}

// "#AARRGGBB" -> Color. Формат идентичен m_pressedColor в keystrokes.cpp.
bedrocktoolsplus::render::Color parseColorHex(const std::string& hex) {
    bedrocktoolsplus::render::Color color{1.0f, 1.0f, 1.0f, 1.0f};
    if (hex.size() != 9 || hex[0] != '#') return color;
    char* end = nullptr;
    const unsigned long argb = std::strtoul(hex.c_str() + 1, &end, 16);
    if (end == hex.c_str() + 1) return color;
    color.a = static_cast<float>((argb >> 24) & 0xFF) / 255.0f;
    color.r = static_cast<float>((argb >> 16) & 0xFF) / 255.0f;
    color.g = static_cast<float>((argb >> 8) & 0xFF) / 255.0f;
    color.b = static_cast<float>(argb & 0xFF) / 255.0f;
    return color;
}

// Color -> "#AARRGGBB".
std::string colorToHex(const bedrocktoolsplus::render::Color& color) {
    char buf[10];
    std::snprintf(buf, sizeof(buf), "#%08X", color.toArgb32());
    return std::string(buf);
}

} // namespace

CrosshairCircleModule::CrosshairCircleModule()
    : Module("Crosshair Circle", "Draws a configurable circle around your crosshair.") {
    m_color = parseColorHex(m_colorHex);
}

void CrosshairCircleModule::onFrame() {
    if (!enabled) return;

    using bedrocktoolsplus::render::RenderBatch;
    using bedrocktoolsplus::render::Vec2;
    using bedrocktoolsplus::render::makeCircleFilled;
    using bedrocktoolsplus::render::makeLine;

    if (m_rainbow) {
        m_rainbowHue += 0.002f * m_rainbowSpeed;
        if (m_rainbowHue > 1.0f) m_rainbowHue -= 1.0f;
        float r, g, b;
        crosshairCircleHSVtoRGB(m_rainbowHue, 1.0f, 1.0f, r, g, b);
        m_color.r = r;
        m_color.g = g;
        m_color.b = b; // альфа остаётся той, что задана цветом в конфиге
    }

    RenderBatch batch(moduleId);
    const Vec2 center{hudPosX, hudPosY};

    if (!m_outline) {
        // Реально существующий примитив -- один DrawCommandType::CircleFilled.
        batch.add(makeCircleFilled(center, m_radius, m_color));
    } else {
        // Кольцо = правильный многоугольник из сегментов Line. Число
        // сегментов фиксировано (не вынесено в конфиг), чтобы не плодить
        // лишние настройки сверх того, о чём просили.
        constexpr int segments = 48;
        constexpr float kTwoPi = 6.28318530718f;
        Vec2 prev{center.x + m_radius, center.y};
        for (int i = 1; i <= segments; ++i) {
            const float angle = (static_cast<float>(i) / static_cast<float>(segments)) * kTwoPi;
            const Vec2 next{center.x + m_radius * std::cos(angle),
                             center.y + m_radius * std::sin(angle)};
            batch.add(makeLine(prev, next, m_outlineThickness, m_color));
            prev = next;
        }
    }

    batch.flush();
}

void CrosshairCircleModule::loadConfig(const nlohmann::json& j) {
    Module::loadConfig(j);
    if (j.contains("m_radius")) m_radius = j["m_radius"].get<float>();
    if (j.contains("m_outline")) m_outline = j["m_outline"].get<bool>();
    if (j.contains("m_outlineThickness")) m_outlineThickness = j["m_outlineThickness"].get<float>();
    if (j.contains("m_rainbow")) m_rainbow = j["m_rainbow"].get<bool>();
    if (j.contains("m_rainbowSpeed")) m_rainbowSpeed = j["m_rainbowSpeed"].get<float>();
    if (j.contains("color")) {
        m_colorHex = j["color"].get<std::string>();
        m_color = parseColorHex(m_colorHex);
    }
    if (j.contains("hudPosX")) hudPosX = j["hudPosX"].get<float>();
    if (j.contains("hudPosY")) hudPosY = j["hudPosY"].get<float>();
    if (j.contains("isHudModule")) isHudModule = j["isHudModule"].get<bool>();
}

void CrosshairCircleModule::saveConfig(nlohmann::json& j) {
    Module::saveConfig(j);
    j["m_radius"] = m_radius;
    j["m_outline"] = m_outline;
    j["m_outlineThickness"] = m_outlineThickness;
    j["m_rainbow"] = m_rainbow;
    j["m_rainbowSpeed"] = m_rainbowSpeed;
    j["color"] = colorToHex(m_color);
    j["hudPosX"] = hudPosX;
    j["hudPosY"] = hudPosY;
    j["isHudModule"] = isHudModule;
}
