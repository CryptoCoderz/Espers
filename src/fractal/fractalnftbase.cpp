// Copyright (c) 2020-2021 The Espers Project/CryptoCoderz Team
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

#include "fractalnftbase.h"
// for logprintf
#include "util/init.h"

// Global RGB(A) value setting
int n;

// External string for input/output (depending on how you view it)
std::string nftBASEOut_String;
// Logging of parse status
bool iLOAD;
// Logging of succesfull run
bool NFTBASE_run;

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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

#undef STB_IMAGE_IMPLEMENTATION// TODO: verify this, good practice would be unload as needed...

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

void write_image(std::string passed_alias, int w, int h, int channels, unsigned char data[], int contract_type, int PNGdata)
{
    if(contract_type < 10) {// TODO: Temporary, will handle JPG and PNG fluidly in the future
        stbi_write_jpg(passed_alias.c_str(), w, h, channels, data, 100);
        stbi_image_free(&data);// Release memory
    } else {
        stbi_write_png(passed_alias.c_str(), w, h, channels, data, PNGdata);
        stbi_image_free(&data);// Release memory
    }
}

#undef STB_IMAGE_WRITE_IMPLEMENTATION// TODO: verify this, good practice would be unload as needed...
