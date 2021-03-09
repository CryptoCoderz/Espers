// Copyright (c) 2017-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


// NOTICE!
//
// This is a completely experimental smart-contract platform written by
// CrpytoCoderz (Jonathan Dan Zaretsky - cryptocoderz@gmail.com)
//
// PLEASE USE AT YOUR OWN RISK!!!
//

#include <string>
#include <cstring>
#include "fractalengine.h"
#include "fractaldataob.h"
#include "fractalcontract.h"

#include "util/util.h"

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

using namespace std;

static const char fractalSCRIPT_charset[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "()[]{}:-+*/_=";

// fractalSCRIPT v0.1a commands/methods
// -------------------------------------
//          COMMAND/METHOD LIST
// -------------------------------------
// SETPARAMS - flags the parser for new smart contract deployment
// OBFUSCATION - flags a smart contract creation to obfuscate stored data (2 layer option, use "=1" or "=2")
// SETALIAS - flags a smart contract creation to use specified alias when creating contract
// SETTYPE - flags smart contract creation type (current valid options: DATA, TOKEN)
// TOKENCOUNT - define total token count for token type smart contract
// TOKENPRMN - define total premine amount (can be 0-100%)
// TOKENPBLK - define tokens per-block to issue (can be set to 0 as off)
// TOKENMINING - define whether or not to generate blocks like the MAINNET
// TOKENSTK - define whether or not to allow token staking
// DATATYP - define what type of data is being stored in the data contract (currently only text may be stored)
// SETTXT - set the text being stored in the data type contract
// EDITSC - flags parser to expect private key (control key), contract alias, and edited fields/data to be set for editing
// VIEWSC - flags parser to expect contract alias to look up and attempt to open
// VIEWPRVSC - flags parser to expect contract alias and private/control key (or) access key
// ROLLBK - set rollback block for token type contract
// TOKENNETADD - toggle wether to use the MAINNET pubkeys or unique pubkeys
// USEMNET - used in conjunction with TOKENNETADD, this defines pubkeys to remain same as MAINNET
// USEUNQ - used in conjuction with TOKNNETADD, this defines pubkeys as
// TERM - used in conjuction with EDITSC, this flags the contract for deletion
// LSTXT - used in conjuction with SETTXT, this presets the parser to not care about formatting the text
// BKTXT - used in conjuction with SETTXT, this presets the parser to organize the text
// TOKENBKSP - defines token type contract's block spacing (set to 0 if you desire per-TX block creation)
// TXT - used in conjuction with DATATYP, this presets the parser to expect text data to be stored
// DATA - used in conjuntction with SETTYPE, this presets the parser to expect a data contract type
// TOKEN - used in conjuction with SETTYPE, this presets the parser to expect a token contract type
// GENSC - flags the parser for new smart contract generation
//
std::string fractalSCRIPT_methods[26] = { "SETPARAMS", "OBFUSCATION", "SETALIAS", "SETTYPE",
"TOKENCOUNT", "TOKENPRMN", "TOKENPBLK", "TOKENMINING", "TOKENSTK", "DATATYP", "SETTXT", "EDITSC",
"VIEWSC", "VIEWPRVSC", "ROLLBK", "TOKENNETADD", "USEMNET", "USEUNQ", "TERM", "LSTXT",
"BKTXT", "TOKENBKSP", "TXT", "DATA", "TOKEN", "GENSC"
};

void write_contractDATA(std::string obfuscated_write_string, std::string contract_alias) {
    //
    //obfuscated_write_string = Obfuscated_Combined_String;
    //contract_alias = selected_contract_alias;
    boost::filesystem::path pathConfigFile("Fractal.conf");
    boost::filesystem::ifstream streamFractalConfig(pathConfigFile);
    if (!streamFractalConfig.good())
    {
        boost::filesystem::path FractalConfPath;
               FractalConfPath = GetDataDir() / "Fractal.conf";
               FILE* ConfFile = fopen(FractalConfPath.string().c_str(), "w");
               fprintf(ConfFile, "%s|%s\n", obfuscated_write_string.c_str(), contract_alias.c_str());
               //fprintf(ConfFile, "data\n");
               //fprintf(ConfFile, "data\n");

               fclose(ConfFile);

               // Returns our config path, created config file is loaded during initial run...
               return ;
    } else {
        boost::filesystem::path FractalConfPath;
               FractalConfPath = GetDataDir() / "Fractal.conf";
               FILE* ConfFile = fopen(FractalConfPath.string().c_str(), "w");
               fprintf(ConfFile, "%s|%s\n", obfuscated_write_string.c_str(), contract_alias.c_str());
               //fprintf(ConfFile, "data\n");
               //fprintf(ConfFile, "data\n");

               fclose(ConfFile);

               // Returns our config path, created config file is loaded during initial run...
               return ;
    }
}

void read_contractDATA(std::string obfuscated_read_string, std::string contract_alias) {
    //
    obfuscated_read_string = Obfuscated_Combined_String;
    contract_alias = selected_contract_alias;
}

void parse_fractalSCRIPT(std::string fractal_SCRIPT) {
    //
    fractal_SCRIPT = set_fractal_SCRIPT;
}
