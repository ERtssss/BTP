#pragma once

#include "../Module.hpp"
#include "../../render/RenderTypes.hpp"

// CrosshairCircleModule -- настраиваемый круг/кольцо вокруг прицела.
//
// Рисуется через bedrocktoolsplus::render (RenderBatch), который сам
// конвертирует команды в pl::modmenu::DrawCommand. Реальный
// DrawCommandType (см. src/modules/ModuleRegistry.hpp:12-20) содержит
// ТОЛЬКО CircleFilled -- отдельного "Circle" (контур) нет. Поэтому:
//   - m_outline == false -> сплошной закрашенный круг (DrawCommandType::CircleFilled)
//   - m_outline == true  -> кольцо-контур, собранное из сегментов Line
//     (тоже подтверждённый тип), а не выдуманный примитив.
//
// Позиционирование: как и у остальных HUD-модулей (compass, breakindicator
// и т.д.), координаты hudPosX/hudPosY -- перетаскиваемая точка в редакторе
// HUD LeviLauncher. У модуля нет доступа к реальному разрешению экрана
// (в исходниках BedrockTools-main такого API не нашлось), поэтому центр
// круга нужно один раз перетащить точно на прицел через редактор HUD --
// как и с любым другим HUD-элементом в этом проекте.
class CrosshairCircleModule : public Module {
public:
    CrosshairCircleModule();

    void onFrame() override;
    void loadConfig(const nlohmann::json& j) override;
    void saveConfig(nlohmann::json& j) override;

private:
    // Перетаскиваемый центр круга в редакторе HUD.
    float hudPosX = 540.0f;
    float hudPosY = 960.0f;
    bool isHudModule = true;

    float m_radius = 6.0f;

    bool m_outline = true;
    float m_outlineThickness = 1.5f;

    std::string m_colorHex = "#FFFFFFFF"; // "#AARRGGBB", как в keystrokes.cpp
    bedrocktoolsplus::render::Color m_color{1.0f, 1.0f, 1.0f, 1.0f}; // кэш m_colorHex, чтобы не парсить строку каждый кадр

    bool m_rainbow = false;
    float m_rainbowSpeed = 1.0f;
    float m_rainbowHue = 0.0f; // не сохраняется, только рантайм-анимация
};
