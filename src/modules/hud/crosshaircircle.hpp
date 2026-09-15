#pragma once

#include "../Module.hpp"

#include <cstdint>
#include <string>

class CrosshairCircleModule : public Module {
public:
    CrosshairCircleModule();

    void onFrame() override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;

private:
    // Позиция центра круга в HUD-координатах.
    float hudPosX = 540.0f;
    float hudPosY = 960.0f;
    bool isHudModule = true;

    // Радиус круга.
    float m_radius = 6.0f;

    // false = заполненный круг.
    // true  = контур из Line-команд.
    bool m_outline = true;

    // Толщина контура.
    float m_outlineThickness = 1.5f;

    // Цвет в формате #AARRGGBB.
    std::string m_colorHex = "#FFFFFFFF";

    // Rainbow.
    bool m_rainbow = false;
    float m_rainbowSpeed = 1.0f;
    float m_rainbowHue = 0.0f;
};