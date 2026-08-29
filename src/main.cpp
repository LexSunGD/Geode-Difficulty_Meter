#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <string>

using namespace geode::prelude;

class $modify(MyDifficultyMeterLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_meterSprite = nullptr;
        std::string m_lastSprite = "";
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontRunLevel) {
        if (!PlayLayer::init(level, useReplay, dontRunLevel)) return false;

        m_fields->m_lastSprite = "";

        // CORRECCIÓN 1: Llamar al archivo directo por su nombre sin el ID del mod adelante
        // Usamos Easy_dif.png como imagen de arranque para forzar al juego a mostrar algo al iniciar
        m_fields->m_meterSprite = CCSprite::create("Easy_dif.png");
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            // Posicionar al centro de la pantalla para que sea imposible no verlo en la prueba
            m_fields->m_meterSprite->setPosition({ winSize.width / 2, winSize.height / 2 });
            m_fields->m_meterSprite->setScale(1.0f);
            m_fields->m_meterSprite->setVisible(true);
            
            // CORRECCIÓN 2: Agregarlo directamente a la capa principal del nivel (PlayLayer)
            // Con una prioridad Z de 100 para garantizar que se dibuje por encima de todos los bloques
            this->addChild(m_fields->m_meterSprite, 100);
        }

        // CORRECCIÓN 3: Forzar al motor a activar el ciclo update de forma obligatoria
        this->scheduleUpdate();

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        // Extraer el porcentaje oficial
        float percentage = this->getCurrentPercent();

        // Regla fija e inmune de prueba en C++
        std::string targetSprite = "Easy_dif.png"; // 0% a 50% muestra Easy
        if (percentage > 50.0f) {
            targetSprite = "Normal_dif.png"; // 51% a 100% cambia a Normal
        }

        // Forzar actualización visual si hay cambio de tramo
        if (m_fields->m_lastSprite != targetSprite) {
            auto texture = CCTextureCache::sharedTextureCache()->addImage(targetSprite.c_str(), false);
            if (texture) {
                m_fields->m_meterSprite->setTexture(texture);
                
                CCRect rect = CCRectZero;
                rect.size = texture->getContentSize();
                m_fields->m_meterSprite->setTextureRect(rect);
                
                m_fields->m_meterSprite->setScale(1.0f);
                m_fields->m_lastSprite = targetSprite;
            }
        }
    }
};
