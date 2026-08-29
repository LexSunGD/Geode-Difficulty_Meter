#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_difficultySprite = nullptr;
        std::string m_lastSpriteName = ""; // Evita recargar la misma textura innecesariamente en cada frame
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontRunActions) {
        if (!PlayLayer::init(level, useReplay, dontRunActions)) return false;

        // Intentamos cargar la textura inicial usando el ID del mod registrado
        // Nota: Geode 5 procesa los nombres de sprites de forma interna de esta manera
        m_fields->m_difficultySprite = CCSprite::createWithSpriteFrameName("lexsungd.difficulty_meter/NA_dif.png");

        if (m_fields->m_difficultySprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            // Ubicación en pantalla: Ajustada en el centro superior debajo de la barra
            m_fields->m_difficultySprite->setPosition({ winSize.width / 2, winSize.height - 35 });
            m_fields->m_difficultySprite->setScale(0.5f); // Modifica el tamaño según tus imágenes

            // Lo agregamos a la interfaz de juego
            this->addChild(m_fields->m_difficultySprite, 100);
            m_fields->m_lastSpriteName = "NA_dif.png";
        }

        return true;
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();

        if (!m_fields->m_difficultySprite) return;

        float percent = this->getCurrentPercent();
        std::string spriteName = "NA_dif.png";

        // Lógica de asignación según tus sprites
        if (percent < 10.0f) {
            spriteName = "Auto_dif.png";
        } else if (percent >= 10.0f && percent < 20.0f) {
            spriteName = "Easy_dif.png";
        } else if (percent >= 20.0f && percent < 30.0f) {
            spriteName = "Normal_dif.png";
        } else if (percent >= 30.0f && percent < 40.0f) {
            spriteName = "Hard_dif.png";
        } else if (percent >= 40.0f && percent < 50.0f) {
            spriteName = "Harder_dif.png";
        } else if (percent >= 50.0f && percent < 60.0f) {
            spriteName = "Insane_dif.png";
        } else if (percent >= 60.0f && percent < 70.0f) {
            spriteName = "EasyDemon_dif.png";
        } else if (percent >= 70.0f && percent < 80.0f) {
            spriteName = "MediumDemon_dif.png";
        } else if (percent >= 80.0f && percent < 90.0f) {
            spriteName = "HardDemon_dif.png";
        } else if (percent >= 90.0f && percent < 95.0f) {
            spriteName = "InsaneDemon_dif.png";
        } else {
            spriteName = "ExtremeDemon_dif.png";
        }

        // Si la textura calculada ya es la que se muestra actualmente, salimos para optimizar rendimiento
        if (m_fields->m_lastSpriteName == spriteName) return;

        std::string fullPath = "lexsungd.difficulty_meter/" + spriteName;
        auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(fullPath.c_str());

        if (frame) {
            m_fields->m_difficultySprite->setDisplayFrame(frame);
            m_fields->m_lastSpriteName = spriteName;
        }
    }
};
