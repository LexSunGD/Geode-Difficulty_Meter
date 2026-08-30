#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* m_customDifficultyLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Crear la etiqueta de texto con "N/A" por defecto
        m_fields->m_customDifficultyLabel = CCLabelBMFont::create("N/A", "bigFont.fnt");
        
        if (m_fields->m_customDifficultyLabel) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Ubicar en la parte superior central de la interfaz
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
        PlayLayer::update(dt); // Mantiene las funciones nativas corriendo

        if (!m_fields->m_customDifficultyLabel || !m_player1) return;

        float percentage = 0.0f;
        float endX = this->getEndPosition().x;

        // Calcular el porcentaje real usando el punto final exacto de la versión 2.2
        if (endX > 0.0f) {
            percentage = (m_player1->m_position.x / endX) * 100.0f;
            if (percentage > 100.0f) percentage = 100.0f;
            if (percentage < 0.0f) percentage = 0.0f;
        }

        std::string diffText = "Normal";

        // Lógica limpia en una sola línea por cada condicional { }
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

        m_fields->m_customDifficultyLabel->setString(diffText.c_str());
    }
};
