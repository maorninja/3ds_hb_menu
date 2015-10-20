#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "regionfree.h"
#include "statusbar.h"
#include "background.h"
#include "colours.h"
#include "MAGFX.h"
#include "filesystem.h"

#include "settingsIconAlphaSort_bin.h"
#include "settingsIconWaterAnimated_bin.h"
#include "settingsIconWaterVisible_bin.h"
#include "settingsIconThirdRow_bin.h"
#include "settingsIconClock24_bin.h"
#include "settingsIconAppBackgrounds_bin.h"
#include "settingsIconColours_bin.h"
#include "settingsIconWrapScrolling_bin.h"
#include "settingsIconTranslucencyTop_bin.h"
#include "settingsIconTranslucencyBottom_bin.h"
#include "settingsIconLogo_bin.h"
#include "settingsIconKeysExciteWater_bin.h"
#include "settingsIconPanels_bin.h"
#include "settingsIconPreloadTitles_bin.h"
#include "settingsIconTheme_bin.h"
#include "settingsIconDPadControls_bin.h"

#include "wallpaper.h"
#include "titles.h"

menu_s settingsMenu;
bool settingsMenuNeedsInit = true;

menu_s themesMenu;

bool showRegionFree = true;
bool sortAlpha = false;
bool showAppBackgrounds = true;
bool wrapScrolling = true;

bool themesLoaded = false;
//#warning Remove all references to this
//buttonList themeButtons;

void loadConfigWithType(int configType);

#include "logText.h"

void setTheme(char * themeName) {
    //Save the selected theme in main config
    setConfigString("currentTheme", themeName, configTypeMain);
    
    //Reload theme config
    loadConfigWithType(configTypeTheme);
    
    //Reload theme variables for menu
    loadThemeConfig();
    
    //Reload wallpaper
    initWallpaper();
    
    //Re-initialise colours
    initColours();
        
    //Force reload of GUI elements
    statusbarNeedsUpdate = true;
    toolbarNeedsUpdate = true;
    alphaImagesDrawn = false;
}

void addThemeToList(char * fullPath, menuEntry_s * me, char * smdhName, int folderPathLen) {
    me->hidden = false;
    me->isTitleEntry = false;
    me->isRegionFreeEntry = false;
    
    char smdhPath[strlen(fullPath) + strlen(smdhName) + 1];
    strcpy(smdhPath, fullPath);
    strcat(smdhPath, smdhName);
    
    bool iconNeedsToBeGenerated = true;
    
    if(fileExists(smdhPath, &sdmcArchive)) {
        static smdh_s tmpSmdh;
        int ret = loadFile(smdhPath, &tmpSmdh, &sdmcArchive, sizeof(smdh_s));
        
        if (!ret) {
            ret = extractSmdhData(&tmpSmdh, me->name, me->description, me->author, me->iconData);
            if (!ret) {
                iconNeedsToBeGenerated = false;
            }
        }
    }
    
    if (iconNeedsToBeGenerated) {
        memcpy(me->iconData, settingsIconTheme_bin, 48*48*3);
    }
    
    if (strcmp(fullPath, "/3ds/") == 0) {
        strcpy(me->name, "3ds");
    }
    else {
        strcpy(me->name, fullPath+folderPathLen);
    }
    
    strcpy(me->description, "");
    strcpy(me->author, "");
    
    me->drawFirstLetterOfName = iconNeedsToBeGenerated;
    me->drawFullTitle = true;
    
    addMenuEntryCopy(&themesMenu, me);
//    themesMenu.numEntries = themesMenu.numEntries + 1;
}

void buildThemesList() {
    if (themesLoaded) {
        return;
    }
    
    themesLoaded = true;
    
    directoryContents * contents = contentsOfDirectoryAtPath(themesPath, true);
    
    themesMenu.entries=NULL;
    themesMenu.numEntries=0;
    themesMenu.selectedEntry=0;
    themesMenu.scrollLocation=0;
    themesMenu.scrollVelocity=0;
    themesMenu.scrollBarSize=0;
    themesMenu.scrollBarPos=0;
    themesMenu.scrollTarget=0;
    themesMenu.atEquilibrium=false;
    
    char * smdhName = "/theme.smdh";
    int folderPathLen = strlen(themesPath);
    
    int i;
    for (i=0; i<contents->numPaths; i++) {
        char * fullPath = contents->paths[i];
        static menuEntry_s me;
        addThemeToList(fullPath, &me, smdhName, folderPathLen);
    }
    
    updateMenuIconPositions(&themesMenu);
    free(contents);
}

