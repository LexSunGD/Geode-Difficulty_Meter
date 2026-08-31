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

        // Inicializa usando el método nativo de Geode para archivos sueltos
        m_fields->m_customDifficultyMeter = CCSprite::create("NA_dif.png"_spr);
        
        if (m_fields->m_customDifficultyMeter) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            
            // Ubicar arriba en el centro
            m_fields->m_customDifficultyMeter->setPosition({ winSize.width / 2, winSize.height - 50.0f });
            m_fields->m_customDifficultyMeter->setScale(1.0f); 
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

        // Tu lógica exacta limpia en una sola línea por cada condicional { }
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

        // MÉTODO INFALIBLE: Buscamos el archivo físico del mod en lugar de usar el SpriteFrameCache
        auto textureCache = CCTextureCache::sharedTextureCache();
        
        // Geode indexa los archivos sueltos en el sistema de archivos con el ID del mod como prefijo
        std::string fullPath = Mod::get()->getID() + "/" + spriteName;
        auto newTexture = textureCache->addImage(fullPath.c_str());
        
        if (newTexture) {
            m_fields->m_customDifficultyMeter->setTexture(newTexture);
            
            // Reajustamos el rectángulo de la textura para que coincida exactamente con los 360x360 píxeles
            CCRect rect = CCRectZero;
            rect.size = newTexture->getContentSize();
            m_fields->m_customDifficultyMeter->setTextureRect(rect);
        }
    }
};
