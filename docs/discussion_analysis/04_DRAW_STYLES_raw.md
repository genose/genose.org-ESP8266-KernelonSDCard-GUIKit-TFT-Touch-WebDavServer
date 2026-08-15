




09:57








Voici une définition complète et optimisée de l'enum WIDGET_DRAW_STYLE pour gérer les styles de dessin de vos widgets dans GUIKit. J'ai inclus des options pour les bords, les dégradés, et les chemins personnalisés (Bezier), avec des valeurs explicites pour une meilleure maintenabilité.

📌 Enum WIDGET_DRAW_STYLE
cpp
Copier

/**
 * @brief Styles de dessin pour les widgets (bords, fond, etc.).
 * @note Utilisé pour définir comment un widget doit être dessiné sur l'écran TFT.
 */
typedef enum {
    // ===== Styles de bordure =====
    WIDGET_DRAW_STYLE_NORMAL = 0,      /**< Bordure carrée standard (défaut). */
    WIDGET_DRAW_STYLE_ROUNDED,         /**< Bordure arrondie (rayon fixe ou personnalisable). */
    WIDGET_DRAW_STYLE_CUSTOM_BEZIER,   /**< Bordure personnalisée via un chemin Bezier. */

    // ===== Styles de fond =====
    WIDGET_DRAW_STYLE_SOLID_COLOR = 10, /**< Fond uni (couleur simple). */
    WIDGET_DRAW_STYLE_COLOR_GRADIENT,  /**< Fond avec dégradé (linéaire ou radial). */

    // ===== Styles combinés (bordure + fond) =====
    WIDGET_DRAW_STYLE_ROUNDED_GRADIENT = 20, /**< Bordure arrondie + dégradé. */
    WIDGET_DRAW_STYLE_CUSTOM_BEZIER_GRADIENT, /**< Chemin Bezier + dégradé. */

    // ===== Style personnalisé (pour extensions) =====
    WIDGET_DRAW_STYLE_CUSTOM = 255,    /**< Style personnalisé (défini par l'utilisateur). */
} WIDGET_DRAW_STYLE;




🔍 Explications des Styles

  
    
      Style
      Description
      Utilisation Typique
    
  
  
    
      NORMAL
      Bordure carrée standard (comme un rectangle classique).
      Boutons, conteneurs, labels.
    
    
      ROUNDED
      Bordure arrondie (rayon définissable via border_radius dans la structure du widget).
      Boutons modernes, cartes.
    
    
      CUSTOM_BEZIER
      Bordure définie par un chemin Bezier (pour des formes complexes).
      Widgets avec des formes organiques.
    
    
      SOLID_COLOR
      Fond uni (couleur simple, définie par background.color).
      Tous les widgets.
    
    
      COLOR_GRADIENT
      Fond avec dégradé (linéaire ou radial, défini par background.gradient_start et gradient_end).
      Boutons, barres de progression.
    
    
      ROUNDED_GRADIENT
      Combinaison de ROUNDED + COLOR_GRADIENT.
      Boutons stylisés.
    
    
      CUSTOM_BEZIER_GRADIENT
      Combinaison de CUSTOM_BEZIER + COLOR_GRADIENT.
      Widgets avec des formes et couleurs avancées.
    
    
      CUSTOM
      Style entièrement personnalisé (pour extensions utilisateur).
      Widgets spécifiques.
    
  





📌 Structures Complémentaires
Pour utiliser ces styles, vous pouvez étendre vos structures existantes avec des champs dédiés. Voici un exemple :
1. Structure Style (pour le fond et la bordure)
cpp
Copier

typedef struct {
    WIDGET_DRAW_STYLE draw_style;       /**< Style de dessin (bordure + fond). */
    uint16_t bg_color;                  /**< Couleur de fond (si SOLID_COLOR). */
    struct {
        uint16_t start_color;          /**< Couleur de départ (si COLOR_GRADIENT). */
        uint16_t end_color;            /**< Couleur de fin (si COLOR_GRADIENT). */
        bool is_radial;                /**< Dégradé radial (true) ou linéaire (false). */
        float angle;                   /**< Angle du dégradé (en degrés, pour linéaire). */
    } gradient;
    struct {
        uint16_t color;                /**< Couleur de la bordure. */
        uint8_t width;                  /**< Épaisseur de la bordure. */
        uint8_t radius;                 /**< Rayon des coins arrondis (si ROUNDED). */
    } border;
    struct {
        // Pour CUSTOM_BEZIER : Tableau de points de contrôle (simplifié ici)
        uint8_t num_points;             /**< Nombre de points de contrôle. */
        void* points;                   /**< Pointeur vers les points de contrôle (ex: BezierPoint*). */
    } bezier_path;
} WidgetStyle;




2. Exemple de BezierPoint (pour CUSTOM_BEZIER)
cpp
Copier

typedef struct {
    uint16_t x;  /**< Coordonnée X. */
    uint16_t y;  /**< Coordonnée Y. */
} Point;

typedef struct {
    Point p0;   /**< Point de départ. */
    Point p1;   /**< Point de contrôle 1. */
    Point p2;   /**< Point de contrôle 2. */
    Point p3;   /**< Point de fin. */
} BezierCubic;   /**< Courbe de Bezier cubique (4 points). */




3. Intégration dans t_widget_base
cpp
Copier

typedef struct {
    WIDGET_TYPE type;
    WidgetStyle style;           /**< Style de dessin. */
    Rect rect;                   /**< Position + taille. */
    Text text;                   /**< Texte (si applicable). */
    void* data;                  /**< Données spécifiques au widget. */
} Widget;




🚀 Exemples d'Utilisation
1. Créer un Bouton avec Bordure Arrondie
cpp
Copier

WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style = WIDGET_DRAW_STYLE_ROUNDED;
button->base.style.border.radius = 10;  // Rayon des coins = 10 pixels
button->base.style.border.width = 2;
button->base.style.border.color = 0xFFFF;  // Blanc
button->base.style.bg_color = 0x001F;     // Bleu




2. Créer un Bouton avec Dégradé
cpp
Copier

WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style = WIDGET_DRAW_STYLE_COLOR_GRADIENT;
button->base.style.gradient.start_color = 0xF800;  // Rouge
button->base.style.gradient.end_color = 0x001F;    // Bleu
button->base.style.gradient.is_radial = false;     // Dégradé linéaire
button->base.style.gradient.angle = 45.0f;         // Angle du dégradé




