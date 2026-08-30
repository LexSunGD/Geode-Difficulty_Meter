#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_difficultySprite = nullptr;
        std::string m_lastSpriteName = "";
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontRunActions) {
        if (!PlayLayer::init(level, useReplay, dontRunActions)) return false;

        std::string spriteName = "NA_dif.png";

        // Obtenemos la ruta física donde Geode extrae el mod
        auto path = Mod::get()->getResourcesDir() / spriteName;

        // Creamos el sprite manteniendo su escala real (1.0f)
        m_fields->m_difficultySprite = CCSprite::create(path.string().c_str());

        if (m_fields->m_difficultySprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Colocamos el sprite centrado arriba (Escala 1.0f para respetar tus píxeles HD)
            m_fields->m_difficultySprite->setPosition({ winSize.width / 2, winSize.height - 35 });
            m_fields->m_difficultySprite->setScale(1.0f); 

            this->addChild(m_fields->m_difficultySprite, 100);
            m_fields->m_lastSpriteName = spriteName;
        }

        return true;
    }

    void updateProgressbar() {
        PlayLayer::updateProgressbar();

        if (!m_fields->m_difficultySprite) return;

        float percent = this->getCurrentPercent();
        std::string spriteName = "NA_dif.png";

        if (percent < 10.0f) { spriteName = "Auto_dif.png"; }
        else if (percent >= 10.0f && percent < 20.0f) { spriteName = "Easy_dif.png"; }
        else if (percent >= 20.0f && percent < 30.0f) { spriteName = "Normal_dif.png"; }
        else if (percent >= 30.0f && percent < 40.0f) { spriteName = "Hard_dif.png"; }
        else if (percent >= 40.0f && percent < 50.0f) { spriteName = "Harder_dif.png"; }
        else if (percent >= 50.0f && percent < 60.0f) { spriteName = "Insane_dif.png"; }
        else if (percent >= 60.0f && percent < 70.0f) { spriteName = "EasyDemon_dif.png"; }
        else if (percent >= 70.0f && percent < 80.0f) { spriteName = "MediumDemon_dif.png"; }
        else if (percent >= 80.0f && percent < 90.0f) { spriteName = "HardDemon_dif.png"; }
        else if (percent >= 90.0f && percent < 95.0f) { spriteName = "InsaneDemon_dif.png"; }
        else { spriteName = "ExtremeDemon_dif.png"; }

        if (m_fields->m_lastSpriteName == spriteName) return;

        auto path = Mod::get()->getResourcesDir() / spriteName;
        
        // Cargamos la textura de forma directa (añadiendo el parámetro false requerido en Android/Windows)
        auto texture = CCTextureCache::sharedTextureCache()->addImage(path.string().c_str(), false);
        
        if (texture) {
            m_fields->m_difficultySprite->setTexture(texture);
            
            // Reajustamos el rectángulo del sprite para que se adapte al tamaño exacto de la nueva imagen
            CCRect rect = CCRectZero;
            rect.size = texture->getContentSize();
            m_fields->m_difficultySprite->setTextureRect(rect);
            
            m_fields->m_lastSpriteName = spriteName;
        }
    }
};
