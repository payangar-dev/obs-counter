// =============================================================================
// Plugin OBS - Point d'entrée principal
// =============================================================================
// Ce fichier est le point d'entrée du plugin OBS. OBS appelle les fonctions
// définies ici lors du chargement et du déchargement du plugin.
//
// Un plugin OBS doit exporter plusieurs fonctions standard :
// - obs_module_load()   : Appelée au chargement du plugin
// - obs_module_unload() : Appelée au déchargement du plugin
// - obs_module_name()   : Retourne le nom du plugin
// =============================================================================

#include <obs-module.h>       // Macros et fonctions principales OBS
#include <obs-frontend-api.h> // API frontend (docks, menus, etc.)

#include <QMainWindow>        // Fenêtre principale Qt
#include <QAction>            // Actions pour les menus

#include "counter-dock.hpp"

// =============================================================================
// Macro OBS_DECLARE_MODULE
// =============================================================================
// Cette macro génère automatiquement les fonctions d'export nécessaires
// pour que OBS reconnaisse ce fichier .so comme un plugin valide.
// Elle définit notamment la version de l'API OBS utilisée.

OBS_DECLARE_MODULE()

// =============================================================================
// Variables globales du module
// =============================================================================
// On garde un pointeur vers le dock pour pouvoir le détruire proprement
// lors du déchargement du plugin.

static CounterDock *counterDock = nullptr;

// =============================================================================
// obs_module_name - Nom du plugin
// =============================================================================
// Retourne le nom affiché dans la liste des plugins OBS
// (Aide → À propos → Plugins)

const char *obs_module_name(void)
{
    return "Counter Dock";
}

// =============================================================================
// obs_module_description - Description du plugin
// =============================================================================
// Description affichée dans les informations du plugin

const char *obs_module_description(void)
{
    return "Dock compteur configurable avec boutons +5, +10, +15 et Reset. "
           "Sauvegarde dans un fichier texte pour affichage en overlay.";
}

// =============================================================================
// obs_module_load - Chargement du plugin
// =============================================================================
// Cette fonction est appelée par OBS quand le plugin est chargé.
// C'est ici qu'on initialise tout : création du dock, enregistrement
// des hotkeys, etc.
//
// Retourne :
// - true si le chargement a réussi
// - false si le plugin ne peut pas démarrer

bool obs_module_load(void)
{
    // Log pour le débogage - visible dans Aide → Fichiers journaux
    blog(LOG_INFO, "[Counter Dock] Loading plugin...");

    // -------------------------------------------------------------------------
    // Récupération de la fenêtre principale OBS
    // -------------------------------------------------------------------------
    // On a besoin de la fenêtre principale pour :
    // 1. L'utiliser comme parent du dock (gestion mémoire Qt)
    // 2. Ajouter le dock à l'interface OBS

    QMainWindow *mainWindow = static_cast<QMainWindow*>(
        obs_frontend_get_main_window()
    );

    if (!mainWindow) {
        blog(LOG_ERROR, "[Counter Dock] Failed to get main window!");
        return false;
    }

    // -------------------------------------------------------------------------
    // Création du dock
    // -------------------------------------------------------------------------

    counterDock = new CounterDock(mainWindow);

    // -------------------------------------------------------------------------
    // Ajout du dock à OBS
    // -------------------------------------------------------------------------
    // obs_frontend_add_dock ajoute notre QDockWidget à la fenêtre principale.
    // Le dock apparaîtra dans le menu Vue → Docks.

#if LIBOBS_API_VER >= MAKE_SEMANTIC_VERSION(30, 0, 0)
    // OBS 30+ utilise une nouvelle API pour les docks
    obs_frontend_add_dock_by_id("CounterDock", "Compteur", counterDock);
#else
    // Ancienne API pour OBS < 30
    obs_frontend_add_dock(counterDock);
#endif

    // -------------------------------------------------------------------------
    // Enregistrement des hotkeys
    // -------------------------------------------------------------------------
    // Les hotkeys permettent de contrôler le compteur au clavier,
    // même quand la fenêtre OBS n'a pas le focus (si l'option est activée).

    counterDock->registerHotkeys();

    blog(LOG_INFO, "[Counter Dock] Plugin loaded successfully!");
    return true;
}

// =============================================================================
// obs_module_unload - Déchargement du plugin
// =============================================================================
// Appelée quand OBS se ferme ou quand le plugin est désactivé.
// On libère toutes les ressources allouées.

void obs_module_unload(void)
{
    blog(LOG_INFO, "[Counter Dock] Unloading plugin...");

    if (counterDock) {
        // Désenregistre les hotkeys AVANT de détruire le dock
        // (sinon les callbacks pointeraient vers de la mémoire libérée)
        counterDock->unregisterHotkeys();

        // Note: Le dock est automatiquement supprimé par Qt
        // car il est enfant de la fenêtre principale.
        // On met juste le pointeur à nullptr pour éviter les accès invalides.
        counterDock = nullptr;
    }

    blog(LOG_INFO, "[Counter Dock] Plugin unloaded");
}
