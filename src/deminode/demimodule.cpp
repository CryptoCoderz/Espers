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
               fprintf(ConfFile, "80.211.102.238:22448\n");
               fprintf(ConfFile, "80.211.27.133:22448\n");
               fprintf(ConfFile, "134.122.23.191:22448\n");
               fprintf(ConfFile, "159.89.114.40:22448\n");
               fprintf(ConfFile, "86.92.83.17:22448\n");
               fprintf(ConfFile, "86.92.83.17\n");
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
