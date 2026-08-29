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

// Lector JSON ultra compacto usando el nuevo sistema de Geode v5 (.as<T>())
std::vector<DifficultyRange> loadConfigFromJson(const std::filesystem::path& path) {
    std::vector<DifficultyRange> ranges;
    std::ifstream file(path);
    if (!file.is_open()) return ranges;

    auto parseResult = matjson::parse(file);
    if (!parseResult.isOk()) return ranges;

    try {
        // En Geode v5 los arrays se convierten directamente a vectores con .as<std::vector>()
        auto jsonVector = parseResult.unwrap().as<std::vector<matjson::Value>>();
        
        for (const auto& element : jsonVector) {
            std::string diffName = element["dificultad"].as<std::string>();
            float minP = static_cast<float>(element["inicio"].as<double>());
            float maxP = static_cast<float>(element["fin"].as<double>());

            std::transform(diffName.begin(), diffName.end(), diffName.begin(), ::toupper);
            if (nameToSprite.count(diffName) != 0) {
                ranges.push_back({nameToSprite[diffName], minP, maxP});
            }
        }
    } catch (...) {
        log::error("Error de formato estructurado dentro del archivo JSON.");
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

            if (!std::filesystem::exists(destConfigPath)) {
                auto resourcePath = Mod::get()->getResourcesDir() / "difficulty_meter.json";
                if (std::filesystem::exists(resourcePath)) {
                    std::filesystem::copy_file(resourcePath, destConfigPath);
                } else {
                    std::ofstream outfile(destConfigPath);
                    outfile << "[\n"
                            << "  { \"dificultad\": \"Easy\", \"inicio\": 0, \"fin\": 10.5 },\n"
                            << "  { \"dificultad\": \"Normal\", \"inicio\": 11, \"fin\": 23 },\n"
                            << "  { \"dificultad\": \"Insane Demon\", \"inicio\": 24, \"fin\": 50 },\n"
                            << "  { \"dificultad\": \"Easy\", \"inicio\": 51, \"fin\": 76 },\n"
                            << "  { \"dificultad\": \"Harder\", \"inicio\": 77, \"fin\": 91 },\n"
                            << "  { \"dificultad\": \"Auto\", \"inicio\": 92, \"fin\": 100 }\n"
                            << "]";
                    outfile.close();
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
