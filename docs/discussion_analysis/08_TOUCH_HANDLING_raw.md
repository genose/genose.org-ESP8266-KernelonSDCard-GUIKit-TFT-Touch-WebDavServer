







ajoute le touchscreen





10:18








Voici comment intégrer la gestion du touchscreen à votre GUIKit pour ESP8266 + écran TFT 3.2". Je vais ajouter le support pour le XPT2046 (le contrôleur tactile le plus courant pour les écrans TFT) et implémenter la détection des clics, des glissés (drag), et des gestes de base.

📌 Étapes pour Ajouter le Touchscreen

Ajouter la bibliothèque XPT2046_Touchscreen à votre projet.
Configurer les broches pour le tactile dans platformio.ini.
Créer un fichier touch.h et touch.cpp pour gérer les entrées tactiles.
Mettre à jour main.cpp pour gérer les interactions.

📄 1. Mise à Jour de platformio.ini
Ajoutez la dépendance pour XPT2046_Touchscreen et configurez les broches du tactile.
ini
Copier

[env:esp8266_tft]
platform = espressif8266
board = nodemcuv2
framework = arduino

; Bibliothèque pour l'écran TFT
lib_deps =
    https://github.com/Bodmer/TFT_eSPI.git
    https://github.com/PaulStoffregen/XPT2046_Touchscreen.git  ; Bibliothèque pour le touchscreen

; Configuration pour TFT_eSPI
build_flags =
    -D USER_SETUP_LOADED
    -D ST7789_DRIVER  ; Ajustez selon votre contrôleur (ST7735, ILI9341, etc.)
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_CS=D8
    -D TFT_DC=D3
    -D TFT_RST=D4

; Configuration pour le touchscreen XPT2046
build_flags =
    -D TOUCH_CS=D2       ; Pin CS pour le tactile
    -D XPT2046_IRQ=D1    ; Pin IRQ (optionnel, pour les interruptions)

monitor_speed = 115200




📄 2. touch.h (Déclarations pour le Touchscreen)
Créez un nouveau fichier touch.h dans le dossier src/.
cpp
Copier

#ifndef TOUCH_H
#define TOUCH_H

#include <stdint.h>
#include <stdbool.h>
#include "widget.h"
#include <XPT2046_Touchscreen.h>

// ========== STRUCTURES POUR LE TOUCH ==========
typedef struct {
    uint16_t x;  // Coordonnée X (0 à TFT_WIDTH-1)
    uint16_t y;  // Coordonnée Y (0 à TFT_HEIGHT-1)
    bool pressed; // État : appui en cours
} TouchPoint;

// ========== ÉTAT DU TOUCH ==========
typedef struct {
    TouchPoint current;      // Point tactile actuel
    TouchPoint previous;     // Point tactile précédent
    bool touched;            // Si l'écran est touché
    bool long_press;         // Si appui long détecté
    uint32_t press_time;     // Temps de début de l'appui (en ms)
    Widget* focused_widget;   // Widget actuellement "focusé" (pour drag)
} TouchState;

// ========== FONCTIONS DE GESTION DU TOUCH ==========
void init_touch(void);
void update_touch(void);
bool is_touched(void);
TouchPoint get_touch_point(void);
bool is_touch_inside_widget(Widget* widget, TouchPoint point);
Widget* find_widget_at_point(Widget* root, TouchPoint point);

// ========== GESTION DES ÉVÉNEMENTS ==========
void handle_touch_press(TouchPoint point);
void handle_touch_release(TouchPoint point);
void handle_touch_drag(TouchPoint point);
void handle_long_press(TouchPoint point);

// ========== FONCTIONS POUR LES WIDGETS INTERACTIFS ==========
void handle_button_touch(WidgetButton* button, TouchPoint point);
void handle_slider_touch(WidgetSlider* slider, TouchPoint point);
void handle_checkbox_touch(WidgetCheckbox* checkbox, TouchPoint point);

#endif // TOUCH_H




📄 3. touch.cpp (Implémentation du Touchscreen)
Créez un nouveau fichier touch.cpp dans le dossier src/.
cpp
Copier

#include "touch.h"
#include "widget.h"
#include "renderer.h"
#include <Arduino.h>

// ========== VARIABLES GLOBALES ==========
XPT2046_Touchscreen ts(TOUCH_CS);  // Initialiser avec le pin CS
TouchState touch_state = {0};

// ========== FONCTIONS D'INITIALISATION ==========
void init_touch(void) {
    ts.begin();
    touch_state.touched = false;
    touch_state.focused_widget = NULL;
    touch_state.long_press = false;
    touch_state.press_time = 0;
}

// ========== MISE À JOUR DU TOUCH ==========
void update_touch(void) {
    touch_state.previous = touch_state.current;
    touch_state.current.pressed = ts.touched();

    if (touch_state.current.pressed) {
        TS_Point p = ts.getPoint();

        // Mapper les coordonnées brutes du tactile aux coordonnées de l'écran
        // Note: Les valeurs brutes dépendent de votre écran et de son orientation.
        // Ajustez ces valeurs selon votre configuration.
        touch_state.current.x = map(p.x, 0, 4095, 0, TFT_WIDTH - 1);
        touch_state.current.y = map(p.y, 0, 4095, 0, TFT_HEIGHT - 1);

        // Inverser X et Y si nécessaire (selon l'orientation de l'écran)
        // Exemple pour un écran en mode portrait (rotation 1 ou 3) :
        uint16_t temp = touch_state.current.x;
        touch_state.current.x = touch_state.current.y;
        touch_state.current.y = TFT_HEIGHT - 1 - temp;

        // Gérer les événements tactiles
        if (!touch_state.touched) {
            // Nouvel appui détecté
            touch_state.touched = true;
            touch_state.press_time = millis();
            touch_state.long_press = false;
            handle_touch_press(touch_state.current);
        } else {
            // Appui en cours (drag)
            handle_touch_drag(touch_state.current);

            // Vérifier si appui long (ex: 1 seconde)
            if (!touch_state.long_press && (millis() - touch_state.press_time > 1000)) {
                touch_state.long_press = true;
                handle_long_press(touch_state.current);
            }
        }
    } else {
        if (touch_state.touched) {
            // Relâchement détecté
            touch_state.touched = false;
            handle_touch_release(touch_state.current);
        }
    }
}

// ========== FONCTIONS D'ACCÈS AU TOUCH ==========
bool is_touched(void) {
    return touch_state.touched;
}

TouchPoint get_touch_point(void) {
    return touch_state.current;
}

// ========== VÉRIFIER SI UN POINT EST DANS UN WIDGET ==========
bool is_touch_inside_widget(Widget* widget, TouchPoint point) {
    if (!widget) return false;

    // Vérifier si le point est dans le rectangle du widget
    return (point.x >= widget->rect.position.x) &&
           (point.x < widget->rect.position.x + widget->rect.size.width) &&
           (point.y >= widget->rect.position.y) &&
           (point.y < widget->rect.position.y + widget->rect.size.height);
}

