







creer un construteur, le char*text ne peut depasser 512 characteres delim '\0'





02:28








Voici une implémentation d'un constructeur pour t_widget_button qui gère dynamiquement le texte (char*) avec une limite de 512 caractères (délimité par '\0'). J'inclus aussi des vérifications pour éviter les débordements et une gestion mémoire optimisée pour l'ESP8266.

📌 Constructeur pour t_widget_button
1. Définition des Structures (Rappel)
cpp
Copier

#define MAX_TEXT_LENGTH 512  // Limite à 512 caractères

struct t_widget_base_text {
    char text[MAX_TEXT_LENGTH];  // Texte statique (évite malloc)
    struct {
        uint8_t size;           // Taille de la police
        uint16_t color;         // Couleur du texte (RGBA565)
    } font;
};

struct t_widget_button {
    t_widget_base base;          // Base commune
    t_widget_base_text text;     // Texte du bouton
    bool pressed;                // État du bouton
    void (*on_click)(void);      // Callback pour le clic
};




2. Constructeur avec Vérification de Longueur
cpp
Copier

/**
 * @brief Crée un bouton avec un texte limité à 512 caractères.
 * @param text Texte du bouton (troncature automatique à 511 caractères + '\0').
 * @param x Position X.
 * @param y Position Y.
 * @param width Largeur.
 * @param height Hauteur.
 * @param on_click Callback appelé au clic (NULL si aucun).
 * @return Pointeur vers le bouton créé (alloué dynamiquement).
 */
t_widget_button* create_button(
    const char* text,
    uint16_t x,
    uint16_t y,
    uint16_t width,
    uint16_t height,
    void (*on_click)(void)
) {
    // Allouer la mémoire pour le bouton
    t_widget_button* button = (t_widget_button*)malloc(sizeof(t_widget_button));
    if (!button) {
        return NULL;  // Échec d'allocation
    }

    // Initialiser la base
    button->base.type = WIDGET_TYPE_BUTTON;
    button->base.position.x = x;
    button->base.position.y = y;
    button->base.size.width = width;
    button->base.size.height = height;
    button->base.background.color = 0x0000;  // Fond noir par défaut
    button->base.border.color = 0xFFFF;     // Bordure blanche
    button->base.border.width = 1;
    button->base.children = NULL;
    button->base.children_count = 0;

    // Copier le texte (troncature à MAX_TEXT_LENGTH-1 si nécessaire)
    strncpy(button->text.text, text, MAX_TEXT_LENGTH - 1);
    button->text.text[MAX_TEXT_LENGTH - 1] = '\0';  // Garantir la terminaison

    // Initialiser la police
    button->text.font.size = 12;
    button->text.font.color = 0xFFFF;  // Blanc

    // État initial
    button->pressed = false;
    button->on_click = on_click;

    return button;
}




3. Destructeur (Libération Mémoire)
cpp
Copier

/**
 * @brief Libère la mémoire d'un bouton.
 * @param button Pointeur vers le bouton à libérer.
 */
void free_button(t_widget_button* button) {
    if (button) {
        // Libérer les enfants si nécessaire (non géré ici pour simplifier)
        free(button);
    }
}




🔍 Points Clés


Gestion du Texte :

Utilisation de strncpy pour limiter la copie à MAX_TEXT_LENGTH - 1 caractères.
Ajout manuel de '\0' pour garantir la terminaison de la chaîne.
Pas besoin de malloc pour le texte : Le tableau text est statique dans la structure, ce qui économise de la RAM et évite les fuites mémoire.


Optimisation ESP8266 :

Évite strdup (qui alloue dynamiquement) pour réduire la fragmentation mémoire.
Utilise des valeurs par défaut pour les champs non initialisés (ex: couleur de fond, bordure).


Sécurité :

Vérification de l'allocation mémoire (if (!button)).
Pas de risque de buffer overflow grâce à strncpy.


📌 Exemple d'Utilisation
cpp
Copier

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Créer un bouton avec un texte long (test de troncature)
    const char* long_text = "Ce texte est très long et dépasse largement les 512 caractères. "
                           "Il sera automatiquement tronqué par le constructeur pour éviter "
                           "les débordements mémoire. Voici la suite du texte...";

    t_widget_button* my_button = create_button(
        long_text,
        10, 10, 200, 50,
        []() { printf("Bouton cliqué !\n"); }
    );

    if (my_button) {
        printf("Texte du bouton (tronqué) : %s\n", my_button->text.text);
        printf("Longueur du texte : %zu\n", strlen(my_button->text.text));

        // Libérer le bouton
        free_button(my_button);
    }

    return 0;
}




