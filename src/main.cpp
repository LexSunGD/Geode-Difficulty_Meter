#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_customDifficultyMeter = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // "difficulty_01_001.png" es la dificultad Easy oficial del juego
        m_fields->m_customDifficultyMeter = CCSprite::createWithSpriteFrameName("difficulty_01_001.png");
        
        if (m_fields->m_customDifficultyMeter) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Colocar arriba en el centro
            m_fields->m_customDifficultyMeter->setPosition({ winSize.width / 2, winSize.height - 35.0f });
            m_fields->m_customDifficultyMeter->setScale(0.8f);
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

        std::string spriteName = "difficulty_01_001.png";

        // Lógica adaptada usando nombres de archivos nativos de Geometry Dash (11 estados)
        if (percentage < 9.0f) { spriteName = "status_01_001.png"; } // Cara feliz (N/A)
        else if (percentage < 18.0f) { spriteName = "difficulty_01_001.png"; } // Easy
        else if (percentage < 27.0f) { spriteName = "difficulty_02_001.png"; } // Normal
        else if (percentage < 36.0f) { spriteName = "difficulty_03_001.png"; } // Hard
        else if (percentage < 45.0f) { spriteName = "difficulty_04_001.png"; } // Harder
        else if (percentage < 54.0f) { spriteName = "difficulty_05_001.png"; } // Insane
        else if (percentage < 63.0f) { spriteName = "difficulty_07_001.png"; } // Easy Demon
        else if (percentage < 72.0f) { spriteName = "difficulty_08_001.png"; } // Medium Demon
        else if (percentage < 81.0f) { spriteName = "difficulty_09_001.png"; } // Hard Demon
        else if (percentage < 90.0f) { spriteName = "difficulty_10_001.png"; } // Insane Demon
        else { spriteName = "difficulty_06_001.png"; } // Extreme Demon

        auto spriteFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(spriteName.c_str());
        if (spriteFrame) {
            m_fields->m_customDifficultyMeter->setDisplayFrame(spriteFrame);
        }
    }
};