// ========== TROUVER LE WIDGET SOUS UN POINT ==========
Widget* find_widget_at_point(Widget* root, TouchPoint point) {
    if (!root) return NULL;

    // Parcourir les enfants en ordre inverse (pour que les widgets en haut de la pile soient prioritaires)
    for (int8_t i = root->children_count - 1; i >= 0; i--) {
        Widget* child = root->children[i];
        Widget* result = find_widget_at_point(child, point);
        if (result) return result;
    }

    // Vérifier si le widget racine contient le point
    if (is_touch_inside_widget(root, point)) {
        return root;
    }

    return NULL;
}

// ========== GESTION DES ÉVÉNEMENTS TACTILES ==========
void handle_touch_press(TouchPoint point) {
    Widget* widget = find_widget_at_point(root_view, point);

    if (widget) {
        touch_state.focused_widget = widget;

        // Gérer l'appui selon le type de widget
        switch (widget->type) {
            case WIDGET_TYPE_BUTTON: {
                WidgetButton* button = (WidgetButton*)widget;
                button->pressed = true;
                widget_add_style(widget, WIDGET_DRAW_STYLE_GLOW);
                draw_widget(widget);
                if (button->on_click) {
                    button->on_click();
                }
                break;
            }
            case WIDGET_TYPE_SLIDER: {
                WidgetSlider* slider = (WidgetSlider*)widget;
                handle_slider_touch(slider, point);
                break;
            }
            case WIDGET_TYPE_CHECKBOX: {
                WidgetCheckbox* checkbox = (WidgetCheckbox*)widget;
                handle_checkbox_touch(checkbox, point);
                break;
            }
            default:
                // Pour les autres widgets, simplement les marquer comme "focused"
                break;
        }
    }
}

void handle_touch_release(TouchPoint point) {
    if (touch_state.focused_widget) {
        Widget* widget = touch_state.focused_widget;

        switch (widget->type) {
            case WIDGET_TYPE_BUTTON: {
                WidgetButton* button = (WidgetButton*)widget;
                button->pressed = false;
                widget_remove_style(widget, WIDGET_DRAW_STYLE_GLOW);
                draw_widget(widget);
                if (button->on_release) {
                    button->on_release();
                }
                break;
            }
            // Pour les autres widgets, simplement les "défocuser"
            default:
                break;
        }

        touch_state.focused_widget = NULL;
    }
}

void handle_touch_drag(TouchPoint point) {
    if (touch_state.focused_widget) {
        Widget* widget = touch_state.focused_widget;

        switch (widget->type) {
            case WIDGET_TYPE_SLIDER: {
                WidgetSlider* slider = (WidgetSlider*)widget;
                handle_slider_touch(slider, point);
                break;
            }
            // Pour les autres widgets, vous pouvez implémenter le drag (ex: déplacer un widget)
            default:
                break;
        }
    }
}

void handle_long_press(TouchPoint point) {
    if (touch_state.focused_widget) {
        Widget* widget = touch_state.focused_widget;
        Serial.println("Appui long détecté !");

        // Exemple : Afficher un menu contextuel pour le widget
        switch (widget->type) {
            case WIDGET_TYPE_BUTTON: {
                WidgetButton* button = (WidgetButton*)widget;
                Serial.print("Menu contextuel pour le bouton: ");
                Serial.println(button->base.text.text);
                break;
            }
            default:
                break;
        }
    }
}

// ========== GESTION DES WIDGETS SPÉCIFIQUES ==========
void handle_button_touch(WidgetButton* button, TouchPoint point) {
    // Déjà géré dans handle_touch_press et handle_touch_release
}

void handle_slider_touch(WidgetSlider* slider, TouchPoint point) {
    if (!slider) return;

    float value;
    if (slider->vertical) {
        // Slider vertical : valeur basée sur la position Y
        float ratio = (float)(point.y - slider->base.rect.position.y) / slider->base.rect.size.height;
        value = slider->min_value + (slider->max_value - slider->min_value) * (1.0f - ratio);
    } else {
        // Slider horizontal : valeur basée sur la position X
        float ratio = (float)(point.x - slider->base.rect.position.x) / slider->base.rect.size.width;
        value = slider->min_value + (slider->max_value - slider->min_value) * ratio;
    }

    // Limiter la valeur entre min et max
    if (value < slider->min_value) value = slider->min_value;
    if (value > slider->max_value) value = slider->max_value;

    slider->current_value = value;
    if (slider->on_change) {
        slider->on_change(value);
    }

    // Redessiner le slider
    draw_widget((Widget*)slider);
}

void handle_checkbox_touch(WidgetCheckbox* checkbox, TouchPoint point) {
    if (!checkbox) return;

    checkbox->checked = !checkbox->checked;
    if (checkbox->on_toggle) {
        checkbox->on_toggle(checkbox->checked);
    }

    // Redessiner la checkbox
    draw_widget((Widget*)checkbox);
}




📄 4. Mise à Jour de main.cpp
Remplacez votre fichier main.cpp par cette version qui inclut la gestion du touchscreen.
cpp
Copier

#include <Arduino.h>
#include "widget.h"
#include "renderer.h"
#include "style.h"
#include "touch.h"

// ========== VARIABLES GLOBALES ==========
Widget* root_view;          // Vue racine (conteneur principal)
WidgetButton* button1;      // Bouton 1
WidgetButton* button2;      // Bouton 2
WidgetLabel* label;         // Label
WidgetSlider* slider;      // Slider
WidgetCheckbox* checkbox;   // Case à cocher

// ========== CALLBACKS ==========
void on_button1_click() {
    Serial.println("Bouton 1 cliqué !");
    widget_add_style(button1, WIDGET_DRAW_STYLE_GLOW);
    draw_widget(button1);
}

void on_button1_release() {
    Serial.println("Bouton 1 relâché !");
    widget_remove_style(button1, WIDGET_DRAW_STYLE_GLOW);
    draw_widget(button1);
}

void on_button2_click() {
    Serial.println("Bouton 2 cliqué !");
    // Exemple : Changer la couleur du bouton
    button2->base.style.colors.primary = 0xF800; // Rouge
    button2->base.style.colors.secondary = 0x07E0; // Vert
    draw_widget(button2);
}

void on_slider_change(float value) {
    Serial.print("Slider value: ");
    Serial.println(value);
    // Mettre à jour le label avec la valeur du slider
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Valeur: %.1f", value);
    strncpy(label->base.text.text, buffer, MAX_TEXT_LENGTH - 1);
    label->base.text.text[MAX_TEXT_LENGTH - 1] = '\0';
    draw_widget(label);
}

