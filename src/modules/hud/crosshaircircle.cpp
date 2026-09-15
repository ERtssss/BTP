#include "crosshaircircle.hpp"
#include "modules/ModuleRegistry.hpp"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

static void hsvToRgb(
    float h,
    float s,
    float v,
    float& r,
    float& g,
    float& b
) {
    if (s <= 0.0f) {
        r = g = b = v;
        return;
    }

    h = std::fmod(h, 1.0f);
    if (h < 0.0f)
        h += 1.0f;

    h *= 6.0f;

    const int i = static_cast<int>(std::floor(h));
    const float f = h - static_cast<float>(i);

    const float p = v * (1.0f - s);
    const float q = v * (1.0f - s * f);
    const float t = v * (1.0f - s * (1.0f - f));

    switch (i % 6) {
        case 0:
            r = v;
            g = t;
            b = p;
            break;

        case 1:
            r = q;
            g = v;
            b = p;
            break;

        case 2:
            r = p;
            g = v;
            b = t;
            break;

        case 3:
            r = p;
            g = q;
            b = v;
            break;

        case 4:
            r = t;
            g = p;
            b = v;
            break;

        default:
            r = v;
            g = p;
            b = q;
            break;
    }
}

static uint32_t parseColor(const std::string& hex) {
    // #AARRGGBB
    if (hex.size() != 9 || hex[0] != '#')
        return 0xFFFFFFFF;

    char* end = nullptr;
    const unsigned long value =
        std::strtoul(hex.c_str() + 1, &end, 16);

    if (end != hex.c_str() + 9)
        return 0xFFFFFFFF;

    return static_cast<uint32_t>(value);
}

static std::string colorToHex(uint32_t color) {
    char buffer[10];

    std::snprintf(
        buffer,
        sizeof(buffer),
        "#%08X",
        color
    );

    return std::string(buffer);
}

} // namespace

CrosshairCircleModule::CrosshairCircleModule()
    : Module(
        "Crosshair Circle",
        "Draws a configurable circle around your crosshair."
    ) {
}

void CrosshairCircleModule::onFrame() {
    if (!enabled)
        return;

    std::vector<PLModMenu_DrawCommand> commands;

    uint32_t color = parseColor(m_colorHex);

    // Rainbow.
    if (m_rainbow) {
        m_rainbowHue += 0.002f * m_rainbowSpeed;

        if (m_rainbowHue >= 1.0f)
            m_rainbowHue -= 1.0f;

        float r;
        float g;
        float b;

        hsvToRgb(
            m_rainbowHue,
            1.0f,
            1.0f,
            r,
            g,
            b
        );

        const uint32_t alpha = color & 0xFF000000;

        color =
            alpha |
            (static_cast<uint32_t>(r * 255.0f) << 16) |
            (static_cast<uint32_t>(g * 255.0f) << 8) |
            static_cast<uint32_t>(b * 255.0f);
    }

    /*
     * Filled circle.
     *
     * CircleFilled уже реально используется в debugmenu.cpp:
     *
     *   cmd.type = PL_DRAW_CIRCLE_FILLED;
     *   cmd.x = centerX;
     *   cmd.y = centerY;
     *   cmd.size = radius;
     *   cmd.color = color;
     */
    if (!m_outline) {
        PLModMenu_DrawCommand circle{};
        circle.type = PL_DRAW_CIRCLE_FILLED;
        circle.x = hudPosX;
        circle.y = hudPosY;
        circle.size = m_radius;
        circle.color = color;

        commands.push_back(circle);
    } else {
        /*
         * У LeviLauncher DrawCommand нет отдельного
         * CircleOutline/Circle.
         *
         * Поэтому контур строится из Line-команд.
         */
        constexpr int segments = 48;
        constexpr float twoPi = 6.28318530717958647692f;

        const float radius =
            std::max(0.0f, m_radius);

        const float thickness =
            std::max(0.1f, m_outlineThickness);

        float previousX =
            hudPosX + radius;

        float previousY =
            hudPosY;

        for (int i = 1; i <= segments; ++i) {
            const float angle =
                twoPi *
                (static_cast<float>(i) /
                 static_cast<float>(segments));

            const float nextX =
                hudPosX +
                std::cos(angle) * radius;

            const float nextY =
                hudPosY +
                std::sin(angle) * radius;

            PLModMenu_DrawCommand line{};
            line.type = PL_DRAW_LINE;

            line.x = previousX;
            line.y = previousY;

            line.w = nextX - previousX;
            line.h = nextY - previousY;

            line.size = thickness;
            line.color = color;

            commands.push_back(line);

            previousX = nextX;
            previousY = nextY;
        }
    }

    submitDrawCommands(moduleId, commands);
}

void CrosshairCircleModule::loadConfig(
    const nlohmann::json& j
) {
    Module::loadConfig(j);

    if (j.contains("hudPosX"))
        hudPosX = j["hudPosX"].get<float>();

    if (j.contains("hudPosY"))
        hudPosY = j["hudPosY"].get<float>();

    if (j.contains("isHudModule"))
        isHudModule = j["isHudModule"].get<bool>();

    if (j.contains("m_radius"))
        m_radius = j["m_radius"].get<float>();

    if (j.contains("m_outline"))
        m_outline = j["m_outline"].get<bool>();

    if (j.contains("m_outlineThickness"))
        m_outlineThickness =
            j["m_outlineThickness"].get<float>();

    if (j.contains("color"))
        m_colorHex =
            j["color"].get<std::string>();

    if (j.contains("m_rainbow"))
        m_rainbow =
            j["m_rainbow"].get<bool>();

    if (j.contains("m_rainbowSpeed"))
        m_rainbowSpeed =
            j["m_rainbowSpeed"].get<float>();

    if (m_radius < 0.0f)
        m_radius = 0.0f;

    if (m_outlineThickness < 0.1f)
        m_outlineThickness = 0.1f;

    if (m_rainbowSpeed < 0.0f)
        m_rainbowSpeed = 0.0f;
}

void CrosshairCircleModule::saveConfig(
    nlohmann::json& j
) {
    Module::saveConfig(j);

    j["hudPosX"] = hudPosX;
    j["hudPosY"] = hudPosY;
    j["isHudModule"] = isHudModule;

    j["m_radius"] = m_radius;
    j["m_outline"] = m_outline;
    j["m_outlineThickness"] = m_outlineThickness;

    j["color"] = m_colorHex;

    j["m_rainbow"] = m_rainbow;
    j["m_rainbowSpeed"] = m_rainbowSpeed;
}