3. Créer un Widget avec Chemin Bezier Personnalisé
cpp
Copier

// Définir un chemin Bezier (ex: courbe cubique)
BezierCubic path = {
    .p0 = {10, 10},   // Point de départ
    .p1 = {50, 100},  // Point de contrôle 1
    .p2 = {100, 10},  // Point de contrôle 2
    .p3 = {150, 50}   // Point de fin
};

// Créer le widget
Widget* custom_widget = new_widget(WIDGET_TYPE_CUSTOM);
custom_widget->style.draw_style = WIDGET_DRAW_STYLE_CUSTOM_BEZIER;
custom_widget->style.bezier_path.num_points = 1;  // 1 courbe Bezier
custom_widget->style.bezier_path.points = malloc(sizeof(BezierCubic));
*(BezierCubic*)custom_widget->style.bezier_path.points = path;




📌 Implémentation du Rendu (Exemple pour TFT_eSPI)
Voici comment dessiner un widget en fonction de son WIDGET_DRAW_STYLE :
cpp
Copier

#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();

void draw_widget(Widget* widget) {
    switch (widget->style.draw_style) {
        case WIDGET_DRAW_STYLE_NORMAL:
            // Dessiner un rectangle standard
            tft.fillRect(widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height, widget->style.bg_color);
            tft.drawRect(widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height, widget->style.border.color);
            break;

        case WIDGET_DRAW_STYLE_ROUNDED:
            // Dessiner un rectangle arrondi
            tft.fillRoundRect(widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height,
                             widget->style.border.radius, widget->style.bg_color);
            tft.drawRoundRect(widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height,
                             widget->style.border.radius, widget->style.border.color);
            break;

        case WIDGET_DRAW_STYLE_COLOR_GRADIENT:
            // Dessiner un dégradé (simplifié ici)
            draw_gradient(widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height,
                         widget->style.gradient.start_color, widget->style.gradient.end_color,
                         widget->style.gradient.is_radial, widget->style.gradient.angle);
            break;

        case WIDGET_DRAW_STYLE_CUSTOM_BEZIER:
            // Dessiner un chemin Bezier (nécessite une implémentation personnalisée)
            draw_bezier_path(widget);
            break;

        default:
            // Style non supporté : dessiner un rectangle standard
            tft.fillRect(widget->rect.x, widget->rect.y, widget->rect.width, widget->rect.height, widget->style.bg_color);
            break;
    }
}

// Exemple de fonction pour dessiner un dégradé linéaire
void draw_gradient(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color1, uint16_t color2, bool is_radial, float angle) {
    if (is_radial) {
        // Implémentation d'un dégradé radial (plus complexe)
    } else {
        // Dégradé linéaire simplifié
        for (uint16_t i = 0; i < w; i++) {
            float ratio = (float)i / (w - 1);
            uint16_t color = interpolate_color(color1, color2, ratio);
            tft.drawFastVLine(x + i, y, h, color);
        }
    }
}

// Interpolation de couleur (RGB565)
uint16_t interpolate_color(uint16_t color1, uint16_t color2, float ratio) {
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;

    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;

    uint8_t r = r1 + (r2 - r1) * ratio;
    uint8_t g = g1 + (g2 - g1) * ratio;
    uint8_t b = b1 + (b2 - b1) * ratio;

    return (r << 11) | (g << 5) | b;
}




💡 Optimisations pour ESP8266


Éviter malloc pour les chemins Bezier :

Utilisez un pool de courbes Bezier préalloué pour éviter la fragmentation mémoire.
Exemple :
cpp
Copier

#define MAX_BEZIER_PATHS 5
BezierCubic bezier_pool[MAX_BEZIER_PATHS];
uint8_t bezier_pool_index = 0;

BezierCubic* allocate_bezier_path() {
    if (bezier_pool_index >= MAX_BEZIER_PATHS) return NULL;
    return &bezier_pool[bezier_pool_index++];
}






Dégradés optimisés :

Pour les dégradés, précalculez les couleurs dans un tableau si possible (au lieu de calculer chaque pixel à la volée).
Exemple :
cpp
Copier

uint16_t gradient_colors[WIDTH]; // Tableau de couleurs précalculées
for (uint16_t i = 0; i < WIDTH; i++) {
    gradient_colors[i] = interpolate_color(color1, color2, (float)i / (WIDTH - 1));
}






Cache des styles :

Si plusieurs widgets partagent le même style, stockez-le une seule fois et référencez-le via un pointeur.


❓ Besoin d'Ajustements ?
Si vous souhaitez :

Ajouter d'autres styles (ex: WIDGET_DRAW_STYLE_DASHED_BORDER pour des bordures en pointillés),
Simplifier l'enum (moins de combinaisons),
Intégrer avec une bibliothèque spécifique (ex: LVGL pour ESP8266),
dites-le-moi et j'adapterai la solution ! 🛠️





09:56










Ajouter d'autres styles





09:57








Voici une version étendue et optimisée de l'enum WIDGET_DRAW_STYLE avec des styles supplémentaires pour couvrir un large éventail de cas d'utilisation dans votre GUIKit pour ESP8266 + écran TFT 3.2". J'ai organisé les styles en catégories logiques et ajouté des options pour des effets avancés tout en gardant une compatibilité avec les contraintes matérielles de l'ESP8266.

📌 WIDGET_DRAW_STYLE (Version Étendue)
cpp
Copier

/**
 * @brief Styles de dessin pour les widgets (bords, fond, ombres, effets).
 * @note Organisé par catégories pour une meilleure lisibilité et maintenabilité.
 *       Les valeurs sont groupées par type pour éviter les conflits.
 */
