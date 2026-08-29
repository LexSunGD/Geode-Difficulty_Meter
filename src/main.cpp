#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/loader/SettingNode.hpp>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>

using namespace geode::prelude;

struct DifficultyRange {
    std::string spriteName;
    float minPercent;
    float maxPercent;
};

std::map<std::string, std::string> shortToFullName = {
    {"NA", "NA_dif.png"}, {"A", "Auto_dif.png"}, {"E", "Easy_dif.png"}, 
    {"N", "Normal_dif.png"}, {"H", "Hard_dif.png"}, {"Hr", "Harder_dif.png"}, 
    {"I", "Insane_dif.png"}, {"ED", "EasyDemon_dif.png"}, {"MD", "MediumDemon_dif.png"}, 
    {"HD", "HardDemon_dif.png"}, {"ID", "InsaneDemon_dif.png"}, {"ExD", "ExtremeDemon_dif.png"}
};

// Función para leer y procesar el archivo JSON desde cualquier ruta seleccionada
std::vector<DifficultyRange> loadRangesFromFile(const ghc::filesystem::path& path) {
    std::vector<DifficultyRange> ranges;
    if (path.empty() || !ghc::filesystem::exists(path)) return ranges;

    auto parseResult = matjson::parseFile(path);
    if (!parseResult.has_value()) return ranges;

    auto jsonObject = parseResult.value();
    if (!jsonObject.is_object()) return ranges;

    for (const auto& [label, rangeStrObj] : jsonObject.as_object()) {
        if (!rangeStrObj.is_string()) continue;
        std::string rangeStr = rangeStrObj.as_string();

        size_t dash = rangeStr.find('-');
        if (dash != std::string::npos) {
            try {
                float minP = std::stof(rangeStr.substr(0, dash));
                float maxP = std::stof(rangeStr.substr(dash + 1));
                if (shortToFullName.count(label)) {
                    ranges.push_back({shortToFullName[label], minP, maxP});
                }
            } catch (...) {}
        }
    }
    return ranges;
}

// --- CREACIÓN DEL NODO PERSONALIZADO PARA EL MENÚ DE CONFIGURACIÓN ---
class FilePickSettingNode : public SettingNode {
protected:
    CCLabelBMFont* m_pathLabel = nullptr;

    bool init(SettingValue* value, float width) {
        if (!SettingNode::init(value, width)) return false;

        this->setContentSize({ width, 40.0f });

        // Crear botón con ícono de carpeta/archivo de Geode
        auto spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        spr->setScale(0.65f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FilePickSettingNode::onPickFile));
        
        auto menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition({ width - 30.0f, 20.0f });
        this->addChild(menu);

        // Texto que muestra la ruta del archivo seleccionado en el menú
        m_pathLabel = CCLabelBMFont::create("Ningún archivo seleccionado", "chatFont.fnt");
        m_pathLabel->setAnchorPoint({ 0.0f, 0.5f });
        m_pathLabel->setPosition({ 20.0f, 20.0f });
        m_pathLabel->setScale(0.5f);
        this->addChild(m_pathLabel);

        this->updateLabel();
        return true;
    }

    void onPickFile(CCObject*) {
        file::FilePickOptions options;
        options.filters = { file::FileFilter("Archivos de Configuración", {"json", "txt"}) };

        file::pickFile(file::PickType::OpenFile, options, [this](ghc::filesystem::path path) {
            auto value = static_cast<SettingValue*>(m_value);
            Mod::get()->setSettingValue<std::string>(value->getKey(), path.string());
            this->updateLabel();
            this->dispatchChanged();
        }, []() {
            // Cancelado por el usuario
        });
    }

    void updateLabel() {
        std::string currentPath = Mod::get()->getSettingValue<std::string>(m_value->getKey());
        if (currentPath.empty()) {
            m_pathLabel->setString("Haz clic en (+) para buscar un archivo...");
        } else {
            auto filename = ghc::filesystem::path(currentPath).filename().string();
            m_pathLabel->setString(filename.c_str());
        }
    }

public:
    void commit() override { this->dispatchCommitted(); }
    void hasUncommittedChanges() override { return; }
    void revert() override { this->updateLabel(); }

    static FilePickSettingNode* create(SettingValue* value, float width) {
        auto ret = new FilePickSettingNode();
        if (ret && ret->init(value, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// Registrar el nodo personalizado en la API de Geode
$execute {
    Mod::get()->registerCustomSettingNode("config-file-path", [](SettingValue* value, float width) {
        return FilePickSettingNode::create(value, width);
    });
}

// --- CLASE QUE CONTROLA EL MEDIDOR DE DIFICULTAD DENTRO DEL NIVEL ---
class $modify(MyDifficultyMeterLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_meterSprite = nullptr;
        std::vector<DifficultyRange> m_parsedRanges;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontRunLevel) {
        if (!PlayLayer::init(level, useReplay, dontRunLevel)) return false;

        // Leer la ruta absoluta guardada por el explorador de archivos nativo
        std::string chosenPathStr = Mod::get()->getSettingValue<std::string>("config-file-path");
        
        if (!chosenPathStr.empty()) {
            m_fields->m_parsedRanges = loadRangesFromFile(ghc::filesystem::path(chosenPathStr));
            log::info("Cargando medidor desde explorador local: {}", chosenPathStr);
        }

        // Crear el sprite inicial
        std::string initialSprite = Mod::get()->getID() + "/NA_dif.png";
        m_fields->m_meterSprite = CCSprite::create(initialSprite.c_str());
        
        if (m_fields->m_meterSprite) {
            auto winSize = CCDirector::sharedDirector()->getWinSize();
            m_fields->m_meterSprite->setPosition({ winSize.width - 80, winSize.height - 80 });
            m_fields->m_meterSprite->setScale(1.0f); 
            this->m_uiLayer->addChild(m_fields->m_meterSprite);
        }

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (!m_fields->m_meterSprite) return;

        float percentage = 0.0f;
        if (this->m_levelLength > 0.0f) {
            percentage = (this->m_player1->m_position.x / this->m_levelLength) * 100.0f;
        }
        percentage = std::clamp(percentage, 0.0f, 100.0f);

        std::string currentSpriteName = ""; 
        for (const auto& range : m_fields->m_parsedRanges) {
            if (percentage >= range.minPercent && percentage <= range.maxPercent) {
                currentSpriteName = range.spriteName;
                break;
            }
        }

        // Si la zona actual no fue configurada en tu archivo, se vuelve invisible al instante
        if (currentSpriteName.empty()) {
            m_fields->m_meterSprite->setVisible(false);
            return;
        }

        m_fields->m_meterSprite->setVisible(true);
        std::string finalPath = Mod::get()->getID() + "/" + currentSpriteName;
        auto texture = CCTextureCache::sharedTextureCache()->addImage(finalPath.c_str(), false);
        
        if (texture) {
            m_fields->m_meterSprite->setTexture(texture);
            m_fields->m_meterSprite->setScale(1.0f);
        }
    }
};
