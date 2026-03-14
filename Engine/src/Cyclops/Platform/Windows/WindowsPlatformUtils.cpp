#include "Cyclops/Utils/PlatformUtils.h"
#include <nfd.h>
#include <iostream>

namespace Cyclops {

    std::string FileDialogs::OpenFile(const char* filter)
    {
        NFD_Init();
        nfdchar_t* outPath;
        nfdfilteritem_t filterItem[1] = { { "Image Files", filter } };

        // [FIX] Use NFD_OKAY instead of NFD_OK
        nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);

        std::string path = "";
        if (result == NFD_OKAY) {  // <--- CHANGED
            path = outPath;
            NFD_FreePath(outPath);
        }
        else if (result == NFD_CANCEL) {
            // User cancelled
        }
        else {
            std::cout << "Error: " << NFD_GetError() << std::endl;
        }

        NFD_Quit();
        return path;
    }

    std::string FileDialogs::SaveFile(const char* filter)
    {
        NFD_Init();
        nfdchar_t* outPath;
        nfdfilteritem_t filterItem[1] = { { "Cyclops Project", filter } };

        nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, NULL, NULL);

        std::string path = "";
        if (result == NFD_OKAY) { // <--- CHANGED
            path = outPath;
            NFD_FreePath(outPath);
        }
        NFD_Quit();
        return path;
    }

    std::string FileDialogs::OpenFolder()
    {
        NFD_Init();
        nfdchar_t* outPath;

        nfdresult_t result = NFD_PickFolder(&outPath, NULL);

        std::string path = "";
        if (result == NFD_OKAY) { // <--- CHANGED
            path = outPath;
            NFD_FreePath(outPath);
        }
        NFD_Quit();
        return path;
    }
}