//void drawThemesList() {
//    buildThemesList();
//    
//    char * currentThemeName = getConfigStringForKey("currentTheme", "Default", configTypeMain);
//    
//    int i;
//    for (i=0; i<themeButtons.numButtons; i++) {
//        button * aButton = themeButtons.buttons[i];
//        if (strcmp(aButton->longText, currentThemeName) == 0) {
//            aButton->selected = true;
//            aButton->enabled = false;
//        }
//        else {
//            aButton->selected = false;
//            aButton->enabled = true;
//        }
//    }
//    
//    btnDrawButtonList(&themeButtons);
//}

void addSettingsMenuEntry(char * name, char * description, u8 * icon, bool * showTick, menu_s *m,  void (*callback)(), void *callbackObject1, void *callbackObject2) {
    static menuEntry_s settingsMenuEntry;
    
    settingsMenuEntry.hidden = false;
    settingsMenuEntry.isTitleEntry = false;
    
    strcpy(settingsMenuEntry.name, name);
    strcpy(settingsMenuEntry.description, description);
    if (icon) {
        int i;
        for (i=0; i<ENTRY_ICONSIZE; i++) {
            settingsMenuEntry.iconData[i] = icon[i];
        }
    }
    else {
        memset(settingsMenuEntry.iconData, 0, ENTRY_ICONSIZE);
    }
    settingsMenuEntry.showTick = showTick;
    settingsMenuEntry.callback = callback;
    settingsMenuEntry.callbackObject1 = callbackObject1;
    settingsMenuEntry.callbackObject2 = callbackObject2;
    
    addMenuEntryCopy(m, &settingsMenuEntry);
}

void settingsToggleBool(bool * aBool) {
    *aBool = !(*aBool);
}

void settingsToggleRegionFree() {
    showRegionFree = !showRegionFree;
    menuRegionFreeToggled();
}

void settingsToggleSortAlpha() {
    sortAlpha = !sortAlpha;
    menuReloadRequired = true;
}

void settingsShowColours() {
    if (colourSelectMenuNeedsInit) {
        initColourSelectMenu();
    }
    
    updateMenuIconPositions(&colourSelectMenu);
    gotoFirstIcon(&colourSelectMenu);
    setMenuStatus(menuStatusColourSettings);
}

void settingsSetMenuStatus(int * status) {
    if (*status == menuStatusThemeSelect) {
        buildThemesList();
        char * currentThemeName = getConfigStringForKey("currentTheme", "Default", configTypeMain);
        updateMenuTicks(&themesMenu, currentThemeName);
    }
    
    setMenuStatus(*status);
}

void initConfigMenu() {
    settingsMenuNeedsInit = false;
    
    settingsMenu.entries=NULL;
    settingsMenu.numEntries=0;
    settingsMenu.selectedEntry=0;
    settingsMenu.scrollLocation=0;
    settingsMenu.scrollVelocity=0;
    settingsMenu.scrollBarSize=0;
    settingsMenu.scrollBarPos=0;
    settingsMenu.scrollTarget=0;
    settingsMenu.atEquilibrium=false;
    
    if (regionFreeAvailable) {
        addSettingsMenuEntry("Region free loader", "Toggle the region free loader in the main menu grid", (u8*)regionfreeEntry.iconData, &showRegionFree, &settingsMenu, &settingsToggleRegionFree, NULL, NULL);
    }
    
    addSettingsMenuEntry("App sorting", "Toggle alphabetic sorting in the main menu grid", (u8*)settingsIconAlphaSort_bin, &sortAlpha, &settingsMenu, &settingsToggleSortAlpha, NULL, NULL);
    
    addSettingsMenuEntry("Top screen water", "Toggle the visibility of the water on the top screen", (u8*)settingsIconWaterVisible_bin, &waterEnabled, &settingsMenu, &settingsToggleBool, &waterEnabled, NULL);
    
    addSettingsMenuEntry("Animated water", "Toggle the animated water effect", (u8*)settingsIconWaterAnimated_bin, &waterAnimated, &settingsMenu, &settingsToggleBool, &waterAnimated, NULL);
    
    addSettingsMenuEntry("Keys excite water", "Pressing D-Pad keys makes the water excite", (u8*)settingsIconKeysExciteWater_bin, &keysExciteWater, &settingsMenu, &settingsToggleBool, &keysExciteWater, NULL);
    
    addSettingsMenuEntry("Third row of icons", "Display a third row of icons in the menu grids", (u8*)settingsIconThirdRow_bin, &thirdRowVisible, &settingsMenu, &toggleThirdRow, NULL, NULL);
    
    addSettingsMenuEntry("24 hour clock", "Displays the clock in 24 hour format", (u8*)settingsIconClock24_bin, &clock24, &settingsMenu, &settingsToggleBool, &clock24, NULL);
    
    addSettingsMenuEntry("Empty icon backgrounds", "Choose whether empty icon positions should show a background", (u8*)settingsIconAppBackgrounds_bin, &showAppBackgrounds, &settingsMenu, &settingsToggleBool, &showAppBackgrounds, NULL);
    
    addSettingsMenuEntry("Wraparound scrolling", "Choose whether page scrolling should wrap around at the beginning and of the grid", (u8*)settingsIconWrapScrolling_bin, &wrapScrolling, &settingsMenu, &settingsToggleBool, &wrapScrolling, NULL);
    
    addSettingsMenuEntry("Theme colours", "Adjust the colour scheme of the launcher", (u8*)settingsIconColours_bin, NULL, &settingsMenu, &settingsShowColours, NULL, NULL);
    
    addSettingsMenuEntry("Translucency (top screen)", "Draw the user interface with translucency - useful if using custom wallpapers", (u8*)settingsIconTranslucencyTop_bin, false, &settingsMenu, &settingsSetMenuStatus, &menuStatusTranslucencyTop, NULL);
    
    addSettingsMenuEntry("Translucency (bottom screen)", "Draw the user interface with translucency - useful if using custom wallpapers", (u8*)settingsIconTranslucencyBottom_bin, false, &settingsMenu, &settingsSetMenuStatus, &menuStatusTranslucencyBottom, NULL);
    
    addSettingsMenuEntry("Logo", "Show the 'Homebrew Launcher' logo at the bottom of the screen", (u8*)settingsIconLogo_bin, &showLogo, &settingsMenu, &settingsToggleBool, &showLogo, NULL);
    
    addSettingsMenuEntry("Panels", "Show panels behind text to make it easier to read against wallpaper", (u8*)settingsIconPanels_bin, NULL, &settingsMenu, &settingsSetMenuStatus, &menuStatusPanelSettings, NULL);
    
    addSettingsMenuEntry("Theme", "Select which theme to use in the launcher", (u8*)settingsIconTheme_bin, NULL, &settingsMenu, &settingsSetMenuStatus, &menuStatusThemeSelect, NULL);
    
    addSettingsMenuEntry("Preload titles", "Load entries for the titles menu in the background when the launcher boots. May be unstable on devices with a large number of titles", (u8*)settingsIconPreloadTitles_bin, &preloadTitles, &settingsMenu, &settingsToggleBool, &preloadTitles, NULL);
    
    addSettingsMenuEntry("D-Pad Navigation", "Allows the use of the corner icons on the bottom screen using the D-Pad", (u8*)settingsIconDPadControls_bin, &dPadNavigation, &settingsMenu, &settingsToggleBool, &dPadNavigation, NULL);
    
    
}

