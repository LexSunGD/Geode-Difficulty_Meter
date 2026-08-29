#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

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
        bool m_settingsLoaded = false; // Bandera para cargar el menú en el momento correcto
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontRunLevel) {
        if (!PlayLayer::init(level, useReplay, dontRunLevel)) return false;

        m_fields->m_allRanges.clear();
        m_fields->m_lastLoadedSprite = "";
        m_fields->m_settingsLoaded = false;

        // OBLIGATORIO: Molde base inicial para que Cocos2d-x registre el objeto visual en memoria
        std::string initialSprite = Mod::get()->getID() + "/NA_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_fields->m_meterSprite->setPosition({ winSize.width - 80, winSize.height - 80 });
            m_fields->m_meterSprite->setScale(1.0f);
            
            // Empezamos invisibles. Si no hay datos en el menú, no mostrará nada
            m_fields->m_meterSprite->setVisible(false);
            
            this->m_uiLayer->addChild(m_fields->m_meterSprite);
        }
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        // ESTRATEGIA: Cargar el menú de Geode aquí adentro cuando los hilos de texto ya estén listos
        if (!m_fields->m_settingsLoaded) {
            m_fields->m_settingsLoaded = true;
            
            std::map<std::string, std::string> settingToSprite = {
                {"diff-na", "NA_dif.png"}, {"diff-auto", "Auto_dif.png"}, {"diff-easy", "Easy_dif.png"},
                {"diff-normal", "Normal_dif.png"}, {"diff-hard", "Hard_dif.png"}, {"diff-harder", "Harder_dif.png"},
                {"diff-insane", "Insane_dif.png"}, {"diff-easy-demon", "EasyDemon_dif.png"}, {"diff-medium-demon", "MediumDemon_dif.png"},
                {"diff-hard-demon", "HardDemon_dif.png"}, {"diff-insane-demon", "InsaneDemon_dif.png"}, {"diff-extreme-demon", "ExtremeDemon_dif.png"}
            };

            for (const auto& [settingKey, spriteName] : settingToSprite) {
                std::string input = Mod::get()->getSettingValue<std::string>(settingKey);
                if (input.empty()) continue;

                // Pasar todo a mayúsculas para procesar "A" e "Y" sin importar cómo escribas
                std::transform(input.begin(), input.end(), input.begin(), ::toupper);

                // --- 1. LÓGICA DE LA "Y" (Separar bloques múltiples) ---
                std::vector<std::string> blocks;
                std::stringstream ssY(input);
                std::string tokenY;
                while (std::getline(ssY, tokenY, 'Y')) { 
                    blocks.push_back(tokenY); 
                }

                // --- 2. LÓGICA DE LA "A" (Extraer límites numéricos por bloque) ---
                for (const auto& block : blocks) {
                    size_t aPos = block.find("A");
                    if (aPos != std::string::npos) {
                        try {
                            float minP = std::stof(block.substr(0, aPos));
                            float maxP = std::stof(block.substr(aPos + 1));
                            m_fields->m_allRanges.push_back({spriteName, minP, maxP});
                        } catch (...) {} // Evita crasheos si hay espacios extra
                    }
                }
            }
        }

        // Calcular porcentaje en tiempo real
        float percentage = 0.0f;
        if (this->m_levelLength > 0.0f) {
            percentage = (this->m_player1->m_position.x / this->m_levelLength) * 100.0f;
        }
        percentage = std::clamp(percentage, 0.0f, 100.0f);

        // Escanear si el porcentaje actual coincide con alguna regla de tus casillas
        std::string targetSpriteName = "";
        for (const auto& range : m_fields->m_allRanges) {
            if (percentage >= range.minPercent && percentage <= range.maxPercent) {
                targetSpriteName = range.spriteName;
                break;
            }
        }

        // CONTROL DE VISIBILIDAD: Si cae en un porcentaje vacío o el menú no tiene nada, desaparece
        if (targetSpriteName.empty()) {
            m_fields->m_meterSprite->setVisible(false);
            m_fields->m_lastLoadedSprite = "";
            return;
        }

        // Si hay una dificultad asignada, se enciende y refresca la imagen
        m_fields->m_meterSprite->setVisible(true);

        if (m_fields->m_lastLoadedSprite != targetSpriteName) {
            std::string finalPath = Mod::get()->getID() + "/" + targetSpriteName;
            auto texture = CCTextureCache::sharedTextureCache()->addImage(finalPath.c_str(), false);
            if (texture) {
                m_fields->m_meterSprite->setTexture(texture);
                
                // Forzar el redibujado con el tamaño real de la imagen entrante
                CCRect rect = CCRectZero;
                rect.size = texture->getContentSize();
                m_fields->m_meterSprite->setTextureRect(rect);
                
                m_fields->m_meterSprite->setScale(1.0f);
                m_fields->m_lastLoadedSprite = targetSpriteName;
            }
        }
    }
};
