#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <cctype>

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
    {"EXTREMEDEMON", "ExtremeDemon_dif.png"}, {"EXTREME DEMON", "ExtremeDemon_dif.png"}
};

// Limpiador agresivo de caracteres basura y saltos de línea de Android
std::string cleanInputString(std::string str) {
    str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c) {
        return c == '\r' || c == '\n' || c == '\"' || c == 'f' || c == 'F' || c == '.';
    }), str.end());
    return str;
}

std::vector<DifficultyRange> loadConfigFromTxt(const std::filesystem::path& path) {
    std::vector<DifficultyRange> ranges;
    if (!std::filesystem::exists(path)) return ranges;

    std::ifstream file(path);
    std::string content;
    if (!std::getline(file, content)) return ranges;
    
    content = cleanInputString(content);
    if (content.empty()) return ranges;

    std::stringstream ss(content);
    std::string item;
    // 1. Separar bloques independientes por las comas
    while (std::getline(ss, item, ',')) {
        if (item.empty()) continue;

        // Separar las palabras de los números usando flujos de extracción limpios
        std::stringstream itemStream(item);
        std::string word, fullDiffName = "", rangePart = "";

        // Leer palabra por palabra hasta encontrar el bloque del rango (ej: "11-23")
        while (itemStream >> word) {
            if (word.find('-') != std::string::npos || std::isdigit(static_cast<unsigned char>(word[0]))) {
                rangePart = word;
                break;
            } else {
                if (!fullDiffName.empty()) fullDiffName += " ";
                fullDiffName += word;
            }
        }

        if (fullDiffName.empty() || rangePart.empty()) continue;

        // Convertir el nombre de la dificultad a mayúsculas
        std::transform(fullDiffName.begin(), fullDiffName.end(), fullDiffName.begin(), ::toupper);
        if (nameToSprite.count(fullDiffName) == 0) continue;
        std::string targetSprite = nameToSprite[fullDiffName];

        // Procesar los números del rango de forma aislada e inmune a espacios
        size_t dashPos = rangePart.find('-');
        if (dashPos != std::string::npos) {
            try {
                float minP = std::stof(rangePart.substr(0, dashPos));
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
            
            // Inicia oculto. Si la traducción tiene éxito, el update lo encenderá de inmediato
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
                    outfile << "Easy 0-10, Normal 11-23, InsaneDemon 24-50, Easy 51-76, Harder 77-91, Auto 92-100";
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
