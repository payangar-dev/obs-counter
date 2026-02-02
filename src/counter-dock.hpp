#ifndef COUNTER_DOCK_HPP
#define COUNTER_DOCK_HPP

// =============================================================================
// CounterDock - Dock OBS pour compteur configurable
// =============================================================================
// Ce fichier déclare la classe CounterDock, un widget Qt qui s'intègre
// comme "dock" (panneau ancrable) dans l'interface d'OBS.
//
// Un dock peut être :
// - Ancré sur les côtés de la fenêtre OBS
// - Détaché comme fenêtre flottante
// - Masqué/affiché via le menu Vue → Docks
// =============================================================================

#include <QDockWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileInfo>
#include <QDir>
#include <QString>

// Inclut le header OBS pour le type obs_hotkey_id
// obs_hotkey_id est un typedef (alias de type), pas une structure,
// donc on ne peut pas faire de forward declaration
#include <obs.h>

// =============================================================================
// Classe CounterDock
// =============================================================================
// Hérite de QDockWidget pour pouvoir être intégré comme dock OBS.
// La macro Q_OBJECT est OBLIGATOIRE pour utiliser les signaux/slots Qt.
// =============================================================================

class CounterDock : public QDockWidget {
    Q_OBJECT  // Macro Qt - active le système de signaux/slots et l'introspection

public:
    // Constructeur - parent est la fenêtre principale OBS
    explicit CounterDock(QWidget *parent = nullptr);

    // Destructeur - libère les ressources (hotkeys, etc.)
    ~CounterDock();

    // =========================================================================
    // Méthodes publiques pour les hotkeys
    // =========================================================================
    // Ces méthodes sont appelées par les callbacks des hotkeys OBS.
    // Elles sont publiques car les callbacks sont des fonctions C externes.

    void addToCounter(int amount);  // Ajoute 'amount' au compteur
    void resetCounter();             // Remet le compteur à zéro

    // =========================================================================
    // Gestion des hotkeys OBS
    // =========================================================================
    // Enregistre les hotkeys auprès d'OBS (appelé après le chargement du plugin)
    void registerHotkeys();

    // Libère les hotkeys (appelé avant le déchargement du plugin)
    void unregisterHotkeys();

private slots:
    // =========================================================================
    // Slots Qt - méthodes appelées en réponse aux signaux (clics, etc.)
    // =========================================================================
    // Le mot-clé "slots" est une extension Qt. Ces méthodes peuvent être
    // connectées aux signaux des widgets (ex: bouton cliqué → slot appelé)

    void onAdd5Clicked();
    void onAdd10Clicked();
    void onAdd15Clicked();
    void onResetClicked();
    void onSubtract1Clicked();   // Bouton -1 pour corriger les erreurs
    void onFormatChanged();       // Quand le format d'affichage change
    void onPrefixChanged();       // Quand le préfixe personnalisé change

private:
    // =========================================================================
    // Méthodes privées
    // =========================================================================

    void setupUI();           // Crée et organise tous les widgets
    void updateDisplay();     // Met à jour l'affichage du compteur
    void saveToFile();        // Sauvegarde le compteur dans le fichier texte
    void loadSettings();      // Charge les paramètres sauvegardés
    void saveSettings();      // Sauvegarde les paramètres
    QString getFilePath();    // Retourne le chemin complet du fichier counter.txt

    // =========================================================================
    // Widgets de l'interface
    // =========================================================================
    // Pointeurs vers les widgets Qt. Ils seront créés dans setupUI().

    QLabel *counterDisplay;      // Affiche la valeur actuelle du compteur
    QPushButton *btnAdd5;        // Bouton +5
    QPushButton *btnAdd10;       // Bouton +10
    QPushButton *btnAdd15;       // Bouton +15
    QPushButton *btnSubtract1;   // Bouton -1 (correction)
    QPushButton *btnReset;       // Bouton Reset

    QCheckBox *chkShowPrefix;    // Case à cocher "Afficher le préfixe"
    QLineEdit *txtPrefix;        // Champ texte pour le préfixe personnalisé
    QLabel *lblFilePath;         // Affiche le chemin du fichier

    // =========================================================================
    // Données du compteur
    // =========================================================================

    int counterValue;            // Valeur actuelle du compteur
    bool showPrefix;             // Afficher le préfixe ou juste le nombre
    QString prefix;              // Préfixe personnalisable (ex: "Pompes : ")

    // =========================================================================
    // Hotkeys OBS
    // =========================================================================
    // Les hotkeys OBS sont identifiées par des IDs uniques.
    // obs_hotkey_id est un type défini par OBS (généralement un entier).

    obs_hotkey_id hotkeyAdd5;
    obs_hotkey_id hotkeyAdd10;
    obs_hotkey_id hotkeyAdd15;
    obs_hotkey_id hotkeySubtract1;
    obs_hotkey_id hotkeyReset;
};

#endif // COUNTER_DOCK_HPP