typedef enum {
    // ===== Styles de bordure de base =====
    WIDGET_DRAW_STYLE_NORMAL = 0,       /**< Bordure carrée standard (défaut). */
    WIDGET_DRAW_STYLE_NO_BORDER,        /**< Sans bordure (fond uniquement). */
    WIDGET_DRAW_STYLE_ROUNDED,          /**< Bordure arrondie (rayon uniforme). */
    WIDGET_DRAW_STYLE_ROUNDED_VARIABLE, /**< Bordure arrondie avec rayons personnalisés par coin. */

    // ===== Styles de bordure avancés =====
    WIDGET_DRAW_STYLE_DOTTED = 10,      /**< Bordure en pointillés. */
    WIDGET_DRAW_STYLE_DASHED,            /**< Bordure en tirets. */
    WIDGET_DRAW_STYLE_DOUBLE,            /**< Double bordure. */
    WIDGET_DRAW_STYLE_ETCHED,            /**< Bordure gravée (effet 3D). */
    WIDGET_DRAW_STYLE_EMBOSSED,          /**< Bordure en relief. */

    // ===== Styles de fond =====
    WIDGET_DRAW_STYLE_SOLID_COLOR = 20, /**< Fond uni (couleur simple). */
    WIDGET_DRAW_STYLE_COLOR_GRADIENT,   /**< Fond avec dégradé linéaire. */
    WIDGET_DRAW_STYLE_RADIAL_GRADIENT,   /**< Fond avec dégradé radial. */
    WIDGET_DRAW_STYLE_CONIC_GRADIENT,    /**< Fond avec dégradé conique (circulaire). */
    WIDGET_DRAW_STYLE_HATCHED,          /**< Fond hachuré (motifs répétitifs). */
    WIDGET_DRAW_STYLE_PATTERN,          /**< Fond avec motif personnalisé (ex: damier). */

    // ===== Styles de chemin personnalisé =====
    WIDGET_DRAW_STYLE_CUSTOM_BEZIER = 30, /**< Bordure définie par un chemin Bezier cubique. */
    WIDGET_DRAW_STYLE_CUSTOM_POLYGON,    /**< Bordure définie par un polygone (liste de points). */
    WIDGET_DRAW_STYLE_CUSTOM_SVG_PATH,   /**< Bordure définie par un chemin SVG simplifié. */

    // ===== Styles combinés (bordure + fond) =====
    WIDGET_DRAW_STYLE_ROUNDED_GRADIENT = 40,       /**< Bordure arrondie + dégradé linéaire. */
    WIDGET_DRAW_STYLE_ROUNDED_RADIAL_GRADIENT,     /**< Bordure arrondie + dégradé radial. */
    WIDGET_DRAW_STYLE_DOTTED_GRADIENT,              /**< Bordure en pointillés + dégradé. */
    WIDGET_DRAW_STYLE_CUSTOM_BEZIER_GRADIENT,       /**< Chemin Bezier + dégradé. */

    // ===== Effets spéciaux =====
    WIDGET_DRAW_STYLE_DROP_SHADOW = 50, /**< Ombre portée (simple). */
    WIDGET_DRAW_STYLE_INNER_SHADOW,      /**< Ombre intérieure. */
    WIDGET_DRAW_STYLE_GLOW,               /**< Effet de lueur (pour boutons actifs). */
    WIDGET_DRAW_STYLE_BLUR,               /**< Effet de flou (simulé). */
    WIDGET_DRAW_STYLE_TRANSPARENT,        /**< Transparent (pas de fond, bordure optionnelle). */

    // ===== Styles pour widgets spécifiques =====
    WIDGET_DRAW_STYLE_CIRCULAR = 60,    /**< Forme circulaire (pour boutons ronds). */
    WIDGET_DRAW_STYLE_ELLIPSE,           /**< Forme elliptique. */
    WIDGET_DRAW_STYLE_PIE,               /**< Secteur circulaire (camembert). */
    WIDGET_DRAW_STYLE_ARC,               /**< Arc de cercle. */

    // ===== Styles dynamiques (animés) =====
    WIDGET_DRAW_STYLE_PULSE = 70,        /**< Effet de pulsation (pour animations). */
    WIDGET_DRAW_STYLE_ANIMATED_GRADIENT, /**< Dégradé animé (ex: défilement). */

    // ===== Style personnalisé (extensions) =====
    WIDGET_DRAW_STYLE_CUSTOM = 255,      /**< Style entièrement personnalisé. */
} WIDGET_DRAW_STYLE;




🔍 Explications des Nouveaux Styles
1. Styles de Bordure Avancés

  
    
      Style
      Description
      Utilisation Typique
    
  
  
    
      DOTTED
      Bordure en pointillés (ex: · · · ·).
      Séparateurs, encadrements légers.
    
    
      DASHED
      Bordure en tirets (ex: — — —).
      Encadrements discontinus.
    
    
      DOUBLE
      Double bordure (deux lignes parallèles).
      Encadrements renforcés.
    
    
      ETCHED
      Bordure gravée (effet 3D avec ombres claires/sombrées).
      Boutons ou conteneurs avec effet "creusé".
    
    
      EMBOSSED
      Bordure en relief (effet 3D avec ombres inversées).
      Boutons ou conteneurs avec effet "soulevé".
    
  





