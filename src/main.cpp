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
    {"MEDIUMDEMON", "MediumDemon_dif.png"}, {"MEDIUM_DEMON", "MediumDemon_dif.png"},
    {"HARDDEMON", "HardDemon_dif.png"}, {"HARD_DEMON", "HardDemon_dif.png"},
    {"INSANEDEMON", "InsaneDemon_dif.png"}, {"INSANE_DEMON", "InsaneDemon_dif.png"},
    {"EXTREMEDEMON", "ExtremeDemon_dif.png"}, {"EXTREME_DEMON", "ExtremeDemon_dif.png"}
};

// Limpiador agresivo de caracteres de control de Android
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
    // 1. Separar bloques por comas
    while (std::getline(ss, item, ',')) {
        if (item.empty()) continue;

        // CORRECCIÓN RADICAL: Buscar dónde empieza el primer número (0-9) en el bloque
        size_t firstDigitPos = std::string::npos;
        for (size_t i = 0; i < item.length(); ++i) {
            if (std::isdigit(static_cast<unsigned char>(item[i]))) {
                firstDigitPos = i;
                break;
            }
        }

        // Si no hay números en el bloque, no es una regla válida
        if (firstDigitPos == std::string::npos || firstDigitPos == 0) continue;

        // Separar limpiamente el nombre de la dificultad y el tramo numérico
        std::string diffName = item.substr(0, firstDigitPos);
        std::string rangePart = item.substr(firstDigitPos);

        // Limpiar espacios remanentes alrededor de las palabras
        diffName.erase(0, diffName.find_first_not_of(" \t"));
        diffName.erase(diffName.find_last_not_of(" \t") + 1);
        
        std::transform(diffName.begin(), diffName.end(), diffName.begin(), ::toupper);
        if (nameToSprite.count(diffName) == 0) continue;
        std::string targetSprite = nameToSprite[diffName];

        // Procesar los números usando el guion '-'
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
            // Posición limpia en la esquina superior derecha, como en tu captura
            m_fields->m_meterSprite->setPosition({ winSize.width - 60, winSize.height - 40 });
            m_fields->m_meterSprite->setScale(1.0f);
            
            // VOLVEMOS A HACERLO INVISIBLE: Si la traducción tiene éxito, el update lo encenderá al instante
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

        // Obtener porcentaje de Geode
        float percentage = this->getCurrentPercent();
        percentage = std::clamp(percentage, 0.0f, 100.0f);

        std::string targetSpriteName = "";
        for (const auto& range : m_fields->m_allRanges) {
            if (percentage >= range.minPercent && percentage <= range.maxPercent) {
                targetSpriteName = range.spriteName;
                break;
            }
        }

        // Invisibilidad reactiva en zonas vacías
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
