#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "DifficultyConfig.hpp"

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
            
            float offsetX = static_cast<float>(Mod::get()->getSettingValue<int64_t>("meter-pos-x"));
            float offsetY = static_cast<float>(Mod::get()->getSettingValue<int64_t>("meter-pos-y"));
            m_fields->m_customDifficultyMeter->setPosition({ (winSize.width / 2) + offsetX, (winSize.height / 2) + offsetY });
            
            float scale = static_cast<float>(Mod::get()->getSettingValue<double>("meter-scale"));
            m_fields->m_customDifficultyMeter->setScale(scale); 

            // CORRECCIÓN AQUÍ: Se añadió ->get() que faltaba y causaba el error de compilación
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

        std::string spriteName = "NA_dif.png"; 

        for (const auto& step : MY_DIFFICULTY_RECIPE) {
            if (percentage >= step.percentage) {
                spriteName = step.spriteName; 
            } else {
                break; 
            }
        }

        auto textureCache = CCTextureCache::sharedTextureCache();
        std::string fullPath = Mod::get()->getID() + "/" + spriteName;
        auto newTexture = textureCache->addImage(fullPath.c_str(), false);
        
        if (newTexture) {
            m_fields->m_customDifficultyMeter->setTexture(newTexture);
            
            CCRect rect = CCRectZero;
            rect.size = newTexture->getContentSize();
            m_fields->m_customDifficultyMeter->setTextureRect(rect);
            
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
