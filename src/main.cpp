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

        // Obtenemos la ruta real en el almacenamiento del mod para "NA_dif.png"
        auto path = Mod::get()->getResourcesDir() / "NA_dif.png";

        // Creamos el sprite directamente usando el archivo del sistema
        m_fields->m_difficultySprite = CCSprite::create(path.string().c_str());

        if (m_fields->m_difficultySprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_fields->m_difficultySprite->setPosition({ winSize.width / 2, winSize.height - 35 });
            m_fields->m_difficultySprite->setScale(0.5f);

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

        // Obtenemos la nueva ruta y actualizamos la textura interna del sprite de manera directa
        auto path = Mod::get()->getResourcesDir() / spriteName;
        auto texture = CCTextureCache::sharedTextureCache()->addImage(path.string().c_str());
        
        if (texture) {
            m_fields->m_difficultySprite->setTexture(texture);
            
            // Reajustamos las dimensiones del rectángulo visible del sprite al tamaño de la nueva imagen
            CCRect rect = CCRectZero;
            rect.size = texture->getContentSize();
            m_fields->m_difficultySprite->setTextureRect(rect);
            
            m_fields->m_lastSpriteName = spriteName;
        }
    }
};