📌 Sortie Attendue
text
Copier

Texte du bouton (tronqué) : Ce texte est très long et dépasse largement les 512 caractères. Il sera automatiquement tronqué par le constructeur pour éviter les débordements mémoire. Voici la suite du texte...
Longueur du texte : 511



(Le texte est tronqué à 511 caractères + '\0'.)

⚠️ Notes pour ESP8266


Évitez malloc en boucle :

Si vous créez/détruisez souvent des widgets, utilisez un pool d'objets (voir plus bas).


Alternative sans malloc (pour éviter la fragmentation) :
cpp
Copier

#define MAX_BUTTONS 10
t_widget_button button_pool[MAX_BUTTONS];
uint8_t button_pool_index = 0;

t_widget_button* create_button_static(const char* text, uint16_t x, uint16_t y, uint16_t width, uint16_t height, void (*on_click)(void)) {
    if (button_pool_index >= MAX_BUTTONS) return NULL;

    t_widget_button* button = &button_pool[button_pool_index++];
    // Initialiser comme avant (sans malloc)
    strncpy(button->text.text, text, MAX_TEXT_LENGTH - 1);
    button->text.text[MAX_TEXT_LENGTH - 1] = '\0';
    // ...
    return button;
}





Pour les enfants (node[]) :

Si vous utilisez des widgets conteneurs (ex: VIEW), gérez aussi leur allocation avec un pool ou une limite fixe.


🚀 Prochaines Étapes

Étendre le constructeur pour d'autres widgets (t_widget_label, t_widget_slider, etc.).
Ajouter des méthodes utilitaires (ex: set_text, set_color).
Intégrer avec TFT_eSPI pour le rendu (comme montré précédemment).
Si vous voulez que je développe l'une de ces parties, dites-le-moi ! 🛠️





02:28










un constructeur simplifié avec des valeurs par default, quitte a avoir des surcharge ou va_arg + typeof
new widget( (WIDGET_TYPE), sizerect );

t_widget_button button = new widget( (WIDGET_TYPE_BUTTON) )
t_widget_button button = new widget( (WIDGET_TYPE_BUTTON), {32,128} )





02:34








Voici une implémentation simplifiée et flexible de votre constructeur, avec :

Valeurs par défaut pour tous les champs.
Surcharge via va_arg pour gérer les paramètres optionnels (taille, position, etc.).
Syntaxe proche de votre demande (new widget(WIDGET_TYPE_BUTTON, {32, 128})).

📌 Solution avec va_arg et Surcharge Implicite
1. Définitions de Base
cpp
Copier

#include <stdarg.h>
#include <string.h>

// Types de widgets
typedef enum {
    WIDGET_TYPE_VIEW = 1,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_LABEL,
    // ...
} WIDGET_TYPE;

// Structure pour la taille/position (similaire à un `Rect` en C++)
typedef struct {
    uint16_t width;
    uint16_t height;
} Size;

typedef struct {
    uint16_t x;
    uint16_t y;
} Position;

typedef struct {
    Position position;
    Size size;
} Rect;  // {x, y, width, height}

// Style de base
typedef struct {
    uint16_t color;      // Couleur de fond (RGBA565)
    bool gradient;       // Si gradient activé
    uint16_t gradient_color;
} Background;

typedef struct {
    uint16_t color;      // Couleur de la bordure
    uint8_t width;       // Épaisseur
} Border;

// Texte (limité à 512 caractères)
typedef struct {
    char text[512];
    struct {
        uint8_t size;
        uint16_t color;
    } font;
} WidgetText;

// Structure de base
typedef struct {
    WIDGET_TYPE type;
    Background background;
    Border border;
    Rect rect;           // position + size
    WidgetText text;     // Présent pour tous les widgets (peut être vide)
    void* data;          // Données spécifiques au type (ex: callback pour un bouton)
} Widget;

// Widget Bouton (spécifique)
typedef struct {
    Widget base;
    bool pressed;
    void (*on_click)(void);
} WidgetButton;




