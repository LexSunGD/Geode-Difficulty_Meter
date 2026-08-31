#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_customDifficultyMeter = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Para archivos PNG sueltos se usa create() en lugar de createWithSpriteFrameName
        m_fields->m_customDifficultyMeter = CCSprite::create("NA_dif.png");
        
        if (m_fields->m_customDifficultyMeter) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Ubicar en la parte superior central de la interfaz
            m_fields->m_customDifficultyMeter->setPosition({ winSize.width / 2, winSize.height - 50.0f });
            
            // Forzar la escala exactamente a 1.0f (360x360 píxeles reales)
            m_fields->m_customDifficultyMeter->setScale(1.0f); 
            m_fields->m_customDifficultyMeter->setID("custom-difficulty-meter"_spr);

            if (m_uiLayer) {
                m_uiLayer->addChild(m_fields->m_customDifficultyMeter);
            }
        }

        return true;
    }
    
    void update(float dt) {
        PlayLayer::update(dt); 

        if (!m_fields->m_customDifficultyMeter || !m_player1) return;

        float percentage = 0.0f;

        // Cálculo estable del progreso del nivel
        if (m_levelLength > 0.0f) {
            percentage = (m_player1->m_position.x / m_levelLength) * 100.0f;
            if (percentage > 100.0f) percentage = 100.0f;
            if (percentage < 0.0f) percentage = 0.0f;
        }

        std::string spriteName = "Normal_dif.png";

        // Lógica limpia en una sola línea por cada condicional asignando las variables { }
        if (percentage < 8.33f) { spriteName = "NA_dif.png"; }
        else if (percentage < 16.66f) { spriteName = "Auto_dif.png"; }
        else if (percentage < 25.0f) { spriteName = "Easy_dif.png"; }
        else if (percentage < 33.33f) { spriteName = "Normal_dif.png"; }
        else if (percentage < 41.66f) { spriteName = "Hard_dif.png"; }
        else if (percentage < 50.0f) { spriteName = "Harder_dif.png"; }
        else if (percentage < 58.33f) { spriteName = "Insane_dif.png"; }
        else if (percentage < 66.66f) { spriteName = "EasyDemon_dif.png"; }
        else if (percentage < 75.0f) { spriteName = "MediumDemon_dif.png"; }
        else if (percentage < 83.33f) { spriteName = "HardDemon_dif.png"; }
        else if (percentage < 91.66f) { spriteName = "InsaneDemon_dif.png"; }
        else { spriteName = "ExtremeDemon_dif.png"; }

        // Creamos una nueva textura en runtime para refrescar el archivo PNG suelto
        auto textureCache = CCTextureCache::sharedTextureCache();
        auto newTexture = textureCache->addImage(spriteName.c_str());
        
        if (newTexture) {
            m_fields->m_customDifficultyMeter->setTexture(newTexture);
            
            // Reajustamos las dimensiones del rectángulo interno para que no se estire ni se bugee
            CCRect rect = CCRectZero;
            rect.size = newTexture->getContentSize();
            m_fields->m_customDifficultyMeter->setTextureRect(rect);
        }
    }
};
