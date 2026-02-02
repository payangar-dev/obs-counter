// =============================================================================
// CounterDock - Implémentation
// =============================================================================
// Ce fichier contient l'implémentation de toutes les méthodes du dock.
// =============================================================================

#include "counter-dock.hpp"

#include <obs-module.h>      // API principale OBS (hotkeys, logging, etc.)
#include <obs-frontend-api.h> // API frontend OBS (interface utilisateur)

#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QFont>
#include <QFrame>
#include <QSpacerItem>

// =============================================================================
// Callbacks pour les hotkeys
// =============================================================================
// Les hotkeys OBS utilisent des callbacks en C pur (pas de méthodes C++).
// On passe le pointeur vers le dock via le paramètre 'data', puis on le
// convertit (cast) pour appeler les méthodes de l'objet.
//
// Paramètres des callbacks:
// - data: pointeur utilisateur (notre dock)
// - id: identifiant de la hotkey
// - hotkey: structure décrivant la hotkey
// - pressed: true si la touche est enfoncée, false si relâchée
// =============================================================================

static void hotkeyAdd5Callback(void *data, obs_hotkey_id id,
                                obs_hotkey_t *hotkey, bool pressed)
{
    // On ignore les paramètres non utilisés pour éviter les warnings
    UNUSED_PARAMETER(id);
    UNUSED_PARAMETER(hotkey);

    // On n'agit que quand la touche est ENFONCÉE (pas au relâchement)
    if (pressed) {
        // Cast du pointeur void* vers CounterDock*
        CounterDock *dock = static_cast<CounterDock*>(data);
        dock->addToCounter(5);
    }
}

static void hotkeyAdd10Callback(void *data, obs_hotkey_id id,
                                 obs_hotkey_t *hotkey, bool pressed)
{
    UNUSED_PARAMETER(id);
    UNUSED_PARAMETER(hotkey);
    if (pressed) {
        CounterDock *dock = static_cast<CounterDock*>(data);
        dock->addToCounter(10);
    }
}

static void hotkeyAdd15Callback(void *data, obs_hotkey_id id,
                                 obs_hotkey_t *hotkey, bool pressed)
{
    UNUSED_PARAMETER(id);
    UNUSED_PARAMETER(hotkey);
    if (pressed) {
        CounterDock *dock = static_cast<CounterDock*>(data);
        dock->addToCounter(15);
    }
}

static void hotkeySubtract1Callback(void *data, obs_hotkey_id id,
                                     obs_hotkey_t *hotkey, bool pressed)
{
    UNUSED_PARAMETER(id);
    UNUSED_PARAMETER(hotkey);
    if (pressed) {
        CounterDock *dock = static_cast<CounterDock*>(data);
        dock->addToCounter(-1);
    }
}

static void hotkeyResetCallback(void *data, obs_hotkey_id id,
                                 obs_hotkey_t *hotkey, bool pressed)
{
    UNUSED_PARAMETER(id);
    UNUSED_PARAMETER(hotkey);
    if (pressed) {
        CounterDock *dock = static_cast<CounterDock*>(data);
        dock->resetCounter();
    }
}

// =============================================================================
// Constructeur
// =============================================================================

CounterDock::CounterDock(QWidget *parent)
    : QDockWidget(parent)       // Appelle le constructeur parent
    , counterValue(0)           // Initialise le compteur à 0
    , showPrefix(false)         // Par défaut, affiche juste le nombre
    , prefix("Pompes : ")       // Préfixe par défaut
    , hotkeyAdd5(OBS_INVALID_HOTKEY_ID)      // Initialise les IDs de hotkeys
    , hotkeyAdd10(OBS_INVALID_HOTKEY_ID)     // à une valeur invalide
    , hotkeyAdd15(OBS_INVALID_HOTKEY_ID)
    , hotkeySubtract1(OBS_INVALID_HOTKEY_ID)
    , hotkeyReset(OBS_INVALID_HOTKEY_ID)
{
    // Titre du dock (affiché dans la barre de titre et le menu Vue → Docks)
    setWindowTitle("Compteur");

    // Permet d'identifier le dock de manière unique dans OBS
    setObjectName("CounterDock");

    // Configure quelles bordures du dock peuvent être utilisées pour l'ancrage
    setAllowedAreas(Qt::AllDockWidgetAreas);

    // Charge les paramètres sauvegardés (préfixe, format, etc.)
    loadSettings();

    // Crée l'interface utilisateur
    setupUI();

    // Met à jour l'affichage initial
    updateDisplay();

    // Sauvegarde initiale dans le fichier (crée le fichier si nécessaire)
    saveToFile();
}

