#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

using namespace geode::prelude;

// Estructura para guardar el rango de cada dificultad
struct DifficultyRange {
    std::string spriteName;
    float minPercent;
    float maxPercent;
};

// Mapa para traducir tus abreviaturas a tus archivos cargados
std::map<std::string, std::string> shortToFullName = {
    {"NA", "NA_dif.png"}, {"A", "Auto_dif.png"}, {"E", "Easy_dif.png"}, 
    {"N", "Normal_dif.png"}, {"H", "Hard_dif.png"}, {"Hr", "Harder_dif.png"}, 
    {"I", "Insane_dif.png"}, {"ED", "EasyDemon_dif.png"}, {"MD", "MediumDemon_dif.png"}, 
    {"HD", "HardDemon_dif.png"}, {"ID", "InsaneDemon_dif.png"}, {"ExD", "ExtremeDemon_dif.png"}
};

// Función para procesar el string de la configuración
std::vector<DifficultyRange> parseConfig(const std::string& configStr) {
    std::vector<DifficultyRange> ranges;
    std::stringstream ss(configStr);
    std::string item;

    while (std::getline(ss, item, ',')) {
        size_t first = item.find_first_not_of(" ");
        if (first == std::string::npos) continue;
        item = item.substr(first);

        std::stringstream itemSs(item);
        std::string label, rangeStr;
        
        if (itemSs >> label >> rangeStr) {
            size_t dash = rangeStr.find('-');
            if (dash != std::string::npos) {
                try {
                    float minP = std::stof(rangeStr.substr(0, dash));
                    float maxP = std::stof(rangeStr.substr(dash + 1));
                    
                    if (shortToFullName.count(label)) {
                        ranges.push_back({shortToFullName[label], minP, maxP});
                    }
                } catch (...) {
                    // Ignora errores si escribes mal un porcentaje
                }
            }
        }
    }
    return ranges;
}

class $modify(MyDifficultyMeterLayer, PlayLayer) {
    CCSprite* m_meterSprite = nullptr;
    std::vector<DifficultyRange> m_parsedRanges;

    bool init(GJGameLevel* level, bool useReplay, bool dontRunLevel) {
        if (!PlayLayer::init(level, useReplay, dontRunLevel)) return false;

        // Leer la configuración del mod.json
        std::string userConfig = Mod::get()->getSettingValue<std::string>("meter-config");
        m_fields->m_parsedRanges = parseConfig(userConfig);

        // Crear la imagen inicial (NA_dif.png) usando tu ID de creador
        std::string initialSprite = Mod::get()->getID() + "/NA_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            // Posicionar en la esquina superior derecha
            m_fields->m_meterSprite->setPosition({ winSize.width - 80, winSize.height - 80 });
            
            // ESCALA EXACTA: Forzamos la escala nativa 1.0f para que no se deforme
            m_fields->m_meterSprite->setScale(1.0f);
            
            // Añadir a la UI del nivel
            this->m_uiLayer->addChild(m_fields->m_meterSprite);
        }

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        // Calcular el porcentaje en tiempo real
        float percentage = 0.0f;
        if (this->m_levelLength > 0.0f) {
            percentage = (this->m_player1->m_position.x / this->m_levelLength) * 100.0f;
        }
        percentage = std::clamp(percentage, 0.0f, 100.0f);

        // Buscar qué dificultad le toca según el porcentaje actual
        std::string currentSpriteName = "NA_dif.png"; 
        for (const auto& range : m_fields->m_parsedRanges) {
            if (percentage >= range.minPercent && percentage <= range.maxPercent) {
                currentSpriteName = range.spriteName;
                break;
            }
        }

        // Construir la ruta y actualizar la textura
        std::string finalPath = Mod::get()->getID() + "/" + currentSpriteName;
        auto texture = CCTextureCache::sharedTextureCache()->addImage(finalPath.c_str(), false);
        
        if (texture) {
            m_fields->m_meterSprite->setTexture(texture);
            // Re-aplicar escala nativa por si cambian las dimensiones entre imágenes
            m_fields->m_meterSprite->setScale(1.0f);
        }
    }
};