2. Styles de Fond Avancés

  
    
      Style
      Description
      Utilisation Typique
    
  
  
    
      RADIAL_GRADIENT
      Dégradé radial (couleur partant d'un point central).
      Boutons circulaires, effets de lumière.
    
    
      CONIC_GRADIENT
      Dégradé conique (couleurs disposées en secteur circulaire).
      Cadran de montre, indicateurs circulaires.
    
    
      HATCHED
      Fond hachuré (motifs répétitifs comme /, \, ou x).
      Arrière-plans texturés.
    
    
      PATTERN
      Fond avec motif personnalisé (ex: damier, rayures).
      Arrière-plans stylisés.
    
  





3. Styles de Chemin Personnalisé

  
    
      Style
      Description
      Utilisation Typique
    
  
  
    
      CUSTOM_POLYGON
      Bordure définie par une liste de points (polygone).
      Formes géométriques complexes.
    
    
      CUSTOM_SVG_PATH
      Bordure définie par un chemin SVG simplifié (séquence de commandes comme M, L, C).
      Formes vectorielles avancées.
    
  





4. Effets Spéciaux

  
    
      Style
      Description
      Utilisation Typique
    
  
  
    
      DROP_SHADOW
      Ombre portée sous le widget.
      Boutons ou conteneurs "flottants".
    
    
      INNER_SHADOW
      Ombre intérieure (effet de creux).
      Boutons ou encadrements "enfoncés".
    
    
      GLOW
      Effet de lueur autour du widget.
      Boutons actifs ou survolés.
    
    
      BLUR
      Effet de flou (simulé via un dégradé de transparence).
      Arrière-plans floutés.
    
    
      TRANSPARENT
      Widget transparent (pas de fond, bordure optionnelle).
      Overlays, widgets superposés.
    
  





5. Styles pour Widgets Spécifiques

  
    
      Style
      Description
      Utilisation Typique
    
  
  
    
      CIRCULAR
      Forme circulaire (pour boutons ronds).
      Boutons circulaires, indicateurs.
    
    
      ELLIPSE
      Forme elliptique.
      Boutons ovales, graphiques.
    
    
      PIE
      Secteur circulaire (camembert).
      Graphiques en camembert.
    
    
      ARC
      Arc de cercle.
      Jauges, indicateurs circulaires.
    
  





6. Styles Dynamiques (Animés)

  
    
      Style
      Description
      Utilisation Typique
    
  
  
    
      PULSE
      Effet de pulsation (changement de taille ou d'opacité).
      Animations pour attirer l'attention.
    
    
      ANIMATED_GRADIENT
      Dégradé animé (ex: couleurs défilantes).
      Arrière-plans dynamiques.
    
  





📌 Structures Complémentaires pour les Nouveaux Styles
Pour supporter ces styles, étendez la structure WidgetStyle comme suit :
cpp
Copier

typedef struct {
    WIDGET_DRAW_STYLE draw_style;       /**< Style de dessin principal. */

    // === Couleurs et fond ===
    uint16_t bg_color;                  /**< Couleur de fond (si SOLID_COLOR). */
    struct {
        uint16_t start_color;          /**< Couleur de départ (dégradés). */
        uint16_t end_color;            /**< Couleur de fin (dégradés). */
        uint16_t middle_color;          /**< Couleur intermédiaire (pour CONIC_GRADIENT). */
        float angle;                   /**< Angle du dégradé (en degrés, pour linéaire/conique). */
        struct {
            uint16_t x;                /**< Position X du centre (pour RADIAL_GRADIENT). */
            uint16_t y;                /**< Position Y du centre (pour RADIAL_GRADIENT). */
            float radius;              /**< Rayon du dégradé radial. */
        } radial_center;
    } gradient;

    // === Bordure ===
    struct {
        uint16_t color;                /**< Couleur de la bordure. */
        uint8_t width;                  /**< Épaisseur de la bordure. */
        union {
            uint8_t radius;             /**< Rayon des coins (si ROUNDED). */
            struct {
                uint8_t top_left;       /**< Rayon du coin haut-gauche. */
                uint8_t top_right;       /**< Rayon du coin haut-droite. */
                uint8_t bottom_right;    /**< Rayon du coin bas-droite. */
                uint8_t bottom_left;     /**< Rayon du coin bas-gauche. */
            } variable_radius;          /**< Rayons personnalisés (si ROUNDED_VARIABLE). */
            struct {
                uint8_t pattern;         /**< Motif de la bordure (DOTTED, DASHED, etc.). */
                uint8_t on_length;        /**< Longueur des segments visibles (pour DOTTED/DASHED). */
                uint8_t off_length;       /**< Longueur des espaces (pour DOTTED/DASHED). */
            } pattern;                  /**< Motif de bordure (si DOTTED/DASHED). */
        };
    } border;

    // === Chemin personnalisé ===
    struct {
        uint8_t num_points;             /**< Nombre de points de contrôle. */
        void* points;                   /**< Pointeur vers les points (BezierCubic*, Point*, etc.). */
        uint8_t path_type;              /**< Type de chemin (BEZIER, POLYGON, SVG). */
    } custom_path;

    // === Effets spéciaux ===
    struct {
        bool has_shadow;               /**< Si une ombre est activée. */
        uint8_t shadow_blur;            /**< Flou de l'ombre (simulé). */
        uint8_t shadow_offset_x;        /**< Décalage X de l'ombre. */
        uint8_t shadow_offset_y;        /**< Décalage Y de l'ombre. */
        uint16_t shadow_color;          /**< Couleur de l'ombre. */
    } shadow;

    // === Style dynamique (animation) ===
    struct {
        bool animated;                  /**< Si le style est animé. */
        uint16_t animation_speed;       /**< Vitesse de l'animation (en ms). */
        uint16_t animation_step;        /**< Étape actuelle de l'animation. */
    } animation;
} WidgetStyle;




🚀 Exemples d'Utilisation des Nouveaux Styles
1. Bouton avec Bordure en Pointillés
cpp
Copier

WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style = WIDGET_DRAW_STYLE_DOTTED;
button->base.style.border.width = 2;
button->base.style.border.color = 0xFFFF;  // Blanc
button->base.style.border.pattern.on_length = 2;   // Longueur des points
button->base.style.border.pattern.off_length = 2;  // Espacement entre points
button->base.style.bg_color = 0x001F;      // Fond bleu




2. Widget avec Dégradé Radial
cpp
Copier

Widget* widget = new_widget(WIDGET_TYPE_VIEW);
widget->style.draw_style = WIDGET_DRAW_STYLE_RADIAL_GRADIENT;
widget->style.gradient.start_color = 0xF800;  // Rouge
widget->style.gradient.end_color = 0x001F;    // Bleu
widget->style.gradient.radial_center.x = 50;  // Centre X du dégradé
widget->style.gradient.radial_center.y = 50;  // Centre Y du dégradé
widget->style.gradient.radial_center.radius = 40; // Rayon du dégradé




3. Bouton Circulaire avec Ombre Portée
cpp
Copier

WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style = WIDGET_DRAW_STYLE_CIRCULAR;
button->base.style.bg_color = 0x07E0;  // Vert
button->base.style.shadow.has_shadow = true;
button->base.style.shadow.shadow_offset_x = 2;
button->base.style.shadow.shadow_offset_y = 2;
button->base.style.shadow.shadow_color = 0x8410;  // Gris
button->base.style.shadow.shadow_blur = 3;




4. Widget avec Chemin Polygonal
cpp
Copier

// Définir un polygone (ex: triangle)
Point triangle_points[] = {
    {10, 10},  // Point 1
    {50, 10},  // Point 2
    {30, 50}   // Point 3
};

// Créer le widget
Widget* polygon_widget = new_widget(WIDGET_TYPE_CUSTOM);
polygon_widget->style.draw_style = WIDGET_DRAW_STYLE_CUSTOM_POLYGON;
polygon_widget->style.custom_path.num_points = 3;
polygon_widget->style.custom_path.points = triangle_points;
polygon_widget->style.custom_path.path_type = 1;  // 1 = POLYGON




5. Bouton avec Effet de Lueur (GLOW)
cpp
Copier

WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style = WIDGET_DRAW_STYLE_GLOW;
button->base.style.bg_color = 0x001F;  // Bleu
button->base.style.shadow.has_shadow = true;
button->base.style.shadow.shadow_color = 0x001F;  // Même couleur que le fond
button->base.style.shadow.shadow_blur = 10;  // Flou élevé pour l'effet de lueur
button->base.style.shadow.shadow_offset_x = 0;
button->base.style.shadow.shadow_offset_y = 0;




💡 Implémentation du Rendu pour les Nouveaux Styles
Voici des exemples de fonctions pour dessiner certains des nouveaux styles avec TFT_eSPI :
1. Dessiner une Bordure en Pointillés
cpp
Copier

void draw_dotted_border(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint8_t width, uint8_t on_len, uint8_t off_len) {
    // Dessiner les 4 côtés
    for (uint16_t i = 0; i < w; i += on_len + off_len) {
        uint16_t segment_len = (i + on_len <= w) ? on_len : (w - i);
        tft.drawFastHLine(x + i, y, segment_len, color);
        tft.drawFastHLine(x + i, y + h - 1, segment_len, color);
    }
    for (uint16_t i = 0; i < h; i += on_len + off_len) {
        uint16_t segment_len = (i + on_len <= h) ? on_len : (h - i);
        tft.drawFastVLine(x + i, y, segment_len, color);
        tft.drawFastVLine(x + w - 1, y + i, segment_len, color);
    }
}




2. Dessiner un Dégradé Radial
cpp
Copier

void draw_radial_gradient(uint16_t cx, uint16_t cy, uint16_t radius, uint16_t color1, uint16_t color2) {
    for (uint16_t r = 0; r <= radius; r++) {
        float ratio = (float)r / radius;
        uint16_t color = interpolate_color(color1, color2, ratio);
        tft.drawCircle(cx, cy, r, color);
    }
}




3. Dessiner un Polygone
cpp
Copier

void draw_polygon(Point* points, uint8_t num_points, uint16_t color) {
    if (num_points < 2) return;
    for (uint8_t i = 0; i < num_points - 1; i++) {
        tft.drawLine(points[i].x, points[i].y, points[i+1].x, points[i+1].y, color);
    }
    tft.drawLine(points[num_points-1].x, points[num_points-1].y, points[0].x, points[0].y, color);
}




4. Dessiner une Ombre Portée
cpp
Copier

void draw_drop_shadow(Widget* widget, uint8_t blur, uint8_t offset_x, uint8_t offset_y, uint16_t color) {
    // Dessiner un rectangle flouté sous le widget
    for (uint8_t b = 0; b <= blur; b++) {
        uint16_t alpha = (255 * (blur - b)) / blur;  // Opacité décroissante
        uint16_t shadow_color = alpha_blend(0x0000, color, alpha);
        tft.drawRect(
            widget->rect.x + offset_x - b,
            widget->rect.y + offset_y - b,
            widget->rect.width + 2 * b,
            widget->rect.height + 2 * b,
            shadow_color
        );
    }
}

// Fonction pour mélanger une couleur avec du noir (simulation de transparence)
uint16_t alpha_blend(uint16_t bg, uint16_t fg, uint8_t alpha) {
    if (alpha == 0) return bg;
    if (alpha == 255) return fg;

    // Extraire les composantes RGB565
    uint8_t r1 = (bg >> 11) & 0x1F;
    uint8_t g1 = (bg >> 5) & 0x3F;
    uint8_t b1 = bg & 0x1F;

    uint8_t r2 = (fg >> 11) & 0x1F;
    uint8_t g2 = (fg >> 5) & 0x3F;
    uint8_t b2 = fg & 0x1F;

    // Interpolation linéaire
    uint8_t r = r1 + (r2 - r1) * alpha / 255;
    uint8_t g = g1 + (g2 - g1) * alpha / 255;
    uint8_t b = b1 + (b2 - b1) * alpha / 255;

    return (r << 11) | (g << 5) | b;
}




📌 Optimisations pour ESP8266


Précalcul des Dégradés :

Pour les dégradés, précalculez les couleurs dans un tableau si la mémoire le permet.
Exemple :
cpp
Copier

void precompute_gradient(uint16_t* colors, uint16_t width, uint16_t color1, uint16_t color2) {
    for (uint16_t i = 0; i < width; i++) {
        float ratio = (float)i / (width - 1);
        colors[i] = interpolate_color(color1, color2, ratio);
    }
}






Pool de Chemins Personnalisés :

Utilisez un pool statique pour les chemins personnalisés (Bezier, polygones) :
cpp
Copier

#define MAX_CUSTOM_PATHS 10
Point path_pool[MAX_CUSTOM_PATHS][20];  // 10 chemins, 20 points max chacun
uint8_t path_pool_index = 0;

Point* allocate_path(uint8_t num_points) {
    if (path_pool_index >= MAX_CUSTOM_PATHS) return NULL;
    return path_pool[path_pool_index++];
}






Cache des Styles :

Si plusieurs widgets partagent le même style, stockez-le une seule fois et utilisez un pointeur :
cpp
Copier

typedef struct {
    WidgetStyle style;
    uint8_t ref_count;  // Nombre de widgets utilisant ce style
} SharedStyle;

#define MAX_SHARED_STYLES 20
SharedStyle shared_styles[MAX_SHARED_STYLES];
uint8_t shared_styles_index = 0;

WidgetStyle* create_shared_style(WidgetStyle style) {
    if (shared_styles_index >= MAX_SHARED_STYLES) return NULL;
    shared_styles[shared_styles_index] = (SharedStyle){style, 1};
    return &shared_styles[shared_styles_index++].style;
}






❓ Besoin d'Ajustements ?
Si vous souhaitez :

Ajouter d'autres styles (ex: WIDGET_DRAW_STYLE_METALIC pour un effet métallique),
Simplifier l'enum (moins de combinaisons),
Optimiser pour un usage spécifique (ex: prioriser les styles les plus utilisés),
Intégrer avec une bibliothèque de rendu existante (ex: LVGL),
dites-le-moi et j'adapterai la solution ! 🛠️





09:56










on doit pouvoir combiner les style





10:00








Pour permettre la combinaison des styles de manière flexible (ex: ROUNDED + GRADIENT + DROP_SHADOW), la meilleure approche est d'utiliser des flags binaires (bitmask) au lieu d'un simple enum. Cela permet de combiner plusieurs styles via des opérations binaires (|, &, etc.).

📌 Solution : WIDGET_DRAW_STYLE en Flags (Bitmask)
1. Définition des Flags
Chaque style est représenté par un bit unique dans un uint32_t. Cela permet de combiner plusieurs styles avec l'opérateur | (OR binaire).
cpp
Copier

/**
 * @brief Flags de style de dessin pour les widgets.
 * @note Chaque flag est un bit unique, permettant des combinaisons via | (OR).
 *       Exemple: ROUNDED | COLOR_GRADIENT | DROP_SHADOW
 */
typedef enum {
    // ===== Styles de bordure =====
    WIDGET_DRAW_STYLE_NORMAL        = 0,            /**< Aucun style spécial (défaut). */
    WIDGET_DRAW_STYLE_NO_BORDER     = 1 << 0,       /**< Sans bordure. */
    WIDGET_DRAW_STYLE_ROUNDED        = 1 << 1,       /**< Bordure arrondie. */
    WIDGET_DRAW_STYLE_ROUNDED_VARIABLE = 1 << 2,    /**< Bordure arrondie avec rayons personnalisés. */
    WIDGET_DRAW_STYLE_DOTTED        = 1 << 3,       /**< Bordure en pointillés. */
    WIDGET_DRAW_STYLE_DASHED        = 1 << 4,       /**< Bordure en tirets. */
    WIDGET_DRAW_STYLE_DOUBLE        = 1 << 5,       /**< Double bordure. */
    WIDGET_DRAW_STYLE_ETCHED        = 1 << 6,       /**< Bordure gravée. */
    WIDGET_DRAW_STYLE_EMBOSSED      = 1 << 7,       /**< Bordure en relief. */

    // ===== Styles de fond =====
    WIDGET_DRAW_STYLE_SOLID_COLOR   = 1 << 8,       /**< Fond uni. */
    WIDGET_DRAW_STYLE_COLOR_GRADIENT = 1 << 9,      /**< Dégradé linéaire. */
    WIDGET_DRAW_STYLE_RADIAL_GRADIENT = 1 << 10,     /**< Dégradé radial. */
    WIDGET_DRAW_STYLE_CONIC_GRADIENT = 1 << 11,      /**< Dégradé conique. */
    WIDGET_DRAW_STYLE_HATCHED        = 1 << 12,      /**< Fond hachuré. */
    WIDGET_DRAW_STYLE_PATTERN        = 1 << 13,      /**< Fond avec motif. */

    // ===== Styles de chemin personnalisé =====
    WIDGET_DRAW_STYLE_CUSTOM_BEZIER = 1 << 14,      /**< Chemin Bezier. */
    WIDGET_DRAW_STYLE_CUSTOM_POLYGON = 1 << 15,      /**< Polygone personnalisé. */

    // ===== Effets spéciaux =====
    WIDGET_DRAW_STYLE_DROP_SHADOW   = 1 << 16,      /**< Ombre portée. */
    WIDGET_DRAW_STYLE_INNER_SHADOW   = 1 << 17,      /**< Ombre intérieure. */
    WIDGET_DRAW_STYLE_GLOW           = 1 << 18,      /**< Effet de lueur. */
    WIDGET_DRAW_STYLE_BLUR           = 1 << 19,      /**< Effet de flou. */
    WIDGET_DRAW_STYLE_TRANSPARENT    = 1 << 20,      /**< Transparent. */

    // ===== Formes spéciales =====
    WIDGET_DRAW_STYLE_CIRCULAR      = 1 << 21,      /**< Forme circulaire. */
    WIDGET_DRAW_STYLE_ELLIPSE        = 1 << 22,      /**< Forme elliptique. */

    // ===== Styles dynamiques =====
    WIDGET_DRAW_STYLE_PULSE          = 1 << 23,      /**< Effet de pulsation. */
    WIDGET_DRAW_STYLE_ANIMATED_GRADIENT = 1 << 24,   /**< Dégradé animé. */

    // ===== Masques pour catégories =====
    WIDGET_DRAW_STYLE_BORDER_MASK    = 0xFF,         /**< Masque pour les styles de bordure. */
    WIDGET_DRAW_STYLE_FILL_MASK      = 0xFF00,       /**< Masque pour les styles de fond. */
    WIDGET_DRAW_STYLE_EFFECT_MASK    = 0xFF0000,     /**< Masque pour les effets spéciaux. */
} WIDGET_DRAW_STYLE;




🔍 Explications des Flags
1. Combinaison de Styles
Avec cette approche, vous pouvez combiner n'importe quel nombre de styles en utilisant l'opérateur | (OR binaire) :
cpp
Copier

// Exemple 1: Bouton arrondi avec dégradé et ombre portée
uint32_t style = WIDGET_DRAW_STYLE_ROUNDED | WIDGET_DRAW_STYLE_COLOR_GRADIENT | WIDGET_DRAW_STYLE_DROP_SHADOW;

// Exemple 2: Widget transparent avec bordure en pointillés
uint32_t style = WIDGET_DRAW_STYLE_TRANSPARENT | WIDGET_DRAW_STYLE_DOTTED;

// Exemple 3: Forme circulaire avec effet de lueur
uint32_t style = WIDGET_DRAW_STYLE_CIRCULAR | WIDGET_DRAW_STYLE_GLOW;




2. Vérification des Styles
Pour vérifier si un widget a un style spécifique, utilisez l'opérateur & (AND binaire) :
cpp
Copier

if (widget->style.draw_style & WIDGET_DRAW_STYLE_ROUNDED) {
    // Le widget a une bordure arrondie
}

if ((widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK) == WIDGET_DRAW_STYLE_COLOR_GRADIENT) {
    // Le fond est un dégradé linéaire
}




3. Masques pour Catégories
Les masques (BORDER_MASK, FILL_MASK, etc.) permettent de filtrer les styles par catégorie :
cpp
Copier

// Récupérer uniquement les styles de bordure
uint32_t border_style = widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK;

// Récupérer uniquement les styles de fond
uint32_t fill_style = widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK;




📌 Structure WidgetStyle Mise à Jour
Voici comment adapter la structure WidgetStyle pour supporter les combinaisons de styles :
cpp
Copier

typedef struct {
    uint32_t draw_style;  /**< Combinaison de flags WIDGET_DRAW_STYLE. */

    // === Couleurs et fond ===
    uint16_t bg_color;     /**< Couleur de fond (si SOLID_COLOR). */
    struct {
        uint16_t start_color;
        uint16_t end_color;
        uint16_t middle_color;  /**< Pour CONIC_GRADIENT. */
        float angle;           /**< Angle du dégradé (pour linéaire/conique). */
        struct {
            uint16_t x;        /**< Centre X (pour RADIAL_GRADIENT). */
            uint16_t y;        /**< Centre Y (pour RADIAL_GRADIENT). */
            float radius;      /**< Rayon du dégradé radial. */
        } radial_center;
    } gradient;

    // === Bordure ===
    struct {
        uint16_t color;
        uint8_t width;
        union {
            uint8_t radius;    /**< Rayon uniforme (si ROUNDED). */
            struct {
                uint8_t top_left;
                uint8_t top_right;
                uint8_t bottom_right;
                uint8_t bottom_left;
            } variable_radius;  /**< Rayons personnalisés (si ROUNDED_VARIABLE). */
            struct {
                uint8_t pattern;  /**< DOTTED, DASHED, etc. */
                uint8_t on_length;
                uint8_t off_length;
            } pattern;          /**< Motif de bordure. */
        };
    } border;

    // === Chemin personnalisé ===
    struct {
        uint8_t num_points;
        void* points;          /**< Pointeur vers les points (BezierCubic*, Point*, etc.). */
        uint8_t path_type;     /**< Type de chemin (BEZIER, POLYGON, etc.). */
    } custom_path;

    // === Effets spéciaux ===
    struct {
        bool has_shadow;
        uint8_t shadow_blur;
        uint8_t shadow_offset_x;
        uint8_t shadow_offset_y;
        uint16_t shadow_color;
    } shadow;

    // === Formes spéciales ===
    struct {
        uint16_t radius;       /**< Rayon (pour CIRCULAR). */
        uint16_t a;            /**< Demi-grand axe (pour ELLIPSE). */
        uint16_t b;            /**< Demi-petit axe (pour ELLIPSE). */
    } shape;

    // === Animation ===
    struct {
        bool animated;
        uint16_t animation_speed;
        uint16_t animation_step;
    } animation;
} WidgetStyle;




🚀 Exemples d'Utilisation avec Combinaisons
1. Bouton Arrondi avec Dégradé et Ombre Portée
cpp
Copier

WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style =
    WIDGET_DRAW_STYLE_ROUNDED |
    WIDGET_DRAW_STYLE_COLOR_GRADIENT |
    WIDGET_DRAW_STYLE_DROP_SHADOW;

// Configurer le dégradé
button->base.style.gradient.start_color = 0xF800;  // Rouge
button->base.style.gradient.end_color = 0x001F;    // Bleu
button->base.style.gradient.angle = 45.0f;        // Angle du dégradé

// Configurer l'ombre
button->base.style.shadow.has_shadow = true;
button->base.style.shadow.shadow_offset_x = 2;
button->base.style.shadow.shadow_offset_y = 2;
button->base.style.shadow.shadow_color = 0x8410;  // Gris
button->base.style.shadow.shadow_blur = 3;

// Configurer la bordure arrondie
button->base.style.border.radius = 10;
button->base.style.border.width = 2;
button->base.style.border.color = 0xFFFF;  // Blanc




2. Widget Transparent avec Bordure en Pointillés
cpp
Copier

Widget* widget = new_widget(WIDGET_TYPE_VIEW);
widget->style.draw_style =
    WIDGET_DRAW_STYLE_TRANSPARENT |
    WIDGET_DRAW_STYLE_DOTTED;

// Configurer la bordure en pointillés
widget->style.border.width = 1;
widget->style.border.color = 0xFFFF;  // Blanc
widget->style.border.pattern.on_length = 2;
widget->style.border.pattern.off_length = 2;




3. Bouton Circulaire avec Effet de Lueur
cpp
Copier

WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style =
    WIDGET_DRAW_STYLE_CIRCULAR |
    WIDGET_DRAW_STYLE_GLOW;

// Configurer la forme circulaire
button->base.style.shape.radius = 30;  // Rayon du cercle

// Configurer l'effet de lueur
button->base.style.shadow.has_shadow = true;
button->base.style.shadow.shadow_color = 0x07E0;  // Vert
button->base.style.shadow.shadow_blur = 10;
button->base.style.shadow.shadow_offset_x = 0;
button->base.style.shadow.shadow_offset_y = 0;

// Configurer le fond
button->base.style.bg_color = 0x07E0;  // Vert




4. Widget avec Polygone Personnalisé et Dégradé Radial
cpp
Copier

// Définir un polygone (ex: triangle)
Point triangle_points[] = {
    {10, 10},  // Point 1
    {50, 10},  // Point 2
    {30, 50}   // Point 3
};

Widget* widget = new_widget(WIDGET_TYPE_CUSTOM);
widget->style.draw_style =
    WIDGET_DRAW_STYLE_CUSTOM_POLYGON |
    WIDGET_DRAW_STYLE_RADIAL_GRADIENT;

// Configurer le polygone
widget->style.custom_path.num_points = 3;
widget->style.custom_path.points = triangle_points;
widget->style.custom_path.path_type = 1;  // POLYGON

// Configurer le dégradé radial
widget->style.gradient.start_color = 0xF800;  // Rouge
widget->style.gradient.end_color = 0x001F;    // Bleu
widget->style.gradient.radial_center.x = 30;   // Centre X
widget->style.gradient.radial_center.y = 30;   // Centre Y
widget->style.gradient.radial_center.radius = 20; // Rayon




📌 Fonctions Utilitaires pour Gérer les Styles
1. Ajouter un Style
cpp
Copier

/**
 * @brief Ajoute un style à un widget.
 * @param widget Widget cible.
 * @param style Style à ajouter.
 */
void widget_add_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    widget->style.draw_style |= style;
}




2. Supprimer un Style
cpp
Copier

/**
 * @brief Supprime un style d'un widget.
 * @param widget Widget cible.
 * @param style Style à supprimer.
 */
void widget_remove_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    widget->style.draw_style &= ~style;
}




