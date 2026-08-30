#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    // Definimos las variables personalizadas del mod de forma segura en Geode
    struct Fields {
        CCSprite* m_difficultySprite = nullptr;
        std::string m_lastSpriteName = "";
    };

    // 1. Creamos el contenedor del sprite al iniciar el nivel
    bool init(GJGameLevel* level, bool useReplay, bool dontRunActions) {
        if (!PlayLayer::init(level, useReplay, dontRunActions)) return false;

        // Obtenemos la ruta inicial hacia la textura por defecto
        auto path = Mod::get()->getResourcesDir() / "NA_dif.png";

        // Creamos el sprite directamente usando el archivo del sistema
        m_fields->m_difficultySprite = CCSprite::create(path.string().c_str());

        if (m_fields->m_difficultySprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Posicionamiento en pantalla: Justo en el centro superior debajo de la barra
            m_fields->m_difficultySprite->setPosition({ winSize.width / 2, winSize.height - 35 });
            m_fields->m_difficultySprite->setScale(0.5f); // Puedes ajustar este valor si tus imágenes son muy grandes

            // Agregamos el sprite a la capa de juego
            this->addChild(m_fields->m_difficultySprite, 100);
            m_fields->m_lastSpriteName = "NA_dif.png";
        }

        return true;
    }

    // 2. Actualizamos la imagen en tiempo real según avanza el porcentaje
    void updateProgressbar() {
        PlayLayer::updateProgressbar();

        // Si el sprite falló al inicializarse, salimos para prevenir crashes
        if (!m_fields->m_difficultySprite) return;

        float percent = this->getCurrentPercent();
        std::string spriteName = "NA_dif.png";

        // --- ASIGNACIÓN DE SPRITES SEGÚN EL PORCENTAJE ---
        if (percent < 10.0f) { 
            spriteName = "Auto_dif.png"; 
        }
        else if (percent >= 10.0f && percent < 20.0f) { 
            spriteName = "Easy_dif.png"; 
        }
        else if (percent >= 20.0f && percent < 30.0f) { 
            spriteName = "Normal_dif.png"; 
        }
        else if (percent >= 30.0f && percent < 40.0f) { 
            spriteName = "Hard_dif.png"; 
        }
        else if (percent >= 40.0f && percent < 50.0f) { 
            spriteName = "Harder_dif.png"; 
        }
        else if (percent >= 50.0f && percent < 60.0f) { 
            spriteName = "Insane_dif.png"; 
        }
        else if (percent >= 60.0f && percent < 70.0f) { 
            spriteName = "EasyDemon_dif.png"; 
        }
        else if (percent >= 70.0f && percent < 80.0f) { 
            spriteName = "MediumDemon_dif.png"; 
        }
        else if (percent >= 80.0f && percent < 90.0f) { 
            spriteName = "HardDemon_dif.png"; 
        }
        else if (percent >= 90.0f && percent < 95.0f) { 
            spriteName = "InsaneDemon_dif.png"; 
        }
        else { 
            spriteName = "ExtremeDemon_dif.png"; 
        }

        // Si el sprite calculado es el mismo que ya está en pantalla, no hacemos nada (Optimización)
        if (m_fields->m_lastSpriteName == spriteName) return;

        // Obtenemos la ruta física de la nueva imagen
        auto path = Mod::get()->getResourcesDir() / spriteName;

        // Creamos un sprite temporal en memoria para extraer su textura de forma segura
        auto tempSprite = CCSprite::create(path.string().c_str());

        if (tempSprite && tempSprite->getTexture()) {
            auto texture = tempSprite->getTexture();
            
            // Aplicamos la nueva textura al sprite visible en pantalla
            m_fields->m_difficultySprite->setTexture(texture);
            
            // Reajustamos las dimensiones del sprite al tamaño real del nuevo PNG
            CCRect rect = CCRectZero;
            rect.size = texture->getContentSize();
            m_fields->m_difficultySprite->setTextureRect(rect);
            
            // Guardamos el nombre actual
            m_fields->m_lastSpriteName = spriteName;
        }
    }
};