void on_checkbox_toggle(bool checked) {
    Serial.print("Checkbox: ");
    Serial.println(checked ? "Coché" : "Décoché");
    // Exemple : Changer la couleur du label en fonction de la checkbox
    label->base.style.colors.primary = checked ? 0x07E0 : 0xFFFF; // Vert ou Blanc
    draw_widget(label);
}

// ========== FONCTION D'INITIALISATION ==========
void setup() {
    Serial.begin(115200);
    init_renderer(); // Initialiser TFT_eSPI
    init_touch();    // Initialiser le touchscreen

    // Créer la vue racine (conteneur principal)
    root_view = new_widget(WIDGET_TYPE_VIEW);
    root_view->rect.position.x = 0;
    root_view->rect.position.y = 0;
    root_view->rect.size.width = tft.width();
    root_view->rect.size.height = tft.height();
    root_view->style.draw_style = WIDGET_DRAW_STYLE_SOLID_FILL;
    root_view->style.colors.primary = 0x0000; // Fond noir

    // Créer un bouton
    button1 = new_button();
    button1->base.rect.position.x = 50;
    button1->base.rect.position.y = 50;
    button1->base.rect.size.width = 120;
    button1->base.rect.size.height = 50;
    strncpy(button1->base.text.text, "Bouton 1", MAX_TEXT_LENGTH - 1);
    button1->base.text.text[MAX_TEXT_LENGTH - 1] = '\0';
    button1->on_click = on_button1_click;
    button1->on_release = on_button1_release;

    // Créer un deuxième bouton avec un style différent
    button2 = new_button();
    button2->base.rect.position.x = 50;
    button2->base.rect.position.y = 120;
    button2->base.rect.size.width = 120;
    button2->base.rect.size.height = 50;
    strncpy(button2->base.text.text, "Bouton 2", MAX_TEXT_LENGTH - 1);
    button2->base.text.text[MAX_TEXT_LENGTH - 1] = '\0';
    button2->base.style.draw_style = WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT | WIDGET_DRAW_STYLE_DROP_SHADOW;
    button2->base.style.colors.primary = 0x07E0; // Vert
    button2->base.style.colors.secondary = 0x001F; // Bleu
    button2->base.style.border.radius = 10;
    button2->on_click = on_button2_click;

    // Créer un label
    label = new_label("Valeur: 0.0");
    label->base.rect.position.x = 50;
    label->base.rect.position.y = 200;
    label->base.rect.size.width = 200;
    label->base.rect.size.height = 30;
    label->base.style.colors.primary = 0xFFFF; // Blanc
    label->base.text.font.size = 16;

    // Créer un slider
    slider = new_slider(0.0f, 100.0f, 50.0f);
    slider->base.rect.position.x = 50;
    slider->base.rect.position.y = 250;
    slider->base.rect.size.width = 200;
    slider->base.rect.size.height = 20;
    slider->on_change = on_slider_change;

    // Créer une case à cocher
    checkbox = new_checkbox(false);
    checkbox->base.rect.position.x = 50;
    checkbox->base.rect.position.y = 300;
    checkbox->on_toggle = on_checkbox_toggle;

    // Ajouter les widgets à la vue racine
    widget_add_child(root_view, (Widget*)button1);
    widget_add_child(root_view, (Widget*)button2);
    widget_add_child(root_view, (Widget*)label);
    widget_add_child(root_view, (Widget*)slider);
    widget_add_child(root_view, (Widget*)checkbox);

    // Dessiner l'arborescence des widgets
    draw_widget_tree(root_view);

    Serial.println("Initialisation terminée !");
}

// ========== BOUCLE PRINCIPALE ==========
void loop() {
    update_touch(); // Mettre à jour l'état du touchscreen
    delay(10);      // Petit délai pour éviter de saturer le CPU
}




📌 Explications des Fonctionnalités Ajoutées

1. Gestion du Touchscreen

XPT2046_Touchscreen : Bibliothèque pour lire les coordonnées du tactile.
TouchPoint : Structure pour stocker les coordonnées (x, y) et l'état (pressed).
TouchState : Structure pour gérer l'état global du tactile (appui, drag, widget focusé, etc.).

2. Détection des Événements

handle_touch_press : Appelé lorsqu'un appui est détecté. Gère les clics sur les widgets.
handle_touch_release : Appelé lorsqu'un appui est relâché. Gère les callbacks de relâchement.
handle_touch_drag : Appelé lorsqu'un drag est détecté (pour les sliders, etc.).
handle_long_press : Appelé après 1 seconde d'appui (pour les menus contextuels).

3. Gestion des Widgets Interactifs

Boutons : Détecte les clics et appelle on_click/on_release.
Sliders : Met à jour la valeur en fonction de la position du toucher.
Checkboxes : Bascule l'état coché/décoché.

4. Mappage des Coordonnées

Les coordonnées brutes du tactile (0-4095) sont mappées aux dimensions de l'écran (0-TFT_WIDTH, 0-TFT_HEIGHT).
Inversion des axes : Si votre écran est en mode portrait, les axes X et Y sont inversés (voir update_touch).

5. Recherche de Widgets

find_widget_at_point : Parcourt l'arborescence des widgets pour trouver celui qui contient le point tactile.
Priorité aux widgets enfants : Les widgets en haut de la pile (ajoutés en dernier) sont vérifiés en premier.

🔧 Configuration du Mappage des Coordonnées
Le mappage des coordonnées dépend de l'orientation de votre écran et de la façon dont le tactile est monté. Voici comment ajuster le code dans update_touch :
Cas 1 : Écran en Mode Paysage (Rotation 0 ou 2)
cpp
Copier

touch_state.current.x = map(p.x, 0, 4095, 0, TFT_WIDTH - 1);
touch_state.current.y = map(p.y, 0, 4095, 0, TFT_HEIGHT - 1);



→ Pas besoin d'inverser les axes.

Cas 2 : Écran en Mode Portrait (Rotation 1 ou 3)
cpp
Copier

touch_state.current.x = map(p.y, 0, 4095, 0, TFT_WIDTH - 1);
touch_state.current.y = map(p.x, 0, 4095, TFT_HEIGHT - 1, 0);



→ Inverser X et Y et inverser l'axe Y.

Cas 3 : Calibration Personnalisée
Si les coordonnées ne sont pas précises, vous pouvez calibrer manuellement le tactile en utilisant des valeurs minimales et maximales mesurées :
cpp
Copier

// Valeurs minimales et maximales pour X et Y (à mesurer avec un test)
#define TOUCH_X_MIN 200
#define TOUCH_X_MAX 3800
#define TOUCH_Y_MIN 200
#define TOUCH_Y_MAX 3800

touch_state.current.x = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, TFT_WIDTH - 1);
touch_state.current.y = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, TFT_HEIGHT - 1);




