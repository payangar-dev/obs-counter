; =============================================================================
; OBS Counter Dock - Installateur NSIS
; =============================================================================
; Ce script crée un installateur Windows pour le plugin OBS Counter Dock.
; Il installe le plugin dans le dossier utilisateur OBS et crée une entrée
; de désinstallation dans "Programmes et fonctionnalités".
; =============================================================================

!include "MUI2.nsh"
!include "FileFunc.nsh"

; -----------------------------------------------------------------------------
; Informations générales
; -----------------------------------------------------------------------------
!define PRODUCT_NAME "OBS Counter Dock"
!define PRODUCT_VERSION "1.0.0"
!define PRODUCT_PUBLISHER "Pierre"
!define PRODUCT_WEB_SITE "https://github.com/pierre/push-up-counter"
!define PLUGIN_NAME "obs-counter-dock"

Name "${PRODUCT_NAME}"
OutFile "obs-counter-dock-installer.exe"
InstallDir "$APPDATA\obs-studio\plugins\${PLUGIN_NAME}"
ShowInstDetails show
ShowUnInstDetails show

; Demande les droits utilisateur (pas admin nécessaire pour %APPDATA%)
RequestExecutionLevel user

; -----------------------------------------------------------------------------
; Interface Modern UI
; -----------------------------------------------------------------------------
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Pages d'installation
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Pages de désinstallation
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Langue française
!insertmacro MUI_LANGUAGE "French"

; -----------------------------------------------------------------------------
; Textes personnalisés
; -----------------------------------------------------------------------------
LangString MUI_TEXT_WELCOME_INFO_TITLE ${LANG_FRENCH} "Installation de ${PRODUCT_NAME}"
LangString MUI_TEXT_WELCOME_INFO_TEXT ${LANG_FRENCH} "Cet assistant va installer ${PRODUCT_NAME} ${PRODUCT_VERSION} sur votre ordinateur.$\r$\n$\r$\nLe plugin sera installé dans le dossier plugins d'OBS Studio.$\r$\n$\r$\nCliquez sur Suivant pour continuer."

; -----------------------------------------------------------------------------
; Section d'installation
; -----------------------------------------------------------------------------
Section "Installation" SecInstall
    SetOutPath "$INSTDIR\bin\64bit"

    ; Copie le fichier DLL du plugin
    File "obs-counter-dock.dll"

    ; Crée le désinstallateur
    WriteUninstaller "$INSTDIR\uninstall.exe"

    ; Enregistre dans le registre pour "Programmes et fonctionnalités"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}" \
        "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}" \
        "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}" \
        "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}" \
        "URLInfoAbout" "${PRODUCT_WEB_SITE}"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}" \
        "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}" \
        "InstallLocation" "$INSTDIR"
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}" \
        "NoModify" 1
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}" \
        "NoRepair" 1

    ; Calcule et enregistre la taille
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}" \
        "EstimatedSize" "$0"
SectionEnd

; -----------------------------------------------------------------------------
; Section de désinstallation
; -----------------------------------------------------------------------------
Section "Uninstall"
    ; Supprime les fichiers
    Delete "$INSTDIR\bin\64bit\obs-counter-dock.dll"
    Delete "$INSTDIR\uninstall.exe"

    ; Supprime les dossiers (seulement s'ils sont vides)
    RMDir "$INSTDIR\bin\64bit"
    RMDir "$INSTDIR\bin"
    RMDir "$INSTDIR"

    ; Supprime l'entrée du registre
    DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}"
SectionEnd

; -----------------------------------------------------------------------------
; Fonctions
; -----------------------------------------------------------------------------
Function .onInit
    ; Vérifie si OBS est installé (dossier obs-studio existe)
    IfFileExists "$APPDATA\obs-studio\*.*" obs_found obs_not_found

    obs_not_found:
        MessageBox MB_YESNO|MB_ICONQUESTION \
            "Le dossier OBS Studio n'a pas été trouvé dans %APPDATA%.$\r$\n$\r$\nOBS doit avoir été lancé au moins une fois pour créer ce dossier.$\r$\n$\r$\nVoulez-vous continuer quand même ?" \
            IDYES obs_found
        Abort

    obs_found:
FunctionEnd

Function .onInstSuccess
    MessageBox MB_OK|MB_ICONINFORMATION \
        "Installation terminée !$\r$\n$\r$\nLe plugin '${PRODUCT_NAME}' a été installé.$\r$\n$\r$\nPour l'utiliser :$\r$\n1. Lancez OBS Studio$\r$\n2. Allez dans Vue > Docks > Compteur$\r$\n$\r$\nBon entraînement !"
FunctionEnd
