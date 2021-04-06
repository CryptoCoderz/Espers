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
#include "fractalcontract.h"

#include "fractal/fractaldataob.h"
#include "fractal/fractalnft.h"

using namespace std;

std::string selected_contract_alias = "";
std::string set_raw_input_data = "";
std::string set_contract_data = "";
std::string set_fractal_SCRIPT = "";

void create_smartCONTRACT(std::string raw_input_data, std::string contract_alias, int contract_type) {
    //
    //contract_alias = selected_contract_alias;
    //raw_input_data = set_raw_input_data;

    if (contract_type == 3) {
        NFTparse(raw_input_data);
        if (iLOAD) {
            priming(nftOut_String, contract_alias, contract_type);
        }
    } else {
        priming(raw_input_data, contract_alias, contract_type);
    }
}

void edit_smartCONTRACT(std::string contract_data, std::string contract_alias) {
    //
    contract_alias = selected_contract_alias;
    contract_data = set_contract_data;
}

void open_smartCONTRACT(std::string contract_data, std::string contract_alias) {
    //
    contract_alias = selected_contract_alias;
    contract_data = set_contract_data;
}


void remove_smartCONTRACT(std::string contract_data, std::string contract_alias) {
    //
    contract_alias = selected_contract_alias;
    contract_data = set_contract_data;
}
