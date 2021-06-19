// Copyright (c) 2020-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// NOTICE!
//
// This is a completely experimental colored binary data storage and retrieval written by
// CryptoCoderz (Jonathan Dan Zaretsky - cryptocoderz@gmail.com)
// and
// SaltineChips (Jeremiah Cook - jeremiahwcook@gmail.com)
// dmEgc2xhdnUgZ29zcG9kZSBib2dlIGUgbmFzaCBzcGFzZXRhbCBlc3VzIGhyaXN0b3M=
//
//
// PLEASE USE AT YOUR OWN RISK!!!
//

#include "fractalbvac.h"

// For Logprintf
#include "util/util.h"
// For image read
#include "fractalnftbase.h"

// For threadsafe messages
#include "ui/ui_interface.h"

#include <string>
#include <cstring>

using namespace std;

std::string mydata = "CeyQ1FkJc4gwqLvRzCJv5CG8TW1Y2s1H"; // PubKey Example
std::string thatdata = ""; // Set and used for decode

// Initialize Valid Bit mapping Array
std::string Bit_String[4] = { "11", "00", "10", "01" };

// Initialize Valid Color mapping Array
// Black, White, Blue, Red
std::string Color_String[4] = { "000", "255255255", "00255", "25500" };
std::string Color_String_TEMP_FORMAT_WORKAROUND[4] = { "0 0 0", "255 255 255", "0 0 255", "255 0 0" };
// Absolute color values
int Color_String_Absolutes[2] = { 0, 255 };
// Acceptable color value window values
int Color_String_Window_Absolutes[2] = {90, 100};

