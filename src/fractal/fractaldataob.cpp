// Copyright (c) 2019-2025 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


// NOTICE!
//
// This is a completely experimental data obfuscation method written by
// CryptoCoderz (Jonathan Dan Zaretsky - cryptocoderz@gmail.com)
// and 
// SaltineChips (Jeremiah Cook - jeremiahwcook@gmail.com)
// dmEgc2xhdnUgZ29zcG9kZSBib2dlIGUgbmFzaCBzcGFzZXRhbCBlc3VzIGhyaXN0b3M=
//
// PLEASE USE AT YOUR OWN RISK!!!
//
// Anyone is welcome to use or modify this code so long as they adheer to
// the MIT/X11 software license terms and include the above copyright in all
// subsequent releases or use.
//
// The sole purpose of this method is more or less a "hold over" until a
// more proper method can be realized and implemented. Currently it is
// deemed sloppy and inificient at the time of writing this as each character
// is obfuscated prior to then obfuscating the word itself.
// Theoretically this could cause lower power CPUs to choke or hang during
// obfuscation and possibly during reassembly as well.
//
// Obfuscation rounds are handled at random by using the amount of words and letters
// used in the input string as mathematical seed for a simple rand calculation
// that is then used to determine how many shifts to make, in which direction
// to make them and whether or not reverse/pivot direction before finishing
// shift rounds.
//
// Once each letter of a word has gone through obfuscation rounds the 
// engine then moves on to obfuscate the entire word as well through
// another series of shift/pivot rounds at which point it cycles through
// the rest of the input string words (anything sperated by a space)
// until we have a obfuscated data input string as whole.
//
// With the entire obfuscated data input string we then take it through
// the engine primer which determines a new set of pivots and shifts with
// a refresh of the system time value seed used for rand and a fresh rand
// value calculated as well. Once this is complete the entire string is
// then obfuscated as a whole and the output saved as the "hash" value.
//
// The shifts/pivtos are logged and output in a "key" file that can be
// read to de-obfuscate the data into something legible once again.
//
// This method can be layered twice over to produce a two layer key method.
// A two layer key method is the same as having a pub/priv key and allows
// a user to reveal some portion of data publicly but not the secondary layer.
// This can also be used a form access control rather than data obfuscation 
// for modifying the data stored.

#include <string>
#include <cstring>
// Read from file
#include <iostream>
#include <fstream>

#include "fractaldataob.h"
//
#include "fractalengine.h"
// for logprintf
#include "util/init.h"
using namespace std;

// Initialize primitives
int input_character_length = 0;
int input_length = 0;
int shifts = 0;
int pivots = 0;
int position = 0;
std::string Obfuscated_String = "";
std::string Obfuscated_Combined_String = "";
std::string PlainText_Combined_String = "";
std::string PlainText_String = "";
std::string SingleLayer_Key = "";
std::string SecondLayer_Key = "";
bool fTokenDecodeSuccess = false;

// Initialize Valid Character String Array
std::string Character_String[62] = { "A", "a", "B", "b", 
"C", "c", "D", "d", "E", "e", "F", "f",
"G", "g", "H", "h", "I", "i", "J", "j",
"K", "k", "L", "l", "M", "m", "N", "n",
"O", "o", "P", "p", "Q", "q", "R", "r",
"S", "s", "T", "t", "U", "u", "V", "v",
"W", "w", "X", "x", "Y", "y", "Z", "z",
"0", "1", "2", "3", "4", "5", "6", "7",
"8", "9"
};

// Letter Unmatch String Array
std::string Letter_Unmatch_String[62] = { "1441", "1341", "1443", "1343", 
"1442", "1342", "1414", "1314", "1411", "1311", "1413", "1313",
"1412", "1312", "1434", "1334", "1431", "1331", "1433", "1333",
"1432", "1332", "1424", "1324", "1421", "1321", "1423", "1323",
"1422", "1322", "1144", "1244", "1141", "1241", "1143", "1243",
"1142", "1242", "1114", "1214", "1111", "1211", "1113", "1213",
"1112", "1212", "1134", "1234", "1131", "1231", "1133", "1233",
"4244", "4241", "4243", "4242", "4214", "4211", "4213", "4212",
"4234", "4231"
};

