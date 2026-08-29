#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <filesystem>

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
    {"EASYDEMON", "EasyDemon_dif.png"}, {"EASY_DEMON", "EasyDemon_dif.png"},
    {"MEDIUMDEMON", "MediumDemon_dif.png"}, {"MEDIUM_DEMON", "MediumDemon_dif.png"},
    {"HARDDEMON", "HardDemon_dif.png"}, {"HARD_DEMON", "HardDemon_dif.png"},
    {"INSANEDEMON", "InsaneDemon_dif.png"}, {"INSANE_DEMON", "InsaneDemon_dif.png"},
    {"EXTREMEDEMON", "ExtremeDemon_dif.png"}, {"EXTREME_DEMON", "ExtremeDemon_dif.png"}
};

// Lector JSON nativo corregido para Geode v5
std::vector<DifficultyRange> loadConfigFromJson(const std::filesystem::path& path) {
    std::vector<DifficultyRange> ranges;
    if (!std::filesystem::exists(path)) return ranges;

    std::ifstream file(path);
    if (!file.is_open()) return ranges;

    // Pasar el flujo del archivo directamente a matjson::parse
    auto parseResult = matjson::parse(file);
    if (!parseResult.has_value()) {
        log::error("El archivo JSON tiene un error de corchetes o sintaxis.");
        return ranges;
    }

    auto jsonArray = parseResult.value();
    if (!jsonArray.is_array()) return ranges;

    for (const auto& element : jsonArray.as_array()) {
        if (!element.is_object()) continue;
        auto obj = element.as_object();

        if (obj.count("dificultad") && obj.count("inicio") && obj.count("fin")) {
            std::string diffName = obj["dificultad"].as_string();
            
            float minP = static_cast<float>(obj["inicio"].as_double());
            float maxP = static_cast<float>(obj["fin"].as_double());

            std::transform(diffName.begin(), diffName.end(), diffName.begin(), ::toupper);
            if (nameToSprite.count(diffName) != 0) {
                ranges.push_back({nameToSprite[diffName], minP, maxP});
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
            auto destConfigPath = configDir / "difficulty_meter.json";

            // CORRECCIÓN: Ahora el código es súper corto y limpio. 
            // Si el archivo no existe en la carpeta config de Android, lo copia directamente desde los archivos del mod.
            if (!std::filesystem::exists(destConfigPath)) {
                auto resourcePath = Mod::get()->getResourcesDir() / "difficulty_meter.json";
                if (std::filesystem::exists(resourcePath)) {
                    std::filesystem::copy_file(resourcePath, destConfigPath);
                    log::info("Plantilla JSON copiada exitosamente a la carpeta config.");
                } else {
                    log::error("No se encontro el archivo difficulty_meter.json en los recursos del mod.");
                }
            }

            m_fields->m_allRanges = loadConfigFromJson(destConfigPath);
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