void handleSettingsMenuSelection() {
    int selectedEntry = settingsMenu.selectedEntry;
    menuEntry_s * selectedMenuEntry = getMenuEntry(&settingsMenu, selectedEntry);
    (selectedMenuEntry->callback)(selectedMenuEntry->callbackObject1);
}

typedef struct {
    char keys[64][64];
    char values[64][64];
    int numConfigEntries;
} configData;

configData mainData;
configData themeData;

bool configInitialLoad = false;

char * configPathForType(int configType) {
    if (configType == configTypeMain) {
        char * path = malloc(strlen(configFilePath)+1);
        strcpy(path, configFilePath);
        return path;
    }
    else if (configType == configTypeTheme) {
        char * themePath = currentThemePath();
        char * filename = malloc(128);
        sprintf(filename, "%shbltheme.cfg", themePath);
        free(themePath);
        return filename;
    }
    else {
        return NULL;
    }
}

configData * configDataForType(int configType) {
    if (configType == configTypeMain) {
        return &mainData;
    }
    else if (configType == configTypeTheme) {
        return &themeData;
    }
    else {
        return NULL;
    }
}

void loadConfigWithType(int configType) {
    configData *data = configDataForType(configType);
    if (!data) {
        return;
    }
    
//    memset(data, 0, sizeof(&data));
    
    char * configPath = configPathForType(configType);
    if (configPath == NULL) {
        return;
    }
    
    FILE* fp = fopen( configPath, "r" );
    char configText[8192];
    int count = 0;
    
    if (fp != NULL) {
        fgets(configText, 8192, fp);

        char lines[64][64];
        
        char * split1;
        split1 = strtok(configText, "|");
        while (split1 != NULL) {
            strcpy(lines[count], split1);
            
            count++;
            split1 = strtok(NULL, "|");
        }
        
        int i;
        
        for (i=0; i<count; i++) {
            char * line = lines[i];
            char * split2;
            
            split2 = strtok(line, "=");
            
            if (strcmp(split2, "translucencyLevel") == 0) {
                count--;
            }
            else {
                strcpy(data->keys[i], split2);
                
                split2 = strtok(NULL, "=");
                strcpy(data->values[i], split2);
            }
        }
    }
    fclose(fp);
    
    data->numConfigEntries = count;

    free(configPath);
}

