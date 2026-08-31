#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Lista de tus 12 imágenes personalizadas para cargarlas en memoria al iniciar el juego
const std::vector<std::string> CUSTOM_SPRITES = {
    "NA_dif.png", "Auto_dif.png", "Easy_dif.png", "Normal_dif.png",
    "Hard_dif.png", "Harder_dif.png", "Insane_dif.png", "EasyDemon_dif.png",
    "MediumDemon_dif.png", "HardDemon_dif.png", "InsaneDemon_dif.png", "ExtremeDemon_dif.png"
};

// Este bloque le indica a Geode que cargue físicamente las texturas apenas se active el mod
$execute {
    for (const auto& sprite : CUSTOM_SPRITES) {
        CCSpriteFrameCache::sharedSpriteFrameCache()->addSpriteFrame(
            CCSpriteFrame::create(sprite.c_str(), CCRectZero), sprite.c_str()
        );
    }
}

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_customDifficultyMeter = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Crear el sprite usando tu imagen precargada
        m_fields->m_customDifficultyMeter = CCSprite::createWithSpriteFrameName("NA_dif.png");
        
        if (m_fields->m_customDifficultyMeter) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Ubicar en la parte superior central de la interfaz
            m_fields->m_customDifficultyMeter->setPosition({ winSize.width / 2, winSize.height - 40.0f });
            
            // Forzar la escala exactamente a 1.0f como solicitaste
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

        if (m_levelLength > 0.0f) {
            percentage = (m_player1->m_position.x / m_levelLength) * 100.0f;
            if (percentage > 100.0f) percentage = 100.0f;
            if (percentage < 0.0f) percentage = 0.0f;
        }

        std::string spriteName = "Normal_dif.png";

        // Lógica limpia en una sola línea por cada condicional
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