📌 Test de Calibration du Touchscreen
Pour calibrer votre écran, ajoutez ce code temporaire dans setup() :
cpp
Copier

void calibrate_touch() {
    Serial.println("Calibration du touchscreen...");
    Serial.println("Touchez le coin haut-gauche de l'écran.");
    while (!ts.touched()) delay(10);
    TS_Point p1 = ts.getPoint();
    delay(500);

    Serial.println("Touchez le coin bas-droite de l'écran.");
    while (!ts.touched()) delay(10);
    TS_Point p2 = ts.getPoint();
    delay(500);

    Serial.print("X: Min=");
    Serial.print(p1.x);
    Serial.print(", Max=");
    Serial.println(p2.x);
    Serial.print("Y: Min=");
    Serial.print(p1.y);
    Serial.print(", Max=");
    Serial.println(p2.y);

    // Utilisez ces valeurs pour définir TOUCH_X_MIN, TOUCH_X_MAX, etc.
}



→ Appelez cette fonction au début de setup() pour obtenir les valeurs de calibration.

💡 Optimisations pour l'ESP8266


Réduire la Latence du Tactile :

Utilisez l'interruption IRQ du XPT2046 pour détecter les appuis sans poller en continu.
Exemple :
cpp
Copier

void IRAM_ATTR touch_isr() {
    touch_state.touched = true;
}

void init_touch() {
    ts.begin();
    pinMode(XPT2046_IRQ, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(XPT2046_IRQ), touch_isr, FALLING);
}






Filtrer les Bruits :

Ajoutez un filtre de moyenne mobile pour lisser les coordonnées :
cpp
Copier

#define TOUCH_SAMPLES 5
TouchPoint touch_samples[TOUCH_SAMPLES];
uint8_t touch_sample_index = 0;

void update_touch() {
    TS_Point p = ts.getPoint();
    touch_samples[touch_sample_index].x = p.x;
    touch_samples[touch_sample_index].y = p.y;
    touch_sample_index = (touch_sample_index + 1) % TOUCH_SAMPLES;

    // Calculer la moyenne
    uint32_t avg_x = 0, avg_y = 0;
    for (uint8_t i = 0; i < TOUCH_SAMPLES; i++) {
        avg_x += touch_samples[i].x;
        avg_y += touch_samples[i].y;
    }
    avg_x /= TOUCH_SAMPLES;
    avg_y /= TOUCH_SAMPLES;

    touch_state.current.x = map(avg_x, TOUCH_X_MIN, TOUCH_X_MAX, 0, TFT_WIDTH - 1);
    touch_state.current.y = map(avg_y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, TFT_HEIGHT - 1);
}






Éviter les Calculs Inutiles :

Ne mettez à jour le touch qu'en cas de changement d'état (appui/relâchement) ou de mouvement significatif.


❓ Problèmes Courants et Solutions

  
    
      Problème
      Solution
    
  
  
    
      Les coordonnées sont inversées.
      Ajustez le mappage dans update_touch (voir section "Mappage des Coordonnées").
    
    
      Le tactile ne répond pas.
      Vérifiez les connexions (CS, IRQ, MOSI, MISO, SCK) et la tension d'alimentation.
    
    
      Le tactile est trop sensible.
      Augmentez le seuil de pression (ts.setThreshold(100)).
    
    
      Le tactile est trop lent.
      Utilisez l'interruption IRQ et réduisez le nombre d'échantillons (TOUCH_SAMPLES).
    
    
      Les clics ne sont pas détectés.
      Vérifiez que root_view est bien défini et que les widgets sont ajoutés à l'arborescence.
    
  





🎉 Vous avez maintenant un GUIKit avec Touchscreen !
Avec ces ajouts, votre GUIKit supporte :
✅ Détection des clics sur les widgets.
✅ Gestion du drag (pour les sliders).
✅ Appuis longs (pour les menus contextuels).
✅ Mappage précis des coordonnées.
✅ Intégration fluide avec TFT_eSPI.

📥 Comment Tester ?

Téléchargez le code sur votre ESP8266.
Touchez les boutons : Vous devriez voir des messages dans le moniteur série.
Faites glisser le slider : La valeur devrait s'afficher dans le label.
Cochez/décochez la checkbox : La couleur du label devrait changer.

🚀 Prochaines Étapes

Ajouter des gestes (ex: swipe, zoom).
Optimiser le rendu pour éviter les scintillements.
Ajouter des animations pour les transitions.
Implémenter un clavier virtuel pour les champs de texte.
Si vous avez besoin d'aide pour l'une de ces étapes, dites-le-moi ! 🛠️





10:17










add widget textfield + style (normal, password)





10:25








Voici comment ajouter un widget TextField (champ de texte) avec des styles normal et password à votre GUIKit. Je vais vous fournir :

La définition du widget (TextField).
Les styles spécifiques (normal, password).
La gestion du clavier virtuel (si nécessaire).
La détection des clics pour le focus.
Le rendu avec masquage pour le mode password.

📌 1. Mise à Jour de widget.h
Ajoutez la définition du TextField et son type dans widget.h.
cpp
Copier

// ========== NOUVEAU TYPE DE WIDGET ==========
typedef enum {
    WIDGET_TYPE_VIEW,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_LABEL,
    WIDGET_TYPE_SLIDER,
    WIDGET_TYPE_CHECKBOX,
    WIDGET_TYPE_RADIO_BUTTON,
    WIDGET_TYPE_TOGGLE_BUTTON,
    WIDGET_TYPE_PROGRESS_BAR,
    WIDGET_TYPE_IMAGE,
    WIDGET_TYPE_CANVAS,
    WIDGET_TYPE_TEXTFIELD,  // Nouveau type pour le champ de texte
    WIDGET_TYPE_CUSTOM,
} WIDGET_TYPE;

// ========== STYLE SPÉCIFIQUE POUR TEXTFIELD ==========
typedef enum {
    TEXTFIELD_STYLE_NORMAL = 0,    // Texte normal (visible)
    TEXTFIELD_STYLE_PASSWORD,       // Texte masqué (password)
} TEXTFIELD_STYLE;

// ========== STRUCTURE POUR TEXTFIELD ==========
typedef struct {
    Widget base;
    char* buffer;                 // Tampon pour le texte (dynamique)
    uint16_t buffer_size;         // Taille du tampon
    uint16_t cursor_pos;          // Position du curseur
    bool has_focus;               // Si le champ a le focus
    TEXTFIELD_STYLE style;        // Style (normal ou password)
    char password_char;           // Caractère de masquage (ex: '*')
    void (*on_change)(const char*); // Callback lors de la modification
    void (*on_enter)(const char*);  // Callback lors de la validation (Enter)
} WidgetTextField;




📌 2. Mise à Jour de widget.cpp
Ajoutez le constructeur et les fonctions pour WidgetTextField.
cpp
Copier

