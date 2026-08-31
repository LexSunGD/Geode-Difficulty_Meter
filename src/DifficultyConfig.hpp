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
    { 0.0f,   "Normal_dif.png" },
    { 16.66f,  "Hard_dif.png" },
    { 33.33f, "Normal_dif.png" },
    { 47.80f,  "Insane_dif.png" },
    { 49.0f, "Hard_dif.png" },
    { 58.35f, "Easy_dif.png" },
    { 65.95f,  "Auto_dif.png" },      // 👈 Ejemplo: Repetimos Normal al 50%
    { 67.30f, "Normal_dif.png" },
    { 72.0f, "Hard_dif.png" },
    { 75.90f,  "EasyDemon_dif.png" },
    { 77.5f, "Harder_dif.png" },
    { 80.80f, "Easy_dif.png" },
    { 83.0f,  "Hard_dif.png" }
};
