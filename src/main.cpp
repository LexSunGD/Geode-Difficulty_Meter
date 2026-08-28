#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// Modificamos la clase PlayLayer del juego
class $modify(PlayLayer) {
    
    void levelComplete() {
        // Ejecuta primero el código original del juego para que aparezca la ventana de victoria
        PlayLayer::levelComplete();
        
        // Buscamos la capa 'EndLevelLayer' que es la ventana que aparece al ganar
        auto endLevelLayer = CCDirector::get()->getRunningScene()->getChildByID("EndLevelLayer");
        
        if (endLevelLayer) {
            // Creamos un texto personalizado usando la fuente del juego
            // Puedes cambiar "¡Nivel Completado!" por lo que tú quieras
            auto miTexto = CCLabelBMFont::create("¡Eres un crack!", "bigFont.fnt");
            
            // Obtenemos el tamaño de la pantalla para centrarlo adecuadamente
            auto winSize = CCDirector::get()->getWinSize();
            
            // Posicionamos el texto (en este ejemplo, un poco más arriba del centro)
            miTexto->setPosition({winSize.width / 2, (winSize.height / 2) + 50});
            miTexto->setScale(0.7f); // Cambia el tamaño del texto
            miTexto->setColor({255, 255, 0}); // Color Amarillo (R, G, B)
            
            // (Opcional) Le asignamos un ID único para evitar conflictos con otros mods
            miTexto->setID("mi-texto-personalizado");
            
            // Añadimos el texto a la ventana de victoria
            endLevelLayer->addChild(miTexto);
        }
    }
};
