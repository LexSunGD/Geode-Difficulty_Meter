#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/loader/SettingV3.hpp> // Nueva API de Geode v5
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

std::map<std::string, std::string> shortToFullName = {
    {"NA", "NA_dif.png"}, {"A", "Auto_dif.png"}, {"E", "Easy_dif.png"}, 
    {"N", "Normal_dif.png"}, {"H", "Hard_dif.png"}, {"Hr", "Harder_dif.png"}, 
    {"I", "Insane_dif.png"}, {"ED", "EasyDemon_dif.png"}, {"MD", "MediumDemon_dif.png"}, 
    {"HD", "HardDemon_dif.png"}, {"ID", "InsaneDemon_dif.png"}, {"ExD", "ExtremeDemon_dif.png"}
};

// Función para procesar tu archivo
std::vector<DifficultyRange> loadRangesFromFile(const std::filesystem::path& path) {
    std::vector<DifficultyRange> ranges;
    if (path.empty() || !std::filesystem::exists(path)) return ranges;

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

// --- CREACIÓN DEL NODO PERSONALIZADO SIGUIENDO EL ESTÁNDAR GEODE V5 (SettingNodeV3) ---
class FilePickSettingNode : public SettingNodeV3 {
protected:
    CCLabelBMFont* m_pathLabel = nullptr;
    std::string m_currentPathValue;

    bool init(std::shared_ptr<SettingValueV3> value, float width) {
        if (!SettingNodeV3::init(value, width)) return false;

        // Establecer un tamaño de contenedor estándar
        this->setContentSize({ width, 40.0f });

        // Crear el botón azul (+) para abrir el explorador de archivos nativo
        auto spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        spr->setScale(0.65f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FilePickSettingNode::onPickFile));
        
        auto menu = CCMenu::create();
        menu->addChild(btn);
        menu->setPosition({ width - 30.0f, 20.0f });
        this->addChild(menu);

        // Texto que describe el estado de la ruta seleccionada
        m_pathLabel = CCLabelBMFont::create("Haz clic en (+) para buscar...", "chatFont.fnt");
        m_pathLabel->setAnchorPoint({ 0.0f, 0.5f });
        m_pathLabel->setPosition({ 20.0f, 20.0f });
        m_pathLabel->setScale(0.5f);
        this->addChild(m_pathLabel);

        // Cargar el valor que esté actualmente guardado en el archivo del mod
        m_currentPathValue = Mod::get()->getSettingValue<std::string>(value->getKey());
        this->updateLabel();
        return true;
    }

    void onPickFile(CCObject*) {
        file::FilePickOptions options;
        options.filters = { file::FileFilter("Archivos de Configuracion", {"json", "txt"}) };

        file::pickFile(file::PickType::OpenFile, options, [this](std::filesystem::path path) {
            m_currentPathValue = path.string();
            this->updateLabel();
            // Avisar a Geode que el usuario modificó la opción de manera provisional en la interfaz
            this->dispatchChanged();
        }, []() {
            // Cancelado por el usuario
        });
    }

    void updateLabel() {
        if (m_currentPathValue.empty()) {
            m_pathLabel->setString("Haz clic en (+) para buscar un archivo...");
        } else {
            auto filename = std::filesystem::path(m_currentPathValue).filename().string();
            m_pathLabel->setString(filename.c_str());
        }
    }

public:
    // --- IMPLEMENTACIÓN OBLIGATORIA DE LOS MÉTODOS PUROS VIRTUALES DE GEODE V5 ---
    void onCommit() override {
        // Se ejecuta cuando el usuario presiona "APPLY" o "OK" en el menú
        Mod::get()->setSettingValue<std::string>(m_value->getKey(), m_currentPathValue);
    }

    void onResetToDefault() override {
        // Se ejecuta cuando el usuario le da al botón de restablecer valores
        m_currentPathValue = "";
        this->updateLabel();
    }

    bool hasUncommittedChanges() const override {
        // Compara si lo que está en pantalla es diferente a lo guardado en el disco
        return m_currentPathValue != Mod::get()->getSettingValue<std::string>(m_value->getKey());
    }

    bool hasNonDefaultValue() const override {
        // Compara si el valor actual es diferente del valor por defecto (vacío)
        return !m_currentPathValue.empty();
    }

    static FilePickSettingNode* create(std::shared_ptr<SettingValueV3> value, float width) {
        auto ret = new FilePickSettingNode();
        if (ret && ret->init(value, width)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// --- REGISTRO DE LA CONFIGURACIÓN USANDO LA NUEVA MACRO DE GEODE V5 ---
$setting("config-file-path") {
    return [](std::shared_ptr<SettingValueV3> value, float width) {
        return FilePickSettingNode::create(value, width);
    };
}

// --- INTERFAZ DEL MEDIDOR EN JUEGO ---
class $modify(MyDifficultyMeterLayer, PlayLayer) {
    struct Fields {
        CCSprite* m_meterSprite = nullptr;
        std::vector<DifficultyRange> m_parsedRanges;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontRunLevel) {
        if (!PlayLayer::init(level, useReplay, dontRunLevel)) return false;

        std::string chosenPathStr = Mod::get()->getSettingValue<std::string>("config-file-path");
        
        if (!chosenPathStr.empty()) {
            m_fields->m_parsedRanges = loadRangesFromFile(std::filesystem::path(chosenPathStr));
            log::info("Cargando medidor desde explorador local: {}", chosenPathStr);
        }

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