// Initialize Obfuscation String Array
std::string Obfuscation_String[62] = { "41141441", "41421441", "41141442", "41421442", 
"41141443", "41411443", "41144114", "41424114", "41144141", "41424141", "41144142", "41424142",
"41144143", "41424143", "41144214", "41424214", "41144241", "41424241", "41144242", "41424242",
"41144243", "41424243", "41144314", "41424314", "41144341", "41424341", "41144342", "41424342",
"41144343", "41424343", "41411414", "41431414", "41411441", "41431441", "41411442", "41431442",
"41411442", "41431443", "41414114", "41434114", "41414141", "41434141", "41414142", "41434142",
"41414143", "41434143", "41414214", "41434214", "41414241", "41434241", "41414242", "41434242",
"14431414", "14431441", "14431442", "14431443", "14434114", "14434141", "14434142", "14434143",
"14434214", "14434241"
};

// Initialize a space used for cleaneliness
std::string blank_space[1] = { " " };

// Logging of each whole word movement (hard limit of 5000 words per obfuscation)
std::string Word_Letter_Movements[5000] = {};

// Logging of each word character movement (hard limit of 5000 characters per obfuscation)
std::string Word_Character_Movements[5000] = {};

// Key teeth, used for layer 2 (hard limit of 5000 lines available for cache placement)
std::string Ob_Cache[5000] = {};

// Preliminary obfuscation proceedure
void character_obfuscation(std::string contract_input, std::string contract_alias, int contract_type, bool layer_2)// TODO: Refactor contract_alias to be able to write later as we want more than just character obbing
{
    // Clear any leftovers in Global from previous run
    SecondLayer_Key = "";
    SingleLayer_Key = "";
    Obfuscated_Combined_String = "";
    Obfuscated_String = "";
    // Set input string
    char Input_String[contract_input.length()+contract_alias.length()];
    strcpy(Input_String, contract_input.c_str());
    // Returns first word
    char *wrdcount = strtok(Input_String, " ");// TODO: Explore cleaning with blank_space[0]

    // Log word count
    int word_total = 1;
    // Log letter count
    int letter_total = 1;
    // Define character loop position
    int character_detection_loop = 0;
    
    // Keep looping through "tokens" while one of the
    // delimiters present in str[]. 
    while (wrdcount != NULL) 
    { 
        // Print for debugging
        //LogPrintf("CharacterObfuscation - current word total: %u, processing word: %s \n", word_total, wrdcount);

        // Word obfuscation
        //
        // Log letter count
        int letter_total_word = 0;
        // Returns word as string for letters
        std::string ltrcount = wrdcount;
        char ch_ltrcount = ltrcount.at(letter_total_word);
        std::string ltrcount_buf = "_";
        char ch_ltrcount_buf = ltrcount_buf.at(0);
        std::string str_ch_ltrcount;
        str_ch_ltrcount.push_back(ch_ltrcount);
        ltrcount.push_back(ch_ltrcount_buf);
        // set word letter count
        int letter_loop_total = ltrcount.length()-1;
        // reset character loop position
        character_detection_loop = 0;

        // Obfuscate letters
        while(letter_total_word < letter_loop_total)
        {
            // Character obfuscation
            //

            // Print for debugging
            //LogPrintf("CharacterObfuscation - tokenize word |%s| by letter, current letter: %s \n", wrdcount, str_ch_ltrcount);

            // loop until we match up string character with known characters
            while(character_detection_loop < 62)
            {
                if(str_ch_ltrcount == Character_String[character_detection_loop])
                {
                    // Break loop when letter match is found
                    break;
                }
                // Move up in loop count if not matched
                character_detection_loop ++;
            }

            // unmatch letter and add obfuscation tail data
            Obfuscated_String += (Letter_Unmatch_String[character_detection_loop] + Obfuscation_String[character_detection_loop]);
            // add syntax safe letter buffer
            Obfuscated_String += Character_String[6];

            // add onto letter count for word
            letter_total ++;
            letter_total_word ++;

            // Reset character detection loop
            character_detection_loop = 0;

            // Move to next letter
            ch_ltrcount = ltrcount.at(letter_total_word);
            str_ch_ltrcount = "";
            str_ch_ltrcount.push_back(ch_ltrcount);
        }

        // Only run layer 2 obfuscation depth if requested
        if(layer_2)
        {
            // Generate pivots/shifts
            obfuscation_shift(Obfuscated_String.length(), Obfuscated_String, true);
        }

        // Set obfuscated values
        Obfuscated_Combined_String += (Obfuscated_String + Character_String[4]);
        Obfuscated_String = "";

        // Move to next word
        wrdcount = strtok(NULL, " ");

        // Break loop if maximum obfuscatable word count is reached
        if(word_total >= 5000)
        {
            break;
        }

        // Move up in word count
        word_total ++;
    }

    // Print for debugging
    //LogPrintf("CharacterObfuscation - cleartext data: %s \n", Obfuscated_Combined_String);

    // Obfuscate whole output data
    if(contract_type != 3) {
        obfuscation_shift(Obfuscated_Combined_String.length(), Obfuscated_Combined_String, false);

        // Print for debugging
        LogPrintf("CharacterObfuscation - encrypted data: %s \n", Obfuscated_Combined_String);

        // Write read key to end of output
        Obfuscated_Combined_String += (Character_String[0] + SingleLayer_Key + Character_String[0]);
        // Print for debugging
        LogPrintf("CharacterObfuscation - embedded PubKey: %s \n", (Character_String[0] + SingleLayer_Key + Character_String[0]));

    }

    // Print for debugging
    LogPrintf("CharacterObfuscation - finished \n");

    // Log total input
    input_length = word_total;
    // Log total character input
    input_character_length = letter_total;

    // --
    // TEST FUNCTION
    // --
    // write obfuscated string to fractal engine
    write_contractDATA(Obfuscated_Combined_String, contract_alias, contract_type);
  
    return; 
}

