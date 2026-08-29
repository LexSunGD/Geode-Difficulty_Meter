#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    // Declaramos un campo personalizado de Geode para guardar la referencia de nuestro sprite
    struct Fields {
        CCSprite* m_difficultySprite = nullptr;
    };

    // 1. Creamos e inicializamos el sprite al cargar el nivel
    bool init(GJGameLevel* level, bool useReplay, bool dontRunActions) {
        if (!PlayLayer::init(level, useReplay, dontRunActions)) return false;

        // Creamos el sprite con una textura inicial por defecto (ej. NA o Auto)
        // Geode antepone automáticamente el ID de tu mod para evitar conflictos con el juego
        m_fields->m_difficultySprite = CCSprite::createWithSpriteFrameName("tu_nombre_usuario.medidor_porcentaje/NA_dif.png");

        if (m_fields->m_difficultySprite) {
            // Posicionamos el sprite en la pantalla. 
            // En este ejemplo se ubica arriba a la izquierda, pero puedes ajustar las coordenadas.
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_fields->m_difficultySprite->setPosition({ winSize.width / 2, winSize.height - 40 });
            m_fields->m_difficultySprite->setScale(0.6f); // Ajusta el tamaño de la cara

            // Agregamos el sprite a la capa del juego (UIlayer) para que sea visible
            this->addChild(m_fields->m_difficultySprite, 100);
        }

        return true;
    }

    // 2. Cambiamos la textura según el porcentaje del nivel
    void updateProgressbar() {
        PlayLayer::updateProgressbar();

        // Si el sprite no se creó correctamente, evitamos ejecutar el código para prevenir cuelgues
        if (!m_fields->m_difficultySprite) return;

        float percent = this->getCurrentPercent();
        std::string spriteName = "NA_dif.png"; // Textura de respaldo si algo falla

        // --- LÓGICA DE ESCALADO DE DIFICULTAD ---
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
            spriteName = "ExtremeDemon_dif.png"; // 95% al 100%
        }

        // --- APLICAR LA NUEVA TEXTURA ---
        // Construimos la ruta completa usando tu ID único configurado en el mod.json
        std::string fullPath = "tu_nombre_usuario.medidor_porcentaje/" + spriteName;
        
        auto spriteFrameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
        auto frame = spriteFrameCache->spriteFrameByName(fullPath.c_str());

        if (frame) {
            m_fields->m_difficultySprite->setDisplayFrame(frame);
        }
    }
};
