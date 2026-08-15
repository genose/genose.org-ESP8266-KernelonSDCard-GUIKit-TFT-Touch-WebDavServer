








pas de LVGL, Ajouter d'autres styles





10:01








Voici une version ultra-complète de WIDGET_DRAW_STYLE en flags binaires (bitmask), avec tous les styles possibles pour votre GUIKit sur ESP8266 + TFT 3.2", sans dépendance à LVGL. J'ai organisé les styles en catégories claires et ajouté des styles avancés tout en restant compatible avec les contraintes matérielles de l'ESP8266.

📌 WIDGET_DRAW_STYLE (Version Ultime avec Combinaisons)
cpp
Copier

/**
 * @brief Flags de style de dessin pour les widgets (combinaisons possibles via |).
 * @note Chaque flag est un bit unique dans un uint32_t.
 *       Exemple: WIDGET_DRAW_STYLE_ROUNDED | WIDGET_DRAW_STYLE_COLOR_GRADIENT | WIDGET_DRAW_STYLE_DROP_SHADOW
 */
typedef enum {
    // ========== STYLES DE BORDURE ==========
    WIDGET_DRAW_STYLE_NO_BORDER        = 0,            /**< Aucune bordure (défaut si aucun flag de bordure). */
    WIDGET_DRAW_STYLE_SOLID_BORDER      = 1 << 0,       /**< Bordure pleine (défaut). */
    WIDGET_DRAW_STYLE_ROUNDED_BORDER    = 1 << 1,       /**< Bordure arrondie (rayon uniforme). */
    WIDGET_DRAW_STYLE_ROUNDED_VARIABLE  = 1 << 2,       /**< Bordure arrondie avec rayons personnalisés par coin. */
    WIDGET_DRAW_STYLE_DOTTED_BORDER     = 1 << 3,       /**< Bordure en pointillés. */
    WIDGET_DRAW_STYLE_DASHED_BORDER     = 1 << 4,       /**< Bordure en tirets. */
    WIDGET_DRAW_STYLE_DOUBLE_BORDER     = 1 << 5,       /**< Double bordure. */
    WIDGET_DRAW_STYLE_ETCHED_BORDER     = 1 << 6,       /**< Bordure gravée (effet 3D creux). */
    WIDGET_DRAW_STYLE_EMBOSSED_BORDER   = 1 << 7,       /**< Bordure en relief (effet 3D saillant). */
    WIDGET_DRAW_STYLE_DOTTED_ROUNDED    = 1 << 8,       /**< Bordure arrondie + pointillés. */
    WIDGET_DRAW_STYLE_DASHED_ROUNDED    = 1 << 9,       /**< Bordure arrondie + tirets. */

    // ========== STYLES DE FOND ==========
    WIDGET_DRAW_STYLE_SOLID_FILL        = 1 << 10,      /**< Fond uni (défaut). */
    WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT = 1 << 11,    /**< Dégradé horizontal. */
    WIDGET_DRAW_STYLE_VERTICAL_GRADIENT = 1 << 12,      /**< Dégradé vertical. */
    WIDGET_DRAW_STYLE_DIAGONAL_GRADIENT = 1 << 13,      /**< Dégradé diagonal (45°). */
    WIDGET_DRAW_STYLE_RADIAL_GRADIENT   = 1 << 14,      /**< Dégradé radial (centré). */
    WIDGET_DRAW_STYLE_CONIC_GRADIENT    = 1 << 15,      /**< Dégradé conique (secteur circulaire). */
    WIDGET_DRAW_STYLE_HATCHED_FILL       = 1 << 16,      /**< Fond hachuré (lignes croisées). */
    WIDGET_DRAW_STYLE_CROSSHATCH_FILL    = 1 << 17,      /**< Fond hachuré croisé (X). */
    WIDGET_DRAW_STYLE_DOT_FILL           = 1 << 18,      /**< Fond en points (motif régulier). */
    WIDGET_DRAW_STYLE_PATTERN_FILL       = 1 << 19,      /**< Fond avec motif personnalisé (ex: damier). */
    WIDGET_DRAW_STYLE_TRANSPARENT_FILL   = 1 << 20,      /**< Fond transparent. */

    // ========== FORMES SPÉCIFIQUES ==========
    WIDGET_DRAW_STYLE_RECTANGLE         = 1 << 21,      /**< Forme rectangulaire (défaut). */
    WIDGET_DRAW_STYLE_CIRCLE            = 1 << 22,      /**< Forme circulaire. */
    WIDGET_DRAW_STYLE_ELLIPSE           = 1 << 23,      /**< Forme elliptique. */
    WIDGET_DRAW_STYLE_TRIANGLE          = 1 << 24,      /**< Forme triangulaire. */
    WIDGET_DRAW_STYLE_POLYGON           = 1 << 25,      /**< Forme polygonale (points personnalisés). */
    WIDGET_DRAW_STYLE_BEZIER_PATH       = 1 << 26,      /**< Forme définie par un chemin Bezier. */
    WIDGET_DRAW_STYLE_ARC               = 1 << 27,      /**< Arc de cercle. */
    WIDGET_DRAW_STYLE_PIE               = 1 << 28,      /**< Secteur circulaire (camembert). */
    WIDGET_DRAW_STYLE_ROUNDED_RECT      = 1 << 29,      /**< Rectangle arrondi (alias pour ROUNDED_BORDER + RECTANGLE). */

    // ========== EFFETS VISUELS ==========
    WIDGET_DRAW_STYLE_DROP_SHADOW       = 1 << 30,      /**< Ombre portée externe. */
    WIDGET_DRAW_STYLE_INNER_SHADOW       = 1 << 31,      /**< Ombre intérieure (creux). */
    WIDGET_DRAW_STYLE_GLOW               = 1UL << 0,     /**< Effet de lueur (halo lumineux). */
    WIDGET_DRAW_STYLE_BLUR               = 1UL << 1,     /**< Effet de flou (simulé). */
    WIDGET_DRAW_STYLE_METALIC            = 1UL << 2,     /**< Effet métallique (reflets). */
    WIDGET_DRAW_STYLE_GLASS              = 1UL << 3,     /**< Effet verre (transparence + reflets). */
    WIDGET_DRAW_STYLE_PLASTIC            = 1UL << 4,     /**< Effet plastique (lisse et brillant). */
    WIDGET_DRAW_STYLE_NEON               = 1UL << 5,     /**< Effet néon (lueur colorée). */
    WIDGET_DRAW_STYLE_GRADIENT_BORDER    = 1UL << 6,     /**< Bordure avec dégradé de couleur. */

    // ========== EFFETS DYNAMIQUES (ANIMATIONS) ==========
    WIDGET_DRAW_STYLE_PULSE              = 1UL << 7,     /**< Effet de pulsation (scale/opacité). */
    WIDGET_DRAW_STYLE_ANIMATED_GRADIENT  = 1UL << 8,     /**< Dégradé animé (défilement). */
    WIDGET_DRAW_STYLE_ROTATE             = 1UL << 9,     /**< Rotation (pour icônes ou formes). */
    WIDGET_DRAW_STYLE_SHAKE              = 1UL << 10,    /**< Effet de secousse (pour notifications). */
    WIDGET_DRAW_STYLE_FADE_IN_OUT        = 1UL << 11,    /**< Fondu (apparition/disparition). */

    // ========== MASQUES POUR CATEGORIES ==========
    WIDGET_DRAW_STYLE_BORDER_MASK        = 0x000003FF,   /**< Masque pour les styles de bordure (bits 0-9). */
    WIDGET_DRAW_STYLE_FILL_MASK          = 0x000FFC00,   /**< Masque pour les styles de fond (bits 10-19). */
    WIDGET_DRAW_STYLE_SHAPE_MASK         = 0x01F00000,   /**< Masque pour les formes (bits 20-29). */
    WIDGET_DRAW_STYLE_EFFECT_MASK        = 0xE0000000,   /**< Masque pour les effets visuels (bits 30-31 + 0-11 de l'octet supérieur). */
    WIDGET_DRAW_STYLE_DYNAMIC_MASK       = 0x1F8000000,  /**< Masque pour les animations (bits 32-42). */
} WIDGET_DRAW_STYLE;