// Determined obfuscation logic shifts
void obfuscation_shift(int input_data_shift, std::string input_data_text, bool char_ob)
{ 
    // Print for debugging
    LogPrintf("Obfuscation_Shift - Starting \n");

    // Decide data type handling
    if(char_ob) {
        //
        ignition(input_data_shift, input_data_text);

    } else {
        //
        flameout(input_data_shift, input_data_text);
    }


  
    return; 
} 

// Setup the obfuscation engine
void priming(std::string contract_input, std::string contract_alias, int contract_type, bool layer_2)
{
    // Run Preliminary obfuscation proceedure
    character_obfuscation(contract_input, contract_alias, contract_type, layer_2);
}

// Ignition of obfuscation engine
void ignition(int input_data_shift, std::string input_data_text)  {
    // Generate random seed data for rounds, and shifts/pivots
    int64_t seed_1, seed_2, seed_3, pre_shift, pre_pivot;
    double seed_tmp = 0;
    int seed_loop = ((input_data_shift + 6) / 3);
    bool collision = false;

    // Print for debugging
    LogPrintf("ignition() - Character Obfuscation Run Selected \n");
    // Set basic seed data
    seed_1 = GetTime();
    seed_2 = pindexBest->GetBlockTime();
    seed_3 = input_data_shift;
    // Create basic rand seed
    seed_tmp = (seed_1 + seed_2) / seed_3;
    // Print for debugging
    LogPrintf("ignition() - seed_1: %u , seed_2: %u , seed_3: %u, seed_tmp: %u \n", seed_1, seed_2, seed_3, seed_tmp);
    // Loop until seed satisfies parameters
    while(seed_tmp > (seed_3 * 10)) {
        // Reduce value
        seed_tmp /= 10;
    }
    // Verfiy lower sanity
    while(seed_tmp < 2) {
        // Raise value
        seed_tmp *= 10;
    }
    // Print for debugging
    LogPrintf("ignition() - Sanity correction seed_tmp now: %u \n", seed_tmp);
    // Set shift values
    pre_shift = seed_tmp + (seed_3 / 2);
    pre_pivot = ((pre_shift - 1) + seed_tmp) / seed_3;
    // Print for debugging
    LogPrintf("ignition() - pre_shift: %u , pre_pivot: %u \n", pre_shift, pre_pivot);
    // Set individual characters to obfuscate
    int letter_loop_total = input_data_text.length();
    char ch_inputcount[letter_loop_total];
    char ch_outputcount[letter_loop_total];
    int logged_used_positions[letter_loop_total];
    strcpy(ch_inputcount, input_data_text.c_str());
    //
    int col_rep = letter_loop_total;
    //
    int64_t ob_move = (pre_pivot + pre_shift) * (seed_loop / 3);
    //
    seed_loop = 0;
    // Print for debugging
    LogPrintf("ignition() [preloop] - move attempt size: %u \n", ob_move);
    // Bring down move within sanity window
    if(ob_move > seed_3) {
        // Print for debugging
        LogPrintf("ignition() [preloop] - dropping move size to fulfill sanity \n");
        while(ob_move > seed_3) {
            ob_move /= 2;
        }
        // Print for debugging
        LogPrintf("ignition() [preloop] - adjust move size: %u \n", ob_move);
    }
    //
    logged_used_positions[seed_loop] = ob_move;
    // Print for debugging
    LogPrintf("ignition() [preloop] - logging used position: %u \n", logged_used_positions[seed_loop]);
    // Set jumbled array data
    ch_outputcount[seed_loop] = ch_inputcount[ob_move];
    //
    seed_loop ++;

    // Loop through and set obfuscate
    while(seed_loop < seed_3) {
        // Print for debugging
        LogPrintf("ignition() - obfuscation loop: %u \n", seed_loop);

        if(!collision) {
            //
            ob_move = (pre_pivot + pre_shift) * (seed_loop / 3);
            // Print for debugging
            LogPrintf("ignition() - move attempt size: %u \n", ob_move);
            // Bring down move within sanity window
            if(ob_move > seed_3) {
                // Print for debugging
                LogPrintf("ignition() - dropping move size to fulfill sanity \n");
                while(ob_move > seed_3) {
                    ob_move /= 2;
                }
                // Print for debugging
                LogPrintf("ignition() - adjust move size: %u \n", ob_move);
            }
        }

        int l = 0;
        bool san_run = false;
        while(l < seed_loop) {
            // Print for debugging
            LogPrintf("ignition() - Running collision check \n");
            //
            if(logged_used_positions[l] == ob_move) {
                //
                collision = true;
                san_run = true;
                // Print for debugging
                LogPrintf("ignition() - Collision occured at attempt index: %u, attempted move index: %u \n", seed_loop, logged_used_positions[l]);
                // Break out of loop after collision
                break;
            }
            //
            if(l == (seed_loop - 1)) {
                // Print for debugging
                LogPrintf("ignition() - checking last collision loop for collision handling \n");
                if(!san_run && collision) {
                    collision = false;
                    // Print for debugging
                    LogPrintf("ignition() - Collision resolved at attempt index: %u, attempted move index: %u \n", seed_loop, logged_used_positions[l]);
                }
            }
            //
            l ++;
        }
        //
        if(!collision) {
            //
            logged_used_positions[seed_loop] = ob_move;
            // Print for debugging
            LogPrintf("ignition() - logging used position: %u \n", logged_used_positions[seed_loop]);
            // Set jumbled array data
            ch_outputcount[seed_loop] = ch_inputcount[ob_move];
            // Set global stored values
            Ob_Cache[seed_loop] = ch_outputcount[seed_loop];
            Word_Character_Movements[seed_loop] = std::to_string(ob_move);
            // Print for debugging
            LogPrintf("ignition() - Done! origin: %u, placement: %u \n", seed_loop, ob_move);
            // Move up in loop position
            seed_loop ++;
        } else {
            //
            if(col_rep > 0) {
                //
                col_rep --;

                //
                ob_move = col_rep;
            }
            // Print for debugging
            LogPrintf("ignition() - Re-Loop due to collision [new placement: %u] \n", ob_move);
        }
    }
    // Generate obfuscated string and key
    keyMaster(seed_3, true);
}