3. Vérifier si un Style est Présent
cpp
Copier

/**
 * @brief Vérifie si un widget a un style spécifique.
 * @param widget Widget cible.
 * @param style Style à vérifier.
 * @return true si le style est présent, false sinon.
 */
bool widget_has_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    return (widget->style.draw_style & style) == style;
}




4. Réinitialiser les Styles
cpp
Copier

/**
 * @brief Réinitialise les styles d'un widget.
 * @param widget Widget cible.
 */
void widget_reset_style(Widget* widget) {
    widget->style.draw_style = WIDGET_DRAW_STYLE_NORMAL;
}




5. Récupérer les Styles de Bordure
cpp
Copier

/**
 * @brief Récupère uniquement les styles de bordure.
 * @param widget Widget cible.
 * @return Combinaison des styles de bordure.
 */
WIDGET_DRAW_STYLE widget_get_border_style(Widget* widget) {
    return widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK;
}




🚀 Implémentation du Rendu avec Combinaisons
Voici comment adapter la fonction draw_widget pour gérer les combinaisons de styles :
cpp
Copier

void draw_widget(Widget* widget) {
    // 1. Dessiner l'ombre (si présente)
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_DROP_SHADOW) {
        draw_drop_shadow(widget,
                         widget->style.shadow.shadow_blur,
                         widget->style.shadow.shadow_offset_x,
                         widget->style.shadow.shadow_offset_y,
                         widget->style.shadow.shadow_color);
    }

    // 2. Dessiner le fond (selon le style de remplissage)
    uint32_t fill_style = widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK;
    switch (fill_style) {
        case WIDGET_DRAW_STYLE_SOLID_COLOR:
            draw_solid_fill(widget);
            break;
        case WIDGET_DRAW_STYLE_COLOR_GRADIENT:
            draw_linear_gradient(widget);
            break;
        case WIDGET_DRAW_STYLE_RADIAL_GRADIENT:
            draw_radial_gradient(widget);
            break;
        case WIDGET_DRAW_STYLE_HATCHED:
            draw_hatched_fill(widget);
            break;
        // ... autres styles de fond
        default:
            // Aucun style de fond : transparent ou SOLID_COLOR par défaut
            if (!(widget->style.draw_style & WIDGET_DRAW_STYLE_TRANSPARENT)) {
                draw_solid_fill(widget);
            }
            break;
    }

    // 3. Dessiner la bordure (selon le style de bordure)
    uint32_t border_style = widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK;
    switch (border_style) {
        case WIDGET_DRAW_STYLE_NO_BORDER:
            // Aucune bordure
            break;
        case WIDGET_DRAW_STYLE_ROUNDED:
            draw_rounded_border(widget);
            break;
        case WIDGET_DRAW_STYLE_DOTTED:
            draw_dotted_border(widget);
            break;
        case WIDGET_DRAW_STYLE_DASHED:
            draw_dashed_border(widget);
            break;
        case WIDGET_DRAW_STYLE_CUSTOM_BEZIER:
            draw_bezier_border(widget);
            break;
        // ... autres styles de bordure
        default:
            // Bordure normale par défaut
            draw_normal_border(widget);
            break;
    }

    // 4. Dessiner les effets spéciaux (glow, etc.)
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_GLOW) {
        draw_glow_effect(widget);
    }

    // 5. Dessiner le contenu (texte, icône, etc.)
    if (widget->text.text[0] != '\0') {
        draw_text(widget);
    }
}




