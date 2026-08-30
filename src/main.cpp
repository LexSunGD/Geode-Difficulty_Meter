#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    // Estructura obligatoria en Geode para añadir variables a un hook
    struct Fields {
        CCSprite* m_customDifficultyMeter = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Crear el sprite usando el caché de Geode
        m_fields->m_customDifficultyMeter = CCSprite::createWithSpriteFrameName("NA_dif.png");
        
        if (m_fields->m_customDifficultyMeter) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Posicionar en la parte superior central
            m_fields->m_customDifficultyMeter->setPosition({ winSize.width / 2, winSize.height - 30.0f });
            
            // Forzar a que mantenga exactamente el tamaño de escala correcto (Ej: 0.5f o 1.0f según tu preferencia visual)
            m_fields->m_customDifficultyMeter->setScale(0.5f);
            m_fields->m_customDifficultyMeter->setID("custom-difficulty-meter"_spr);

            if (m_uiLayer) {
                m_uiLayer->addChild(m_fields->m_customDifficultyMeter);
            }
        }

        return true;
    }
    
    void update(float dt) {
        PlayLayer::update(dt);

        if (!m_fields->m_customDifficultyMeter || m_levelLength <= 0.0f) return;

        float percentage = (m_player1->m_position.x / m_levelLength) * 100.0f;
        if (percentage > 100.0f) percentage = 100.0f;
        if (percentage < 0.0f) percentage = 0.0f;

        std::string spriteName = "Normal_dif.png";

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

        auto spriteFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(spriteName.c_str());
        if (spriteFrame) {
            m_fields->m_customDifficultyMeter->setDisplayFrame(spriteFrame);
        }
    }
};
