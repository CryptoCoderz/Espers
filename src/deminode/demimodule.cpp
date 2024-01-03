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
               fprintf(ConfFile, "n1.espers.io:22448\n");
               fprintf(ConfFile, "n2.espers.io:22448\n");
               fprintf(ConfFile, "n3.espers.io:22448\n");
               fprintf(ConfFile, "n4.espers.io:22448\n");
               fprintf(ConfFile, "n5.espers.io:22448\n");
               fprintf(ConfFile, "n6.espers.io:22448\n");
               fprintf(ConfFile, "n7.espers.io:22448\n");
               fprintf(ConfFile, "n8.espers.io:22448\n");
               fprintf(ConfFile, "n9.espers.io:22448\n");
               fprintf(ConfFile, "n10.espers.io:22448\n");
               fprintf(ConfFile, "n1.espers.io\n");
               fprintf(ConfFile, "n2.espers.io\n");
               fprintf(ConfFile, "n3.espers.io\n");
               fprintf(ConfFile, "n4.espers.io\n");
               fprintf(ConfFile, "n5.espers.io\n");
               fprintf(ConfFile, "n6.espers.io\n");
               fprintf(ConfFile, "n7.espers.io\n");
               fprintf(ConfFile, "n8.espers.io\n");
               fprintf(ConfFile, "n9.espers.io\n");
               fprintf(ConfFile, "n10.espers.io\n");
               fclose(ConfFile);
    }

    // Open requested config file
    LogPrintf("ReadDemiConfigFile - INFO - Loading Demi-nodes from: %s \n", GetDemiConfigFile().c_str());
    std::ifstream file;
    file.open(GetDemiConfigFile().c_str());
    if(!file.is_open()) {
        // Print for debugging
        LogPrintf("ReadDemiConfigFile - ERROR 00 - Cannot open file!\n");
        return;
    } else {
        // Print for debugging
        LogPrintf("ReadDemiConfigFile - INFO - File successfully opened!\n");
    }

    // Read data
    //
    // Print for debugging
    LogPrintf("ReadDemiConfigFile - INFO - Reading file...\n");
    std::string line;
    while(file.good()) {
        // Loop through lines
        std::getline(file, line);
        // Print for debugging
        LogPrintf("ReadDemiConfigFile - INFO - Got line data...\n");
        if (!line.empty()) {
            if (line[0] != '#') {
                // Print for debugging
                LogPrintf("ReadDemiConfigFile - INFO - Read data success!\n");
                // Combine input string
                if(peerReadAddr == line) {
                    // Print for debugging
                    LogPrintf("ReadDemiConfigFile - INFO - Set data success!\n");
                    fDemiFound = true;
                    break;
                }
            } else {
                // Print for debugging
                LogPrintf("ReadDemiConfigFile - WARNING - Comment detected! \n");
            }
        } else {
            // Print for debugging
            LogPrintf("ReadDemiConfigFile - WARNING - Empty line detected! \n");
        }
    }

    file.close();
}
