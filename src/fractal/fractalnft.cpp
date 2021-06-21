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

// Logging of each character count (hard limit of 1 line per encoding)
std::string Pixel_Cache[1] = {};

// Logging of succesfull run
bool NFT_run = false;

void NFTrender(std::string input_nft_data, std::string input_alias, string render_alias, int contract_type) {
    // Set input string
    char NFT_String[input_nft_data.length()];
    strcpy(NFT_String, input_nft_data.c_str());
    // Returns first word
    char *pixel_count = strtok(NFT_String, " ");// TODO: Explore cleaning with blank_space[1]

    // Clear previously set data
    NFTparserdata = "";

    // Set found count
    int log_found = 0;
    // Set starting loop position
    int start_position = 0;
    //
    int channels = 3;
    //
    int w, h;
    // Loop through all words in input
    while (pixel_count != NULL)
    {
        // Set array data
        NFTparserdata += pixel_count;
        // Break loop if maximum de-obfuscatable pixel line count is reached
        if(log_found > 300)
        {
            break;
        }
        // Log word count found
        log_found ++;
        // Move to next word
        pixel_count = strtok(NULL, " ");
    }
    // Print for debugging
    LogPrintf("NFT Render - INFO - Found %u pixel lines!\n", log_found);
    // Set dimensions
    w = log_found;
    h = w;

    // Print for debugging
    LogPrintf("NFT Render - INFO - Set the following Pixel data for printing: %s\n", NFTparserdata);

    // Encoded data to write
    unsigned char data[w * h * channels];
    // Set rgb parse values
    char Parse_String[NFTparserdata.length()];
    strcpy(Parse_String, NFTparserdata.c_str());
    // Set subvalues of pixel value
    char *parse_count = strtok(Parse_String, "C");
    // Set array position
    int dataPosition = 0;

    // Write pixel data
    while (start_position < (w * h))
    {
        // Set and Move to next data sets
        std::string str_parse_count = parse_count;
        int parse_cache = int(std::stoi(str_parse_count));
        data[dataPosition++] = parse_cache;
        parse_count = strtok(NULL, "C");
        str_parse_count = parse_count;
        parse_cache = int(std::stoi(str_parse_count));
        data[dataPosition++] = parse_cache;
        parse_count = strtok(NULL, "C");
        str_parse_count = parse_count;
        parse_cache = int(std::stoi(str_parse_count));
        data[dataPosition++] = parse_cache;
        // Print for debugging
        LogPrintf("NFT Render - INFO - Writing Rendered RGB: %s, %s, %s\n", data[(dataPosition - 2)], data[(dataPosition - 1)], data[dataPosition]);
        start_position ++;
        // Break loop if exceeded available colors
        if(parse_count == NULL) {
            break;
        }
        // Move to next word
        parse_count = strtok(NULL, "C");
    }
    // Print for debugging
    LogPrintf("NFT Render - INFO - Rendering to file: %s!\n", render_alias);

    // Print for debugging
    LogPrintf("NFT Render - INFO - Rendered NFT: %s!\n", input_alias);

    NFTprintRENDER(data, w, h, channels, render_alias, contract_type);
}

void NFTprintRENDER(unsigned char NFTdata[], int w, int h, int channels, std::string passed_alias, int contract_type) {

    // Inform user of image generation
    uiInterface.ThreadSafeMessageBox("Your NFT image has been rendered!", "", CClientUIInterface::MSG_INFORMATION);
    // Print image
    int PNGdata = (w * channels);
    write_image(passed_alias.c_str(), w, h, channels, NFTdata, contract_type, PNGdata);
}

void NFTparse(std::string image_to_deCode) {
    // Clear possible left over data
    NFTparserdata = "";
    // Set values
    NFT_run = false;
    NFTBASE_run = NFT_run;
    int width, height;
    int r, g, b;//, a;
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

    if (RGBA != 3)
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

        // Move to get next pixel of RGB
        index += RGBA;

        str_x = std::to_string(x);
        str_y = std::to_string(y);
        str_r = std::to_string(r);
        str_g = std::to_string(g);
        str_b = std::to_string(b);
        //str_a = std::to_string(a);
        // Print for debugging
        LogPrintf("NFT Parser - Pixel |%u| Position x=%s y=%s - RGB data parsed: %s, %s, %s\n", p, str_x, str_y, str_r, str_g, str_b);
        // Write data to array
        Pixel_Cache[0] = str_r + nftBUF + str_g + nftBUF + str_b + nftBUF;// + str_a + nftBUF;
        NFTparserdata += Pixel_Cache[0];
        // Move up in loop logic and pixel position
        if (yLOOP == width)
        {
            NFTparserdata += nftPAD;
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

    // Inform user of image generation
    uiInterface.ThreadSafeMessageBox("Image was successfully parsed into a NFT!", "", CClientUIInterface::MSG_INFORMATION);
}
