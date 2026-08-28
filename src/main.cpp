#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(PlayLayer) {
    
    void levelComplete() {
        // 1. Ejecutamos el código original del juego primero
        PlayLayer::levelComplete();
        
        // 2. Usamos la utilidad de Geode para ejecutar código un poco después (en el siguiente frame)
        // Esto reemplaza al scheduler problemático y compila perfectamente
        Loader::get()->queueInMainThread([]() {
            // Buscamos la pantalla de victoria
            auto endLevelLayer = CCDirector::get()->getRunningScene()->getChildByID("EndLevelLayer");
            
            if (endLevelLayer) {
                // Intentamos buscar el contenedor interno ('main-layer') automático de Geode
                auto mainLayer = endLevelLayer->getChildByID("main-layer");
                auto targetNode = mainLayer ? mainLayer : endLevelLayer;

                // Creamos tu texto personalizado "SKILLFUL!"
                auto miTexto = CCLabelBMFont::create("HOLAAA!", "goldFont.fnt");
                
                // Obtenemos las dimensiones del contenedor
                auto targetSize = targetNode->getContentSize();
                
                // Lo posicionamos centrado horizontalmente y un poco abajo (encima de los botones)
                miTexto->setPosition({targetSize.width / 2, (targetSize.height / 2) - 45});
                miTexto->setScale(0.7f);
                miTexto->setID("mi-texto-personalizado");
                
                // El 100 asegura que se dibuje por encima del fondo marrón
                targetNode->addChild(miTexto, 100);
            }
        });
    }
};