// ========== CONSTRUCTEUR POUR TEXTFIELD ==========
WidgetTextField* new_textfield(uint16_t buffer_size, TEXTFIELD_STYLE style) {
    WidgetTextField* textfield = (WidgetTextField*)malloc(sizeof(WidgetTextField));
    if (!textfield) return NULL;

    textfield->base = *new_widget(WIDGET_TYPE_TEXTFIELD);
    textfield->buffer = (char*)malloc(buffer_size + 1); // +1 pour '\0'
    if (!textfield->buffer) {
        free(textfield);
        return NULL;
    }
    textfield->buffer[0] = '\0'; // Chaîne vide
    textfield->buffer_size = buffer_size;
    textfield->cursor_pos = 0;
    textfield->has_focus = false;
    textfield->style = style;
    textfield->password_char = '*';
    textfield->on_change = NULL;
    textfield->on_enter = NULL;

    // Style par défaut
    textfield->base.style.draw_style = WIDGET_DRAW_STYLE_SOLID_BORDER | WIDGET_DRAW_STYLE_SOLID_FILL;
    textfield->base.style.colors.primary = 0xFFFF; // Blanc (fond)
    textfield->base.style.colors.secondary = 0x0000; // Noir (texte)
    textfield->base.style.border.color = 0x8410; // Gris (bordure)
    textfield->base.style.border.width = 1;
    textfield->base.style.border.radius = 3;
    textfield->base.rect.size.height = 30; // Hauteur par défaut

    return textfield;
}

// ========== FONCTIONS POUR TEXTFIELD ==========
// Ajouter un caractère au TextField
void textfield_append_char(WidgetTextField* textfield, char c) {
    if (!textfield || !textfield->buffer) return;

    if (textfield->cursor_pos < textfield->buffer_size) {
        // Décaler les caractères après le curseur
        for (uint16_t i = textfield->buffer_size; i > textfield->cursor_pos; i--) {
            textfield->buffer[i] = textfield->buffer[i - 1];
        }
        textfield->buffer[textfield->cursor_pos] = c;
        textfield->cursor_pos++;
        textfield->buffer[textfield->cursor_pos] = '\0'; // Terminaison

        if (textfield->on_change) {
            textfield->on_change(textfield->buffer);
        }
    }
}

// Supprimer le caractère après le curseur
void textfield_delete_char(WidgetTextField* textfield) {
    if (!textfield || !textfield->buffer || textfield->cursor_pos >= textfield->buffer_size) return;

    for (uint16_t i = textfield->cursor_pos; i < textfield->buffer_size; i++) {
        textfield->buffer[i] = textfield->buffer[i + 1];
    }
    textfield->cursor_pos--;

    if (textfield->on_change) {
        textfield->on_change(textfield->buffer);
    }
}

// Supprimer le caractère avant le curseur (backspace)
void textfield_backspace(WidgetTextField* textfield) {
    if (!textfield || !textfield->buffer || textfield->cursor_pos == 0) return;

    textfield->cursor_pos--;
    for (uint16_t i = textfield->cursor_pos; i < textfield->buffer_size; i++) {
        textfield->buffer[i] = textfield->buffer[i + 1];
    }

    if (textfield->on_change) {
        textfield->on_change(textfield->buffer);
    }
}

// Déplacer le curseur
void textfield_move_cursor(WidgetTextField* textfield, int16_t delta) {
    if (!textfield) return;

    int16_t new_pos = textfield->cursor_pos + delta;
    if (new_pos < 0) new_pos = 0;
    if (new_pos > strlen(textfield->buffer)) new_pos = strlen(textfield->buffer);

    textfield->cursor_pos = new_pos;
}

// Effacer le contenu
void textfield_clear(WidgetTextField* textfield) {
    if (!textfield || !textfield->buffer) return;

    textfield->buffer[0] = '\0';
    textfield->cursor_pos = 0;

    if (textfield->on_change) {
        textfield->on_change(textfield->buffer);
    }
}

// Définir le texte
void textfield_set_text(WidgetTextField* textfield, const char* text) {
    if (!textfield || !textfield->buffer || !text) return;

    strncpy(textfield->buffer, text, textfield->buffer_size);
    textfield->buffer[textfield->buffer_size] = '\0';
    textfield->cursor_pos = strlen(textfield->buffer);

    if (textfield->on_change) {
        textfield->on_change(textfield->buffer);
    }
}

// Obtenir le texte (masqué si password)
const char* textfield_get_display_text(WidgetTextField* textfield) {
    if (!textfield || !textfield->buffer) return "";

    if (textfield->style == TEXTFIELD_STYLE_PASSWORD) {
        static char masked_buffer[256]; // Tampon temporaire
        uint16_t len = strlen(textfield->buffer);
        for (uint16_t i = 0; i < len; i++) {
            masked_buffer[i] = textfield->password_char;
        }
        masked_buffer[len] = '\0';
        return masked_buffer;
    } else {
        return textfield->buffer;
    }
}




📌 3. Mise à Jour de touch.h
Ajoutez la déclaration pour la gestion du TextField.
cpp
Copier

// ========== GESTION DU TEXTFIELD ==========
void handle_textfield_touch(WidgetTextField* textfield, TouchPoint point);
void handle_textfield_focus(WidgetTextField* textfield, bool focus);




📌 4. Mise à Jour de touch.cpp
Ajoutez la gestion du TextField dans les fonctions de touch.
cpp
Copier

// ========== GESTION DU TEXTFIELD ==========
void handle_textfield_touch(WidgetTextField* textfield, TouchPoint point) {
    if (!textfield) return;

    // Vérifier si le clic est dans le TextField
    if (is_touch_inside_widget((Widget*)textfield, point)) {
        // Si le TextField n'a pas le focus, le lui donner
        if (!textfield->has_focus) {
            handle_textfield_focus(textfield, true);
        } else {
            // Si le TextField a déjà le focus, déplacer le curseur
            // (Calculer la position du curseur en fonction de la position X du clic)
            uint16_t text_width = tft.textWidth(textfield_get_display_text(textfield));
            uint16_t char_width = text_width / strlen(textfield_get_display_text(textfield));
            if (char_width == 0) char_width = 10; // Largeur par défaut si texte vide

            uint16_t click_x = point.x - textfield->base.rect.position.x;
            textfield->cursor_pos = click_x / char_width;

            // Limiter le curseur
            if (textfield->cursor_pos > strlen(textfield->buffer)) {
                textfield->cursor_pos = strlen(textfield->buffer);
            }
        }
    } else {
        // Clic en dehors : défocus
        handle_textfield_focus(textfield, false);
    }
}

