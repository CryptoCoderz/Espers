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
    std::string ConfPath = GetDataDir().string().c_str();
    std::string ConfigFileAlias = "/Demi.conf";
    ConfPath += ConfigFileAlias.c_str();
    FILE* ConfFile = fopen(ConfPath.c_str(), "w");
    fprintf(ConfFile, "88931 version\n");
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
    fclose(ConfFile);
}

void ReadDemiConfigFile(std::string peerReadAddr)
{
    fDemiFound = false;

    // Open requested config file
    std::ifstream file;
    file.open(GetDemiConfigFile().c_str());
    if(!file.is_open()) {
        // Print for debugging
        LogPrintf("ReadDemiConfigFile - ERROR - Cannot open file!\n");
        return;
    }

    // Read data
    //
    std::string line;
    while(file.good()) {
        // Loop through lines
        std::getline(file, line);
        if(!line.empty()) {
            if(line[0] != '#') {
                // Combine input string
                if(peerReadAddr == line) {
                    fDemiFound = true;
                    break;
                }
            }
        }
    }
    file.close();
}

void UpdateDemiConfigFile()
{
    bool fVersionFound = false;
    bool fFlagUpdate = false;
    // Open requested config file
    std::ifstream file;
    file.open(GetDemiConfigFile().c_str());
    if(!file.is_open()) {
        // Print for debugging
        LogPrintf("UpdateDemiConfigFile - ERROR - Cannot open file!\n");
        return;
    }

    // Read data
    //
    std::string line;
    int iVersion = 88931;// Version number
    int lVersion;
    while(file.good()) {
        // Loop through lines
        std::getline(file, line);
        if(!line.empty()) {
            // Ensure line contains expected data
            if(line.find("version") != std::string::npos) {
                fVersionFound = true;
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
                    fDemiUpdate = false;
                    break;
                }
            }
        }
    }
    file.close();

    // Rebuild Demi.conf if needed
    if(fFlagUpdate || !fVersionFound) {
        // Print for debugging
        LogPrintf("UpdateDemiConfigFile - Updating Demi Config to version: %i \n", iVersion);
        BuildDemiConfigFile();
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