// Flameout at obfuscation threshold
void flameout(int input_data_shift, std::string input_data_text) {
    // Generate random seed data for rounds, and shifts/pivots
    int64_t seed_1, seed_2, seed_3, pre_shift, pre_pivot;
    double seed_tmp = 0;
    int seed_loop = ((input_data_shift + 6) / 3);
    int KeyTeeth_Loop = 0;
    bool collision = false;

    // Print for debugging
    LogPrintf("ignition() - Character Obfuscation Run Selected \n");
    // Set basic seed data
    seed_1 = GetTime();
    seed_2 = pindexBest->GetBlockTime();
    seed_3 = input_data_shift;
    // Create basic rand seed
    seed_tmp = (seed_1 + seed_2) / seed_3;
    // Print for debugging
    LogPrintf("ignition() - seed_1: %u , seed_2: %u , seed_3: %u, seed_tmp: %u \n", seed_1, seed_2, seed_3, seed_tmp);
    // Loop until seed satisfies parameters
    while(seed_tmp > (seed_3 * 10)) {
        // Reduce value
        seed_tmp /= 10;
    }
    // Verfiy lower sanity
    while(seed_tmp < 2) {
        // Raise value
        seed_tmp *= 10;
    }
    // Print for debugging
    LogPrintf("ignition() - Sanity correction seed_tmp now: %u \n", seed_tmp);
    // Set shift values
    pre_shift = seed_tmp + (seed_3 / 2);
    pre_pivot = ((pre_shift - 1) + seed_tmp) / seed_3;
    // Print for debugging
    LogPrintf("ignition() - pre_shift: %u , pre_pivot: %u \n", pre_shift, pre_pivot);
    // Set individual characters to obfuscate
    int letter_loop_total = input_data_text.length();
    char ch_inputcount[letter_loop_total];
    char ch_outputcount[letter_loop_total];
    int logged_used_positions[letter_loop_total];
    strcpy(ch_inputcount, input_data_text.c_str());
    //
    int col_rep = letter_loop_total;
    //
    int64_t ob_move = (pre_pivot + pre_shift) * (seed_loop / 3);
    //
    seed_loop = 0;
    // Print for debugging
    LogPrintf("ignition() [preloop] - move attempt size: %u \n", ob_move);
    // Bring down move within sanity window
    if(ob_move > seed_3) {
        // Print for debugging
        LogPrintf("ignition() [preloop] - dropping move size to fulfill sanity \n");
        while(ob_move > seed_3) {
            ob_move /= 2;
        }
        // Print for debugging
        LogPrintf("ignition() [preloop] - adjust move size: %u \n", ob_move);
    }
    //
    logged_used_positions[seed_loop] = ob_move;
    // Print for debugging
    LogPrintf("ignition() [preloop] - logging used position: %u \n", logged_used_positions[seed_loop]);
    // Set jumbled array data
    ch_outputcount[ob_move] = ch_inputcount[seed_loop];
    // Set global stored values
    Word_Letter_Movements[seed_loop] = (std::to_string(ob_move) + Character_String[2]);
    //
    seed_loop ++;

    // Loop through and set obfuscate
    while(seed_loop < seed_3) {
        // Print for debugging
        LogPrintf("ignition() - obfuscation loop: %u \n", seed_loop);

        if(!collision) {
            //
            ob_move = (pre_pivot + pre_shift) * (seed_loop / 3);
            // Print for debugging
            LogPrintf("ignition() - move attempt size: %u \n", ob_move);
            // Bring down move within sanity window
            if(ob_move > seed_3) {
                // Print for debugging
                LogPrintf("ignition() - dropping move size to fulfill sanity \n");
                while(ob_move > seed_3) {
                    ob_move /= 2;
                }
                // Print for debugging
                LogPrintf("ignition() - adjust move size: %u \n", ob_move);
            }
        }

        int l = 0;
        bool san_run = false;
        while(l < seed_loop) {
            // Print for debugging
            LogPrintf("ignition() - Running collision check \n");
            //
            if(logged_used_positions[l] == ob_move) {
                //
                collision = true;
                san_run = true;
                // Print for debugging
                LogPrintf("ignition() - Collision occured at attempt index: %u, attempted move index: %u \n", seed_loop, logged_used_positions[l]);
                // Break out of loop after collision
                break;
            }
            //
            if(l == (seed_loop - 1)) {
                // Print for debugging
                LogPrintf("ignition() - checking last collision loop for collision handling \n");
                if(!san_run && collision) {
                    collision = false;
                    // Print for debugging
                    LogPrintf("ignition() - Collision resolved at attempt index: %u, attempted move index: %u \n", seed_loop, logged_used_positions[l]);
                }
            }
            //
            l ++;
        }
        //
        if(!collision) {
            //
            logged_used_positions[seed_loop] = ob_move;
            // Print for debugging
            LogPrintf("ignition() - logging used position: %u \n", logged_used_positions[seed_loop]);
            // Set jumbled array data
            ch_outputcount[ob_move] = ch_inputcount[seed_loop];
            // Set global stored values
            Word_Letter_Movements[seed_loop] = (std::to_string(ob_move) + Character_String[2]);
            // Print for debugging
            LogPrintf("ignition() - Done! origin: %u, placement: %u \n", seed_loop, ob_move);
            // Move up in loop position
            seed_loop ++;
        } else {
            //
            if(col_rep > 0) {
                //
                col_rep --;

                //
                ob_move = col_rep;
            }
            // Print for debugging
            LogPrintf("ignition() - Re-Loop due to collision [new placement: %u] \n", ob_move);
        }
    }
    // Generate Key Teeth data (globally)
    while(KeyTeeth_Loop < seed_3) {
        Ob_Cache[KeyTeeth_Loop] = ch_outputcount[KeyTeeth_Loop];
        KeyTeeth_Loop ++;
    }
    // Generate obfuscated string and key
    keyMaster(seed_3, false);
}

