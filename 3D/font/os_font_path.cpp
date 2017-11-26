/* SPDX-License-Identifier: GPL-3.0 */

#include "os_font_path.h"

#ifdef _WIN32
#include <sstream>
#include <vector>
#include <windows.h>

static const char *font_ext = "\\Fonts\\arialbd.ttf";

std::string default_font_path() {
    std::vector<char> buf(MAX_PATH);
    size_t l = GetWindowsDirectoryA(reinterpret_cast<LPSTR>(&buf[0]), MAX_PATH);
    std::stringstream ss;
    ss.write(&buf[0], l);
    ss.write(font_ext, strlen(font_ext));
    return ss.str();
}

#elif __APPLE__
std::string default_font_path() {
    return std::string("/Library/Fonts/Arial Bold.ttf");
}
#elif __linux__
std::string default_font_path() {
    return std::string("/usr/share/fonts/truetype/freefont/FreeSansBold.ttf");
}
#endif

