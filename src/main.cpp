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

std::map<std::string, std::string> nameToSprite = {
    {"NA", "NA_dif.png"}, {"AUTO", "Auto_dif.png"}, {"EASY", "Easy_dif.png"},
    {"NORMAL", "Normal_dif.png"}, {"HARD", "Hard_dif.png"}, {"HARDER", "Harder_dif.png"},
    {"INSANE", "Insane_dif.png"}, 
    {"EASYDEMON", "EasyDemon_dif.png"}, {"EASY DEMON", "EasyDemon_dif.png"},
    {"MEDIUMDEMON", "MediumDemon_dif.png"}, {"MEDIUM DEMON", "MediumDemon_dif.png"},
    {"HARDDEMON", "HardDemon_dif.png"}, {"HARD DEMON", "HardDemon_dif.png"},
    {"INSANEDEMON", "InsaneDemon_dif.png"}, {"INSANE DEMON", "InsaneDemon_dif.png"},
    {"EXTREMEDEMON", "ExtremeDemon_dif.png"}, {"EXTREME_DEMON", "ExtremeDemon_dif.png"}
};

// Limpiador corregido: SOLAMENTE quita comillas y saltos de linea invisibles de Android (\r, \n)
std::string cleanAndroidStr(std::string str) {
    str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c) {
        return c == '\r' || c == '\n' || c == '\"';
    }), str.end());
    return str;
}

std::vector<DifficultyRange> loadConfigFromTxt(const std::filesystem::path& path) {
    std::vector<DifficultyRange> ranges;
    if (!std::filesystem::exists(path)) return ranges;

    std::ifstream file(path);
    std::string content;
    if (!std::getline(file, content)) return ranges;
    
    content = cleanAndroidStr(content);
    if (content.empty()) return ranges;

    std::stringstream ss(content);
    std::string item;
    // 1. Separar bloques por comas
    while (std::getline(ss, item, ',')) {
        if (item.empty()) continue;

        std::stringstream itemStream(item);
        std::vector<std::string> tokens;
        std::string token;

        // Extraer todos los fragmentos separados por espacios dentro del bloque
        while (itemStream >> token) {
            tokens.push_back(token);
        }

        // Un bloque valido debe tener al menos el nombre de la dificultad y 2 numeros (ej: Easy 0 10.5)
        if (tokens.size() < 3) continue;

        // Los dos ultimos elementos del bloque son OBLIGATORIAMENTE los porcentajes min y max
        std::string maxStr = tokens.back(); tokens.pop_back();
        std::string minStr = tokens.back(); tokens.pop_back();

        // Todo lo que quedo antes en el vector forma el nombre de la dificultad (soporta espacios)
        std::string diffName = "";
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (i > 0) diffName += " ";
            diffName += tokens[i];
        }

        // Convertir nombre a mayusculas para buscar en el mapa
        std::transform(diffName.begin(), diffName.end(), diffName.begin(), ::toupper);
        if (nameToSprite.count(diffName) == 0) continue;
        std::string targetSprite = nameToSprite[diffName];

        try {
            // Conversion limpia a decimales flotantes reales sin interferencia de letras o guiones
            float minP = std::stof(minStr);
            float maxP = std::stof(maxStr);
            ranges.push_back({targetSprite, minP, maxP});
        } catch (...) {}
    }
    return ranges;
}

class $modify(MyDifficultyMeterLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_meterSprite = nullptr;
        std::vector<DifficultyRange> m_allRanges;
        std::string m_lastLoadedSprite = "";
        bool m_configLoaded = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontRunLevel) {
        if (!PlayLayer::init(level, useReplay, dontRunLevel)) return false;

        m_fields->m_allRanges.clear();
        m_fields->m_lastLoadedSprite = "";
        m_fields->m_configLoaded = false;

        std::string initialSprite = Mod::get()->getID() + "/NA_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_fields->m_meterSprite->setPosition({ winSize.width - 60, winSize.height - 40 });
            m_fields->m_meterSprite->setScale(1.0f);
            m_fields->m_meterSprite->setVisible(false); 
            this->addChild(m_fields->m_meterSprite, 100);
        }

        this->scheduleUpdate();
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        if (!m_fields->m_configLoaded) {
            m_fields->m_configLoaded = true;

            auto configDir = Mod::get()->getConfigDir();
            std::filesystem::create_directories(configDir);
            auto destConfigPath = configDir / "difficulty_meter.txt";

            if (!std::filesystem::exists(destConfigPath)) {
                auto resourcePath = Mod::get()->getResourcesDir() / "difficulty_meter.txt";
                if (std::filesystem::exists(resourcePath)) {
                    std::filesystem::copy_file(resourcePath, destConfigPath);
                } else {
                    std::ofstream outfile(destConfigPath);
                    // Formato nuevo limpio sin guiones en el archivo inicial por si acaso
                    outfile << "Easy 0 10, Normal 11 23, InsaneDemon 24 50, Easy 51 76, Harder 77 91, Auto 92 100";
                    outfile.close();
                }
            }

            m_fields->m_allRanges = loadConfigFromTxt(destConfigPath);
        }

        float percentage = this->getCurrentPercent();
        percentage = std::clamp(percentage, 0.0f, 100.0f);

        std::string targetSpriteName = "";
        for (const auto& range : m_fields->m_allRanges) {
            if (percentage >= range.minPercent && percentage <= range.maxPercent) {
                targetSpriteName = range.spriteName;
                break;
            }
        }

        if (targetSpriteName.empty()) {
            m_fields->m_meterSprite->setVisible(false);
            m_fields->m_lastLoadedSprite = "";
            return;
        }

        m_fields->m_meterSprite->setVisible(true);

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