void keyMaster(int loop_threshold, bool layer_2) {
    // Set loop start
    int cur_loop = 0;
    // Initialize string for assembled data
    std::string Teeth_assembled;
    std::string Key_assembled;
    // Loop through until assembled
    while(cur_loop < loop_threshold) {
        // Assemble key teeth
        Teeth_assembled += Ob_Cache[cur_loop];
        if(layer_2) {
            Key_assembled += Word_Character_Movements[cur_loop];
        } else {
            Key_assembled += Word_Letter_Movements[cur_loop];
        }
        // move up in loop round
        cur_loop ++;
    }
    // Set globally used data
    if(layer_2) {
        Obfuscated_String = Teeth_assembled;
        SecondLayer_Key += Key_assembled;
    } else {
        Obfuscated_Combined_String = Teeth_assembled;
        SingleLayer_Key = Key_assembled;
    }

}

void gateKeeper(std::string contract_decode, int contract_type) {
    // Clean previous run data values
    PlainText_Combined_String = "";
    // Open requested contract data
    LogPrintf("gateKeeper - INFO - Opening file %s for decoding\n", contract_decode);
    std::ifstream file;
    file.open(contract_decode.c_str());
    if(!file.is_open()) {
        // Print for debugging
        LogPrintf("gateKeeper - ERROR 00 - Cannot open file for decoding!\n");
        return;
    } else {
        // Print for debugging
        LogPrintf("gateKeeper - INFO - File successfully opened!\n");
    }
    // Set read data
    std::string line;
    std::string key;
    std::string teeth;
    Obfuscated_Combined_String = "";
    // Read data
    while(file.good()) {
        // Print for debugging
        LogPrintf("gateKeeper - INFO - Reading file...\n");
        // Loop through lines
        std::getline(file, line);
        // Print for debugging
        LogPrintf("gateKeeper - INFO - Got line data...\n");
        if (!line.empty()) {
            if (line[0] != '#') {
                // Print for debugging
                LogPrintf("gateKeeper - INFO - Read data success!\n");
                // Combine input string
                Obfuscated_Combined_String += line;
                // Print for debugging
                LogPrintf("gateKeeper - INFO - Set data success!\n");
            } else {
                // Print for debugging
                LogPrintf("gateKeeper - WARNING - Comment detected! \n");
            }
        } else {
            // Print for debugging
            LogPrintf("gateKeeper - WARNING - Empty line detected! \n");
        }
    }
    file.close();
    // Handle individual contract types
    if(contract_type != 3) {
        // Set Teeth data
        std::string ob_sets = Obfuscated_Combined_String.substr(0, Obfuscated_Combined_String.find("A"));
        teeth = ob_sets;
        // Set Key as focus
        ob_sets = strtok(NULL, "A");// TODO: verify use
        Obfuscated_Combined_String.erase(0, Obfuscated_Combined_String.find("A") + std::string("A").length());
        ob_sets = Obfuscated_Combined_String.substr(0, Obfuscated_Combined_String.find("A"));
        key = ob_sets;
    } else {
        // Set Teeth data
        std::string ob_sets = Obfuscated_Combined_String.substr(0, Obfuscated_Combined_String.find("|"));
        teeth = ob_sets;
        key = "BLANK";
    }
    // Print for debugging
    //LogPrintf("gateKeeper - INFO - sending contract data to re-assembler! | teeth: %s | key: %s | \n", teeth, key);
    if(contract_type != 3) {
        // Run re-assembly function
        reassembly(teeth, key);
    } else {
        //
        PlainText_Combined_String = teeth;
    }
    // Finish deob sequences
    character_deob(PlainText_Combined_String, contract_type);
}