// Initialize Valid Character String Array
std::string BVAC_Character_String[62] = { "A", "a", "B", "b",
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
std::string BVAC_Conversion_String[62] = { "01 00 00 01", "01 10 00 01", "01 00 00 10", "01 10 00 10",
"01 00 00 11", "01 10 00 11", "01 00 01 00", "01 10 01 00", "01 00 01 01", "01 10 01 01", "01 00 01 10", "01 10 01 10",
"01 00 01 11", "01 10 01 11", "01 00 10 00", "01 10 10 00", "01 00 10 01", "01 10 10 01", "01 00 10 10", "01 10 10 10",
"01 00 10 11", "01 10 10 11", "01 00 11 00", "01 10 11 00", "01 00 11 01", "01 10 11 01", "01 00 11 10", "01 10 11 10",
"01 00 11 11", "01 10 11 11", "01 01 00 00", "01 11 00 00", "01 01 00 01", "01 11 00 01", "01 01 00 10", "01 11 00 10",
"01 01 00 11", "01 11 00 11", "01 01 01 00", "01 11 01 00", "01 01 01 01", "01 11 01 01", "01 01 01 10", "01 11 01 10",
"01 01 01 11", "01 11 01 11", "01 01 10 00", "01 11 10 00", "01 01 10 01", "01 11 10 01", "01 01 10 10", "01 11 10 10",
"00 00 00 00", "00 00 00 01", "00 00 00 10", "00 00 00 11", "00 00 01 00", "00 00 01 01", "00 00 01 10", "00 00 01 11",
"00 00 10 00", "00 00 10 01"
};

// Logging of each character count (hard limit of 5000 characters per encoding)
std::string Encoding_Letter_Count[5000] = {};

// Logging of each character count (hard limit of 5000 characters per encoding)
std::string Decoding_Letter_Count[5000] = {};

// Logging of each character count (hard limit of 5000 characters per encoding)
std::string Decoding_Bits_Count[5000] = {};

// Logging of succesfull run
bool BVAC_run = false;

void enCode(string input_mydata, string input_alias) {
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
         LogPrintf("BVAC - tokenize string |%s| by input type, current character: %s \n", ltrcount, str_ch_ltrcount);

         // loop until we match up sting character with known characters
         while(character_detection_loop < 62)
         {
             if(str_ch_ltrcount == BVAC_Character_String[character_detection_loop])
             {
                 // Print for debugging
                 LogPrintf("BVAC - matched character |%s| with: %s \n", str_ch_ltrcount, BVAC_Conversion_String[character_detection_loop]);
                 // Break loop when letter match is found
                 break;
             }
             // Move up in loop count if not matched
             character_detection_loop ++;
         }

         // encode letter
         Encoded_String = (BVAC_Conversion_String[character_detection_loop]);// + ltrcount_buf2);

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

     printCODED(letter_loop_total, 16, 16, 3, input_alias);
}

void printCODED(int char_TOTAL, int w, int h, int channels, std::string passed_alias) {

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
    LogPrintf("BVAC - setting data from input: %s \n", array_input);
    // Set data from encoded string
    while (logic_position < logic_threshold)
    {
        // Verify data inventory sanity
        if (logic_dataend)
        {
            // Print for debugging
            LogPrintf("BVAC - reached end of input data - SETTING BLANK DATA\n");
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
                    LogPrintf("BVAC - comparing bit data |%s| with: %s \n", Bit_match_str, Bit_String[logic_match]);
                    // Match
                    if (Bit_match_str == Bit_String[logic_match])
                    {
                        // Print for debugging
                        LogPrintf("BVAC - matched bit data |%s| with RGB data: %s \n", Bit_match_str, Color_String[logic_match]);

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

    // Inform user of image generation
    uiInterface.ThreadSafeMessageBox("Your BVAC image has been generated!", "", CClientUIInterface::MSG_INFORMATION);
    // Print image
    write_image(passed_alias.c_str(), w, h, channels, data);
}

void deCode(std::string image_to_deCode) {
    BVAC_run = false;
    int width, height;
    int r, g, b;
    std::vector<unsigned char> image;
    std::string str_x, str_y, str_r, str_g, str_b;
    std::string bvacBUF = " ";
    // Set image data
    bool success = load_image(image, image_to_deCode, width, height);
    // Report failure
    if (!success)
    {
        // Print for debugging
        LogPrintf("BVAC deCode - ERROR - could not open or load image!\n");
        // Reset RGB(A) index value for next attempt
        n = 0;
        return;
    }
    // Hard limit maximum BVAC pixel size
    // Limit set to 16x16 for testing
    // This can be removed later
    // as BVAC only cares about amount of data
    // to decode and not really how many pixels are there
    else if ((width * height) > 256 || width != height)
    {
        // Print for debugging
        LogPrintf("NFTBASEparse - ERROR - image is not 16x16 pixels!\n");
        // Reset RGB(A) index value for next attempt
        n = 0;
        return;
    }

    // Print for debugging
    LogPrintf("BVAC Decode - Image dimensions (width x height): %u %u\n", width, height);

    // Define pixel position
    int x = 1;
    int y = 1;
    int p = 1;
    int positionLOOP = 0;
    int yLOOP = 1;

    // Either 3 or 4
    size_t RGBA = n;
    size_t index = 0;

    if (RGBA == 4)
    {
        // Throw error
        BVAC_run = false;
        // Print for debugging
        LogPrintf("BVAC Decode - ERROR - Invalid RGB type: expected RGB, parsed RGBA\n");
        // Reset RGB(A) index value for next attempt
        n = 0;
        return;
    }

    // Loop handling for a 16x16 image
    while (positionLOOP < (width * height))
    {
        // Handle RGB decoding
        r = static_cast<int>(image[index + 0]);
        g = static_cast<int>(image[index + 1]);
        b = static_cast<int>(image[index + 2]);

        // Handle RGB degradation
        if (r < Color_String_Window_Absolutes[0])
        {
            // Set absolute
            r = Color_String_Absolutes[0];
        } else if (r > Color_String_Window_Absolutes[1])
        {
            // Set absolute
            r = Color_String_Absolutes[1];
        }

        // Handle RGB degradation
        if (g < Color_String_Window_Absolutes[0])
        {
            // Set absolute
            g = Color_String_Absolutes[0];
        } else if (g > Color_String_Window_Absolutes[1])
        {
            // Set absolute
            g = Color_String_Absolutes[1];
        }

        // Handle RGB degradation
        if (b < Color_String_Window_Absolutes[0])
        {
            // Set absolute
            b = Color_String_Absolutes[0];
        } else if (b > Color_String_Window_Absolutes[1])
        {
            // Set absolute
            b = Color_String_Absolutes[1];
        }


        // Move to get next pixel of RGB
        index += RGBA;

        str_x = std::to_string(x);
        str_y = std::to_string(y);
        str_r = std::to_string(r);
        str_g = std::to_string(g);
        str_b = std::to_string(b);
        // Print for debugging
        LogPrintf("BVAC Decode - Pixel |%u| Position x=%s y=%s - RGB data parsed: %s, %s, %s\n", p, str_x, str_y, str_r, str_g, str_b);
        // Write data to array
        Decoding_Letter_Count[positionLOOP] = str_r + bvacBUF + str_g + bvacBUF + str_b;
        // Move up in loop logic and pixel position
        if (yLOOP == width)
        {
            y = 0;
            yLOOP = 0;
            x++;
        }
        y++;
        p++;
        yLOOP++;
        positionLOOP++;
    }

    // Reset RGB(A) index value for next attempt
    n = 0;

    // Match data and get ready for use
    match_deCode(Decoding_Letter_Count[0]);
}

void match_deCode(std::string input_thatdata) {
    // Set mydata value with input value from toggle
    thatdata = input_thatdata;
    int letter_total_word = 0;
    // Returns word as string for letters
    std::string str_ch_ltrcount = thatdata;
    // set word letter count
    int letter_loop_total = 4;// 34 more after
    // reset character loop position
    int character_detection_loop = 0;
    int bits_match_loop = 0;
    // Encoded data string
    std::string Decoded_String = "";
    std::string Bitmatch_String = "";
    std::string bvacBUF = " ";

    // loop until we match RGB with known bit values
    while(bits_match_loop < 34)//size of pubkey
    {
        while(letter_total_word < letter_loop_total)
        {
            LogPrintf("BVAC Decoding - decoding string |%s| by matching RGB values \n", str_ch_ltrcount);

            // loop until we match up string character with known characters
            while(character_detection_loop < 4)
            {
                if(str_ch_ltrcount == Color_String_TEMP_FORMAT_WORKAROUND[character_detection_loop])
                {
                    // Print for debugging
                    LogPrintf("BVAC Decoding - matched value |%s| with: %s \n", str_ch_ltrcount, Bit_String[character_detection_loop]);
                    // Break loop when letter match is found
                    break;
                }
                // Move up in loop count if not matched
                character_detection_loop ++;
            }

            // decode bits
            if(letter_total_word == (letter_loop_total - 1)) {
                Bitmatch_String += (Bit_String[character_detection_loop]);
            } else {
                Bitmatch_String += (Bit_String[character_detection_loop] + bvacBUF);
            }

            // Reset character detection loop
            character_detection_loop = 0;

            // add onto values count for encoding
            letter_total_word ++;

            // Set  next values
            str_ch_ltrcount = Decoding_Letter_Count[letter_total_word];
        }

        // Reset outer loop
        letter_loop_total += 4;


        // Set grouped data into staging array
        // Staging array hardlimit 5000 character bit assortments
        Decoding_Bits_Count[bits_match_loop] = Bitmatch_String;
        // Clear bit match data
        Bitmatch_String = "";
        // Reset inner loop
        character_detection_loop = 0;
        // Move up in round
        bits_match_loop ++;
    }

    // Reset looping for next section
    bits_match_loop = 0;

    // loop until we match bit values with known characters
    while(bits_match_loop < 34)//size of pubkey
    {
        // Print for debugging
        LogPrintf("BVAC Decoding - processing bit cluster |%s| \n", Decoding_Bits_Count[bits_match_loop]);
        //
        while(character_detection_loop < 62) {
            if(Decoding_Bits_Count[bits_match_loop] == BVAC_Conversion_String[character_detection_loop])
            {
                // Print for debugging
                LogPrintf("BVAC Decoding - matched bit cluster |%s| with: %s \n", Decoding_Bits_Count[bits_match_loop], BVAC_Character_String[character_detection_loop]);

                Decoded_String += BVAC_Character_String[character_detection_loop];

                // Break loop when letter match is found
                break;
            }
            // Move up in loop count if not matched
            character_detection_loop ++;
        }
        // Reset character detection loop
        character_detection_loop = 0;

        bits_match_loop ++;
    }

    thatdata = Decoded_String;

    BVAC_run = true;

}
