// Copyright (c) 2019-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


// NOTICE!
//
// This is a completely experimental data obfuscation method written by
// CryptoCoderz (Jonathan Dan Zaretsky - cryptocoderz@gmail.com)
// and 
// SaltineChips (Jeremiah Cook - jeremiahwcook@gmail.com)
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
std::string SingleLayer_Key = "";
std::string SecondLayer_Key = "";

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

// Logging of each word character count (hard limit of 5000 words per obfuscation)
std::string Word_Letter_Count[5000] = {};

// Preliminary obfuscation proceedure
void character_obfuscation(std::string contract_input, std::string contract_alias, int contract_type)// TODO: Refactor contract_alias to be able to write later as we want more than just character obbing
{ 
    // Set input string
    char Input_String[contract_input.length()+contract_alias.length()];
    //std::string combined_string = contract_input + contract_alias;
    //memset(Input_String, 0, (contract_input.length()+contract_alias.length())*sizeof(int));
    strcpy(Input_String, contract_input.c_str());
    // Returns first word
    char *wrdcount = strtok(Input_String, " ");

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
        LogPrintf("CharacterObfuscation - current word total: %u, processing word: %s \n", word_total, wrdcount);

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
            LogPrintf("CharacterObfuscation - tokenize word |%s| by letter, current letter: %s \n", wrdcount, str_ch_ltrcount);

            // loop until we match up sting character with known characters
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

        // Generate pivots/shifts
        // obfuscation_shift(letter_total_word, Obfuscated_String, true);

        // Set obfuscated values
        Obfuscated_Combined_String += (Obfuscated_String + blank_space[0]);
        Obfuscated_String = "";

        // Move to next word
        wrdcount = strtok(NULL, " ");

        // Break loop if maximum obfuscatable word count is reached
        if(word_total >= 5000)
        {
            break;
        }

        Word_Letter_Count[word_total] = letter_total_word;

        // Move up in word count
        word_total ++;
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
    // Generate random seed data for rounds, and shifts/pivots
    int64_t seed_1, seed_2, seed_3, seed_tmp = 0;
    double pre_shift = 0;
    double pre_pivot = 0;

    // Decide data type handling
    if(char_ob) {
        seed_1 = GetTime();
        seed_2 = pindexBest->GetBlockTime();
        seed_3 = input_data_shift;

        seed_tmp = seed_1 + seed_2 + 0;// TODO finish here man
    } else {
        //
    }


  
    return; 
} 

// Setup the obfuscation engine
void priming(std::string contract_input, std::string contract_alias, int contract_type)
{
    // Run Preliminary obfuscation proceedure
    character_obfuscation(contract_input, contract_alias, contract_type);
    // Determine sifts/pivots
    //obfuscation_shift();

	// Set Input Data
    //std::string Output_String = Obfuscated_String;
 
	// Initialize Output String Array
    //std::string Output_String[input_length];

	// Shifting Points
    //int shifting_points[shifts];

	// Shifting Direction
	//
	// False == Right
	// True  == Left
    //bool shifting_direction = false;

	// Overlap Logic
    //int overlap_count = 0;
 
    // Print Strings
    //for (int i = 0; i < input_length; i++)
    //    std::cout << Output_String[i] << "\n";
}

// Ignition of obfuscation engine
void ignition()  {
    // Functionality
}

// Flameout at obfuscation threshold
void flameout() {
    // Functionality
}

// Reassemble a flameout
void reassembly() {
    //Functionality
}
