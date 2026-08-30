#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    // Declaramos un puntero para rastrear nuestro sprite personalizado
    CCSprite* m_customDifficultyMeter = nullptr;

    bool init(GJGameLevel* level, bool useReplay, bool dontRunActions) {
        if (!PlayLayer::init(level, useReplay, dontRunActions)) {
            return false;
        }

        // Creamos el sprite inicial (por ejemplo, Normal_dif.png)
        auto fullSpriteName = Mod::get()->expandSpriteName("Normal_dif.png");
        m_fields->m_customDifficultyMeter = CCSprite::createWithSpriteFrameName(fullSpriteName);

        if (m_fields->m_customDifficultyMeter) {
            // Ajustamos el tamaño basándonos en la escala del juego (360px a escala UHD / HD se adapta automáticamente)
            // Forzamos un tamaño lógico inicial de 90x90 para que se vea idéntico en calidad alta
            m_fields->m_customDifficultyMeter->setScale(90.0f / m_fields->m_customDifficultyMeter->getContentSize().width);

            // Posicionamos el medidor en pantalla (ejemplo: Esquina superior izquierda)
            auto winSize = CCDirector::get()->getWinSize();
            m_fields->m_customDifficultyMeter->setPosition({ 60.f, winSize.height - 60.f });
            m_fields->m_customDifficultyMeter->setID("difficulty-meter-sprite"_spr);

            // Lo añadimos a la interfaz (UI) del juego
            this->addChild(m_fields->m_customDifficultyMeter, 999);
        }

        return true;
    }

    // El método update se ejecuta en cada frame del juego
    void update(float dt) {
        PlayLayer::update(dt);

        if (!m_fields->m_customDifficultyMeter) return;

        // 1. Calcular el porcentaje actual del nivel de forma precisa
        float positionX = m_player1->getPositionX();
        float levelLength = m_levelLength;
        
        float percentage = 0.0f;
        if (levelLength > 0.0f) {
            percentage = (positionX / levelLength) * 100.0f;
        }

        // Aseguramos que el porcentaje se mantenga entre 0 y 100
        if (percentage < 0.0f) percentage = 0.0f;
        if (percentage > 100.0f) percentage = 100.0f;

        // 2. Determinar qué sprite usar según el porcentaje actual
        std::string spriteName = "Easy_dif.png"; // 0% - 20%

        if (percentage > 20.0f && percentage <= 40.0f) {
            spriteName = "Normal_dif.png";
        } else if (percentage > 40.0f && percentage <= 60.0f) {
            spriteName = "Hard_dif.png";
        } else if (percentage > 60.0f && percentage <= 80.0f) {
            spriteName = "Harder_dif.png";
        } else if (percentage > 80.0f && percentage < 100.0f) {
            spriteName = "Insane_dif.png";
        } else if (percentage >= 100.0f) {
            spriteName = "EasyDemon_dif.png"; // ¡Nivel completado!
        }

        // 3. Actualizar la textura del sprite dinámicamente sin recrear el objeto
        auto fullSpriteName = Mod::get()->expandSpriteName(spriteName.c_str());
        auto spriteFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(fullSpriteName);
        
        if (spriteFrame) {
            m_fields->m_customDifficultyMeter->setDisplayFrame(spriteFrame);
            // Re-ajustamos la escala por si acaso las imágenes tienen ligeras diferencias de pixeles
            m_fields->m_customDifficultyMeter->setScale(90.0f / m_fields->m_customDifficultyMeter->getContentSize().width);
        }
    }
};
