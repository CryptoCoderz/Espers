// Copyright (c) 2021-2024 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "demimodule.h"
#include "util/util.h"
#include "database/txdb.h"

bool fDemiFound = false;
bool fDemiUpdate = true;

// Store Demi-node list from config data
std::vector<std::string> vecRegisteredDemiNodes;
// Store Demi-node list from seen peers
std::vector<std::string> vecSeenDemiNodes;

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
    fprintf(ConfFile, "#89131 version\n");
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
    fprintf(ConfFile, "172.105.11.34\n");
    fprintf(ConfFile, "172.105.11.34:22448\n");
    fclose(ConfFile);
}

void ReadDemiConfigFile()
{
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
                // Add Demi-Node to registered vector list
                vecRegisteredDemiNodes.push_back(line);
            }
        }
    }
    fileConfigRead.close();
}

void FindRegisteredDemi(std::string peerReadAddr)
{
    // Reset Demi-node found toggle
    fDemiFound = false;

    // Search Registered Demi-node list
    for(std::string& DemiRegList : vecRegisteredDemiNodes) {
        // Check for match
        if(peerReadAddr == DemiRegList) {
            fDemiFound = true;
            break;
        }
    }
}

bool FindSeenDemi(std::string peerReadAddr)
{
    // Check if there is anything to search
    if (vecSeenDemiNodes.empty()) {
        return false;
    }

    // Search Registered Demi-node list
    for(std::string& DemiSeenList : vecSeenDemiNodes) {
        // Check for match
        if(peerReadAddr == DemiSeenList) {
            return true;
        }
    }

    return false;
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
    int iVersion = 89131;// Version number
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

void UpdateSeenDemi(std::string peerReadAddr, bool fAddDemi)
{
    std::vector<std::string> vecTempDemiList = vecSeenDemiNodes;
    unsigned int vecLoopList = 0;

    // Check if adding initial value
    if (vecSeenDemiNodes.empty()) {
        // Add Demi-Node to registered vector list
        vecSeenDemiNodes.push_back(peerReadAddr);
        return;
    }

    // Search Seen Demi-node list
    for(std::vector<std::string>::iterator it = vecTempDemiList.begin(); it != vecTempDemiList.end();) {
        // Check for match
        if(peerReadAddr == vecTempDemiList.at(vecLoopList)) {
            if (!fAddDemi) {
                // Remove seen Demi-node from list
                vecSeenDemiNodes.erase(it);
            }
            return;
        }
        // Move up per round
        vecLoopList ++;
         ++it;
    }

    // If adding, we do so now
    if (fAddDemi) {
        // Add Demi-Node to registered vector list
        vecSeenDemiNodes.push_back(peerReadAddr);
    }
}

void ResetRegisteredDemi()
{
    // Clear current registered list
    vecRegisteredDemiNodes.clear();

    // Read Demi.conf
    ReadDemiConfigFile();
}

void ResetSeenDemi()
{
    // Clear current registered list
    vecSeenDemiNodes.clear();

    // Loop through peers/nodes
    LOCK(cs_vNodes);
    for(CNode* pnode : vNodes) {
        // Skip obsolete nodes
        if(pnode->nVersion < DEMINODE_VERSION) {
            continue;
        }
        // Add Demi-Node to registered vector list
        FindRegisteredDemi(pnode->addrName);
        if(fDemiFound) {
            UpdateSeenDemi(pnode->addrName, true);
        }

    }
}

void InitializeDemiConfigFile()
{
    std::ifstream streamConfig(GetDemiConfigFile().c_str());
    if(!streamConfig.good())
    {
        // Create Demi.conf
        BuildDemiConfigFile();
        // Then read it...
        ReadDemiConfigFile();
    } else {
        // Check if Demi.conf needs update (once)
        if(fDemiUpdate) {
            UpdateDemiConfigFile();
        }
        // Read Demi.conf
        ReadDemiConfigFile();
    }
}