🔍 Explications des Nouveaux Styles

📌 Styles de Bordure Avancés

  
    
      Style
      Description
      Combinaisons Possibles
    
  
  
    
      DOTTED_ROUNDED
      Bordure arrondie et en pointillés.
      `ROUNDED_BORDER
    
    
      DASHED_ROUNDED
      Bordure arrondie et en tirets.
      `ROUNDED_BORDER
    
    
      GRADIENT_BORDER
      Bordure avec un dégradé de couleur (ex: rouge → bleu).
      Combinable avec SOLID_BORDER ou ROUNDED_BORDER.
    
  





📌 Styles de Fond Avancés

  
    
      Style
      Description
      Exemple d'Utilisation
    
  
  
    
      HORIZONTAL_GRADIENT
      Dégradé horizontal (gauche → droite).
      Boutons, barres de progression.
    
    
      VERTICAL_GRADIENT
      Dégradé vertical (haut → bas).
      Arrière-plans de conteneurs.
    
    
      DIAGONAL_GRADIENT
      Dégradé diagonal (45°).
      Boutons stylisés.
    
    
      CROSSHATCH_FILL
      Fond avec hachures croisées (motif en X).
      Arrière-plans texturés.
    
    
      DOT_FILL
      Fond avec un motif de points (ex: poivré).
      Textures légères.
    
    
      PATTERN_FILL
      Fond avec un motif personnalisé (ex: damier, rayures).
      Arrière-plans complexes.
    
  





