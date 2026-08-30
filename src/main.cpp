#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    // Variable para guardar el puntero del sprite personalizado del medidor
    CCSprite* m_customDifficultyMeter = nullptr;

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Crear el sprite inicialmente con la textura por defecto de "NA"
        m_customDifficultyMeter = CCSprite::createWithSpriteFrameName("NA_dif.png");
        
        if (m_customDifficultyMeter) {
            // Posicionar el medidor en la pantalla (Ejemplo: Esquina superior izquierda)
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_customDifficultyMeter->setPosition({ winSize.width / 2, winSize.height - 30.0f });
            m_customDifficultyMeter->setScale(0.8f);
            m_customDifficultyMeter->setID("custom-difficulty-meter"_spr);

            // Añadir el sprite a la capa de la interfaz de usuario (m_uiLayer)
            if (m_uiLayer) {
                m_uiLayer->addChild(m_customDifficultyMeter);
            }
        }

        return true;
    }
    
    void update(float dt) {
        PlayLayer::update(dt);

        // Si el sprite no se inicializó correctamente o no hay longitud, no hacer nada
        if (!m_customDifficultyMeter || m_levelLength <= 0.0f) return;

        // Calcular el porcentaje real del nivel basado en el jugador principal
        float percentage = (m_player1->m_position.x / m_levelLength) * 100.0f;
        if (percentage > 100.0f) percentage = 100.0f;
        if (percentage < 0.0f) percentage = 0.0f;

        std::string spriteName = "Normal_dif.png";

        // Lógica de porcentajes en una sola línea por condicional
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

        // Actualizar dinámicamente la textura del medidor en tiempo de ejecución
        auto spriteFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(spriteName.c_str());
        if (spriteFrame) {
            m_customDifficultyMeter->setDisplayFrame(spriteFrame);
        }
    }
};
