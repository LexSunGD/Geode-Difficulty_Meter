#pragma once
#include <string>
#include <vector>

// Estructura interna de la receta
struct DifficultyStep {
    float percentage;          // Porcentaje inicial donde se activa
    std::string spriteName;    // Imagen que se mostrará
};

// CONFIGURACIÓN DE TU RECETA:
// - Asegúrate de que los porcentajes vayan SIEMPRE de menor a mayor (de 0.0f a 100.0f).
// - Puedes repetir imágenes las veces que quieras en distintos porcentajes.
// - Puedes añadir más de 12 o menos si lo necesitas.
const std::vector<DifficultyStep> MY_DIFFICULTY_RECIPE = {
    { 0.0f,   "NA_dif.png" },
    { 8.33f,  "Auto_dif.png" },
    { 16.66f, "Easy_dif.png" },
    { 25.0f,  "Normal_dif.png" },
    { 33.33f, "Hard_dif.png" },
    { 41.66f, "Harder_dif.png" },
    { 50.0f,  "Normal_dif.png" },      // 👈 Ejemplo: Repetimos Normal al 50%
    { 58.33f, "Insane_dif.png" },
    { 66.66f, "EasyDemon_dif.png" },
    { 75.0f,  "MediumDemon_dif.png" },
    { 83.33f, "HardDemon_dif.png" },
    { 91.66f, "InsaneDemon_dif.png" },
    { 95.0f,  "ExtremeDemon_dif.png" }
};