void loadConfig() {
    loadConfigWithType(configTypeMain);
    
    
    /*
     This flag states that the initial load of the config has been completed.
     It MUST go after loading the main config and before loading the theme config.
     This is because of the following:
     - The functions which retrieve data from the config check whether configInitialLoad is false
     - If it is false, they call loadConfig() (this function)
     - Loading the theme config requires retrieving the currentTheme setting from the main config
     - If loadConfigWithType(configTypeTheme) is called while configInitialLoad is false, then 
       retrieving the value of currentTheme will trigger loadConfig() to be called again, and we'll
       enter an infinite loop
     - To prevent this, we load the main config (so we know that the value of currentTheme will be
       available), then we set configInitialLoad to true to prevent further calls to loadConfig(),
       and only then call loadConfigWithType(configTypeTheme).
     */
    
    configInitialLoad = true;
    
    
    loadConfigWithType(configTypeTheme);
}

void saveConfigWithType(int configType) {
    configData *data = configDataForType(configType);
    if (!data) {
        return;
    }
    
    //Start size at zero bytes
    int size = 0;
    
    //For each config entry, add the size of the value, key and delimeters to the overall file size
    int i;
    for (i=0; i<data->numConfigEntries; i++) {
        size += strlen(data->keys[i]);
        size += strlen(data->values[i]);
        size += 2;
    }
    
    //Create a string big enough to accomodate all of the keys, values and delimeters
    char out[size];
    
    strcpy(out, "");
    
    //For each config entry, add its key, value and delimeters to the output string
    for (i=0; i<data->numConfigEntries; i++) {
        strncat(out, data->keys[i], strlen(data->keys[i]));
        strncat(out, "=", 1);
        strncat(out, data->values[i], strlen(data->values[i]));
        strncat(out, "|", 1);
    }
    
    char * configPath = configPathForType(configType);
    if (configPath == NULL) {
        return;
    }
    
    //Open the config file path for writing and save the output string to it
    FILE* fp = fopen( configPath, "w" );
    if (fp != NULL) {
        fputs(out, fp);
    }

    fclose(fp);
    free(configPath);
}

void saveConfig() {
    saveConfigWithType(configTypeMain);
    saveConfigWithType(configTypeTheme);
}

void setConfigString(char* key, char* value, int configType) {
    if (!configInitialLoad) {
        loadConfig();
    }
    
    configData *data = configDataForType(configType);
    if (!data) {
        return;
    }
    
    //Assume that the config entry does not currently exist
    int currentIndex = -1;
    
    //Loop through the existing entries
    int i;
    for (i=0; i<data->numConfigEntries; i++) {
        if (strcmp(key, data->keys[i]) == 0) {
            //If the key of the current entry matches the key being added, then the config entry already exists
            //Set current index to the index of the existing entry and break
            currentIndex = i;
            break;
        }
    }
    
    //If the entry already exists in the array, overwrite it
    if (currentIndex > -1) {
        memset(&(data->keys[currentIndex][0]), 0, 64);
        memset(&(data->values[currentIndex][0]), 0, 64);
        
        strcpy(data->keys[currentIndex], key);
        strcpy(data->values[currentIndex], value);
    }
    
    //If the entry does not yet exist in the array, add it
    else {
        strcpy(data->keys[data->numConfigEntries], key);
        strcpy(data->values[data->numConfigEntries], value);
        data->numConfigEntries = data->numConfigEntries + 1;
    }
}

void setConfigBool(char* key, bool value, int configType) {
    setConfigString(key, value ? "1" : "0", configType);
}

void setConfigInt(char* key, int value, int configType) {
    char sValue[sizeof(int)+1];
    sprintf(sValue, "%d", value);
    setConfigString(key, sValue, configType);
}

char * getConfigValueForKey(char * key, int configType) {
    if (!configInitialLoad) {
        loadConfig();
    }
    
    configData *data = configDataForType(configType);
    if (!data) {
        return NULL;
    }

    int i;
    
    for (i=0; i<data->numConfigEntries; i++) {
        char * check = data->keys[i];
        if (strcmp(check, key) == 0) {
            return data->values[i];
        }
    }
    
    return NULL;
}

bool getConfigBoolForKey(char* key, bool defaultValue, int configType) {
    char * sValue = getConfigValueForKey(key, configType);
    
    if (sValue) {
        if (strcmp(sValue, "1") == 0) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return defaultValue;
    }
}

int getConfigIntForKey(char* key, int defaultValue, int configType) {
    char * sValue = getConfigValueForKey(key, configType);
    
    if (sValue) {
        int iValue = atoi(sValue);
        return iValue;
    }
    else {
        return defaultValue;
    }
}

char * getConfigStringForKey(char* key, char* defaultValue, int configType) {
    char * sValue = getConfigValueForKey(key, configType);
    
    if (sValue) {
        return sValue;
    }
    else {
        return defaultValue;
    }
}