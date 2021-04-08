// Copyright (c) 2020-2021 The Espers Project/CryptoCoderz Team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


// NOTICE!
//
// This is a completely experimental smart-contract platform written by
// CrpytoCoderz (Jonathan Dan Zaretsky - cryptocoderz@gmail.com)
//
// PLEASE USE AT YOUR OWN RISK!!!
//

#include "fractalnft.h"
// for logprintf
#include "util/init.h"

#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// Global RGB(A) value setting
int n;

// External string for input/output (depending on how you view it)
std::string nftOut_String;
bool iLOAD;


// Load RGB(A) file, determining whether we have alpha channel
bool load_image(std::vector<unsigned char>& image, const std::string& filename, int& x, int& y)
{
    // Allow for auto selected color channel types, we later correct this anyway
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, 0);
    if (data != nullptr)
    {
        image = std::vector<unsigned char>(data, data + x * y * n);
    }
    stbi_image_free(data);
    iLOAD = (data != nullptr);
    return (data != nullptr);
}

void NFTparse(std::string filename)
{
    // Set base definitions
    nftOut_String = "";
    int width, height;
    int r, g, b, a;
    std::string str_x, str_y, str_r, str_g, str_b, str_a;
    std::string nftBUF = " ";
    std::string nftBUF2 = "A";
    std::string nftBUF3 = "B";
    std::vector<unsigned char> image;
    // Set image data
    bool success = load_image(image, filename, width, height);
    // Report failure
    if (!success)
    {
        // Print for debugging
        LogPrintf("NFTparse - ERROR - could not open or load image!\n");
        // Reset RGB(A) index value for next attempt
        n = 0;
        return;
    }
    // Hard limit maximum NFT pixel size
    // Limit set to 16x16 for testing
    else if ((width * height) > 256 || width != height)
    {
        // Print for debugging
        LogPrintf("NFTparse - ERROR - image is not 16x16 pixels!\n");
        return;
    }

    // Print for debugging
    LogPrintf("NFTparse - Image dimensions (width x height): %u %u\n", width, height);

    // Either 3 or 4
    const size_t RGBA = n;

    // Define pixel position
    int x = 1;
    int y = 1;
    int p = 1;
    int positionLOOP = 0;
    int yLOOP = 1;

    // Loop handling for a 16x16 image
    while (positionLOOP < (width * height))
    {
        // Set pixel data
        size_t index = RGBA * (y * width + x);

        // Convert RGB to RGBA else handle RGBA natively
        r = static_cast<int>(image[index + 0]);
        g = static_cast<int>(image[index + 1]);
        if (RGBA < 4) {
            b = static_cast<int>(image[index + 2]);
            a = 255;
        } else {
            b = static_cast<int>(image[index + 2]);
            a = static_cast<int>(image[index + 3]);
        }

        // Print for debugging
        //LogPrintf("NFTparse - Image Pixel Position x=%u y=%u - RGBA data parsed: %u, %u, %u, %u\n", x, y, r, g, b, a);
        str_x = std::to_string(x);
        str_y = std::to_string(y);
        str_r = std::to_string(r);
        str_g = std::to_string(g);
        str_b = std::to_string(b);
        str_a = std::to_string(a);
        // Print for debugging
        LogPrintf("NFTparse - Pixel |%u| Position x=%s y=%s - RGBA data parsed: %s, %s, %s, %s\n", p, str_x, str_y, str_r, str_g, str_b, str_a);
        nftOut_String += str_x + nftBUF3 + str_y + nftBUF2 + str_r + str_g + str_b + str_a + nftBUF;
        // Print for debugging
        //LogPrintf("NFTparse - Image Pixel Data: %s\n", nftOut_String);
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

    // Write data to string
    //
    // TODO: replace with string and return data

    // Reset RGB(A) index value for next attempt
    n = 0;
    return;
}

#undef STB_IMAGE_IMPLEMENTATION// TODO: verify this, good practice would be unload as needed...
