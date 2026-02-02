# =============================================================================
# Plugin OBS - Compteur (Push-up Counter)
# =============================================================================
# Ce fichier configure le build du plugin avec CMake.
# CMake est un outil qui génère les fichiers de compilation (Makefiles sur
# Linux, projets Visual Studio sur Windows) à partir de cette configuration.
#
# Ce CMakeLists.txt est CROSS-PLATFORM : il fonctionne sur Linux et Windows.
# =============================================================================

cmake_minimum_required(VERSION 3.16)

# Nom du projet - utilisé pour nommer le fichier final (.so sur Linux, .dll sur Windows)
project(obs-counter-dock VERSION 1.0.0)

# On utilise le standard C++17 (nécessaire pour certaines fonctionnalités modernes)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Active le traitement automatique des fichiers Qt (MOC, UIC, RCC)
# MOC = Meta-Object Compiler, nécessaire pour les signaux/slots Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================
# Recherche des dépendances
# =============================================================================

# Trouve Qt6 Widgets (pour l'interface graphique du dock)
# Qt6 fonctionne de la même manière sur Linux et Windows avec find_package
# Note: OBS 32+ nécessite Qt6 (Qt5 n'est plus supporté)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# -----------------------------------------------------------------------------
# OBS - Différent selon la plateforme
# -----------------------------------------------------------------------------
# Sur Linux : on utilise pkg-config (standard Linux pour trouver les libs)
# Sur Windows : on utilise des chemins explicites passés via CMake

if(UNIX AND NOT APPLE)
    # =========================================================================
    # Linux - Utilise pkg-config
    # =========================================================================
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OBS REQUIRED libobs)

    # Les variables OBS_INCLUDE_DIRS et OBS_LIBRARIES sont définies par pkg-config
    set(OBS_INCLUDES ${OBS_INCLUDE_DIRS})
    set(OBS_LIBS ${OBS_LIBRARIES})

elseif(WIN32)
    # =========================================================================
    # Windows - Utilise les chemins passés en paramètres CMake
    # =========================================================================
    # Ces variables doivent être définies lors de l'appel à cmake :
    # cmake .. -DOBS_SOURCE_DIR="C:/obs-studio" -DOBS_LIB_DIR="C:/obs/bin/64bit"

    if(NOT DEFINED OBS_SOURCE_DIR)
        message(FATAL_ERROR
            "OBS_SOURCE_DIR non défini!\n"
            "Sur Windows, vous devez spécifier le chemin vers les sources OBS:\n"
            "cmake .. -DOBS_SOURCE_DIR=\"C:/chemin/vers/obs-studio\""
        )
    endif()

    if(NOT DEFINED OBS_LIB_DIR)
        message(FATAL_ERROR
            "OBS_LIB_DIR non défini!\n"
            "Sur Windows, vous devez spécifier le chemin vers les libs OBS:\n"
            "cmake .. -DOBS_LIB_DIR=\"C:/chemin/vers/obs/bin/64bit\""
        )
    endif()

    # Chemins d'inclusion - où trouver les headers OBS
    set(OBS_INCLUDES
        "${OBS_SOURCE_DIR}/libobs"
        "${OBS_SOURCE_DIR}/UI/obs-frontend-api"
    )

    # Bibliothèques à lier
    # On cherche les fichiers .lib dans le dossier spécifié
    find_library(OBS_LIB obs PATHS "${OBS_LIB_DIR}" NO_DEFAULT_PATH)
    find_library(OBS_FRONTEND_LIB obs-frontend-api PATHS "${OBS_LIB_DIR}" NO_DEFAULT_PATH)

    if(NOT OBS_LIB)
        message(WARNING "obs.lib non trouvé dans ${OBS_LIB_DIR}")
    endif()

    set(OBS_LIBS ${OBS_LIB} ${OBS_FRONTEND_LIB})
endif()

# =============================================================================
# Configuration du plugin
# =============================================================================

# Liste des fichiers sources du plugin
set(PLUGIN_SOURCES
    src/plugin-main.cpp
    src/counter-dock.cpp
    src/counter-dock.hpp
)

# Crée une bibliothèque partagée (MODULE)
# MODULE = bibliothèque chargée dynamiquement (plugin)
# Produit : .so sur Linux, .dll sur Windows
add_library(${PROJECT_NAME} MODULE ${PLUGIN_SOURCES})

# Ajoute les chemins d'inclusion (headers) nécessaires
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OBS_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Lie les bibliothèques nécessaires au plugin
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OBS_LIBS}
    Qt6::Widgets
)

# Options spécifiques à la plateforme
if(UNIX AND NOT APPLE)
    # Linux : options de pkg-config
    target_compile_options(${PROJECT_NAME} PRIVATE ${OBS_CFLAGS_OTHER})
elseif(WIN32)
    # Windows : définitions pour l'export DLL
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        UNICODE
        _UNICODE
    )
    # Pas de préfixe "lib" sur Windows
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
endif()

# =============================================================================
# Installation
# =============================================================================

if(UNIX AND NOT APPLE)
    # Linux : installe dans ~/.config/obs-studio/plugins/
    set(OBS_PLUGIN_DIR "$ENV{HOME}/.config/obs-studio/plugins/${PROJECT_NAME}/bin/64bit")

    install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${OBS_PLUGIN_DIR}
    )

    # Cible personnalisée pour installation rapide pendant le développement
    add_custom_target(install-plugin
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OBS_PLUGIN_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${OBS_PLUGIN_DIR}/
        DEPENDS ${PROJECT_NAME}
        COMMENT "Installation du plugin dans ${OBS_PLUGIN_DIR}"
    )

elseif(WIN32)
    # Windows : installe dans %APPDATA%/obs-studio/plugins/
    set(OBS_PLUGIN_DIR "$ENV{APPDATA}/obs-studio/plugins/${PROJECT_NAME}/bin/64bit")

    # Pour un MODULE (plugin), CMake utilise LIBRARY même sur Windows
    install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${OBS_PLUGIN_DIR}
    )
endif()
