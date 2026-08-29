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

// Traducción interna de tus textos a los nombres reales
std::map<std::string, std::string> nameToSprite = {
    {"NA", "NA_dif.png"}, 
    {"AUTO", "Auto_dif.png"}, 
    {"EASY", "Easy_dif.png"},
    {"NORMAL", "Normal_dif.png"}, 
    {"HARD", "Hard_dif.png"}, 
    {"HARDER", "Harder_dif.png"},
    {"INSANE", "Insane_dif.png"}, 
    {"EASYDEMON", "EasyDemon_dif.png"}, {"EASY DEMON", "EasyDemon_dif.png"},
    {"MEDIUMDEMON", "MediumDemon_dif.png"}, {"MEDIUM DEMON", "MediumDemon_dif.png"},
    {"HARDDEMON", "HardDemon_dif.png"}, {"HARD DEMON", "HardDemon_dif.png"},
    {"INSANEDEMON", "InsaneDemon_dif.png"}, {"INSANE DEMON", "InsaneDemon_dif.png"},
    {"EXTREMEDEMON", "ExtremeDemon_dif.png"}, {"EXTREME_DEMON", "ExtremeDemon_dif.png"}
};

// Limpia comillas, puntos y saltos de línea de Android
std::string trimStr(std::string str) {
    if (str.empty()) return str;
    str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c) {
        return c == '\r' || c == '\n' || c == '.' || c == '\"';
    }), str.end());
    
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

// Procesa el archivo de texto continuo
std::vector<DifficultyRange> loadConfigFromTxt(const std::filesystem::path& path) {
    std::vector<DifficultyRange> ranges;
    if (!std::filesystem::exists(path)) return ranges;

    std::ifstream file(path);
    std::string content;
    std::getline(file, content);
    content = trimStr(content);
    if (content.empty()) return ranges;

    std::stringstream ss(content);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item = trimStr(item);
        if (item.empty()) continue;

        size_t lastSpace = item.find_last_of(" \t");
        if (lastSpace == std::string::npos) continue;

        std::string diffName = trimStr(item.substr(0, lastSpace));
        std::string rangePart = trimStr(item.substr(lastSpace + 1));

        std::transform(diffName.begin(), diffName.end(), diffName.begin(), ::toupper);
        if (nameToSprite.count(diffName) == 0) continue;
        std::string targetSprite = nameToSprite[diffName];

        size_t dashPos = rangePart.find('-');
        if (dashPos != std::string::npos) {
            try {
                float minP = std::stof(trimStr(rangePart.substr(0, dashPos)));
                float maxP = std::stof(rangePart.substr(dashPos + 1));
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

        // 1. Crear la ruta de almacenamiento seguro
        auto configDir = Mod::get()->getConfigDir();
        std::filesystem::create_directories(configDir);
        auto destConfigPath = configDir / "difficulty_meter.txt";
        
        // 2. Copiar o generar archivo inicial si no existe
        if (!std::filesystem::exists(destConfigPath)) {
            auto resourcePath = Mod::get()->getResourcesDir() / "difficulty_meter.txt";
            if (std::filesystem::exists(resourcePath)) {
                std::filesystem::copy_file(resourcePath, destConfigPath);
            } else {
                std::ofstream outfile(destConfigPath);
                outfile << "Easy 0-10, Normal 11-23, InsaneDemon 24-50, Easy 51-76, Harder 77-91, Auto 92-100";
                outfile.close();
            }
        }

        // 3. Procesar las reglas del archivo de texto
        m_fields->m_allRanges = loadConfigFromTxt(destConfigPath);

        // 4. Crear sprite con la ruta corregida usando el ID para evitar el cuadro rosa
        std::string initialSprite = Mod::get()->getID() + "/NA_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            // Posición estándar en la esquina superior derecha
            m_fields->m_meterSprite->setPosition({ winSize.width - 80, winSize.height - 80 });
            m_fields->m_meterSprite->setScale(1.0f);
            
            // Invisibilidad automática por defecto
            m_fields->m_meterSprite->setVisible(false); 
            
            this->addChild(m_fields->m_meterSprite, 100);
        }

        this->scheduleUpdate();
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        // Extraer porcentaje oficial
        float percentage = this->getCurrentPercent();
        percentage = std::clamp(percentage, 0.0f, 100.0f);

        // Escanear tramos
        std::string targetSpriteName = "";
        for (const auto& range : m_fields->m_allRanges) {
            if (percentage >= range.minPercent && percentage <= range.maxPercent) {
                targetSpriteName = range.spriteName;
                break;
            }
        }

        // Si cae en zona vacía, se vuelve invisible al instante
        if (targetSpriteName.empty()) {
            m_fields->m_meterSprite->setVisible(false);
            m_fields->m_lastLoadedSprite = "";
            return;
        }

        m_fields->m_meterSprite->setVisible(true);

        // Cambiar texturas de forma segura adjuntando el ID del Mod
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
