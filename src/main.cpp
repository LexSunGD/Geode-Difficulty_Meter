#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <string>
#include <vector>
#include <map>

using namespace geode::prelude;

struct DifficultyRange {
    std::string spriteName;
    float minPercent;
    float maxPercent;
};

class $modify(MyDifficultyMeterLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_meterSprite = nullptr;
        std::vector<DifficultyRange> m_allRanges;
        std::string m_lastLoadedSprite = "";
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontRunLevel) {
        if (!PlayLayer::init(level, useReplay, dontRunLevel)) return false;

        m_fields->m_allRanges.clear();
        m_fields->m_lastLoadedSprite = "";

        // =========================================================================
        // 🛠️ CONFIGURA TU PROGRAMACIÓN PARA TU VIDEO AQUÍ DIRECTAMENTE:
        // Formato: { "Nombre_de_la_imagen.png", Porcentaje_Inicio, Porcentaje_Fin }
        // Puedes repetir dificultades en cualquier orden y dejar tramos vacíos.
        // =========================================================================
        m_fields->m_allRanges.push_back({ "Easy_dif.png", 0.0f, 10.5f });
        m_fields->m_allRanges.push_back({ "Normal_dif.png", 11.0f, 23.0f });
        m_fields->m_allRanges.push_back({ "InsaneDemon_dif.png", 24.0f, 50.0f });
        m_fields->m_allRanges.push_back({ "Easy_dif.png", 51.0f, 76.0f });       // Se repite Easy
        m_fields->m_allRanges.push_back({ "Harder_dif.png", 77.0f, 91.0f });
        m_fields->m_allRanges.push_back({ "Auto_dif.png", 92.0f, 100.0f });
        // =========================================================================

        // Crear el contenedor gráfico obligatorio en la memoria de Cocos2d-x
        std::string initialSprite = Mod::get()->getID() + "/NA_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            // Posicionar arriba a la derecha de forma limpia para tu medidor
            m_fields->m_meterSprite->setPosition({ winSize.width - 60, winSize.height - 40 });
            m_fields->m_meterSprite->setScale(1.0f);
            
            // Inicia oculto. El ciclo update lo encenderá inmediatamente si el 0% tiene una regla
            m_fields->m_meterSprite->setVisible(false); 
            
            // Adjuntar a la interfaz del nivel (UI) para que no se mueva con la cámara del cubo
            this->m_uiLayer->addChild(m_fields->m_meterSprite, 100);
        }

        this->scheduleUpdate();
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        // LÓGICA DE BARRA DE PROGRESO: Extrae el porcentaje real de la barra superior del juego
        float percentage = 0.0f;
        if (this->m_sliderBar && this->m_sliderBar->isVisible()) {
            // Evaluamos la escala horizontal de llenado de la barra física (va de 0.0 a 1.0)
            percentage = this->m_sliderBar->getScaleX() * 100.0f;
        } else {
            // Fallback: Si juegas con la barra de progreso oculta en las opciones de GD
            if (this->m_levelLength > 0.0f && this->m_player1) {
                percentage = (this->m_player1->m_position.x / this->m_levelLength) * 100.0f;
            }
        }
        percentage = std::clamp(percentage, 0.0f, 100.0f);

        // Escanear si el porcentaje actual cae en alguna de tus reglas del C++
        std::string targetSpriteName = "";
        for (const auto& range : m_fields->m_allRanges) {
            if (percentage >= range.minPercent && percentage <= range.maxPercent) {
                targetSpriteName = range.spriteName;
                break;
            }
        }

        // INVISIBILIDAD: Si el tramo actual está vacío o no lo pusiste en la lista, se oculta al instante
        if (targetSpriteName.empty()) {
            m_fields->m_meterSprite->setVisible(false);
            m_fields->m_lastLoadedSprite = "";
            return;
        }

        // Si hay una dificultad válida, encender visibilidad y refrescar la textura
        m_fields->m_meterSprite->setVisible(true);

        if (m_fields->m_lastLoadedSprite != targetSpriteName) {
            std::string finalPath = Mod::get()->getID() + "/" + targetSpriteName;
            auto texture = CCTextureCache::sharedTextureCache()->addImage(finalPath.c_str(), false);
            if (texture) {
                m_fields->m_meterSprite->setTexture(texture);
                
                // Forzar reajuste de las dimensiones reales de la nueva cara píxel por píxel
                CCRect rect = CCRectZero;
                rect.size = texture->getContentSize();
                m_fields->m_meterSprite->setTextureRect(rect);
                
                m_fields->m_meterSprite->setScale(1.0f);
                m_fields->m_lastLoadedSprite = targetSpriteName;
            }
        }
    }
};