// =============================================================================
// Destructeur
// =============================================================================

CounterDock::~CounterDock()
{
    // Sauvegarde les paramètres avant la fermeture
    saveSettings();

    // Les hotkeys sont désenregistrées dans unregisterHotkeys()
    // qui est appelé par le plugin avant la destruction
}

// =============================================================================
// Configuration de l'interface utilisateur
// =============================================================================

void CounterDock::setupUI()
{
    // -------------------------------------------------------------------------
    // Widget central
    // -------------------------------------------------------------------------
    // Un QDockWidget a besoin d'un widget central qui contient tout le contenu

    QWidget *centralWidget = new QWidget(this);
    setWidget(centralWidget);

    // Layout principal vertical - empile les éléments de haut en bas
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);  // Marges extérieures
    mainLayout->setSpacing(10);                       // Espacement entre éléments

    // -------------------------------------------------------------------------
    // Affichage du compteur
    // -------------------------------------------------------------------------

    counterDisplay = new QLabel(this);
    counterDisplay->setAlignment(Qt::AlignCenter);

    // Style CSS pour un affichage grand et visible
    counterDisplay->setStyleSheet(
        "QLabel {"
        "  font-size: 48px;"
        "  font-weight: bold;"
        "  color: #2196F3;"          // Bleu Material Design
        "  background-color: #1a1a1a;"
        "  border: 2px solid #333;"
        "  border-radius: 8px;"
        "  padding: 20px;"
        "  min-height: 60px;"
        "}"
    );

    mainLayout->addWidget(counterDisplay);

    // -------------------------------------------------------------------------
    // Boutons d'ajout (+5, +10, +15)
    // -------------------------------------------------------------------------

    QHBoxLayout *addButtonsLayout = new QHBoxLayout();
    addButtonsLayout->setSpacing(5);

    // Style commun pour les boutons d'ajout (vert)
    QString addButtonStyle =
        "QPushButton {"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "  color: white;"
        "  background-color: #4CAF50;"  // Vert
        "  border: none;"
        "  border-radius: 5px;"
        "  padding: 10px 15px;"
        "  min-width: 50px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #45a049;"   // Vert plus foncé au survol
        "}"
        "QPushButton:pressed {"
        "  background-color: #3d8b40;"   // Encore plus foncé au clic
        "}";

    btnAdd5 = new QPushButton("+5", this);
    btnAdd5->setStyleSheet(addButtonStyle);
    connect(btnAdd5, &QPushButton::clicked, this, &CounterDock::onAdd5Clicked);
    addButtonsLayout->addWidget(btnAdd5);

    btnAdd10 = new QPushButton("+10", this);
    btnAdd10->setStyleSheet(addButtonStyle);
    connect(btnAdd10, &QPushButton::clicked, this, &CounterDock::onAdd10Clicked);
    addButtonsLayout->addWidget(btnAdd10);

    btnAdd15 = new QPushButton("+15", this);
    btnAdd15->setStyleSheet(addButtonStyle);
    connect(btnAdd15, &QPushButton::clicked, this, &CounterDock::onAdd15Clicked);
    addButtonsLayout->addWidget(btnAdd15);

    mainLayout->addLayout(addButtonsLayout);

    // -------------------------------------------------------------------------
    // Boutons de contrôle (-1 et Reset)
    // -------------------------------------------------------------------------

    QHBoxLayout *controlButtonsLayout = new QHBoxLayout();
    controlButtonsLayout->setSpacing(5);

    // Style pour le bouton -1 (orange)
    QString subtractButtonStyle =
        "QPushButton {"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  color: white;"
        "  background-color: #FF9800;"  // Orange
        "  border: none;"
        "  border-radius: 5px;"
        "  padding: 8px 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #F57C00;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #E65100;"
        "}";

    btnSubtract1 = new QPushButton("-1", this);
    btnSubtract1->setStyleSheet(subtractButtonStyle);
    btnSubtract1->setToolTip("Corrige une erreur (-1)");
    connect(btnSubtract1, &QPushButton::clicked, this, &CounterDock::onSubtract1Clicked);
    controlButtonsLayout->addWidget(btnSubtract1);

    // Style pour le bouton Reset (rouge)
    QString resetButtonStyle =
        "QPushButton {"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  color: white;"
        "  background-color: #f44336;"  // Rouge
        "  border: none;"
        "  border-radius: 5px;"
        "  padding: 8px 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #da190b;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #b71c1c;"
        "}";

    btnReset = new QPushButton("Reset", this);
    btnReset->setStyleSheet(resetButtonStyle);
    connect(btnReset, &QPushButton::clicked, this, &CounterDock::onResetClicked);
    controlButtonsLayout->addWidget(btnReset);

    mainLayout->addLayout(controlButtonsLayout);

    // -------------------------------------------------------------------------
    // Section configuration du format
    // -------------------------------------------------------------------------

    QGroupBox *formatGroup = new QGroupBox("Format d'affichage", this);
    QVBoxLayout *formatLayout = new QVBoxLayout(formatGroup);

    // Case à cocher pour activer/désactiver le préfixe
    chkShowPrefix = new QCheckBox("Afficher un préfixe", this);
    chkShowPrefix->setChecked(showPrefix);
    connect(chkShowPrefix, &QCheckBox::toggled, this, &CounterDock::onFormatChanged);
    formatLayout->addWidget(chkShowPrefix);

    // Champ texte pour le préfixe personnalisé
    QHBoxLayout *prefixLayout = new QHBoxLayout();
    QLabel *prefixLabel = new QLabel("Préfixe :", this);
    txtPrefix = new QLineEdit(prefix, this);
    txtPrefix->setPlaceholderText("Ex: Pompes : ");
    txtPrefix->setEnabled(showPrefix);  // Désactivé si le préfixe n'est pas affiché
    connect(txtPrefix, &QLineEdit::textChanged, this, &CounterDock::onPrefixChanged);
    prefixLayout->addWidget(prefixLabel);
    prefixLayout->addWidget(txtPrefix);
    formatLayout->addLayout(prefixLayout);

    mainLayout->addWidget(formatGroup);

    // -------------------------------------------------------------------------
    // Chemin du fichier (informatif)
    // -------------------------------------------------------------------------

    QGroupBox *fileGroup = new QGroupBox("Fichier de sortie", this);
    QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);

    lblFilePath = new QLabel(this);
    lblFilePath->setWordWrap(true);
    lblFilePath->setStyleSheet("color: #888; font-size: 11px;");
    lblFilePath->setText(getFilePath());
    fileLayout->addWidget(lblFilePath);

    QLabel *helpLabel = new QLabel(
        "Ajoutez ce fichier comme source 'Texte (GDI+)' dans OBS",
        this
    );
    helpLabel->setStyleSheet("color: #666; font-size: 10px; font-style: italic;");
    helpLabel->setWordWrap(true);
    fileLayout->addWidget(helpLabel);

    mainLayout->addWidget(fileGroup);

    // Ajoute un espace extensible en bas pour pousser tout vers le haut
    mainLayout->addStretch();

    // Taille minimale du dock
    setMinimumWidth(200);
    setMinimumHeight(300);
}