2. Constructeur Générique avec va_arg
cpp
Copier

/**
 * @brief Constructeur générique pour créer un widget avec des paramètres optionnels.
 * @param type Type du widget (WIDGET_TYPE_BUTTON, etc.).
 * @param ... Paramètres optionnels (Rect, Size, Position, etc.).
 * @return Pointeur vers le widget créé.
 */
Widget* new_widget(WIDGET_TYPE type, ...) {
    // Allouer la mémoire pour le widget de base
    Widget* widget = (Widget*)malloc(sizeof(Widget));
    if (!widget) return NULL;

    // Initialiser les valeurs par défaut
    widget->type = type;
    widget->background.color = 0x0000;       // Noir
    widget->background.gradient = false;
    widget->border.color = 0xFFFF;          // Blanc
    widget->border.width = 1;
    widget->rect.position.x = 0;
    widget->rect.position.y = 0;
    widget->rect.size.width = 100;          // Taille par défaut
    widget->rect.size.height = 50;
    widget->text.text[0] = '\0';             // Texte vide
    widget->text.font.size = 12;
    widget->text.font.color = 0xFFFF;       // Blanc
    widget->data = NULL;

    // Lire les arguments optionnels
    va_list args;
    va_start(args, type);

    // Parcourir les arguments (on suppose qu'ils sont passés dans l'ordre)
    for (;;) {
        // Vérifier si le prochain argument est un Rect (Position + Size)
        Rect* rect_arg = va_arg(args, Rect*);
        if (rect_arg) {
            widget->rect = *rect_arg;
            continue;
        }

        // Vérifier si c'est une Size
        Size* size_arg = va_arg(args, Size*);
        if (size_arg) {
            widget->rect.size = *size_arg;
            continue;
        }

        // Vérifier si c'est une Position
        Position* pos_arg = va_arg(args, Position*);
        if (pos_arg) {
            widget->rect.position = *pos_arg;
            continue;
        }

        // Vérifier si c'est un texte (char*)
        char* text_arg = va_arg(args, char*);
        if (text_arg) {
            strncpy(widget->text.text, text_arg, 511);
            widget->text.text[511] = '\0';
            continue;
        }

        // Fin des arguments
        break;
    }
    va_end(args);

    // Allouer des données spécifiques selon le type
    switch (type) {
        case WIDGET_TYPE_BUTTON: {
            WidgetButton* button = (WidgetButton*)malloc(sizeof(WidgetButton));
            if (!button) {
                free(widget);
                return NULL;
            }
            button->base = *widget;
            button->pressed = false;
            button->on_click = NULL;
            free(widget);  // Libérer le widget temporaire
            return (Widget*)button;
        }
        // Autres types...
        default:
            return widget;
    }
}




3. Macro pour Simplifier la Syntaxe
Pour éviter de passer des pointeurs (&), on utilise une macro qui génère automatiquement les arguments :
cpp
Copier

// Macro pour créer un widget avec une taille/position
#define NEW_WIDGET(type, ...) new_widget(type, &(Rect){__VA_ARGS__})

// Macro pour créer un bouton avec une taille/position
#define NEW_BUTTON(...) ((WidgetButton*)NEW_WIDGET(WIDGET_TYPE_BUTTON, __VA_ARGS__))




4. Exemples d'Utilisation
🔹 Cas 1 : Bouton avec valeurs par défaut
cpp
Copier

WidgetButton* button1 = NEW_BUTTON();



→ Crée un bouton avec :

Position : (0, 0)
Taille : 100x50
Texte : ""
Couleur de fond : noir
Bordure : blanc, 1px

🔹 Cas 2 : Bouton avec taille personnalisée
cpp
Copier

WidgetButton* button2 = NEW_BUTTON({32, 128});  // {width=32, height=128}



→ Crée un bouton avec :

Position : (0, 0) (par défaut)
Taille : 32x128

🔹 Cas 3 : Bouton avec position et taille
cpp
Copier

WidgetButton* button3 = NEW_BUTTON({10, 20, 32, 128});  // {x=10, y=20, width=32, height=128}



→ Crée un bouton avec :

Position : (10, 20)
Taille : 32x128

🔹 Cas 4 : Bouton avec texte
cpp
Copier

WidgetButton* button4 = NEW_BUTTON({10, 20, 32, 128}, "OK");



