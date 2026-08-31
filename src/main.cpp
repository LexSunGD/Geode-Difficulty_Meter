#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_customDifficultyMeter = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Construimos el nombre inicial usando el ID del mod registrado en tu mod.json
        std::string initialSprite = Mod::get()->getID() + "/NA_dif.png";

        // Crear el sprite buscando directamente en el caché de texturas unificado
        m_fields->m_customDifficultyMeter = CCSprite::createWithSpriteFrameName(initialSprite.c_str());
        
        if (m_fields->m_customDifficultyMeter) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Ubicar en la parte superior central de la pantalla
            m_fields->m_customDifficultyMeter->setPosition({ winSize.width / 2, winSize.height - 40.0f });
            
            // Escala fijada exactamente en 1.0f (360x360 píxeles nativos)
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

        // Cálculo del porcentaje utilizando m_levelLength (verificado en tus bindings)
        if (m_levelLength > 0.0f) {
            percentage = (m_player1->m_position.x / m_levelLength) * 100.0f;
            if (percentage > 100.0f) percentage = 100.0f;
            if (percentage < 0.0f) percentage = 0.0f;
        }

        std::string spriteName = "Normal_dif.png";

        // Estructura en una sola línea exacta por cada bloque condicional { }
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

        // Formateamos el nombre final con el prefijo del ID de tu mod para que Cocos2d-x lo localice
        std::string fullSpritePath = Mod::get()->getID() + "/" + spriteName;

        auto spriteFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(fullSpritePath.c_str());
        if (spriteFrame) {
            m_fields->m_customDifficultyMeter->setDisplayFrame(spriteFrame);
        }
    }
};