// Reassemble a flameout
void reassembly(std::string input1, std::string input2) {
    // Print for debugging
    LogPrintf("reassembly - INFO - starting... \n");
    // Set local values from global
    std::string builder_key = input2;
    std::string builder_teeth = input1;
    int64_t key_size = input2.length();
    int64_t teeth_size = input1.length();
    // Set loop start
    int cur_loop = 0;
    // Set first phase loop threshold
    int loop_threshold = key_size;
    int clean_threshold = 0;
    std::string str_grab_loop = "B";
    while(cur_loop < loop_threshold) {
        // Count teeth peaks and valleys
        if(builder_key.at(cur_loop) == str_grab_loop.at(0)) {
            clean_threshold ++;
        }
        cur_loop ++;
    }
    // Reset looping logic
    cur_loop = 0;
    // Print for debugging
    LogPrintf("reassembly - INFO - set clean loop threshold: %u \n", clean_threshold);
    // Set input string
    char Key_input[key_size+1];
    char Teeth_input[teeth_size+1];
    std::string input_buf = "_";
    char ch_input_buf = input_buf.at(0);
    builder_key.push_back(ch_input_buf);
    builder_teeth.push_back(ch_input_buf);
    strcpy(Key_input, builder_key.c_str());
    strcpy(Teeth_input, builder_teeth.c_str());
    // Set key array data
    char *ob_sets = strtok(Key_input, "B");
    std::string str_ob_sets = std::string(ob_sets);
    // Print for debugging
    LogPrintf("reassembly - INFO - set first movement from position: 0 to position: %u \n", str_ob_sets);
    // Loop through and extract key data
    while(cur_loop < clean_threshold) {
        // Convert value for proper comparison
        std::string::size_type string_sz;
        int deob_set = std::stoi (str_ob_sets, &string_sz);
        // Print for debugging
        LogPrintf("reassembly - INFO - found movement at position: %u to position: %u \n", cur_loop, deob_set);
        // Set key array data
        std::string str_ch_convrt;
        str_ch_convrt.push_back(Teeth_input[deob_set]);
        // Print for debugging
        LogPrintf("reassembly - INFO - set decoded value: %u \n", str_ch_convrt);
        Ob_Cache[cur_loop] = str_ch_convrt;
        // Move up in loop round
        cur_loop ++;
        // Move onto next key piece
        ob_sets = strtok(NULL, "B");
        str_ob_sets = std::string(ob_sets);
    }
    // Reset looping logic
    cur_loop = 0;
    // Assemble decoded data
    while(cur_loop < clean_threshold) {
        // Set combined data
        PlainText_Combined_String += Ob_Cache[cur_loop];
        // Move up in loop round
        cur_loop ++;
    }
    // Print for debugging
    LogPrintf("reassembly - OUTPUT - %s \n", PlainText_Combined_String);
}