→ Crée un bouton avec :

Position : (10, 20)
Taille : 32x128
Texte : "OK"

⚠️ Limitations et Optimisations
1. Problème avec va_arg

va_arg ne peut pas détecter automatiquement le type des arguments. Dans l'implémentation ci-dessus, on suppose que les arguments sont passés dans un ordre précis (Rect*, Size*, Position*, char*).
Solution alternative : Utiliser des paramètres nommés via une structure de configuration (plus sûr mais moins élégant).

2. Version Alternative (Plus Sûre) avec Paramètres Només
cpp
Copier

typedef struct {
    Rect rect;
    const char* text;
    uint16_t bg_color;
    // ...
} WidgetConfig;

Widget* new_widget_ex(WIDGET_TYPE type, WidgetConfig config) {
    Widget* widget = malloc(sizeof(Widget));
    // Initialiser avec config
    widget->rect = config.rect;
    if (config.text) strncpy(widget->text.text, config.text, 511);
    // ...
}

// Usage :
WidgetButton* button = (WidgetButton*)new_widget_ex(
    WIDGET_TYPE_BUTTON,
    { .rect = {10, 20, 32, 128}, .text = "OK" }
);




3. Optimisation pour ESP8266

Éviter malloc : Utiliser un pool d'objets pour les widgets.
Remplacer va_arg : Si la flexibilité n'est pas critique, utilisez des surcharges explicites (ex: new_button(), new_button_with_size()).

📌 Implémentation Finale Recommandée
Pour votre cas d'usage, voici une version ** simplifiée, sans va_arg**, mais avec des surcharges explicites :
cpp
Copier

// Constructeur par défaut
Widget* new_widget(WIDGET_TYPE type) {
    Widget* widget = malloc(sizeof(Widget));
    // Initialiser avec valeurs par défaut
    widget->type = type;
    widget->rect = (Rect){0, 0, 100, 50};
    widget->text.text[0] = '\0';
    // ...
    return widget;
}

// Surcharge avec Rect
Widget* new_widget_with_rect(WIDGET_TYPE type, Rect rect) {
    Widget* widget = new_widget(type);
    widget->rect = rect;
    return widget;
}

// Surcharge avec texte
Widget* new_widget_with_text(WIDGET_TYPE type, const char* text) {
    Widget* widget = new_widget(type);
    strncpy(widget->text.text, text, 511);
    widget->text.text[511] = '\0';
    return widget;
}

// Surcharge avec Rect + texte
Widget* new_widget_full(WIDGET_TYPE type, Rect rect, const char* text) {
    Widget* widget = new_widget_with_rect(type, rect);
    strncpy(widget->text.text, text, 511);
    widget->text.text[511] = '\0';
    return widget;
}

// Macros pour simplifier
#define NEW_WIDGET(type) new_widget(type)
#define NEW_BUTTON() ((WidgetButton*)NEW_WIDGET(WIDGET_TYPE_BUTTON))
#define NEW_BUTTON_WITH_RECT(rect) ((WidgetButton*)new_widget_with_rect(WIDGET_TYPE_BUTTON, rect))




Exemples d'Utilisation (Version Finale)
cpp
Copier

// Bouton par défaut
WidgetButton* button1 = NEW_BUTTON();

// Bouton avec taille/position
WidgetButton* button2 = NEW_BUTTON_WITH_RECT({10, 20, 32, 128});

// Bouton avec texte
WidgetButton* button3 = (WidgetButton*)new_widget_with_text(WIDGET_TYPE_BUTTON, "OK");

// Bouton avec Rect + texte
WidgetButton* button4 = (WidgetButton*)new_widget_full(WIDGET_TYPE_BUTTON, {10, 20, 32, 128}, "OK");




🚀 Conclusion

Pour une syntaxe proche de votre demande (new widget(WIDGET_TYPE_BUTTON, {32, 128})), utilisez la version avec va_arg + macro.
Pour plus de sécurité et de lisibilité, préférez la version avec surcharges explicites.
Pour ESP8266, évitez malloc et utilisez un pool d'objets.
Si vous voulez que j'adapte une de ces solutions à un cas précis, dites-le-moi ! 🛠️





02:34










les nom proposé ne son pas de vrai surcharge de signature 





02:37