📌 Formes Spécifiques

  
    
      Style
      Description
      Exemple d'Utilisation
    
  
  
    
      TRIANGLE
      Forme triangulaire (3 points).
      Flèches, icônes.
    
    
      ARC
      Arc de cercle (portion de cercle).
      Jauges, indicateurs circulaires.
    
    
      PIE
      Secteur circulaire (camembert).
      Graphiques en camembert.
    
    
      ROUNDED_RECT
      Rectangle arrondi (alias pour ROUNDED_BORDER + RECTANGLE).
      Boutons modernes.
    
  





📌 Effets Visuels

  
    
      Style
      Description
      Exemple d'Utilisation
    
  
  
    
      METALIC
      Effet métallique (reflets, ombres pour simuler du métal).
      Boutons ou conteneurs "high-tech".
    
    
      GLASS
      Effet verre (transparence + reflets).
      Fenêtres, overlays.
    
    
      PLASTIC
      Effet plastique (surface lisse et légèrement brillante).
      Boutons ou interfaces modernes.
    
    
      NEON
      Effet néon (lueur colorée intense).
      Boutons ou textes mis en avant.
    
    
      GRADIENT_BORDER
      Bordure avec dégradé (ex: rouge → transparent).
      Boutons ou encadrements stylisés.
    
  





