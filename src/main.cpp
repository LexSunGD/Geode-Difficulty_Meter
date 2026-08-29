#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <sstream>

using namespace geode::prelude;

struct DifficultyRange {
    std::string spriteName;
    float minPercent;
    float maxPercent;
};

// Mapa que traduce tus nombres del Bloc de Notas a tus archivos de imagen reales
std::map<std::string, std::string> nameToSprite = {
    {"NA", "NA_dif.png"}, {"AUTO", "Auto_dif.png"}, {"EASY", "Easy_dif.png"},
    {"NORMAL", "Normal_dif.png"}, {"HARD", "Hard_dif.png"}, {"HARDER", "Harder_dif.png"},
    {"INSANE", "Insane_dif.png"}, {"EASY DEMON", "EasyDemon_dif.png"}, {"MEDIUM DEMON", "MediumDemon_dif.png"},
    {"HARD DEMON", "HardDemon_dif.png"}, {"INSANE DEMON", "InsaneDemon_dif.png"}, {"EXTREME DEMON", "ExtremeDemon_dif.png"}
};

// Limpia espacios en blanco alrededor de una cadena
std::string trimStr(std::string str) {
    if (str.empty()) return str;
    size_t first = str.find_first_not_of(" \t\r\n.");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n.");
    return str.substr(first, (last - first + 1));
}

// Procesa el archivo TXT con el nuevo formato: "Easy 0-10, Normal 11-23"
std::vector<DifficultyRange> loadConfigFromTxt(const std::filesystem::path& path) {
    std::vector<DifficultyRange> ranges;
    if (!std::filesystem::exists(path)) return ranges;

    std::ifstream file(path);
    std::string content;
    std::getline(file, content); // Leer toda la línea continua del archivo
    content = trimStr(content);
    if (content.empty()) return ranges;

    // Separar por comas (cada bloque es una dificultad con su rango)
    std::stringstream ss(content);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = trimStr(item);
        if (item.empty()) continue;

        // Buscar el último espacio en blanco que separa el Nombre del Rango Numérico (Ej: "Insane Demon" y "24-50")
        size_t lastSpace = item.find_last_of(" \t");
        if (lastSpace == std::string::npos) continue;

        std::string diffName = trimStr(item.substr(0, lastSpace));
        std::string rangePart = trimStr(item.substr(lastSpace + 1));

        std::transform(diffName.begin(), diffName.end(), diffName.begin(), ::toupper);
        if (nameToSprite.count(diffName) == 0) continue;
        std::string targetSprite = nameToSprite[diffName];

        // Separar los números por el guion '-'
        size_t dashPos = rangePart.find('-');
        if (dashPos != std::string::npos) {
            try {
                float minP = std::stof(trimStr(rangePart.substr(0, dashPos)));
                float maxP = std::stof(trimStr(rangePart.substr(dashPos + 1)));
                ranges.push_back({targetSprite, minP, maxP});
            } catch (...) {}
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

        // 1. Obtener la ruta de la carpeta de guardado/configuración editable del mod
        auto configDir = Mod::get()->getConfigDir();
        std::filesystem::create_directories(configDir);
        auto destConfigPath = configDir / "difficulty_meter.txt";
        
        // 2. Si el usuario no tiene el archivo en su carpeta config, lo copiamos desde los recursos del mod automáticamente
        if (!std::filesystem::exists(destConfigPath)) {
            auto resourcePath = Mod::get()->getResourcesDir() / "difficulty_meter.txt";
            if (std::filesystem::exists(resourcePath)) {
                std::filesystem::copy_file(resourcePath, destConfigPath);
            } else {
                // Fallback por si acaso: si no encuentra el recurso, crea uno básico
                std::ofstream outfile(destConfigPath);
                outfile << "Easy 0-10, Normal 11-23, Insane Demon 24-50, Easy 51-76, Harder 77-91, Auto 92-100.";
                outfile.close();
            }
        }

        // 3. Cargar los rangos de dificultades procesando el archivo editable
        m_fields->m_allRanges = loadConfigFromTxt(destConfigPath);

        // Crear el cascarón obligatorio del sprite en memoria de Cocos2d-x
        std::string initialSprite = Mod::get()->getID() + "/NA_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_fields->m_meterSprite->setPosition({ winSize.width - 80, winSize.height - 80 });
            m_fields->m_meterSprite->setScale(1.0f);
            
            // Empezar oculto por defecto (si está vacío, no se verá nada)
            m_fields->m_meterSprite->setVisible(false); 
            
            this->m_uiLayer->addChild(m_fields->m_meterSprite);
        }
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        // Calcular porcentaje en tiempo real
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
                break;
            }
        }

        // Si caemos en una zona vacía o el archivo no tiene reglas para este porcentaje, ocultar
        if (targetSpriteName.empty()) {
            m_fields->m_meterSprite->setVisible(false);
            m_fields->m_lastLoadedSprite = "";
            return;
        }

        m_fields->m_meterSprite->setVisible(true);

        // Cambiar la imagen manteniendo sus proporciones nativas píxel por píxel
        if (m_fields->m_lastLoadedSprite != targetSpriteName) {
            std::string finalPath = Mod::get()->getID() + "/" + targetSpriteName;
            auto texture = CCTextureCache::sharedTextureCache()->addImage(finalPath.c_str(), false);
            if (texture) {
                m_fields->m_meterSprite->setTexture(texture);
                
                CCRect rect = CCRectZero;
                rect.size = texture->getContentSize();
                m_fields->m_meterSprite->setTextureRect(rect);
                
                m_fields->m_meterSprite->setScale(1.0f);
                m_fields->m_lastLoadedSprite = targetSpriteName;
            }
        }
    }
};