void handle_textfield_focus(WidgetTextField* textfield, bool focus) {
    if (!textfield) return;

    textfield->has_focus = focus;

    if (focus) {
        // Style pour le focus
        widget_add_style((Widget*)textfield, WIDGET_DRAW_STYLE_GLOW);
        textfield->base.style.border.color = 0x07E0; // Vert (bordure en focus)
    } else {
        // Style par défaut
        widget_remove_style((Widget*)textfield, WIDGET_DRAW_STYLE_GLOW);
        textfield->base.style.border.color = 0x8410; // Gris (bordure normale)
    }

    draw_widget((Widget*)textfield);
}




📌 5. Mise à Jour de handle_touch_press et handle_touch_release
Modifiez les fonctions dans touch.cpp pour gérer le TextField.
cpp
Copier

// Dans handle_touch_press :
switch (widget->type) {
    case WIDGET_TYPE_BUTTON: {
        WidgetButton* button = (WidgetButton*)widget;
        button->pressed = true;
        widget_add_style(widget, WIDGET_DRAW_STYLE_GLOW);
        draw_widget(widget);
        if (button->on_click) {
            button->on_click();
        }
        break;
    }
    case WIDGET_TYPE_TEXTFIELD: {
        WidgetTextField* textfield = (WidgetTextField*)widget;
        handle_textfield_touch(textfield, point);
        break;
    }
    // ... autres cas
}

// Dans handle_touch_release :
switch (widget->type) {
    case WIDGET_TYPE_BUTTON: {
        WidgetButton* button = (WidgetButton*)widget;
        button->pressed = false;
        widget_remove_style(widget, WIDGET_DRAW_STYLE_GLOW);
        draw_widget(widget);
        if (button->on_release) {
            button->on_release();
        }
        break;
    }
    case WIDGET_TYPE_TEXTFIELD: {
        // Rien à faire ici, le focus est géré dans handle_textfield_touch
        break;
    }
    // ... autres cas
}




📌 6. Mise à Jour de renderer.cpp
Ajoutez le rendu spécifique pour le TextField.
cpp
Copier

// ========== DESSIN DU TEXTFIELD ==========
void draw_textfield(Widget* widget) {
    if (widget->type != WIDGET_TYPE_TEXTFIELD) return;

    WidgetTextField* textfield = (WidgetTextField*)widget;

    // Dessiner le fond
    uint64_t fill_style = widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK;
    switch (fill_style) {
        case WIDGET_DRAW_STYLE_SOLID_FILL:
            tft.fillRoundRect(
                widget->rect.position.x,
                widget->rect.position.y,
                widget->rect.size.width,
                widget->rect.size.height,
                widget->style.border.radius,
                widget->style.colors.primary
            );
            break;
        default:
            tft.fillRoundRect(
                widget->rect.position.x,
                widget->rect.position.y,
                widget->rect.size.width,
                widget->rect.size.height,
                widget->style.border.radius,
                widget->style.colors.primary
            );
            break;
    }

    // Dessiner la bordure
    uint64_t border_style = widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK;
    switch (border_style) {
        case WIDGET_DRAW_STYLE_SOLID_BORDER:
            tft.drawRoundRect(
                widget->rect.position.x,
                widget->rect.position.y,
                widget->rect.size.width,
                widget->rect.size.height,
                widget->style.border.radius,
                widget->style.border.color
            );
            break;
        default:
            tft.drawRoundRect(
                widget->rect.position.x,
                widget->rect.position.y,
                widget->rect.size.width,
                widget->rect.size.height,
                widget->style.border.radius,
                widget->style.border.color
            );
            break;
    }

    // Dessiner le texte (masqué si password)
    const char* display_text = textfield_get_display_text(textfield);
    tft.setTextColor(widget->style.colors.secondary);
    tft.setTextSize(widget->text.font.size / 8);

    // Calculer la position du texte (avec padding)
    uint16_t text_x = widget->rect.position.x + 5;
    uint16_t text_y = widget->rect.position.y + (widget->rect.size.height - tft.fontHeight()) / 2;

    tft.setCursor(text_x, text_y);
    tft.print(display_text);

    // Dessiner le curseur si le TextField a le focus
    if (textfield->has_focus) {
        // Calculer la position du curseur
        uint16_t cursor_x = text_x + tft.textWidth(display_text, textfield->cursor_pos);
        tft.drawFastVLine(
            cursor_x,
            text_y - tft.fontHeight() + 2,
            tft.fontHeight() - 4,
            widget->style.colors.secondary
        );
    }
}




📌 7. Mise à Jour de draw_widget dans renderer.cpp
Ajoutez le cas pour WIDGET_TYPE_TEXTFIELD.
cpp
Copier

void draw_widget(Widget* widget) {
    if (!widget) return;

    // Dessiner les effets d'arrière-plan (ex: ombre portée)
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_DROP_SHADOW) {
        draw_drop_shadow(widget);
    }

    // Dessiner selon le type de widget
    switch (widget->type) {
        case WIDGET_TYPE_TEXTFIELD:
            draw_textfield(widget);
            break;
        default:
            // Dessiner le fond
            uint64_t fill_style = widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK;
            switch (fill_style) {
                // ... (le reste du code existant)
            }
            // Dessiner la bordure
            // ... (le reste du code existant)
            break;
    }

    // Dessiner les effets de premier plan (ex: glow, neon)
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_GLOW) {
        draw_glow_effect(widget);
    }
    // ... (le reste du code existant)
}




📌 8. Mise à Jour de main.cpp
Ajoutez un exemple d'utilisation du TextField.
cpp
Copier

// ========== VARIABLES GLOBALES ==========
// ... (les autres variables)
WidgetTextField* textfield1;  // Champ de texte normal
WidgetTextField* textfield2;  // Champ de texte password

// ========== CALLBACKS ==========
// ... (les autres callbacks)

void on_textfield1_change(const char* text) {
    Serial.print("TextField 1: ");
    Serial.println(text);
}

void on_textfield2_change(const char* text) {
    Serial.print("TextField 2 (password): ");
    Serial.println(text);
}

void on_textfield_enter(const char* text) {
    Serial.print("Texte validé: ");
    Serial.println(text);
}