📌 Effets Dynamiques (Animations)

  
    
      Style
      Description
      Exemple d'Utilisation
    
  
  
    
      PULSE
      Pulsation (changement de taille ou d'opacité).
      Animations pour attirer l'attention.
    
    
      ANIMATED_GRADIENT
      Dégradé animé (couleurs défilantes).
      Arrière-plans dynamiques.
    
    
      ROTATE
      Rotation (pour icônes ou formes).
      Animations de chargement.
    
    
      SHAKE
      Secousse (mouvement rapide gauche/droite).
      Notifications ou erreurs.
    
    
      FADE_IN_OUT
      Fondu (apparition/disparition progressive).
      Transitions entre écrans.
    
  





📌 Structure WidgetStyle Mise à Jour
Voici la structure complète pour supporter toutes les combinaisons de styles :
cpp
Copier

typedef struct {
    uint64_t draw_style;  /**< Combinaison de flags WIDGET_DRAW_STYLE (uint64_t pour 64 bits). */

    // ===== Couleurs et Fond =====
    struct {
        uint16_t primary;   /**< Couleur principale (pour SOLID_FILL). */
        uint16_t secondary; /**< Couleur secondaire (pour dégradés). */
        uint16_t tertiary;  /**< Couleur tertiaire (pour CONIC_GRADIENT). */
    } colors;

    struct {
        float start_x;      /**< Position X de départ (pour dégradés linéaires). */
        float start_y;      /**< Position Y de départ. */
        float end_x;        /**< Position X de fin. */
        float end_y;        /**< Position Y de fin. */
        float angle;        /**< Angle du dégradé (pour DIAGONAL_GRADIENT). */
        struct {
            uint16_t center_x;  /**< Centre X (pour RADIAL_GRADIENT). */
            uint16_t center_y;  /**< Centre Y. */
            float radius;       /**< Rayon du dégradé radial. */
        } radial;
    } gradient;

    // ===== Bordure =====
    struct {
        uint8_t width;          /**< Épaisseur de la bordure. */
        uint16_t color;         /**< Couleur de la bordure (pour SOLID_BORDER). */
        struct {
            uint16_t start;     /**< Couleur de départ (pour GRADIENT_BORDER). */
            uint16_t end;       /**< Couleur de fin. */
        } gradient;
        union {
            uint8_t radius;     /**< Rayon uniforme (pour ROUNDED_BORDER). */
            struct {
                uint8_t top_left;     /**< Rayon du coin haut-gauche. */
                uint8_t top_right;    /**< Rayon du coin haut-droite. */
                uint8_t bottom_left;  /**< Rayon du coin bas-gauche. */
                uint8_t bottom_right; /**< Rayon du coin bas-droite. */
            } variable_radius;  /**< Rayons personnalisés (pour ROUNDED_VARIABLE). */
            struct {
                uint8_t on_length;   /**< Longueur des segments visibles (pour DOTTED/DASHED). */
                uint8_t off_length;  /**< Longueur des espaces. */
            } pattern;
        };
    } border;

    // ===== Formes Personnalisées =====
    struct {
        uint8_t num_points;     /**< Nombre de points (pour POLYGON, BEZIER_PATH). */
        Point* points;          /**< Pointeur vers les points (pour POLYGON, BEZIER_PATH). */
        uint8_t path_type;      /**< Type de chemin (0=POLYGON, 1=BEZIER_CUBIC, etc.). */
    } custom_shape;

    // ===== Effets Spéciaux =====
    struct {
        bool enabled;           /**< Si l'effet est activé. */
        uint8_t blur_radius;    /**< Rayon du flou (pour BLUR). */
        uint8_t offset_x;       /**< Décalage X (pour DROP_SHADOW). */
        uint8_t offset_y;       /**< Décalage Y. */
        uint16_t color;         /**< Couleur de l'effet (pour GLOW, NEON, etc.). */
        uint8_t intensity;      /**< Intensité de l'effet (0-255). */
    } effect;

    // ===== Formes Prédéfines =====
    struct {
        uint16_t radius;        /**< Rayon (pour CIRCLE). */
        struct {
            uint16_t a;         /**< Demi-grand axe (pour ELLIPSE). */
            uint16_t b;         /**< Demi-petit axe. */
        } ellipse;
        struct {
            uint16_t start_angle; /**< Angle de départ (pour ARC, PIE). */
            uint16_t end_angle;   /**< Angle de fin. */
        } arc;
    } shape;

    // ===== Animation =====
    struct {
        bool enabled;           /**< Si l'animation est activée. */
        uint16_t duration;      /**< Durée de l'animation (en ms). */
        uint16_t current_time;  /**< Temps écoulé depuis le début. */
        float progress;        /**< Progression (0.0 à 1.0). */
    } animation;
} WidgetStyle;




🚀 Exemples d'Utilisation avec Combinaisons

1. Bouton Arrondi avec Dégradé Horizontal et Ombre Portée
cpp
Copier

WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style =
    WIDGET_DRAW_STYLE_ROUNDED_BORDER |
    WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT |
    WIDGET_DRAW_STYLE_DROP_SHADOW;

// Configurer le dégradé
button->base.style.colors.primary = 0xF800;   // Rouge
button->base.style.colors.secondary = 0x001F; // Bleu
button->base.style.gradient.start_x = 0;
button->base.style.gradient.start_y = 0;
button->base.style.gradient.end_x = 1;
button->base.style.gradient.end_y = 0; // Dégradé horizontal

// Configurer la bordure arrondie
button->base.style.border.radius = 10;
button->base.style.border.width = 2;
button->base.style.border.color = 0xFFFF; // Blanc

// Configurer l'ombre
button->base.style.effect.enabled = true;
button->base.style.effect.offset_x = 2;
button->base.style.effect.offset_y = 2;
button->base.style.effect.color = 0x8410; // Gris
button->base.style.effect.blur_radius = 3;




2. Widget Transparent avec Bordure en Pointillés et Effet Néon
cpp
Copier

Widget* widget = new_widget(WIDGET_TYPE_VIEW);
widget->style.draw_style =
    WIDGET_DRAW_STYLE_TRANSPARENT_FILL |
    WIDGET_DRAW_STYLE_DOTTED_BORDER |
    WIDGET_DRAW_STYLE_NEON;

// Configurer la bordure en pointillés
widget->style.border.width = 1;
widget->style.border.color = 0x07E0; // Vert
widget->style.border.pattern.on_length = 2;
widget->style.border.pattern.off_length = 2;

// Configurer l'effet néon
widget->style.effect.enabled = true;
widget->style.effect.color = 0x07E0; // Vert (même couleur que la bordure)
widget->style.effect.intensity = 200; // Intensité élevée




3. Bouton Circulaire avec Dégradé Radial et Effet Métallique
cpp
Copier

WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style =
    WIDGET_DRAW_STYLE_CIRCLE |
    WIDGET_DRAW_STYLE_RADIAL_GRADIENT |
    WIDGET_DRAW_STYLE_METALIC;

// Configurer la forme circulaire
button->base.style.shape.radius = 30;

// Configurer le dégradé radial
button->base.style.colors.primary = 0xC618;   // Or
button->base.style.colors.secondary = 0x8410; // Gris
button->base.style.gradient.radial.center_x = 30;
button->base.style.gradient.radial.center_y = 30;
button->base.style.gradient.radial.radius = 30;

// Configurer l'effet métallique
button->base.style.effect.enabled = true;
button->base.style.effect.intensity = 150;




4. Widget avec Forme Polygonale et Fond Hachuré
cpp
Copier

// Définir un polygone (ex: hexagone)
Point hexagon_points[] = {
    {50, 10}, {70, 20}, {70, 40},
    {50, 50}, {30, 40}, {30, 20}
};

Widget* widget = new_widget(WIDGET_TYPE_CUSTOM);
widget->style.draw_style =
    WIDGET_DRAW_STYLE_POLYGON |
    WIDGET_DRAW_STYLE_HATCHED_FILL;

// Configurer le polygone
widget->style.custom_shape.num_points = 6;
widget->style.custom_shape.points = hexagon_points;
widget->style.custom_shape.path_type = 0; // 0 = POLYGON

// Configurer le fond hachuré
widget->style.colors.primary = 0xFFFF; // Blanc
widget->style.colors.secondary = 0x0000; // Noir




5. Bouton avec Animation de Pulsation et Effet de Verre
cpp
Copier

WidgetButton* button = new_widget(WIDGET_TYPE_BUTTON);
button->base.style.draw_style =
    WIDGET_DRAW_STYLE_ROUNDED_RECT |
    WIDGET_DRAW_STYLE_GLASS |
    WIDGET_DRAW_STYLE_PULSE;

// Configurer la forme
button->base.style.border.radius = 15;

// Configurer l'effet verre
button->base.style.effect.enabled = true;
button->base.style.effect.intensity = 100;
button->base.style.colors.primary = 0x07FF; // Cyan (effet verre)

// Configurer l'animation de pulsation
button->base.style.animation.enabled = true;
button->base.style.animation.duration = 1000; // 1 seconde
button->base.style.animation.progress = 0.0f; // Départ à 0%




📌 Fonctions Utilitaires pour Gérer les Styles

1. Macros pour les Combinaisons Fréquentes
cpp
Copier

// Combinaisons de styles courantes
#define STYLE_DEFAULT                   (WIDGET_DRAW_STYLE_SOLID_FILL | WIDGET_DRAW_STYLE_SOLID_BORDER | WIDGET_DRAW_STYLE_RECTANGLE)
#define STYLE_ROUNDED_BUTTON            (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT | WIDGET_DRAW_STYLE_DROP_SHADOW)
#define STYLE_METALIC_BUTTON             (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_METALIC | WIDGET_DRAW_STYLE_DROP_SHADOW)
#define STYLE_GLOW_CIRCLE                (WIDGET_DRAW_STYLE_CIRCLE | WIDGET_DRAW_STYLE_GLOW | WIDGET_DRAW_STYLE_SOLID_FILL)
#define STYLE_TRANSPARENT_DOTTED         (WIDGET_DRAW_STYLE_TRANSPARENT_FILL | WIDGET_DRAW_STYLE_DOTTED_BORDER)
#define STYLE_ANIMATED_GRADIENT_BUTTON   (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_ANIMATED_GRADIENT | WIDGET_DRAW_STYLE_GLOW)




2. Fonctions pour Manipuler les Styles
cpp
Copier

// Ajouter un style
void widget_add_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    widget->style.draw_style |= style;
}

