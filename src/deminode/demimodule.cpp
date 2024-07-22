// Copyright (c) 2021-2024 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "demimodule.h"
#include "util/util.h"

bool fDemiFound = false;

std::string GetDemiConfigFile()
{
    //TODO: include GetArg for user defined directories
    //example: ConfigFileAlias(GetArg("-demiconf", "Demi.conf"))
    std::string pathConfigFile = GetDataDir().string().c_str();
    std::string ConfigFileAlias = "/Demi.conf";
    pathConfigFile += ConfigFileAlias.c_str();

    return pathConfigFile;
}

void ReadDemiConfigFile(std::string peerReadAddr)
{
    fDemiFound = false;
    std::ifstream streamConfig(GetDemiConfigFile().c_str());
    if (!streamConfig.good())
    {
               std::string ConfPath = GetDataDir().string().c_str();
               std::string ConfigFileAlias = "/Demi.conf";
               ConfPath += ConfigFileAlias.c_str();
               FILE* ConfFile = fopen(ConfPath.c_str(), "w");
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
        if (!line.empty()) {
            if (line[0] != '#') {
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