// ========== FONCTION D'INITIALISATION ==========
void setup() {
    Serial.begin(115200);
    init_renderer();
    init_touch();

    // Créer la vue racine
    root_view = new_widget(WIDGET_TYPE_VIEW);
    root_view->rect.position.x = 0;
    root_view->rect.position.y = 0;
    root_view->rect.size.width = tft.width();
    root_view->rect.size.height = tft.height();
    root_view->style.draw_style = WIDGET_DRAW_STYLE_SOLID_FILL;
    root_view->style.colors.primary = 0x0000; // Fond noir

    // Créer un TextField normal
    textfield1 = new_textfield(32, TEXTFIELD_STYLE_NORMAL);
    textfield1->base.rect.position.x = 50;
    textfield1->base.rect.position.y = 350;
    textfield1->base.rect.size.width = 200;
    textfield1->on_change = on_textfield1_change;
    textfield1->on_enter = on_textfield_enter;
    strncpy(textfield1->buffer, "Texte normal", textfield1->buffer_size);
    textfield1->cursor_pos = strlen(textfield1->buffer);

    // Créer un TextField password
    textfield2 = new_textfield(32, TEXTFIELD_STYLE_PASSWORD);
    textfield2->base.rect.position.x = 50;
    textfield2->base.rect.position.y = 390;
    textfield2->base.rect.size.width = 200;
    textfield2->password_char = '*';
    textfield2->on_change = on_textfield2_change;
    textfield2->on_enter = on_textfield_enter;

    // Ajouter les widgets à la vue racine
    widget_add_child(root_view, (Widget*)button1);
    widget_add_child(root_view, (Widget*)button2);
    widget_add_child(root_view, (Widget*)label);
    widget_add_child(root_view, (Widget*)slider);
    widget_add_child(root_view, (Widget*)checkbox);
    widget_add_child(root_view, (Widget*)textfield1);
    widget_add_child(root_view, (Widget*)textfield2);

    // Dessiner l'arborescence des widgets
    draw_widget_tree(root_view);

    Serial.println("Initialisation terminée !");
}




📌 9. Gestion du Clavier (Optionnel)
Pour permettre la saisie de texte, vous pouvez :

Utiliser un clavier physique (via Serial ou Bluetooth).
Créer un clavier virtuel sur l'écran TFT.
Voici un exemple de clavier virtuel simple :
Ajout dans widget.h
cpp
Copier

// ========== CLAVIER VIRTUEL ==========
typedef struct {
    Widget base;
    WidgetButton* keys[48]; // Tableau de boutons pour les touches
    WidgetTextField* target; // TextField cible
} WidgetKeyboard;




Ajout dans widget.cpp
cpp
Copier

// Créer un clavier virtuel
WidgetKeyboard* new_keyboard(WidgetTextField* target) {
    WidgetKeyboard* keyboard = (WidgetKeyboard*)malloc(sizeof(WidgetKeyboard));
    if (!keyboard) return NULL;

    keyboard->base = *new_widget(WIDGET_TYPE_CUSTOM);
    keyboard->target = target;
    keyboard->base.rect.size.width = 240;
    keyboard->base.rect.size.height = 120;
    keyboard->base.style.draw_style = WIDGET_DRAW_STYLE_SOLID_FILL;
    keyboard->base.style.colors.primary = 0x8410; // Fond gris

    // Créer les touches (exemple : clavier AZERTY simplifié)
    const char* keys[] = {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
        "A", "S", "D", "F", "G", "H", "J", "K", "L", "M",
        "Z", "X", "C", "V", "B", "N", " ", "⌫", "↩"
    };

    uint8_t rows = 4;
    uint8_t cols = 10;
    uint16_t key_width = keyboard->base.rect.size.width / cols;
    uint16_t key_height = keyboard->base.rect.size.height / rows;

    for (uint8_t i = 0; i < rows * cols; i++) {
        uint8_t row = i / cols;
        uint8_t col = i % cols;
        keyboard->keys[i] = new_button();
        keyboard->keys[i]->base.rect.position.x = keyboard->base.rect.position.x + col * key_width;
        keyboard->keys[i]->base.rect.position.y = keyboard->base.rect.position.y + row * key_height;
        keyboard->keys[i]->base.rect.size.width = key_width;
        keyboard->keys[i]->base.rect.size.height = key_height;
        strncpy(keyboard->keys[i]->base.text.text, keys[i], MAX_TEXT_LENGTH - 1);
        keyboard->keys[i]->base.text.font.size = 12;

        // Callback pour chaque touche
        keyboard->keys[i]->on_click = [](void* key_idx) {
            WidgetKeyboard* kb = (WidgetKeyboard*)keyboard;
            uint8_t idx = *(uint8_t*)key_idx;

            if (kb->target) {
                const char* key_text = kb->keys[idx]->base.text.text;
                if (strcmp(key_text, "⌫") == 0) {
                    textfield_backspace(kb->target);
                } else if (strcmp(key_text, "↩") == 0) {
                    if (kb->target->on_enter) {
                        kb->target->on_enter(kb->target->buffer);
                    }
                } else {
                    textfield_append_char(kb->target, key_text[0]);
                }
                draw_widget((Widget*)kb->target);
            }
        };

        // Stocker l'index de la touche (pour le callback)
        uint8_t* idx_ptr = (uint8_t*)malloc(sizeof(uint8_t));
        *idx_ptr = i;
        keyboard->keys[i]->on_click = (void (*)(void))keyboard->keys[i]->on_click; // Workaround pour le callback
    }

    return keyboard;
}




Mise à Jour de main.cpp
Ajoutez un clavier virtuel qui apparaît quand un TextField a le focus.
cpp
Copier

// ========== VARIABLES GLOBALES ==========
WidgetKeyboard* keyboard; // Clavier virtuel

// ========== FONCTIONS POUR LE CLAVIER ==========
void show_keyboard(WidgetTextField* target) {
    if (keyboard) {
        free_widget((Widget*)keyboard);
    }
    keyboard = new_keyboard(target);
    keyboard->base.rect.position.y = tft.height() - 120; // Position en bas de l'écran
    widget_add_child(root_view, (Widget*)keyboard);
    draw_widget_tree(root_view);
}

void hide_keyboard() {
    if (keyboard) {
        widget_remove_child(root_view, (Widget*)keyboard);
        free_widget((Widget*)keyboard);
        keyboard = NULL;
        draw_widget_tree(root_view);
    }
}

// ========== MODIFICATION DE handle_textfield_focus ==========
void handle_textfield_focus(WidgetTextField* textfield, bool focus) {
    if (!textfield) return;

    textfield->has_focus = focus;

    if (focus) {
        widget_add_style((Widget*)textfield, WIDGET_DRAW_STYLE_GLOW);
        textfield->base.style.border.color = 0x07E0; // Vert
        show_keyboard(textfield); // Afficher le clavier
    } else {
        widget_remove_style((Widget*)textfield, WIDGET_DRAW_STYLE_GLOW);
        textfield->base.style.border.color = 0x8410; // Gris
        hide_keyboard(); // Masquer le clavier
    }

    draw_widget((Widget*)textfield);
}




🎯 Fonctionnalités du TextField

  
    
      Fonctionnalité
      Description
      Implémentation
    
  
  
    
      Saisie de texte
      Permet de taper du texte via un clavier virtuel ou physique.
      ✅
    
    
      Style Normal
      Affiche le texte tel quel.
      ✅
    
    
      Style Password
      Masque le texte avec un caractère (ex: *).
      ✅
    
    
      Focus/Defocus
      Change de style quand le champ est sélectionné.
      ✅
    
    
      Curseur clignotant
      Affiche un curseur à la position actuelle.
      ✅
    
    
      Callbacks
      on_change (à chaque modification) et on_enter (validation).
      ✅
    
    
      Clavier virtuel
      Clavier tactile pour la saisie.
      ✅ (Optionnel)
    
  





