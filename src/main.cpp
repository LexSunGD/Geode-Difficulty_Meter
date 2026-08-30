#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(MyPlayLayer, PlayLayer) {
    
    void update(float dt) {
        PlayLayer::update(dt); // Llama a la función original del juego

        // 1. Calcular el porcentaje actual del nivel
        // Evitamos división por cero asegurándonos de que la longitud sea válida
        if (m_levelLength <= 0.0f) return; 
        
        float percentage = (m_player1->m_position.x / m_levelLength) * 100.0f;
        if (percentage > 100.0f) percentage = 100.0f;
        if (percentage < 0.0f) percentage = 0.0f;

        // 2. Determinar qué sprite usar según el porcentaje (12 dificultades)
        std::string spriteName = "Normal_dif.png"; // Por defecto

        if (percentage < 8.33f) {
            spriteName = "NA_dif.png";
        } else if (percentage < 16.66f) {
            spriteName = "Auto_dif.png";
        } else if (percentage < 25.0f) {
            spriteName = "Easy_dif.png";
        } else if (percentage < 33.33f) {
            spriteName = "Normal_dif.png";
        } else if (percentage < 41.66f) {
            spriteName = "Hard_dif.png";
        } else if (percentage < 50.0f) {
            spriteName = "Harder_dif.png";
        } else if (percentage < 58.33f) {
            spriteName = "Insane_dif.png";
        } else if (percentage < 66.66f) {
            spriteName = "EasyDemon_dif.png";
        } else if (percentage < 75.0f) {
            spriteName = "MediumDemon_dif.png";
        } else if (percentage < 83.33f) {
            spriteName = "HardDemon_dif.png";
        } else if (percentage < 91.66f) {
            spriteName = "InsaneDemon_dif.png";
        } else {
            spriteName = "ExtremeDemon_dif.png";
        }

        // 3. Buscar el sprite del medidor de dificultad en la interfaz (UILayer)
        // Nota: Dependiendo de dónde esté el medidor original en la interfaz de la 2.2081,
        // podrías necesitar buscarlo por su ID de nodo asignado por el juego o Geode.
        if (m_uiLayer) {
            // Ejemplo conceptual para actualizar la textura si el sprite ya está cargado en el juego
            auto spriteFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(spriteName.c_str());
            
            if (spriteFrame) {
                // Aquí deberás obtener el puntero exacto al medidor de dificultad. 
                // Si creas tu propio sprite personalizado para mostrarlo en pantalla:
                // miSpriteCustom->setDisplayFrame(spriteFrame);
            }
        }
    }
};
