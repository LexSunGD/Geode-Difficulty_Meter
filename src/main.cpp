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

        // Nombre del archivo inicial en tu carpeta de recursos
        std::string spriteName = "NA_dif.png";

        // Usamos CCFileUtils para que Cocos encuentre el archivo físico en Android de forma nativa
        auto fileUtils = CCFileUtils::sharedFileUtils();
        std::string fullPath = fileUtils->fullPathForFilename(spriteName.c_str(), false);

        // Creamos el sprite con su tamaño nativo de origen
        m_fields->m_difficultySprite = CCSprite::create(fullPath.c_str());

        if (m_fields->m_difficultySprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Posicionamiento centrado debajo de la barra.
            // Nota: Mantenemos la escala 1.0f para respetar el tamaño exacto de tu imagen HD
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

        // Asignación de texturas según el porcentaje
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

        // Buscamos la ruta completa del nuevo archivo mediante el buscador de Cocos2d-x
        auto fileUtils = CCFileUtils::sharedFileUtils();
        std::string fullPath = fileUtils->fullPathForFilename(spriteName.c_str(), false);

        // Cargamos la textura asegurando la compatibilidad multiplataforma de argumentos en GD 2.2
        auto texture = CCTextureCache::sharedTextureCache()->addImage(fullPath.c_str(), false);
        
        if (texture) {
            m_fields->m_difficultySprite->setTexture(texture);
            
            // Reajustamos las dimensiones al tamaño real y nativo del nuevo archivo PNG
            CCRect rect = CCRectZero;
            rect.size = texture->getContentSize();
            m_fields->m_difficultySprite->setTextureRect(rect);
            
            m_fields->m_lastSpriteName = spriteName;
        }
    }
};
