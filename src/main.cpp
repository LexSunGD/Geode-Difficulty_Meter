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

// Función auxiliar para convertir un texto a mayúsculas (para procesar "A" e "Y" sin importar cómo las escribas)
std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

// Procesa una sola casilla (ej: "0 A 20 Y 50 A 60") y extrae sus rangos numéricos
std::vector<DifficultyRange> parseTextSetting(const std::string& input, const std::string& spriteName) {
    std::vector<DifficultyRange> ranges;
    if (input.empty()) return ranges;

    std::string upperInput = toUpper(input);
    std::stringstream ss(upperInput);
    std::string token;
    std::vector<std::string> blocks;

    // 1. Separar los bloques por la letra "Y"
    while (std::getline(ss, token, 'Y')) {
        // Limpiar espacios en blanco innecesarios
        token.erase(0, token.find_first_not_of(" "));
        token.erase(token.find_last_not_of(" ") + 1);
        if (!token.empty()) {
            blocks.push_back(token);
        }
    }

    // 2. Por cada bloque, buscar la letra "A" para extraer los números
    for (const auto& block : blocks) {
        size_t aPos = block.find(" A ");
        // Si el usuario no puso espacios, buscar la "A" directa por seguridad
        if (aPos == std::string::npos) aPos = block.find("A"); 
        
        if (aPos != std::string::npos) {
            try {
                std::string minStr = block.substr(0, aPos);
                std::string maxStr = block.substr(aPos + (block.find(" A ") != std::string::npos ? 3 : 1));

                float minP = std::stof(minStr);
                float maxP = std::stof(maxStr);

                ranges.push_back({spriteName, minP, maxP});
            } catch (...) {
                // Ignorar si el bloque contiene errores de escritura
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

        // Estructura de vinculación: Clave del mod.json -> Nombre del archivo sprite
        std::map<std::string, std::string> settingToSprite = {
            {"diff-na", "NA_dif.png"}, {"diff-auto", "Auto_dif.png"}, {"diff-easy", "Easy_dif.png"},
            {"diff-normal", "Normal_dif.png"}, {"diff-hard", "Hard_dif.png"}, {"diff-harder", "Harder_dif.png"},
            {"diff-insane", "Insane_dif.png"}, {"diff-easy-demon", "EasyDemon_dif.png"}, {"diff-medium-demon", "MediumDemon_dif.png"},
            {"diff-hard-demon", "HardDemon_dif.png"}, {"diff-insane-demon", "InsaneDemon_dif.png"}, {"diff-extreme-demon", "ExtremeDemon_dif.png"}
        };

        // Leer cada una de las 12 casillas del menú e indexar sus rangos
        for (const auto& [settingKey, spriteName] : settingToSprite) {
            std::string userStr = Mod::get()->getSettingValue<std::string>(settingKey);
            auto extractedRanges = parseTextSetting(userStr, spriteName);
            m_fields->m_allRanges.insert(m_fields->m_allRanges.end(), extractedRanges.begin(), extractedRanges.end());
        }

        // Crear el sprite inicial (Imagen Base Segura para arrancar el nivel)
        std::string initialSprite = Mod::get()->getID() + "/NA_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            // Posición estándar en la esquina superior derecha
            m_fields->m_meterSprite->setPosition({ winSize.width - 80, winSize.height - 80 });
            m_fields->m_meterSprite->setScale(1.0f); // Tamaño real exacto sin deformaciones
            this->m_uiLayer->addChild(m_fields->m_meterSprite);
        }

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        // Calcular el porcentaje exacto de avance
        float percentage = 0.0f;
        if (this->m_levelLength > 0.0f) {
            percentage = (this->m_player1->m_position.x / this->m_levelLength) * 100.0f;
        }
        percentage = std::clamp(percentage, 0.0f, 100.0f);

        // Buscar qué dificultad le corresponde al porcentaje actual
        std::string targetSpriteName = "";
        for (const auto& range : m_fields->m_allRanges) {
            if (percentage >= range.minPercent && percentage <= range.maxPercent) {
                targetSpriteName = range.spriteName;
                break; // Romper el ciclo de búsqueda: la primera regla válida encontrada gana prioridad
            }
        }

        // SISTEMA DE INVISIBILIDAD: Si el porcentaje actual no está cubierto por ninguna regla, ocultar
        if (targetSpriteName.empty()) {
            m_fields->m_meterSprite->setVisible(false);
            return;
        }

        // Si es un rango válido, encender visibilidad y renderizar la textura correspondiente
        m_fields->m_meterSprite->setVisible(true);
        std::string finalPath = Mod::get()->getID() + "/" + targetSpriteName;
        auto texture = CCTextureCache::sharedTextureCache()->addImage(finalPath.c_str(), false);
        
        if (texture) {
            m_fields->m_meterSprite->setTexture(texture);
            m_fields->m_meterSprite->setScale(1.0f); // Respetar píxel por píxel la altura/ancho de cada cara
        }
    }
};