void character_deob(std::string to_deob, int contract_type) {
    // Print for debugging
    LogPrintf("deob - INFO - starting... \n");
    // Set input string
    std::string ob_count = to_deob.substr(0, to_deob.find("C"));
    // Set found count
    int log_found = 0;
    // Set starting loop position
    int start_position = 0;
    // Print for debugging
    LogPrintf("deob - INFO - Set base values... \n");
    // Loop through all words in input
    size_t position = 0;
    while((position = to_deob.find("C")) != std::string::npos) {
        // Set array data
        Ob_Cache[log_found] = ob_count;
        // Log word count found
        log_found ++;
        // Break loop if maximum de-obfuscatable word count is reached
        if(log_found >= 5000)
        {
            break;
        }
        to_deob.erase(0, to_deob.find("C") + std::string("C").length());
        ob_count = to_deob.substr(0, to_deob.find("C"));

    }

    // Print for debugging
    LogPrintf("deob - INFO - Processed through unmatched words... \n");

    // Log word count
    int word_total = 1;
    // Log letter count
    int letter_total = 1;
    // Define character loop position
    int character_detection_loop = 0;

    // Clear any leftovers in Global from previous run
    PlainText_String = "";
    PlainText_Combined_String = "";
    fTokenDecodeSuccess = false;

    // Loop through input data and parse
    while (start_position < log_found)
    {
        // Print for debugging
        //LogPrintf("Character_DeOb - current word total: %u, processing word: %s \n", word_total, Ob_Cache[start_position]);

        // Word de-obfuscation
        //

        // Log letter count
        int letter_total_word = 0;
        // Set word as string for looping logic
        std::string wrdcount = Ob_Cache[start_position];

        // Padd incoming value set
        std::string catch_buf = "0";
        char ch_catch_buf = catch_buf.at(0);
        wrdcount.push_back(ch_catch_buf);

        // Set letter division string
        char division_String[wrdcount.length()];
        strcpy(division_String, wrdcount.c_str());

        // Returns word as string for letters
        char *ob_bits = strtok(division_String, "D");
        std::string ltrcount = ob_bits;

        // Print for debugging
        //LogPrintf("Character_DeOb - First letter is: %s, in word: %s \n", ltrcount, wrdcount);

        // set word letter count
        int letter_loop_total = 0;
        // set letter loop count
        int ltr_loop = 0;
        int ltr_threshold = wrdcount.length();
        // Set catch value
        std::string letter_Catch = "D";

        // Set first value for comparison
        char ch_ob_count = wrdcount.at(0);
        std::string str_ch_ob_count;
        str_ch_ob_count.push_back(ch_ob_count);

        // Print for debugging
        //LogPrintf("Character_DeOb - Selected position ZERO, which is character: %s \n", str_ch_ob_count);

        // Create clean loop threshold
        while(ltr_loop < (ltr_threshold - 1))
        {
            //
            if(str_ch_ob_count == letter_Catch){
                //
                letter_loop_total ++;
                // Print for debugging
                //LogPrintf("Character_DeOb - Found letter in loop: %u \n", ltr_loop);
            } else {
                // Print for debugging
                //LogPrintf("Character_DeOb - Letter catch was unable to match: %s, to expected value: %s \n", str_ch_ob_count, letter_Catch);
            }
            // Move up in round position
            ltr_loop++;

            // Set next value
            ch_ob_count = wrdcount.at(ltr_loop);
            str_ch_ob_count = "";
            str_ch_ob_count.push_back(ch_ob_count);

        }
        // reset character loop position
        character_detection_loop = 0;

        // Print for debugging
        //LogPrintf("Character_DeOb - loop start position: %u, loop end position: %u \n", letter_total_word, letter_loop_total);

        // De-Obfuscate letters
        while(letter_total_word < letter_loop_total)
        {
            // Character de-obfuscation
            //

            // Print for debugging
            //LogPrintf("Character_DeOb - re-matching letter |%s| in word: %s \n", ltrcount, wrdcount);

            // loop until we match up string character with known characters
            while(character_detection_loop < 62)
            {
                if(ltrcount == (Letter_Unmatch_String[character_detection_loop] + Obfuscation_String[character_detection_loop]))
                {
                    // Break loop when letter match is found
                    break;
                }
                // Move up in loop count if not matched
                character_detection_loop ++;
            }

            // match letter and obfuscation tail data
            PlainText_String += Character_String[character_detection_loop];

            // add onto letter count for word
            letter_total ++;
            letter_total_word ++;

            // Reset character detection loop
            character_detection_loop = 0;

            // Move to next letter
            ob_bits = strtok(NULL, "D");
            ltrcount = std::string(ob_bits);
        }

        // Set plaintext values
        PlainText_Combined_String += (PlainText_String + " ");
        PlainText_String = "";

        // Move to next word
        start_position ++;

        // Break loop if maximum de-obfuscatable word count is reached
        if(word_total >= 5000)
        {
            break;
        }

        // Move up in word count
        word_total ++;
    }

    // Print for debugging
    LogPrintf("Character_DeOb - Finished de-obfuscation \n");
    //LogPrintf("Character_DeOb - PlainText Output: %s \n", PlainText_Combined_String);

    if(contract_type != 3) {
        // TESTING FUNCTION
        if(PlainText_Combined_String == "this is only a test ") {
            fTokenDecodeSuccess = true;
        }
    }
}
