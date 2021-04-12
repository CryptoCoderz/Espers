// Copyright (c) 2020-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// NOTICE!
//
// This is a completely experimental colored binary data storage and retrieval written by
// CryptoCoderz (Jonathan Dan Zaretsky - cryptocoderz@gmail.com)
// and
// SaltineChips (Jeremiah Cook - jeremiahwcook@gmail.com)
//
//
// PLEASE USE AT YOUR OWN RISK!!!
//

#include "fractaldvac.h"

// For Logprintf
#include "util/util.h"

#include <string>
#include <cstring>

using namespace std;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

std::string mydata = "CeyQ1FkJc4gwqLvRzCJv5CG8TW1Y2s1H"; // PubKey Example

// Initialize Valid Bit mapping Array
std::string Bit_String[4] = { "11", "00", "10", "01" };

// Initialize Valid Color mapping Array
// Black, White, Blue, Red
std::string Color_String[4] = { "000", "255255255", "00255", "25500" };

// Initialize Valid Character String Array
std::string DVAC_Character_String[62] = { "A", "a", "B", "b",
"C", "c", "D", "d", "E", "e", "F", "f",
"G", "g", "H", "h", "I", "i", "J", "j",
"K", "k", "L", "l", "M", "m", "N", "n",
"O", "o", "P", "p", "Q", "q", "R", "r",
"S", "s", "T", "t", "U", "u", "V", "v",
"W", "w", "X", "x", "Y", "y", "Z", "z",
"0", "1", "2", "3", "4", "5", "6", "7",
"8", "9"
};

// Initialize Valid Conversion data
std::string DVAC_Conversion_String[62] = { "01 00 00 01", "01 10 00 01", "01 00 00 10", "01 10 00 10",
"01 00 00 11", "01 10 00 11", "01 00 01 00", "01 10 01 00", "01 00 01 01", "01 10 01 01", "01 00 01 10", "01 10 01 10",
"01 00 01 11", "01 10 01 11", "01 00 10 00", "01 10 10 00", "01 00 10 01", "01 10 10 01", "01 00 10 10", "01 10 10 10",
"01 00 10 11", "01 10 10 11", "01 00 11 00", "01 10 11 00", "01 00 11 01", " 01 10 11 01", "01 00 11 10", "01 10 11 10",
"01 00 11 11", "01 10 11 11", "01 01 00 00", "01 11 00 00", "01 01 00 01", "01 11 00 01", "01 01 00 10", "01 11 00 10",
"01 01 00 11", "01 11 00 11", "01 01 01 00", "01 11 01 00", "01 01 01 01", "01 11 01 01", "01 01 01 10", "01 11 01 10",
"01 01 01 11", "01 11 01 11", "01 01 10 00", "01 11 10 00", "01 01 10 01", "01 11 10 01", "01 01 10 10", "01 11 10 10",
"00 00 00 00", "00 00 00 01", "00 00 00 10", "00 00 00 11", "00 00 01 00", "00 00 01 01", "00 00 01 10", "00 00 01 11",
"00 00 10 00", "00 00 10 01"
};

// Logging of each character count (hard limit of 5000 characters per encoding)
std::string Encoding_Letter_Count[5000] = {};

void enCode(string input_mydata) {
    // Set mydata value with input value from toggle
    mydata = input_mydata;
    int letter_total_word = 0;
    // Returns word as string for letters
    std::string ltrcount = mydata;
    char ch_ltrcount = ltrcount.at(letter_total_word);
    std::string ltrcount_buf = "_";
    //std::string ltrcount_buf2 = " ";
    char ch_ltrcount_buf = ltrcount_buf.at(0);
    std::string str_ch_ltrcount;
    str_ch_ltrcount.push_back(ch_ltrcount);
    ltrcount.push_back(ch_ltrcount_buf);
    // set word letter count
    int letter_loop_total = ltrcount.length()-1;
    // reset character loop position
    int character_detection_loop = 0;
    // Encoded data string
    std::string Encoded_String = "";

     while(letter_total_word < letter_loop_total) {
         // Print for debugging
         LogPrintf("DVAC - tokenize string |%s| by input type, current character: %s \n", ltrcount, str_ch_ltrcount);

         // loop until we match up sting character with known characters
         while(character_detection_loop < 62)
         {
             if(str_ch_ltrcount == DVAC_Character_String[character_detection_loop])
             {
                 // Print for debugging
                 LogPrintf("DVAC - matched character |%s| with: %s \n", str_ch_ltrcount, DVAC_Conversion_String[character_detection_loop]);
                 // Break loop when letter match is found
                 break;
             }
             // Move up in loop count if not matched
             character_detection_loop ++;
         }

         // encode letter
         Encoded_String = (DVAC_Conversion_String[character_detection_loop]);// + ltrcount_buf2);

         // store encoding
         Encoding_Letter_Count[letter_total_word] = Encoded_String;

         // add onto letter count for encoding
         letter_total_word ++;

         // Reset character detection loop
         character_detection_loop = 0;

         // Move to next letter
         ch_ltrcount = ltrcount.at(letter_total_word);
         str_ch_ltrcount = "";
         str_ch_ltrcount.push_back(ch_ltrcount);
	}

     printCODED(letter_loop_total, 16, 16, 3);
}

