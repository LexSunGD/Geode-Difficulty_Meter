#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_customDifficultyMeter = nullptr;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (level->isPlatformer()) return true;

        m_fields->m_customDifficultyMeter = CCSprite::create("NA_dif.png"_spr);
        
        if (m_fields->m_customDifficultyMeter) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // 1. Obtener posiciones (Offsets de configuración)
            float offsetX = static_cast<float>(Mod::get()->getSettingValue<int64_t>("meter-pos-x"));
            float offsetY = static_cast<float>(Mod::get()->getSettingValue<int64_t>("meter-pos-y"));
            
            // Si el offset es 0,0 se posicionará exactamente en el centro de la pantalla
            m_fields->m_customDifficultyMeter->setPosition({ (winSize.width / 2) + offsetX, (winSize.height / 2) + offsetY });
            
            // 2. Obtener escala de configuración
            float scale = static_cast<float>(Mod::get()->getSettingValue<double>("meter-scale"));
            m_fields->m_customDifficultyMeter->setScale(scale); 

            // 3. Obtener opacidad (Cocos2d-x requiere un rango de 0 a 255)
            int64_t opacityPercent = Mod::get()->getSettingValue<int64_t>("meter-opacity");
            GLubyte alphaValue = static_cast<GLubyte>((opacityPercent * 255) / 100);
            m_fields->m_customDifficultyMeter->setOpacity(alphaValue);

            m_fields->m_customDifficultyMeter->setID("custom-difficulty-meter"_spr);

            if (m_uiLayer) {
                m_uiLayer->addChild(m_fields->m_customDifficultyMeter);
            }
        }

        return true;
    }
    
    void updateProgressbar() {
        PlayLayer::updateProgressbar(); 

        if (!m_fields->m_customDifficultyMeter) return;

        float percentage = this->getCurrentPercent();

        if (percentage > 100.0f) percentage = 100.0f;
        if (percentage < 0.0f) percentage = 0.0f;

        std::string spriteName = "Normal_dif.png";

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

        auto textureCache = CCTextureCache::sharedTextureCache();
        std::string fullPath = Mod::get()->getID() + "/" + spriteName;
        auto newTexture = textureCache->addImage(fullPath.c_str(), false);
        
        if (newTexture) {
            m_fields->m_customDifficultyMeter->setTexture(newTexture);
            
            CCRect rect = CCRectZero;
            rect.size = newTexture->getContentSize();
            m_fields->m_customDifficultyMeter->setTextureRect(rect);
            
            // Actualizar transformaciones en tiempo real por si cambian en los ajustes
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            float offsetX = static_cast<float>(Mod::get()->getSettingValue<int64_t>("meter-pos-x"));
            float offsetY = static_cast<float>(Mod::get()->getSettingValue<int64_t>("meter-pos-y"));
            m_fields->m_customDifficultyMeter->setPosition({ (winSize.width / 2) + offsetX, (winSize.height / 2) + offsetY });
            
            float scale = static_cast<float>(Mod::get()->getSettingValue<double>("meter-scale"));
            m_fields->m_customDifficultyMeter->setScale(scale);

            int64_t opacityPercent = Mod::get()->getSettingValue<int64_t>("meter-opacity");
            GLubyte alphaValue = static_cast<GLubyte>((opacityPercent * 255) / 100);
            m_fields->m_customDifficultyMeter->setOpacity(alphaValue);
        }
    }
};