// Supprimer un style
void widget_remove_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    widget->style.draw_style &= ~style;
}

// Vérifier si un widget a un style
bool widget_has_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    return (widget->style.draw_style & style) == style;
}

// Réinitialiser tous les styles
void widget_reset_style(Widget* widget) {
    widget->style.draw_style = STYLE_DEFAULT;
}

// Vérifier si un widget a un style de bordure
bool widget_has_border_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    return (widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK & style) == style;
}

// Vérifier si un widget a un style de fond
bool widget_has_fill_style(Widget* widget, WIDGET_DRAW_STYLE style) {
    return (widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK & style) == style;
}




🚀 Implémentation du Rendu avec Combinaisons
Voici comment adapter la fonction draw_widget pour gérer toutes les combinaisons de styles. Je vais me concentrer sur les cas les plus complexes.

1. Fonction Principale de Rendu
cpp
Copier

void draw_widget(Widget* widget) {
    // 1. Dessiner les effets qui doivent être en arrière-plan (ex: ombre portée)
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_DROP_SHADOW) {
        draw_drop_shadow(widget);
    }

    // 2. Dessiner le fond (selon le style de remplissage)
    draw_fill(widget);

    // 3. Dessiner la bordure (selon le style de bordure)
    draw_border(widget);

    // 4. Dessiner les effets qui doivent être au premier plan (ex: glow, neon)
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_GLOW) {
        draw_glow(widget);
    }
    if (widget->style.draw_style & WIDGET_DRAW_STYLE_NEON) {
        draw_neon_effect(widget);
    }

    // 5. Dessiner le contenu (texte, icône, etc.)
    if (widget->text.text[0] != '\0') {
        draw_text(widget);
    }
}