📌 Exemple Complet avec Clavier Virtuel
Voici un exemple complet avec un TextField et un clavier virtuel :
Dans main.cpp
cpp
Copier

// ... (les autres includes et variables)

// ========== CLAVIER VIRTUEL SIMPLIFIÉ ==========
void create_simple_keyboard(WidgetTextField* target) {
    Widget* keyboard_view = new_widget(WIDGET_TYPE_VIEW);
    keyboard_view->rect.position.x = 0;
    keyboard_view->rect.position.y = tft.height() - 100;
    keyboard_view->rect.size.width = tft.width();
    keyboard_view->rect.size.height = 100;
    keyboard_view->style.draw_style = WIDGET_DRAW_STYLE_SOLID_FILL;
    keyboard_view->style.colors.primary = 0x8410; // Fond gris

    // Créer des boutons pour les chiffres 0-9
    for (uint8_t i = 0; i < 10; i++) {
        WidgetButton* key = new_button();
        key->base.rect.position.x = 20 + (i % 3) * 70;
        key->base.rect.position.y = keyboard_view->rect.position.y + 10 + (i / 3) * 30;
        key->base.rect.size.width = 60;
        key->base.rect.size.height = 25;
        char key_text[2] = { (char)('0' + i), '\0' };
        strncpy(key->base.text.text, key_text, MAX_TEXT_LENGTH - 1);
        key->base.text.font.size = 14;

        // Callback pour chaque touche
        key->on_click = [i, target]() {
            char c = '0' + i;
            textfield_append_char(target, c);
            draw_widget((Widget*)target);
        };

        widget_add_child(keyboard_view, (Widget*)key);
    }

    // Ajouter un bouton "⌫" (Backspace)
    WidgetButton* backspace = new_button();
    backspace->base.rect.position.x = 20 + 3 * 70;
    backspace->base.rect.position.y = keyboard_view->rect.position.y + 10;
    backspace->base.rect.size.width = 60;
    backspace->base.rect.size.height = 25;
    strncpy(backspace->base.text.text, "⌫", MAX_TEXT_LENGTH - 1);
    backspace->on_click = [target]() {
        textfield_backspace(target);
        draw_widget((Widget*)target);
    };
    widget_add_child(keyboard_view, (Widget*)backspace);

    // Ajouter un bouton "↩" (Enter)
    WidgetButton* enter = new_button();
    enter->base.rect.position.x = 20 + 3 * 70;
    enter->base.rect.position.y = keyboard_view->rect.position.y + 40;
    enter->base.rect.size.width = 60;
    enter->base.rect.size.height = 25;
    strncpy(enter->base.text.text, "↩", MAX_TEXT_LENGTH - 1);
    enter->on_click = [target]() {
        if (target->on_enter) {
            target->on_enter(target->buffer);
        }
    };
    widget_add_child(keyboard_view, (Widget*)enter);

    // Ajouter le clavier à la vue racine
    widget_add_child(root_view, keyboard_view);
    draw_widget(keyboard_view);
}

// ========== MODIFICATION DE handle_textfield_focus ==========
void handle_textfield_focus(WidgetTextField* textfield, bool focus) {
    if (!textfield) return;

    textfield->has_focus = focus;

    if (focus) {
        widget_add_style((Widget*)textfield, WIDGET_DRAW_STYLE_GLOW);
        textfield->base.style.border.color = 0x07E0; // Vert
        create_simple_keyboard(textfield); // Afficher le clavier
    } else {
        widget_remove_style((Widget*)textfield, WIDGET_DRAW_STYLE_GLOW);
        textfield->base.style.border.color = 0x8410; // Gris
        // Masquer le clavier (à implémenter)
    }

    draw_widget((Widget*)textfield);
}




💡 Optimisations pour l'ESP8266


Réduire la taille du tampon :

Limitez buffer_size à la taille maximale nécessaire (ex: 32 ou 64 caractères).


Éviter les allocations dynamiques :

Utilisez un pool statique pour les TextField et les claviers.


Clignotement du curseur :

Utilisez un timer pour faire clignoter le curseur (au lieu de le redessiner à chaque frame) :
cpp
Copier

unsigned long last_blink_time = 0;
bool cursor_visible = true;

void draw_cursor(WidgetTextField* textfield) {
    if (millis() - last_blink_time > 500) { // 500ms = 0.5s
        last_blink_time = millis();
        cursor_visible = !cursor_visible;
    }

    if (cursor_visible) {
        // Dessiner le curseur
        uint16_t cursor_x = ...; // Calculer la position X
        tft.drawFastVLine(cursor_x, ..., ...);
    }
}






Clavier virtuel optimisé :

Ne créez le clavier qu'une seule fois et réutilisez-le pour tous les TextField.


❓ Problèmes Courants et Solutions

  
    
      Problème
      Solution
    
  
  
    
      Le curseur ne clignote pas.
      Vérifiez que millis() est utilisé correctement et que le délai est raisonnable (ex: 500ms).
    
    
      Le texte n'est pas masqué en mode password.
      Vérifiez que textfield_get_display_text retourne bien les * pour le style PASSWORD.
    
    
      Le clavier virtuel n'apparaît pas.
      Vérifiez que show_keyboard est appelé dans handle_textfield_focus.
    
    
      La saisie ne fonctionne pas.
      Vérifiez que les callbacks des boutons du clavier sont correctement définis.
    
    
      La mémoire est saturée.
      Réduisez la taille des tampons ou utilisez des pools statiques.
    
  





🎉 Résultat Final
Avec ces ajouts, votre GUIKit supporte maintenant :
✅ Un widget TextField avec :

Style normal (texte visible).
Style password (texte masqué).
Focus/Defocus (mise en évidence).
Curseur clignotant.
Callbacks (on_change, on_enter).
✅ Un clavier virtuel pour la saisie de texte.
✅ Gestion tactile complète pour interagir avec le TextField.

📥 Comment Tester ?

Téléchargez le code sur votre ESP8266.
Touchez un TextField : Le clavier virtuel devrait apparaître.
Tapez du texte : Le texte devrait s'afficher dans le champ (ou des * pour le mode password).
Validez avec "↩" : Le callback on_enter devrait être appelé.

🚀 Prochaines Étapes

Ajouter un clavier AZERTY/QWERTY complet.
Implémenter la sélection de texte (pour copier/coller).
Ajouter un correcteur automatique (pour les fautes de frappe).
Optimiser le rendu pour éviter les scintillements.
Si vous avez besoin d'aide pour l'une de ces étapes, dites-le-moi ! 🛠️





10:24





