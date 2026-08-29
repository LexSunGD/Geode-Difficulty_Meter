#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/utils/JsonValidation.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace geode::prelude;

// Estructura para almacenar cada tramo de dificultad
struct DifficultyRange {
    std::string dificultad;
    float inicio;
    float fin;
};

// Lista global para almacenar la configuración leída del JSON
std::vector<DifficultyRange> g_difficultyRanges;

// Función auxiliar para convertir el nombre del JSON al nombre del sprite compilado por Geode
std::string getSpriteName(const std::string& jsonName) {
    std::string cleanName = jsonName;
    // Eliminar los espacios (ej. "Insane Demon" -> "InsaneDemon")
    cleanName.erase(std::remove_if(cleanName.begin(), cleanName.end(), ::isspace), cleanName.end());
    
    // Geode antepone el ID del mod a los recursos para evitar conflictos
    // Estructura: "ID_DEL_MOD/NombreArchivo.png"
    return "lexsungd.difficulty_meter/" + cleanName + "_dif.png";
}

// Función para cargar el JSON desde la carpeta config del mod
void loadDifficultyJson() {
    g_difficultyRanges.clear();
    
    // Obtener la ruta del archivo en la carpeta 'config' de Geode
    auto configPath = Mod::get()->getConfigDir() / "difficulty_meter.json";
    
    // Si el archivo no existe en config, lo copiamos desde los recursos del mod (instalación limpia)
    if (!ghc::filesystem::exists(configPath)) {
        auto resourcesPath = Mod::get()->getResourcesDir() / "difficulty_meter.json";
        if (ghc::filesystem::exists(resourcesPath)) {
            try {
                ghc::filesystem::copy(resourcesPath, configPath);
            } catch (...) {
                log::error("No se pudo copiar el JSON inicial a la carpeta config.");
            }
        }
    }

    // Leer el archivo JSON
    std::ifstream file(configPath);
    if (!file.is_open()) {
        log::error("No se pudo abrir el archivo difficulty_meter.json en config.");
        return;
    }

    try {
        auto json = matjson::parse(file);
        if (json.is_array()) {
            for (const auto& item : json.as_array()) {
                DifficultyRange range;
                range.dificultad = item["dificultad"].as_string();
                
                // Soportar tanto enteros como decimales para el porcentaje
                range.inicio = static_cast<float>(item["inicio"].as_double());
                range.fin = static_cast<float>(item["fin"].as_double());
                
                g_difficultyRanges.push_back(range);
            }
            log::info("JSON de dificultades cargado exitosamente. Tramos: {}", g_difficultyRanges.size());
        }
    } catch (const std::exception& e) {
        log::error("Error parseando el JSON: {}", e.what());
    }
}

// Modificamos PlayLayer para añadir e interactuar con el medidor
class $modify(MyPlayLayer, PlayLayer) {
    // Variable para identificar nuestro sprite mediante puntero seguro
    CCSprite* m_customDifficultySprite = nullptr;
    std::string m_currentLoadedSpriteName = "";

    bool init(GJGameLevel* level, bool useReplay, bool dontRunActions) {
        if (!PlayLayer::init(level, useReplay, dontRunActions)) return false;

        // Recargar el archivo JSON cada vez que se entra a un nivel por si el usuario lo editó
        loadDifficultyJson();

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // Crear contenedor base
        auto container = CCNode::create();
        container->setID("difficulty-meter-container"_spr);
        container->setPosition({ winSize.width - 60.f, winSize.height - 60.f }); // Esquina superior derecha

        // Inicializar el sprite con una textura por defecto (NA_dif por ejemplo)
        m_fields->m_currentLoadedSpriteName = getSpriteName("NA");
        m_fields->m_customDifficultySprite = CCSprite::createWithSpriteFrameName(m_fields->m_currentLoadedSpriteName.c_str());
        
        if (m_fields->m_customDifficultySprite) {
            m_fields->m_customDifficultySprite->setScale(0.7f);
            container->addChild(m_fields->m_customDifficultySprite);
        }

        // Añadir a la interfaz del juego (Z-Order alto para que esté visible sobre todo)
        this->addChild(container, 999);

        return true;
    }

    // Hook al método update que ejecuta el juego frame a frame
    void update(float dt) {
        PlayLayer::update(dt);

        if (!m_fields->m_customDifficultySprite || g_difficultyRanges.empty()) return;

        // 1. Calcular el porcentaje exacto actual del nivel
        // Usamos la posición del icono del jugador principal respecto a la longitud total del mapa
        float length = m_levelLength;
        if (length <= 0.f) return;

        float currentPosition = m_player1->m_position.x;
        float percent = (currentPosition / length) * 100.f;

        // Limitar entre 0 y 100 por seguridad
        if (percent < 0.f) percent = 0.f;
        if (percent > 100.f) percent = 100.f;

        // 2. Buscar qué dificultad corresponde a este porcentaje
        std::string targetDifficulty = "NA"; // Por defecto si no encuentra match
        for (const auto& range : g_difficultyRanges) {
            if (percent >= range.inicio && percent <= range.fin) {
                targetDifficulty = range.dificultad;
                break;
            }
        }

        // 3. Convertir al nombre del recurso e inyectarlo si cambió
        std::string targetSpriteName = getSpriteName(targetDifficulty);

        if (m_fields->m_currentLoadedSpriteName != targetSpriteName) {
            auto frameCache = CCSpriteFrameCache::sharedSpriteFrameCache();
            auto targetFrame = frameCache->spriteFrameByName(targetSpriteName.c_str());
            
            if (targetFrame) {
                m_fields->m_customDifficultySprite->setDisplayFrame(targetFrame);
                m_fields->m_currentLoadedSpriteName = targetSpriteName;
            } else {
                // Si el frame no existe, intentamos usar NA_dif para prevenir crashes
                auto naFrame = frameCache->spriteFrameByName(getSpriteName("NA").c_str());
                if (naFrame) {
                    m_fields->m_customDifficultySprite->setDisplayFrame(naFrame);
                }
            }
        }
    }
};