2. Dessiner le Fond (draw_fill)
cpp
Copier

void draw_fill(Widget* widget) {
    uint64_t fill_style = widget->style.draw_style & WIDGET_DRAW_STYLE_FILL_MASK;

    if (fill_style == WIDGET_DRAW_STYLE_TRANSPARENT_FILL) {
        return; // Rien à dessiner
    }

    if (fill_style == WIDGET_DRAW_STYLE_SOLID_FILL) {
        draw_solid_fill(widget);
    }
    else if (fill_style == WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT) {
        draw_horizontal_gradient(widget);
    }
    else if (fill_style == WIDGET_DRAW_STYLE_VERTICAL_GRADIENT) {
        draw_vertical_gradient(widget);
    }
    else if (fill_style == WIDGET_DRAW_STYLE_RADIAL_GRADIENT) {
        draw_radial_gradient(widget);
    }
    else if (fill_style == WIDGET_DRAW_STYLE_HATCHED_FILL) {
        draw_hatched_fill(widget);
    }
    else if (fill_style == WIDGET_DRAW_STYLE_CROSSHATCH_FILL) {
        draw_crosshatched_fill(widget);
    }
    else if (fill_style == WIDGET_DRAW_STYLE_DOT_FILL) {
        draw_dot_fill(widget);
    }
    else {
        // Style de fond non supporté : dessiner un fond uni
        draw_solid_fill(widget);
    }
}




3. Dessiner la Bordure (draw_border)
cpp
Copier

void draw_border(Widget* widget) {
    uint64_t border_style = widget->style.draw_style & WIDGET_DRAW_STYLE_BORDER_MASK;

    if (border_style == WIDGET_DRAW_STYLE_NO_BORDER) {
        return; // Aucune bordure
    }

    if (border_style & WIDGET_DRAW_STYLE_ROUNDED_BORDER) {
        if (border_style & WIDGET_DRAW_STYLE_DOTTED_BORDER) {
            draw_rounded_dotted_border(widget);
        }
        else if (border_style & WIDGET_DRAW_STYLE_DASHED_BORDER) {
            draw_rounded_dashed_border(widget);
        }
        else if (border_style & WIDGET_DRAW_STYLE_GRADIENT_BORDER) {
            draw_rounded_gradient_border(widget);
        }
        else {
            draw_rounded_border(widget);
        }
    }
    else if (border_style & WIDGET_DRAW_STYLE_DOTTED_BORDER) {
        draw_dotted_border(widget);
    }
    else if (border_style & WIDGET_DRAW_STYLE_DASHED_BORDER) {
        draw_dashed_border(widget);
    }
    else if (border_style & WIDGET_DRAW_STYLE_DOUBLE_BORDER) {
        draw_double_border(widget);
    }
    else if (border_style & WIDGET_DRAW_STYLE_ETCHED_BORDER) {
        draw_etched_border(widget);
    }
    else if (border_style & WIDGET_DRAW_STYLE_EMBOSSED_BORDER) {
        draw_embossed_border(widget);
    }
    else {
        // Bordure solide par défaut
        draw_solid_border(widget);
    }
}




4. Dessiner les Formes Personnalisées (draw_shape)
cpp
Copier

