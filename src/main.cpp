#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(PlayLayer) {
    
    void levelComplete() {
        // 1. Ejecutamos el método original del juego
        PlayLayer::levelComplete();
        
        // 2. Usamos un scheduler para esperar al siguiente frame.
        // Esto evita que busquemos la capa antes de que termine de crearse en memoria.
        this->getScheduler()->scheduleSelector(
            schedule_selector(PlayLayer::delayedTextSetup), this, 0.0f, 0, 0.05f, false
        );
    }

    // Creamos una función personalizada para añadir el texto de forma segura
    void delayedTextSetup(float dt) {
        // Buscamos la pantalla de victoria
        auto endLevelLayer = CCDirector::get()->getRunningScene()->getChildByID("EndLevelLayer");
        
        if (endLevelLayer) {
            // Buscamos el contenedor principal de la interfaz para meter el texto ahí dentro
            // Si no lo encuentra, usaremos la capa base
            auto mainLayer = endLevelLayer->getChildByID("main-layer");
            auto targetNode = mainLayer ? mainLayer : endLevelLayer;

            // Creamos tu texto personalizado ("SKILLFUL!")
            auto miTexto = CCLabelBMFont::create("HOLAAA!", "goldFont.fnt"); // Usamos goldFont para el estilo amarillo clásico
            
            // Obtenemos el tamaño del nodo objetivo para centrarlo bien
            auto targetSize = targetNode->getContentSize();
            
            // Lo posicionamos abajo, justo encima de los botones inferiores
            miTexto->setPosition({targetSize.width / 2, (targetSize.height / 2) - 45});
            miTexto->setScale(0.7f);
            miTexto->setID("mi-texto-personalizado");
            
            // El ZOrder alto (100) asegura que se pinte por encima de cualquier fondo marrón
            targetNode->addChild(miTexto, 100);
        }
    }
};
