# ======================= ofxCMake Vers. 0.1 =============
#  PUT THIS FILE INTO YOUR OPENFRAMEWORKS PROJECT FOLDER

# ========================================================
# ===================== CMake Settings ===================
# ========================================================
cmake_minimum_required( VERSION 3.3 )
project( openframeworks )

# ========================================================
# ===================== User Settings ====================
# ========================================================
# ---------------------- App name  -----------------------
set( APP_NAME lightDrum )

# ------------------------ OF Path -----------------------
# --- If outside the OF structure, set an absolute OF path
set( OF_DIRECTORY_BY_USER "../../.." )

# --------------------- Source Files ---------------------
set( ${APP_NAME}_SOURCE_FILES
        src/main.cpp
        src/ofApp.cpp
        src/Drum.cpp
        )


# ------------------------ AddOns  -----------------------
set( OFX_ADDONS_ACTIVE
        ofxXmlSettings
        ofxPoco
        ofxNetwork
        ofxDatGui
        ofxLedMapper
        ofxMidi )

# =========================================================================
# ============================== OpenFrameworks ===========================
# =========================================================================
include( ${OF_DIRECTORY_BY_USER}/addons/ofxCMake/modules/main.cmake )
# =========================================================================

if( APPLE ) # Set App icon
    set_source_files_properties(${APP_NAME}.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

#ADD_CUSTOM_COMMAND( TARGET ${APP_NAME}
#        POST_BUILD
#        COMMAND /bin/cp
#        ARGS ${PROJECT_SOURCE_DIR}/${APP_NAME}.icns ${PROJECT_SOURCE_DIR}/bin/${APP_NAME}.app/Contents/Resources
#        )

ADD_CUSTOM_COMMAND(
        TARGET ${APP_NAME}
        POST_BUILD
        COMMAND rsync
        ARGS -av --exclude='.DS_Store'
        "${PROJECT_SOURCE_DIR}/bin/data/"
        "${PROJECT_SOURCE_DIR}/bin/${APP_NAME}.app/Contents/Resources"
        COMMENT "Copying Bundle Stuff")

endif()