💡 Optimisations pour ESP8266
1. Précalcul des Combinaisons Fréquentes
Si certaines combinaisons de styles sont fréquemment utilisées (ex: ROUNDED | COLOR_GRADIENT), vous pouvez les précalculer et les stocker dans un tableau pour éviter de recalculer les mêmes paramètres à chaque rendu.
cpp
Copier

// Tableau de styles précalculés
typedef struct {
    uint32_t style;
    WidgetStyle precomputed_style;
} CachedStyle;

#define MAX_CACHED_STYLES 20
CachedStyle cached_styles[MAX_CACHED_STYLES];
uint8_t cached_styles_count = 0;

// Fonction pour obtenir un style précalculé
WidgetStyle* get_cached_style(uint32_t style) {
    for (uint8_t i = 0; i < cached_styles_count; i++) {
        if (cached_styles[i].style == style) {
            return &cached_styles[i].precomputed_style;
        }
    }
    return NULL;  // Non trouvé
}




2. Utilisation de Pointeurs vers des Styles Partagés
Si plusieurs widgets partagent le même style, utilisez un pointeur vers une structure WidgetStyle partagée pour économiser de la mémoire.
cpp
Copier

// Structure pour un style partagé
typedef struct {
    WidgetStyle style;
    uint8_t ref_count;  // Nombre de widgets utilisant ce style
} SharedStyle;

