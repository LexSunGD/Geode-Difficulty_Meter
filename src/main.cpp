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
    {"MEDIUMDEMON", "MediumDemon_dif.png"}, {"MEDIUM_DEMON", "MediumDemon_dif.png"},
    {"HARDDEMON", "HardDemon_dif.png"}, {"HARD_DEMON", "HardDemon_dif.png"},
    {"INSANEDEMON", "InsaneDemon_dif.png"}, {"INSANE_DEMON", "InsaneDemon_dif.png"},
    {"EXTREMEDEMON", "ExtremeDemon_dif.png"}, {"EXTREME_DEMON", "ExtremeDemon_dif.png"}
};

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

        // Crear el sprite inicial
        std::string initialSprite = Mod::get()->getID() + "/NA_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_fields->m_meterSprite->setPosition({ winSize.width - 60, winSize.height - 40 });
            m_fields->m_meterSprite->setScale(1.0f);
            
            // QUITADO LO INVISIBLE: Ahora el sprite inicia encendido y visible al 100%
            m_fields->m_meterSprite->setVisible(true); 
            
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

            std::ifstream file(destConfigPath);
            std::string content;
            if (std::getline(file, content)) {
                content.erase(std::remove_if(content.begin(), content.end(), [](unsigned char c) {
                    return c == '\r' || c == '\n' || c == '\"' || c == 'f' || c == 'F';
                }), content.end());

                std::stringstream ss(content);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    size_t first = item.find_first_not_of(" \t");
                    size_t last = item.find_last_not_of(" \t."); 
                    if (first == std::string::npos) continue;
                    item = item.substr(first, (last - first + 1));

                    size_t lastSpace = item.find_last_of(" \t");
                    if (lastSpace == std::string::npos) continue;

                    std::string diffName = item.substr(0, lastSpace);
                    std::string rangePart = item.substr(lastSpace + 1);

                    std::transform(diffName.begin(), diffName.end(), diffName.begin(), ::toupper);
                    if (nameToSprite.count(diffName) == 0) continue;
                    std::string targetSprite = nameToSprite[diffName];

                    size_t dashPos = rangePart.find('-');
                    if (dashPos != std::string::npos) {
                        try {
                            float minP = std::stof(rangePart.substr(0, dashPos));
                            float maxP = std::stof(rangePart.substr(dashPos + 1));
                            m_fields->m_allRanges.push_back({targetSprite, minP, maxP});
                        } catch (...) {}
                    }
                }
            }
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

        // QUITADO LO INVISIBLE: Si no hay tramo, se queda puesta la última imagen cargada
        // o la de NA por defecto en lugar de apagarse.
        if (targetSpriteName.empty()) {
            if (m_fields->m_lastLoadedSprite.empty()) {
                targetSpriteName = "EasyDemon_dif.png";
            } else {
                targetSpriteName = m_fields->m_lastLoadedSprite;
            }
        }

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
