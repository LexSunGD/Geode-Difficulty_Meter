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

// Función para convertir a mayúsculas
std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

// Limpia espacios en blanco al inicio y al final de un texto
std::string trim(std::string str) {
    if (str.empty()) return str;
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Divide una cadena de texto por un delimitador específico
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

// Procesa una casilla de Geode de forma ultra-segura
std::vector<DifficultyRange> parseTextSetting(const std::string& input, const std::string& spriteName) {
    std::vector<DifficultyRange> ranges;
    std::string cleanedInput = trim(input);
    if (cleanedInput.empty()) return ranges;

    // Convertir todo a mayúsculas y separar por la letra "Y"
    std::string upperInput = toUpper(cleanedInput);
    std::vector<std::string> blocks = split(upperInput, "Y");

    for (auto& block : blocks) {
        block = trim(block);
        if (block.empty()) continue;

        // Separar cada bloque por la palabra clave " A "
        std::vector<std::string> parts = split(block, " A ");
        
        // Si el usuario no puso espacios (ej: "0A20"), intentar separar por la "A" directa
        if (parts.size() < 2) {
            parts = split(block, "A");
        }

        if (parts.size() >= 2) {
            try {
                float minP = std::stof(trim(parts[0]));
                float maxP = std::stof(trim(parts[1]));

                ranges.push_back({spriteName, minP, maxP});
                log::info("Regla registrada exitosamente -> {}: {}% a {}%", spriteName, minP, maxP);
            } catch (...) {
                log::error("Error de formato al procesar el bloque: '{}'", block);
            }
        }
    }
    return ranges;
}

class $modify(MyDifficultyMeterLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_meterSprite = nullptr;
        std::vector<DifficultyRange> m_allRanges;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontRunLevel) {
        if (!PlayLayer::init(level, useReplay, dontRunLevel)) return false;

        // Limpiar rangos previos por seguridad al reiniciar el nivel
        m_fields->m_allRanges.clear();

        // Mapa de vinculación con tu mod.json
        std::map<std::string, std::string> settingToSprite = {
            {"diff-na", "NA_dif.png"}, {"diff-auto", "Auto_dif.png"}, {"diff-easy", "Easy_dif.png"},
            {"diff-normal", "Normal_dif.png"}, {"diff-hard", "Hard_dif.png"}, {"diff-harder", "Harder_dif.png"},
            {"diff-insane", "Insane_dif.png"}, {"diff-easy-demon", "EasyDemon_dif.png"}, {"diff-medium-demon", "MediumDemon_dif.png"},
            {"diff-hard-demon", "HardDemon_dif.png"}, {"diff-insane-demon", "InsaneDemon_dif.png"}, {"diff-extreme-demon", "ExtremeDemon_dif.png"}
        };

        // Leer e indexar el texto de las 12 casillas del menú de Geode
        for (const auto& [settingKey, spriteName] : settingToSprite) {
            std::string userStr = Mod::get()->getSettingValue<std::string>(settingKey);
            auto extractedRanges = parseTextSetting(userStr, spriteName);
            m_fields->m_allRanges.insert(m_fields->m_allRanges.end(), extractedRanges.begin(), extractedRanges.end());
        }

        // Crear la imagen base inicial en pantalla
        std::string initialSprite = Mod::get()->getID() + "/ExtremeDemon_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_fields->m_meterSprite->setPosition({ winSize.width - 80, winSize.height - 80 });
            m_fields->m_meterSprite->setScale(1.0f); // Tamaño nativo píxel por píxel
            this->m_uiLayer->addChild(m_fields->m_meterSprite);
        }

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        // Calcular el porcentaje real del nivel en juego
        float percentage = 0.0f;
        if (this->m_levelLength > 0.0f) {
            percentage = (this->m_player1->m_position.x / this->m_levelLength) * 100.0f;
        }
        percentage = std::clamp(percentage, 0.0f, 100.0f);

        // Buscar qué sprite corresponde al porcentaje actual
        std::string targetSpriteName = "";
        for (const auto& range : m_fields->m_allRanges) {
            if (percentage >= range.minPercent && percentage <= range.maxPercent) {
                targetSpriteName = range.spriteName;
                break; // Prioridad al primer tramo coincidente encontrado
            }
        }

        // Si el porcentaje cae en un tramo vacío o no configurado, se vuelve invisible
        if (targetSpriteName.empty()) {
            m_fields->m_meterSprite->setVisible(false);
            return;
        }

        // Actualizar la textura manteniendo las proporciones exactas
        m_fields->m_meterSprite->setVisible(true);
        std::string finalPath = Mod::get()->getID() + "/" + targetSpriteName;
        auto texture = CCTextureCache::sharedTextureCache()->addImage(finalPath.c_str(), false);
        
        if (texture) {
            m_fields->m_meterSprite->setTexture(texture);
            m_fields->m_meterSprite->setScale(1.0f);
        }
    }
};