// =============================================================================
// Gestion du compteur
// =============================================================================

void CounterDock::addToCounter(int amount)
{
    counterValue += amount;

    // Empêche les valeurs négatives
    if (counterValue < 0) {
        counterValue = 0;
    }

    updateDisplay();
    saveToFile();
}

void CounterDock::resetCounter()
{
    counterValue = 0;
    updateDisplay();
    saveToFile();
}

// =============================================================================
// Mise à jour de l'affichage
// =============================================================================

void CounterDock::updateDisplay()
{
    QString displayText;

    if (showPrefix && !prefix.isEmpty()) {
        displayText = prefix + QString::number(counterValue);
    } else {
        displayText = QString::number(counterValue);
    }

    counterDisplay->setText(displayText);
}

// =============================================================================
// Gestion du fichier
// =============================================================================

QString CounterDock::getFilePath()
{
    // Le fichier est sauvegardé dans le même dossier que le plugin OBS
    // On utilise le dossier de configuration OBS pour la portabilité
    QString configPath = QDir::homePath() + "/.config/obs-studio/plugins/obs-counter-dock";

    // Crée le dossier s'il n'existe pas
    QDir().mkpath(configPath);

    return configPath + "/counter.txt";
}

void CounterDock::saveToFile()
{
    QString filePath = getFilePath();
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);

        // Écrit le contenu selon le format choisi
        if (showPrefix && !prefix.isEmpty()) {
            stream << prefix << counterValue;
        } else {
            stream << counterValue;
        }

        file.close();

        // Log pour le débogage (visible dans le journal OBS)
        blog(LOG_DEBUG, "[Counter Dock] Saved to file: %s", filePath.toUtf8().constData());
    } else {
        // Erreur d'écriture - log l'erreur
        blog(LOG_WARNING, "[Counter Dock] Failed to save to file: %s",
             filePath.toUtf8().constData());
    }
}