void printCODED(int char_TOTAL, int w, int h, int channels) {

    // Encoded data to write
    unsigned char data[w * h * channels];
    // Set primitives
    int char_LOOP = 0;
    int logic_bytes = 0;
    int logic_bytes2 = 1;
    int logic_match = 0;
    int logic_position = 0;
    int logic_batch = 0;
    int set_position = 0;
    int logic_threshold = (w * h);
    int logic_saturation = (char_TOTAL - 1);
    bool logic_dataend = false;
    // Toggle 5 digit RGB print
    std::string RGB_toggle_str = "2";
    int RGB_toggle_int = RGB_toggle_str.length();
    char RGB_toggle[RGB_toggle_int];
    strcpy(RGB_toggle, RGB_toggle_str.c_str());
    // RGB
    int r = int(255);
    int g = int(255);
    int b = int(255);
    // Set pixel primitives
    std::string array_input = Encoding_Letter_Count[set_position];
    std::string Bit_match_str = "";
    int letter_loop_total = array_input.length();
    // Set input string
    char ch_inputcount[letter_loop_total];
    strcpy(ch_inputcount, array_input.c_str());

    // Print for debugging
    LogPrintf("DVAC - setting data from input: %s \n", array_input);
    // Set data from encoded string
    while (logic_position < logic_threshold)
    {
        // Verify data inventory sanity
        if (logic_dataend)
        {
            // Print for debugging
            LogPrintf("DVAC - reached end of input data - SETTING BLANK DATA\n");
            // Write blank data at end of input data
            r = int(255);
            g = int(255);
            b = int(255);
            // Set RGB data into print data
            data[char_LOOP++] = r;
            data[char_LOOP++] = g;
            data[char_LOOP++] = b;
            // Move up in pixel position
            logic_position ++;
        } else {
            // Returns first word
            char *ch_input_position = strtok(ch_inputcount, " ");
            // Loop through encoding bits
            while (logic_batch < 4)
            {
                // Set encoded data characters
                Bit_match_str = ch_input_position;
                // Loop and match
                while (logic_match < 4)
                {
                    // Print for debugging
                    LogPrintf("DVAC - comparing bit data |%s| with: %s \n", Bit_match_str, Bit_String[logic_match]);
                    // Match
                    if (Bit_match_str == Bit_String[logic_match])
                    {
                        // Print for debugging
                        LogPrintf("DVAC - matched bit data |%s| with RGB data: %s \n", Bit_match_str, Color_String[logic_match]);

                        std::string RGB_str = Color_String[logic_match];
                        int RGB_str_total = RGB_str.length();
                        char RGB_char[RGB_str_total];
                        strcpy(RGB_char, RGB_str.c_str());
                        if (RGB_str_total > 3)
                        {
                            // Verify string logic size
                            if (RGB_str_total > 5)
                            {
                                // Set White color values
                                r = int(255);// TODO: this should reference arrays, pet peev, not a big deal
                                g = int(255);// TODO: this should reference arrays, pet peev, not a big deal
                                b = int(255);// TODO: this should reference arrays, pet peev, not a big deal
                            } else  {
                                // Toggle between Red/Blue
                                if (RGB_char[0] == RGB_toggle[0])
                                {
                                    // Set Red color values
                                    r = int(255);// TODO: this should reference arrays, pet peev, not a big deal
                                    g = int(RGB_char[3]);
                                    b = int(RGB_char[4]);
                                } else {
                                    // Set Blue color values
                                    r = int(RGB_char[0]);
                                    g = int(RGB_char[1]);
                                    b = int(255);// TODO: this should reference arrays, pet peev, not a big deal
                                }
                            }
                        } else {
                            // Set Black color values
                            r = int(RGB_char[0]);
                            g = int(RGB_char[1]);
                            b = int(RGB_char[2]);
                        }
                        break;
                    }
                    // Move up in match position
                    logic_match ++;
                }
                // Set RGB data into print data
                data[char_LOOP++] = r;
                data[char_LOOP++] = g;
                data[char_LOOP++] = b;
                // Set next logic array
                logic_bytes += 2;
                logic_bytes2 += 2;
                // Move in round
                logic_batch ++;
                // Move up in pixel position
                logic_position ++;
                // Reset match looping
                logic_match = 0;
                // Move to next word
                ch_input_position = strtok(NULL, " ");
            }
        }
        // Reset batch looping
        logic_batch = 0;
        // Check for end of input data
        if (set_position < logic_saturation) {
            // Set next input
            set_position ++;
            array_input = Encoding_Letter_Count[set_position];
            strcpy(ch_inputcount, array_input.c_str());
        } else {
            // Set end of input data
            logic_dataend = true;
        }
    }

    // Print image
    stbi_write_jpg("jpg_test_001.jpg", w, h, channels, data, 100);
}


#undef STB_IMAGE_WRITE_IMPLEMENTATION// TODO: verify this, good practice would be unload as needed...