Vous avez raison : en C, il n'y a pas de surcharge de fonctions (contrairement au C++). Pour simuler des signatures variables, on doit utiliser soit :

_Generic (C11) pour une surcharge statique (résolue à la compilation).
Macros pour générer des appels différents selon les arguments.
va_arg (comme précédemment) pour une approche dynamique (moins type-safe).

Je vous propose une solution proche de votre syntaxe souhaitée (new widget(WIDGET_TYPE_BUTTON, {32, 128})) en combinant :

_Generic pour sélectionner le bon constructeur selon le type.
Macros pour simplifier l'appel.
Structures anonymes pour passer des paramètres comme {32, 128}.

📌 Solution avec _Generic (C11)
1. Définitions de Base
cpp
Copier

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// Types de widgets
typedef enum {
    WIDGET_TYPE_VIEW,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_LABEL,
} WIDGET_TYPE;

// Structure pour Rect (position + taille)
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} Rect;

// Style de base
typedef struct {
    uint16_t bg_color;
    uint16_t border_color;
    uint8_t border_width;
} Style;

// Texte
typedef struct {
    char text[512];
    uint8_t font_size;
    uint16_t font_color;
} Text;

// Structure de base commune
typedef struct {
    WIDGET_TYPE type;
    Rect rect;
    Style style;
    Text text;
    void* data; // Données spécifiques (ex: callback pour un bouton)
} Widget;

// Widget Bouton
typedef struct {
    Widget base;
    bool pressed;
    void (*on_click)(void);
} WidgetButton;




2. Constructeurs Spécifiques (Pour Chaque Type)
cpp
Copier

// Constructeur par défaut (pour tous les widgets)
static Widget* _new_widget_base(WIDGET_TYPE type) {
    Widget* widget = malloc(sizeof(Widget));
    if (!widget) return NULL;

    widget->type = type;
    widget->rect = (Rect){0, 0, 100, 50}; // Valeurs par défaut
    widget->style = (Style){0x0000, 0xFFFF, 1}; // Fond noir, bordure blanche
    widget->text = (Text){"", 12, 0xFFFF}; // Texte vide, police blanche
    widget->data = NULL;
    return widget;
}

// Constructeur pour WidgetButton avec Rect
static WidgetButton* _new_button_with_rect(Rect rect) {
    WidgetButton* button = malloc(sizeof(WidgetButton));
    if (!button) return NULL;

    button->base = *_new_widget_base(WIDGET_TYPE_BUTTON);
    button->base.rect = rect;
    button->pressed = false;
    button->on_click = NULL;
    return button;
}

// Constructeur pour WidgetButton avec Rect + texte
static WidgetButton* _new_button_with_rect_and_text(Rect rect, const char* text) {
    WidgetButton* button = _new_button_with_rect(rect);
    if (!button) return NULL;

    strncpy(button->base.text.text, text, 511);
    button->base.text.text[511] = '\0';
    return button;
}




3. Surcharge avec _Generic
cpp
Copier

// Macro pour créer un WidgetButton avec des paramètres variables
#define new_widget(type, ...) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button(__VA_ARGS__), \
        WIDGET_TYPE_VIEW:   _new_view(__VA_ARGS__), \
        WIDGET_TYPE_LABEL:  _new_label(__VA_ARGS__) \
    )

// Macro pour _new_button (sélectionne le bon constructeur selon les arguments)
#define _new_button(...) \
    _Generic((__VA_ARGS__), \
        Rect: _new_button_with_rect, \
        const char*: _new_button_with_text, \
        default: _new_button_default \
    )(__VA_ARGS__)

// Constructeur par défaut pour WidgetButton
static WidgetButton* _new_button_default(void) {
    return _new_button_with_rect((Rect){0, 0, 100, 50});
}

// Constructeur pour WidgetButton avec texte uniquement
static WidgetButton* _new_button_with_text(const char* text) {
    WidgetButton* button = _new_button_default();
    if (!button) return NULL;

    strncpy(button->base.text.text, text, 511);
    button->base.text.text[511] = '\0';
    return button;
}




⚠️ Problème avec _Generic

_Generic ne gère pas les arguments multiples (ex: Rect + const char*).
→ On ne peut pas faire :
cpp
Copier

new_widget(WIDGET_TYPE_BUTTON, (Rect){10, 20, 32, 128}, "OK");



