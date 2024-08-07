// Copyright (c) 2021-2024 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "demimodule.h"
#include "util/util.h"

bool fDemiFound = false;
bool fDemiUpdate = true;

std::string GetDemiConfigFile()
{
    std::string pathConfigFile = GetDataDir().string().c_str();
    std::string ConfigFileAlias = "/Demi.conf";
    pathConfigFile += ConfigFileAlias.c_str();

    return pathConfigFile;
}

void BuildDemiConfigFile()
{
    FILE* ConfFile = fopen(GetDemiConfigFile().c_str(), "w");
    fprintf(ConfFile, "#89021 version\n");
    fprintf(ConfFile, "46.18.47.191\n");
    fprintf(ConfFile, "80.7.228.11:22448\n");
    fprintf(ConfFile, "95.39.17.203\n");
    fprintf(ConfFile, "95.39.17.203:22448\n");
    fprintf(ConfFile, "172.105.121.51\n");
    fprintf(ConfFile, "172.105.121.51:22448\n");
    fprintf(ConfFile, "198.58.109.214\n");
    fprintf(ConfFile, "198.58.109.214:22448\n");
    fprintf(ConfFile, "188.164.197.250\n");
    fprintf(ConfFile, "188.164.197.250:22448\n");
    fprintf(ConfFile, "188.164.198.102\n");
    fprintf(ConfFile, "188.164.198.102:22448\n");
    fprintf(ConfFile, "172.234.87.233\n");
    fprintf(ConfFile, "172.234.87.233:22448\n");
    fclose(ConfFile);
}

void ReadDemiConfigFile(std::string peerReadAddr)
{
    fDemiFound = false;

    // Open requested config file
    std::ifstream fileConfigRead;
    fileConfigRead.open(GetDemiConfigFile().c_str());
    if(!fileConfigRead.is_open()) {
        // Print for debugging
        LogPrintf("ReadDemiConfigFile - ERROR - Cannot open file!\n");
        return;
    }

    // Read data
    std::string line;
    while(fileConfigRead.good()) {
        // Loop through lines
        std::getline(fileConfigRead, line);
        if(!line.empty()) {
            if(line[0] != '#') {
                // Check for match
                if(peerReadAddr == line) {
                    fDemiFound = true;
                    break;
                }
            }
        }
    }
    fileConfigRead.close();
}

void UpdateDemiConfigFile()
{
    // Set standard values
    bool fVersionFound = false;
    bool fFlagUpdate = false;
    // Open requested config file
    std::ifstream fileConfigRead;
    fileConfigRead.open(GetDemiConfigFile().c_str());
    if(!fileConfigRead.is_open()) {
        // Print for debugging
        LogPrintf("UpdateDemiConfigFile - ERROR - Cannot open file!\n");
        return;
    }

    // Read data
    std::string line;
    int iVersion = 89021;// Version number
    int lVersion;
    while(fileConfigRead.good()) {
        // Loop through lines
        std::getline(fileConfigRead, line);
        if(!line.empty()) {
            // Ensure line contains expected data
            if(line.find("version") != std::string::npos) {
                fVersionFound = true;
                // Set version as focus
                line.erase(0, line.find("#") + std::string("#").length());
                // Convert string to integer value
                std::string::size_type string_sz;
                lVersion = std::stoi(line, &string_sz);
                // Print for debugging
                LogPrintf("UpdateDemiConfigFile - Logged Version - %i \n", lVersion);
                // Verify version update
                if(lVersion >= iVersion) {
                    fDemiUpdate = false;
                    break;
                } else {
                    fFlagUpdate = true;
                    break;
                }
            }
        }
    }
    fileConfigRead.close();

    // Rebuild Demi.conf if needed
    if(fFlagUpdate || !fVersionFound) {
        // Print for debugging
        LogPrintf("UpdateDemiConfigFile - Updating Demi Config to version: %i \n", iVersion);
        // Rebuild without any previous user-defined data
        BuildDemiConfigFile();
        fDemiUpdate = false;
    }
}

void InitializeDemiConfigFile(std::string peerReadAddr)
{
    std::ifstream streamConfig(GetDemiConfigFile().c_str());
    if(!streamConfig.good())
    {
        // Create Demi.conf
        BuildDemiConfigFile();
        // Then read it...
        ReadDemiConfigFile(peerReadAddr);
    } else {
        // Check if Demi.conf needs update (once)
        if(fDemiUpdate) {
            UpdateDemiConfigFile();
        }
        // Read Demi.conf
        ReadDemiConfigFile(peerReadAddr);
    }
}