// =============================================================================
// Gestion des paramètres (persistance)
// =============================================================================

void CounterDock::loadSettings()
{
    // QSettings sauvegarde dans ~/.config/obs-studio/plugins/obs-counter-dock/
    QSettings settings(
        QDir::homePath() + "/.config/obs-studio/plugins/obs-counter-dock/settings.ini",
        QSettings::IniFormat
    );

    counterValue = settings.value("counter/value", 0).toInt();
    showPrefix = settings.value("format/showPrefix", false).toBool();
    prefix = settings.value("format/prefix", "Pompes : ").toString();
}

void CounterDock::saveSettings()
{
    QSettings settings(
        QDir::homePath() + "/.config/obs-studio/plugins/obs-counter-dock/settings.ini",
        QSettings::IniFormat
    );

    settings.setValue("counter/value", counterValue);
    settings.setValue("format/showPrefix", showPrefix);
    settings.setValue("format/prefix", prefix);
    settings.sync();  // Force l'écriture immédiate sur le disque
}

// =============================================================================
// Slots (gestionnaires d'événements)
// =============================================================================

void CounterDock::onAdd5Clicked()
{
    addToCounter(5);
}

void CounterDock::onAdd10Clicked()
{
    addToCounter(10);
}

void CounterDock::onAdd15Clicked()
{
    addToCounter(15);
}

void CounterDock::onSubtract1Clicked()
{
    addToCounter(-1);
}

void CounterDock::onResetClicked()
{
    resetCounter();
}

void CounterDock::onFormatChanged()
{
    showPrefix = chkShowPrefix->isChecked();
    txtPrefix->setEnabled(showPrefix);  // Active/désactive le champ préfixe
    updateDisplay();
    saveToFile();
    saveSettings();
}

void CounterDock::onPrefixChanged()
{
    prefix = txtPrefix->text();
    updateDisplay();
    saveToFile();
    saveSettings();
}

// =============================================================================
// Gestion des hotkeys OBS
// =============================================================================

void CounterDock::registerHotkeys()
{
    // obs_hotkey_register_frontend enregistre une hotkey globale
    // Les paramètres sont :
    // - nom unique (utilisé pour sauvegarder/restaurer le raccourci)
    // - description (affichée dans Paramètres → Raccourcis)
    // - callback (fonction appelée quand la hotkey est pressée)
    // - data (pointeur passé au callback - ici, notre dock)

    hotkeyAdd5 = obs_hotkey_register_frontend(
        "counter_add_5",                    // Nom unique
        "Compteur: +5",                     // Description
        hotkeyAdd5Callback,                 // Callback
        this                                // Données (pointeur vers le dock)
    );

    hotkeyAdd10 = obs_hotkey_register_frontend(
        "counter_add_10",
        "Compteur: +10",
        hotkeyAdd10Callback,
        this
    );

    hotkeyAdd15 = obs_hotkey_register_frontend(
        "counter_add_15",
        "Compteur: +15",
        hotkeyAdd15Callback,
        this
    );

    hotkeySubtract1 = obs_hotkey_register_frontend(
        "counter_subtract_1",
        "Compteur: -1",
        hotkeySubtract1Callback,
        this
    );

    hotkeyReset = obs_hotkey_register_frontend(
        "counter_reset",
        "Compteur: Reset",
        hotkeyResetCallback,
        this
    );

    blog(LOG_INFO, "[Counter Dock] Hotkeys registered");
}

void CounterDock::unregisterHotkeys()
{
    // Libère les hotkeys pour éviter les fuites de mémoire
    // et permettre leur réenregistrement si le plugin est rechargé

    if (hotkeyAdd5 != OBS_INVALID_HOTKEY_ID) {
        obs_hotkey_unregister(hotkeyAdd5);
    }
    if (hotkeyAdd10 != OBS_INVALID_HOTKEY_ID) {
        obs_hotkey_unregister(hotkeyAdd10);
    }
    if (hotkeyAdd15 != OBS_INVALID_HOTKEY_ID) {
        obs_hotkey_unregister(hotkeyAdd15);
    }
    if (hotkeySubtract1 != OBS_INVALID_HOTKEY_ID) {
        obs_hotkey_unregister(hotkeySubtract1);
    }
    if (hotkeyReset != OBS_INVALID_HOTKEY_ID) {
        obs_hotkey_unregister(hotkeyReset);
    }

    blog(LOG_INFO, "[Counter Dock] Hotkeys unregistered");
}