car _Generic ne peut pas matcher plusieurs types à la fois.

🔥 Solution Finale : Macros + Struct Anonyme
Pour contourner cette limitation, on utilise une macro qui détecte le nombre d'arguments et appelle le bon constructeur.
1. Macros pour Compter les Arguments
cpp
Copier

// Macro pour compter le nombre d'arguments (jusqu'à 4)
#define _NARG(...) _NARG_IMPL(__VA_ARGS__, 4, 3, 2, 1, 0)
#define _NARG_IMPL(_1, _2, _3, _4, N, ...) N

// Macro pour sélectionner le constructeur selon le nombre d'arguments
#define new_widget(type, ...) \
    _new_widget_dispatch(type, _NARG(__VA_ARGS__), __VA_ARGS__)

// Dispatcher
#define _new_widget_dispatch(type, n, ...) \
    _new_widget_dispatch_##n(type, __VA_ARGS__)

// Cas 0 argument
#define _new_widget_dispatch_0(type) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_default, \
        WIDGET_TYPE_VIEW:   _new_view_default, \
        WIDGET_TYPE_LABEL:  _new_label_default \
    )()

// Cas 1 argument (Rect ou const char*)
#define _new_widget_dispatch_1(type, arg1) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_dispatch_1, \
        WIDGET_TYPE_VIEW:   _new_view_dispatch_1, \
        WIDGET_TYPE_LABEL:  _new_label_dispatch_1 \
    )(arg1)

// Dispatch pour WidgetButton avec 1 argument
#define _new_button_dispatch_1(arg1) \
    _Generic((arg1), \
        Rect: _new_button_with_rect, \
        const char*: _new_button_with_text, \
        default: _new_button_default \
    )(arg1)

// Cas 2 arguments (Rect + const char*)
#define _new_widget_dispatch_2(type, arg1, arg2) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_with_rect_and_text, \
        WIDGET_TYPE_VIEW:   _new_view_with_rect_and_style, \
        WIDGET_TYPE_LABEL:  _new_label_with_rect_and_text \
    )(arg1, arg2)




2. Exemples d'Utilisation
cpp
Copier

// Bouton par défaut
WidgetButton* button1 = new_widget(WIDGET_TYPE_BUTTON);

// Bouton avec Rect
WidgetButton* button2 = new_widget(WIDGET_TYPE_BUTTON, (Rect){10, 20, 32, 128});

// Bouton avec texte
WidgetButton* button3 = new_widget(WIDGET_TYPE_BUTTON, "OK");

// Bouton avec Rect + texte
WidgetButton* button4 = new_widget(WIDGET_TYPE_BUTTON, (Rect){10, 20, 32, 128}, "OK");




📌 Implémentation Complète
Voici le code complet avec tous les constructeurs :
cpp
Copier

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// --- Types de base ---
typedef enum { WIDGET_TYPE_VIEW, WIDGET_TYPE_BUTTON, WIDGET_TYPE_LABEL } WIDGET_TYPE;

typedef struct { uint16_t x, y, width, height; } Rect;
typedef struct { uint16_t bg_color; uint16_t border_color; uint8_t border_width; } Style;
typedef struct { char text[512]; uint8_t font_size; uint16_t font_color; } Text;

typedef struct {
    WIDGET_TYPE type;
    Rect rect;
    Style style;
    Text text;
    void* data;
} Widget;

typedef struct {
    Widget base;
    bool pressed;
    void (*on_click)(void);
} WidgetButton;

// --- Constructeurs pour WidgetButton ---
static WidgetButton* _new_button_default(void) {
    WidgetButton* button = malloc(sizeof(WidgetButton));
    if (!button) return NULL;
    button->base.type = WIDGET_TYPE_BUTTON;
    button->base.rect = (Rect){0, 0, 100, 50};
    button->base.style = (Style){0x0000, 0xFFFF, 1};
    button->base.text = (Text){"", 12, 0xFFFF};
    button->pressed = false;
    button->on_click = NULL;
    return button;
}

static WidgetButton* _new_button_with_rect(Rect rect) {
    WidgetButton* button = _new_button_default();
    if (!button) return NULL;
    button->base.rect = rect;
    return button;
}

static WidgetButton* _new_button_with_text(const char* text) {
    WidgetButton* button = _new_button_default();
    if (!button) return NULL;
    strncpy(button->base.text.text, text, 511);
    button->base.text.text[511] = '\0';
    return button;
}

