// Copyright (c) 2020-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// NOTICE!
//
// This is a completely experimental NFT data storage and retrieval written by
// CryptoCoderz (Jonathan Dan Zaretsky - cryptocoderz@gmail.com)
// and
// SaltineChips (Jeremiah Cook - jeremiahwcook@gmail.com)
// dmEgc2xhdnUgZ29zcG9kZSBib2dlIGUgbmFzaCBzcGFzZXRhbCBlc3VzIGhyaXN0b3M=
//
//
// PLEASE USE AT YOUR OWN RISK!!!
//

#include "fractalnft.h"

// For Logprintf
#include "util/util.h"
// For image read
#include "fractalnftbase.h"

// For threadsafe messages
#include "ui/ui_interface.h"

#include <string>
#include <cstring>

using namespace std;

std::string nft_data = "CeyQ1FkJc4gwqLvRzCJv5CG8TW1Y2s1H"; // PubKey Example
std::string NFTparserdata = ""; // Set and used for parser

// Logging of each character count (hard limit of 90000 characters per encoding)
std::string Encoding_Pixel_Count[90000] = {};

// Logging of each character count (hard limit of 90000 characters per encoding)
std::string Decoding_Pixel_Count[90000] = {};

// Logging of each character count (hard limit of 90000 characters per encoding)
std::string Dimension_Logic[90000] = {};

// Logging of succesfull run
bool NFT_run = false;

void NFTrender(string input_nft_data, string input_alias) {
    // Set nft_data value with input value from toggle
    nft_data = input_nft_data;
    int letter_total_word = 0;
    // Returns word as string for letters
    std::string ltrcount = nft_data;
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
         LogPrintf("NFT - tokenize string |%s| by input type, current character: %s \n", ltrcount, str_ch_ltrcount);

         // encode letter
         Encoded_String = " ";// TODO:

         // store encoding
         //Render_Pixel_Count[letter_total_word] = Encoded_String;

         // add onto letter count for encoding
         letter_total_word ++;

         // Reset character detection loop
         character_detection_loop = 0;

         // Move to next letter
         ch_ltrcount = ltrcount.at(letter_total_word);
         str_ch_ltrcount = "";
         str_ch_ltrcount.push_back(ch_ltrcount);
	}

     NFTprintRENDER(letter_loop_total, 256, 256, 4, input_alias);
}

void NFTprintRENDER(int char_TOTAL, int w, int h, int channels, std::string passed_alias) {

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
    std::string array_input = "";//Encoding_Letter_Count[set_position];
    std::string Bit_match_str = "";
    int letter_loop_total = array_input.length();
    // Set input string
    char ch_inputcount[letter_loop_total];
    strcpy(ch_inputcount, array_input.c_str());

    // Print for debugging
    LogPrintf("NFT - setting data from input: %s \n", array_input);
    // Set data from encoded string
    while (logic_position < logic_threshold)
    {
        // Verify data inventory sanity
        if (logic_dataend)
        {
            // Print for debugging
            LogPrintf("NFT - reached end of input data - SETTING BLANK DATA\n");
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
                    //LogPrintf("NFT - comparing bit data |%s| with: %s \n", Bit_match_str, Bit_String[logic_match]);
                    // Match
                    if (Bit_match_str == "")//Bit_String[logic_match]
                    {
                        // Print for debugging
                        //LogPrintf("NFT - matched bit data |%s| with RGB data: %s \n", Bit_match_str, Color_String[logic_match]);

                        std::string RGB_str = "";//Color_String[logic_match];
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
            array_input = "";//Encoding_Letter_Count[set_position];
            strcpy(ch_inputcount, array_input.c_str());
        } else {
            // Set end of input data
            logic_dataend = true;
        }
    }

    // Inform user of image generation
    uiInterface.ThreadSafeMessageBox("Your NFT image has been generated!", "", CClientUIInterface::MSG_INFORMATION);
    // Print image
    write_image(passed_alias.c_str(), w, h, channels, data);
}

void NFTparse(std::string image_to_deCode) {
    // Clear possible left over data
    NFTparserdata = "";
    // Set values
    NFT_run = false;
    NFTBASE_run = NFT_run;
    int width, height;
    int r, g, b, a;
    std::vector<unsigned char> image;
    std::string str_x, str_y, str_r, str_g, str_b, str_a;
    std::string nftBUF = "C";
    std::string nftPAD = " ";
    // Set image data
    bool success = load_image(image, image_to_deCode, width, height);
    // Report failure
    if (!success)
    {
        // Print for debugging
        LogPrintf("NFT Parser - ERROR - could not open or load image!\n");
        // Reset RGB(A) index value for next attempt
        n = 0;
        return;
    }
    // Hard limit maximum NFT pixel size
    // Limit set to 300x300 for testing
    // This can be removed later
    // as NFT only cares about amount of data
    // to decode and not really how many pixels are there
    else if ((width * height) > 90000 || width != height)
    {
        // Print for debugging
        LogPrintf("NFT Parser - ERROR - Image is either too large or invalid aspect ratio!\n");
        // Reset RGB(A) index value for next attempt
        n = 0;
        return;
    }

    // Print for debugging
    LogPrintf("NFT Decode - Image dimensions (width x height): %u %u\n", width, height);

    // Define pixel position
    int x = 1;
    int y = 1;
    int p = 1;
    int positionLOOP = 0;
    int yLOOP = 1;

    // Either 3 or 4
    size_t RGBA = n;
    size_t index = 0;

    if (RGBA != 4)
    {
        // Throw error
        //NFT_run = false;
        // Print for debugging
        LogPrintf("NFT Parser - WARNING - Invalid RGB type: expected RGB, parsed RGBA\n");
        // Reset RGB(A) index value for next attempt
        //n = 0;
        //return;
    }

    // Loop handling for a maximum 300x300 image
    while (positionLOOP < (width * height))
    {
        // Handle RGB parsing
        r = static_cast<int>(image[index + 0]);
        g = static_cast<int>(image[index + 1]);
        b = static_cast<int>(image[index + 2]);
        if(RGBA < 4) {
            a = 255;
        } else {
            a = static_cast<int>(image[index + 3]);
        }

        // Move to get next pixel of RGB
        index += RGBA;

        str_x = std::to_string(x);
        str_y = std::to_string(y);
        str_r = std::to_string(r);
        str_g = std::to_string(g);
        str_b = std::to_string(b);
        str_a = std::to_string(a);
        // Print for debugging
        LogPrintf("NFT Parser - Pixel |%u| Position x=%s y=%s - RGB data parsed: %s, %s, %s, %s\n", p, str_x, str_y, str_r, str_g, str_b, str_a);
        // Write data to array
        Decoding_Pixel_Count[positionLOOP] = str_r + nftBUF + str_g + nftBUF + str_b + nftBUF + str_a;
        NFTparserdata += Decoding_Pixel_Count[positionLOOP] + nftPAD;
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
    NFTenCode(NFTparserdata);
}

void NFTenCode(std::string input_pixeldata) {

    nftBASEOut_String = input_pixeldata;
    NFT_run = true;
    NFTBASE_run = NFT_run;

}