#define MAX_SHARED_STYLES 10
SharedStyle shared_styles[MAX_SHARED_STYLES];
uint8_t shared_styles_count = 0;

// Fonction pour créer ou réutiliser un style partagé
WidgetStyle* get_shared_style(WidgetStyle style) {
    // Rechercher un style existant
    for (uint8_t i = 0; i < shared_styles_count; i++) {
        if (memcmp(&shared_styles[i].style, &style, sizeof(WidgetStyle)) == 0) {
            shared_styles[i].ref_count++;
            return &shared_styles[i].style;
        }
    }

    // Créer un nouveau style partagé
    if (shared_styles_count >= MAX_SHARED_STYLES) return NULL;
    shared_styles[shared_styles_count] = (SharedStyle){style, 1};
    return &shared_styles[shared_styles_count++].style;
}




3. Macros pour les Combinaisons Fréquentes
Définissez des macros pour les combinaisons de styles les plus utilisées afin de simplifier le code :
cpp
Copier

// Combinaisons fréquentes
#define STYLE_DEFAULT               (WIDGET_DRAW_STYLE_NORMAL | WIDGET_DRAW_STYLE_SOLID_COLOR)
#define STYLE_ROUNDED_BUTTON        (WIDGET_DRAW_STYLE_ROUNDED | WIDGET_DRAW_STYLE_COLOR_GRADIENT | WIDGET_DRAW_STYLE_DROP_SHADOW)
#define STYLE_TRANSPARENT_DOTTED    (WIDGET_DRAW_STYLE_TRANSPARENT | WIDGET_DRAW_STYLE_DOTTED)
#define STYLE_GLOW_CIRCULAR         (WIDGET_DRAW_STYLE_CIRCULAR | WIDGET_DRAW_STYLE_GLOW)




❓ Besoin d'Ajustements ?
Si vous souhaitez :

Ajouter d'autres styles (ex: WIDGET_DRAW_STYLE_METALIC),
Simplifier la gestion des combinaisons,
Optimiser pour un cas d'usage spécifique (ex: prioriser les performances),
Intégrer avec une bibliothèque de rendu existante (ex: LVGL),
dites-le-moi et j'adapterai la solution ! 🛠️





09:59


