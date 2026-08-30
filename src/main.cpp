#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_customDifficultyLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Crear etiqueta de texto con la tipografía oficial del juego
        m_fields->m_customDifficultyLabel = CCLabelBMFont::create("Cargando...", "bigFont.fnt");
        
        if (m_fields->m_customDifficultyLabel) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Posicionar arriba en el centro
            m_fields->m_customDifficultyLabel->setPosition({ winSize.width / 2, winSize.height - 40.0f });
            m_fields->m_customDifficultyLabel->setScale(0.6f);
            m_fields->m_customDifficultyLabel->setID("custom-difficulty-label"_spr);

            if (m_uiLayer) {
                m_uiLayer->addChild(m_fields->m_customDifficultyLabel);
            }
        }

        return true;
    }
    
    void update(float dt) {
        PlayLayer::update(dt);

        if (!m_fields->m_customDifficultyLabel) return;

        // Usamos el método oficial de Geometry Dash para obtener el porcentaje preciso
        float percentage = this->getCurrentPercent();

        std::string diffText = "Normal";

        // Lógica de porcentajes limpia en una sola línea por cada condicional
        if (percentage < 8.33f) { diffText = "N/A"; }
        else if (percentage < 16.66f) { diffText = "Auto"; }
        else if (percentage < 25.0f) { diffText = "Easy"; }
        else if (percentage < 33.33f) { diffText = "Normal"; }
        else if (percentage < 41.66f) { diffText = "Hard"; }
        else if (percentage < 50.0f) { diffText = "Harder"; }
        else if (percentage < 58.33f) { diffText = "Insane"; }
        else if (percentage < 66.66f) { diffText = "Easy Demon"; }
        else if (percentage < 75.0f) { diffText = "Medium Demon"; }
        else if (percentage < 83.33f) { diffText = "Hard Demon"; }
        else if (percentage < 91.66f) { diffText = "Insane Demon"; }
        else { diffText = "Extreme Demon"; }

        // Refrescar el texto en pantalla en cada frame de ejecución
        m_fields->m_customDifficultyLabel->setString(diffText.c_str());
    }
};