void draw_shape(Widget* widget) {
    uint64_t shape_style = widget->style.draw_style & WIDGET_DRAW_STYLE_SHAPE_MASK;

    if (shape_style == WIDGET_DRAW_STYLE_CIRCLE) {
        tft.fillCircle(widget->rect.x + widget->rect.width / 2,
                       widget->rect.y + widget->rect.height / 2,
                       widget->style.shape.radius,
                       widget->style.colors.primary);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_ELLIPSE) {
        draw_ellipse(widget);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_POLYGON) {
        draw_polygon(widget);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_BEZIER_PATH) {
        draw_bezier_path(widget);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_ARC) {
        draw_arc(widget);
    }
    else if (shape_style == WIDGET_DRAW_STYLE_PIE) {
        draw_pie(widget);
    }
    else {
        // Forme par défaut : rectangle
        draw_rectangle(widget);
    }
}




5. Dessiner les Effets Spéciaux
Ombre Portée (draw_drop_shadow)
cpp
Copier

void draw_drop_shadow(Widget* widget) {
    if (!widget->style.effect.enabled) return;

    uint8_t blur = widget->style.effect.blur_radius;
    uint8_t offset_x = widget->style.effect.offset_x;
    uint8_t offset_y = widget->style.effect.offset_y;
    uint16_t color = widget->style.effect.color;

    // Dessiner un rectangle flouté sous le widget
    for (uint8_t b = 0; b <= blur; b++) {
        uint8_t alpha = (200 * (blur - b)) / blur; // Opacité décroissante (0-200)
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




Effet Néon (draw_neon_effect)
cpp
Copier

void draw_neon_effect(Widget* widget) {
    if (!widget->style.effect.enabled) return;

    uint16_t color = widget->style.effect.color;
    uint8_t intensity = widget->style.effect.intensity;

    // Dessiner un halo lumineux autour du widget
    for (uint8_t r = 1; r <= 5; r++) {
        uint8_t alpha = intensity - (r * 20); // Réduire l'intensité avec la distance
        if (alpha <= 0) break;

        uint16_t neon_color = alpha_blend(0x0000, color, alpha);

        // Dessiner un rectangle légèrement plus grand
        tft.drawRoundRect(
            widget->rect.x - r,
            widget->rect.y - r,
            widget->rect.width + 2 * r,
            widget->rect.height + 2 * r,
            widget->style.border.radius + r,
            neon_color
        );
    }
}




Effet Métallique (draw_metallic_effect)
cpp
Copier

void draw_metallic_effect(Widget* widget) {
    if (!(widget->style.draw_style & WIDGET_DRAW_STYLE_METALIC)) return;

    // Dessiner un dégradé diagonal pour simuler des reflets
    uint16_t color1 = lighten_color(widget->style.colors.primary, 20); // +20% de luminosité
    uint16_t color2 = darken_color(widget->style.colors.primary, 20); // -20% de luminosité

    for (uint16_t i = 0; i < widget->rect.width; i++) {
        float ratio = (float)i / widget->rect.width;
        uint16_t color = interpolate_color(color1, color2, ratio);
        tft.drawFastVLine(widget->rect.x + i, widget->rect.y, widget->rect.height, color);
    }

    // Ajouter une bordure claire pour simuler un reflet
    tft.drawRoundRect(
        widget->rect.x + 1,
        widget->rect.y + 1,
        widget->rect.width - 2,
        widget->rect.height - 2,
        widget->style.border.radius - 1,
        0xFFFF // Blanc
    );
}

// Fonctions utilitaires pour éclaircir/assombrir une couleur
uint16_t lighten_color(uint16_t color, uint8_t percent) {
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;

    r = min(31, r + (31 - r) * percent / 100);
    g = min(63, g + (63 - g) * percent / 100);
    b = min(31, b + (31 - b) * percent / 100);

    return (r << 11) | (g << 5) | b;
}

uint16_t darken_color(uint16_t color, uint8_t percent) {
    uint8_t r = (color >> 11) & 0x1F;
    uint8_t g = (color >> 5) & 0x3F;
    uint8_t b = color & 0x1F;

    r = r * (100 - percent) / 100;
    g = g * (100 - percent) / 100;
    b = b * (100 - percent) / 100;

    return (r << 11) | (g << 5) | b;
}




💡 Optimisations pour ESP8266

1. Précalcul des Dégradés
Les dégradés peuvent être coûteux en calcul si recalculés à chaque rendu. Précalculez-les dans un tableau si possible :
cpp
Copier

// Tableau pour stocker les couleurs précalculées d'un dégradé
uint16_t gradient_cache[320]; // Pour un écran de 320px de large

void precompute_gradient(Widget* widget) {
    uint16_t width = widget->rect.width;
    uint16_t color1 = widget->style.colors.primary;
    uint16_t color2 = widget->style.colors.secondary;

    for (uint16_t i = 0; i < width; i++) {
        float ratio = (float)i / (width - 1);
        gradient_cache[i] = interpolate_color(color1, color2, ratio);
    }
}

void draw_horizontal_gradient(Widget* widget) {
    precompute_gradient(widget);
    for (uint16_t i = 0; i < widget->rect.width; i++) {
        tft.drawFastVLine(
            widget->rect.x + i,
            widget->rect.y,
            widget->rect.height,
            gradient_cache[i]
        );
    }
}




2. Pool de Formes Personnalisées
Pour éviter d'allouer dynamiquement des tableaux de points (pour POLYGON ou BEZIER_PATH), utilisez un pool statique :
cpp
Copier

#define MAX_CUSTOM_SHAPES 10
#define MAX_POINTS_PER_SHAPE 20

Point shape_pool[MAX_CUSTOM_SHAPES][MAX_POINTS_PER_SHAPE];
uint8_t shape_pool_index = 0;

Point* allocate_shape_points(uint8_t num_points) {
    if (shape_pool_index >= MAX_CUSTOM_SHAPES) return NULL;
    if (num_points > MAX_POINTS_PER_SHAPE) return NULL;
    return shape_pool[shape_pool_index++];
}




3. Cache des Styles Fréquents
Stockez les styles fréquemment utilisés dans un cache pour éviter de recalculer leurs paramètres :
cpp
Copier

#define MAX_CACHED_STYLES 20

typedef struct {
    uint64_t style;
    WidgetStyle cached_style;
} CachedStyle;

CachedStyle style_cache[MAX_CACHED_STYLES];
uint8_t style_cache_count = 0;

WidgetStyle* get_cached_style(uint64_t style) {
    for (uint8_t i = 0; i < style_cache_count; i++) {
        if (style_cache[i].style == style) {
            return &style_cache[i].cached_style;
        }
    }
    return NULL;
}

void cache_style(uint64_t style, WidgetStyle* src_style) {
    if (style_cache_count >= MAX_CACHED_STYLES) return;
    style_cache[style_cache_count].style = style;
    style_cache[style_cache_count].cached_style = *src_style;
    style_cache_count++;
}




4. Macros pour les Styles Fréquents
Définissez des macros pour les styles les plus utilisés afin de simplifier le code et réduire les erreurs :
cpp
Copier

// Styles de base
#define STYLE_SOLID_RED_FILL            (WIDGET_DRAW_STYLE_SOLID_FILL | WIDGET_DRAW_STYLE_RECTANGLE)
#define STYLE_ROUNDED_BUTTON            (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_HORIZONTAL_GRADIENT | WIDGET_DRAW_STYLE_DROP_SHADOW)
#define STYLE_METALIC_BUTTON            (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_METALIC | WIDGET_DRAW_STYLE_GLOW)
#define STYLE_GLASS_PANEL               (WIDGET_DRAW_STYLE_RECTANGLE | WIDGET_DRAW_STYLE_GLASS | WIDGET_DRAW_STYLE_DROP_SHADOW)
#define STYLE_NEON_BUTTON               (WIDGET_DRAW_STYLE_CIRCLE | WIDGET_DRAW_STYLE_NEON | WIDGET_DRAW_STYLE_SOLID_FILL)

// Styles avec animations
#define STYLE_PULSING_BUTTON           (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_PULSE | WIDGET_DRAW_STYLE_SOLID_FILL)
#define STYLE_ANIMATED_GRADIENT_BUTTON  (WIDGET_DRAW_STYLE_ROUNDED_BORDER | WIDGET_DRAW_STYLE_ANIMATED_GRADIENT | WIDGET_DRAW_STYLE_GLOW)




❓ Besoin d'Ajustements ?
Si vous souhaitez :

Ajouter d'autres styles (ex: WIDGET_DRAW_STYLE_WOOD pour un effet bois),
Simplifier la gestion des combinaisons (ex: limiter le nombre de styles combinables),
Optimiser pour un cas d'usage spécifique (ex: prioriser les performances pour les animations),
Adapter à une bibliothèque de rendu spécifique (autre que TFT_eSPI),
Dites-le-moi ! Je peux affiner cette solution pour répondre exactement à vos besoins. 🛠️





10:00

