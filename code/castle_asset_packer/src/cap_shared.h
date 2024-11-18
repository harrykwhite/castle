#pragma once

#include <string>
#include <fstream>

bool pack_textures(std::ofstream &assets_file_ofs, const std::string &assets_dir);
bool pack_shader_progs(std::ofstream &assets_file_ofs, const std::string &assets_dir);
bool pack_fonts(std::ofstream &assets_file_ofs, const std::string &assets_dir);

extern const int k_tex_cnt;
extern const int k_shader_prog_cnt;
extern const int k_font_cnt;
