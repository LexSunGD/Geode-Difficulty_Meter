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

// Convierte un string a mayúsculas para procesar "A" e "Y" sin importar cómo se escriban
std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

// Limpia espacios en blanco alrededor del texto
std::string trim(std::string str) {
    if (str.empty()) return str;
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Divide una cadena de texto basándose en un delimitador
std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    std::string s = str;
    size_t pos = 0;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        tokens.push_back(s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);
    return tokens;
}

// Lógica de Procesamiento: Separa por "Y" y luego extrae los límites con "A"
std::vector<DifficultyRange> parseTextSetting(const std::string& input, const std::string& spriteName) {
    std::vector<DifficultyRange> ranges;
    std::string cleanedInput = trim(input);
    if (cleanedInput.empty()) return ranges;

    std::string upperInput = toUpper(cleanedInput);
    
    // 1. La "Y" separa bloques independientes (Ej: "0 A 20" Y "50 A 60")
    std::vector<std::string> blocks = split(upperInput, "Y");

    for (auto& block : blocks) {
        block = trim(block);
        if (block.empty()) continue;

        // 2. La "A" define el inicio y el fin del tramo (Ej: "0" A "20")
        std::vector<std::string> parts = split(block, " A ");
        if (parts.size() < 2) {
            parts = split(block, "A"); // Por si no se pusieron espacios
        }

        if (parts.size() >= 2) {
            try {
                float minP = std::stof(trim(parts[0]));
                float maxP = std::stof(trim(parts[1]));

                ranges.push_back({spriteName, minP, maxP});
            } catch (...) {
                // Evita crasheos si hay letras mal escritas
            }
        }
    }
    return ranges;
}

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

        // Vinculación estricta con las casillas del mod.json
        std::map<std::string, std::string> settingToSprite = {
            {"diff-na", "NA_dif.png"}, {"diff-auto", "Auto_dif.png"}, {"diff-easy", "Easy_dif.png"},
            {"diff-normal", "Normal_dif.png"}, {"diff-hard", "Hard_dif.png"}, {"diff-harder", "Harder_dif.png"},
            {"diff-insane", "Insane_dif.png"}, {"diff-easy-demon", "EasyDemon_dif.png"}, {"diff-medium-demon", "MediumDemon_dif.png"},
            {"diff-hard-demon", "HardDemon_dif.png"}, {"diff-insane-demon", "InsaneDemon_dif.png"}, {"diff-extreme-demon", "ExtremeDemon_dif.png"}
        };

        for (const auto& [settingKey, spriteName] : settingToSprite) {
            std::string userStr = Mod::get()->getSettingValue<std::string>(settingKey);
            auto extractedRanges = parseTextSetting(userStr, spriteName);
            m_fields->m_allRanges.insert(m_fields->m_allRanges.end(), extractedRanges.begin(), extractedRanges.end());
        }

        // Cargar la textura inicial de respaldo de forma segura
        std::string initialSprite = Mod::get()->getID() + "/Normal_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_fields->m_meterSprite->setPosition({ winSize.width - 80, winSize.height - 80 });
            m_fields->m_meterSprite->setScale(1.0f);
            this->m_uiLayer->addChild(m_fields->m_meterSprite);
        }

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        float percentage = 0.0f;
        if (this->m_levelLength > 0.0f) {
            percentage = (this->m_player1->m_position.x / this->m_levelLength) * 100.0f;
        }
        percentage = std::clamp(percentage, 0.0f, 100.0f);

        std::string targetSpriteName = "";
        for (const auto& range : m_fields->m_allRanges) {
            if (percentage >= range.minPercent && percentage <= range.maxPercent) {
                targetSpriteName = range.spriteName;
                break;
            }
        }

        // Si caemos en una zona vacía, ocultamos el medidor de inmediato
        if (targetSpriteName.empty()) {
            m_fields->m_meterSprite->setVisible(false);
            m_fields->m_lastLoadedSprite = "";
            return;
        }

        m_fields->m_meterSprite->setVisible(true);

        // OPTIMIZACIÓN DE RENDERIZADO: Solo cambia la textura si realmente cambió de dificultad
        if (m_fields->m_lastLoadedSprite != targetSpriteName) {
            std::string finalPath = Mod::get()->getID() + "/" + targetSpriteName;
            auto texture = CCTextureCache::sharedTextureCache()->addImage(finalPath.c_str(), false);
            
            if (texture) {
                m_fields->m_meterSprite->setTexture(texture);
                
                // SOLUCIÓN AL BUG DE COCOS2D-X: Redibuja el contenedor exacto con las nuevas medidas reales
                CCRect rect = CCRectZero;
                rect.size = texture->getContentSize();
                m_fields->m_meterSprite->setTextureRect(rect);
                
                m_fields->m_meterSprite->setScale(1.0f); // Mantener escala real exacta píxel por píxel
                m_fields->m_lastLoadedSprite = targetSpriteName;
            }
        }
    }
};
