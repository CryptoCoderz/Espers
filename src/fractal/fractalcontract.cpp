// Copyright (c) 2017-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


// NOTICE!
//
// This is a completely experimental smart-contract platform written by
// CryptoCoderz (Jonathan Dan Zaretsky - cryptocoderz@gmail.com)
// dmEgc2xhdnUgZ29zcG9kZSBib2dlIGUgbmFzaCBzcGFzZXRhbCBlc3VzIGhyaXN0b3M=
//
// PLEASE USE AT YOUR OWN RISK!!!
//

#include <string>
#include <cstring>
#include "fractalcontract.h"

#include "fractal/fractaldataob.h"
#include "fractal/fractalnftbase.h"
#include "fractal/fractalnft.h"

using namespace std;

std::string selected_contract_alias = "";
std::string set_raw_input_data = "";
std::string set_contract_data = "";
std::string set_fractal_SCRIPT = "";
bool fextTokenDecodeSuccess = false;
std::string ext_Contract_Path = "";

void create_smartCONTRACT(std::string raw_input_data, std::string contract_alias, int contract_type) {

    if (contract_type == 3) {
        NFTparse(raw_input_data);
        if (iLOAD) {
            if (NFTBASE_run) {
                priming(nftBASEOut_String, contract_alias, contract_type, false);// set true for layer 2 encryption
            }
        }
    } else {
        priming(raw_input_data, contract_alias, contract_type, false);// set true for layer 2 encryption
    }
}

void edit_smartCONTRACT(std::string contract_data, std::string contract_alias) {
    //
    contract_alias = selected_contract_alias;
    contract_data = set_contract_data;
}

void open_smartCONTRACT(std::string contract_alias, int contract_type) {
    //
    if(contract_type == 0) {
        gateKeeper(contract_alias, contract_type);
        fextTokenDecodeSuccess = fTokenDecodeSuccess;
    } else if(contract_type == 3) {
        //
        gateKeeper(contract_alias, contract_type);
        NFTrender(PlainText_Combined_String, contract_alias, ext_Contract_Path, contract_type);
    }
}


void remove_smartCONTRACT(std::string contract_data, std::string contract_alias) {
    //
    contract_alias = selected_contract_alias;
    contract_data = set_contract_data;
}