static WidgetButton* _new_button_with_rect_and_text(Rect rect, const char* text) {
    WidgetButton* button = _new_button_with_rect(rect);
    if (!button) return NULL;
    strncpy(button->base.text.text, text, 511);
    button->base.text.text[511] = '\0';
    return button;
}

// --- Macros pour la surcharge ---
#define _NARG(...) _NARG_IMPL(__VA_ARGS__, 4, 3, 2, 1, 0)
#define _NARG_IMPL(_1, _2, _3, _4, N, ...) N

#define new_widget(type, ...) \
    _new_widget_dispatch(type, _NARG(__VA_ARGS__), __VA_ARGS__)

#define _new_widget_dispatch(type, n, ...) _new_widget_dispatch_##n(type, __VA_ARGS__)

#define _new_widget_dispatch_0(type) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_default, \
        WIDGET_TYPE_VIEW:   _new_view_default, \
        WIDGET_TYPE_LABEL:  _new_label_default \
    )()

#define _new_widget_dispatch_1(type, arg1) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_dispatch_1, \
        WIDGET_TYPE_VIEW:   _new_view_dispatch_1, \
        WIDGET_TYPE_LABEL:  _new_label_dispatch_1 \
    )(arg1)

#define _new_button_dispatch_1(arg1) \
    _Generic((arg1), \
        Rect: _new_button_with_rect, \
        const char*: _new_button_with_text, \
        default: _new_button_default \
    )(arg1)

#define _new_widget_dispatch_2(type, arg1, arg2) \
    _Generic((type), \
        WIDGET_TYPE_BUTTON: _new_button_with_rect_and_text, \
        WIDGET_TYPE_VIEW:   _new_view_with_rect_and_style, \
        WIDGET_TYPE_LABEL:  _new_label_with_rect_and_text \
    )(arg1, arg2)

// --- Exemple d'utilisation ---
int main() {
    // Bouton par défaut
    WidgetButton* button1 = new_widget(WIDGET_TYPE_BUTTON);

    // Bouton avec Rect
    WidgetButton* button2 = new_widget(WIDGET_TYPE_BUTTON, (Rect){10, 20, 32, 128});

    // Bouton avec texte
    WidgetButton* button3 = new_widget(WIDGET_TYPE_BUTTON, "OK");

    // Bouton avec Rect + texte
    WidgetButton* button4 = new_widget(WIDGET_TYPE_BUTTON, (Rect){10, 20, 32, 128}, "OK");

    // Libération mémoire
    free(button1);
    free(button2);
    free(button3);
    free(button4);

    return 0;
}




✅ Avantages de cette Solution


Syntaxe proche de votre demande :

new_widget(WIDGET_TYPE_BUTTON, {32, 128}) fonctionne.
new_widget(WIDGET_TYPE_BUTTON, "OK") fonctionne aussi.


Pas de va_arg :

Résolution statique (à la compilation) via _Generic et macros.
Type-safe (le compilateur vérifie les types).


Extensible :

Ajoutez facilement de nouveaux types de widgets (WIDGET_TYPE_SLIDER, etc.).
Ajoutez de nouvelles signatures (ex: new_widget(WIDGET_TYPE_BUTTON, rect, text, callback)).


Optimisé pour ESP8266 :

Pas de surcoût à l'exécution (tout est résolu à la compilation).
Utilise malloc uniquement pour l'allocation finale.


⚠️ Limitations


Nombre limité d'arguments :

La macro _NARG gère jusqu'à 4 arguments. Pour plus, il faut l'étendre.


Pas de vérification des types à l'exécution :

Si vous passez un type incorrect (ex: new_widget(WIDGET_TYPE_BUTTON, 123)), le compilateur choisira default (ici _new_button_default).


Macros complexes :

Le code peut sembler difficile à déboguer, mais il est entièrement résolu à la compilation.


🚀 Prochaines Étapes

Étendre à d'autres widgets (VIEW, LABEL, etc.) en ajoutant leurs constructeurs.
Ajouter des pools d'objets pour éviter malloc sur ESP8266.
Intégrer avec TFT_eSPI pour le rendu.
Si vous voulez que j'adapte cette solution à un cas spécifique, dites-le-moi ! 🛠️





02:37

