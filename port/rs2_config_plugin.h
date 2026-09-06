// Config.txt / Language.txt / *2.txt script smoke (#58).
// Matches CConfigMode::Load/Save and CPlugin::LoadHeader grammar, not UI.

#pragma once

#include <cstddef>
#include <string>

#ifndef RS2_CONFIG_VERSION
#define RS2_CONFIG_VERSION 2.15f
#endif

struct Rs2Config {
	float version;
	bool hide_top_panel;
	bool hide_right_panel;
	bool window_shadow;
	bool full_screen;
	int resolution[2];
	bool rail_mipmap;
	bool train_mipmap;
	bool struct_mipmap;
	bool surface_mipmap;
	bool compass;
	bool wind_meter;
	bool show_map;
	bool shadow;
	bool linear_filter;
	bool env_map;
	bool specular_light;
	bool sun_lens_flare;
	bool sun_whiteout;
	bool misc_lens_flare;
	bool misc_particle;
	bool wind;
	bool interface_sound;
	bool rail_sound;
	bool train_sound;
	bool struct_sound;
	bool surface_sound;
	bool use_undo;
	bool stereo_enabled;
	int stereo_method;
	float stereo_interval;
	std::string selected_rail;
	std::string selected_tie;
	std::string selected_girder;
	std::string selected_pier;
	std::string selected_line;
	std::string selected_pole;
	std::string selected_train;
	std::string selected_station;
	std::string selected_struct;
	std::string selected_surface;
	std::string selected_env;
	std::string selected_skin;
};

struct Rs2PluginHeader {
	float version;
	std::string type;
	std::string name;
	std::string author;
	std::string icon;
	std::string description;
};

struct Rs2LanguageHeader {
	float version;
	std::string name;
};

void rs2_config_defaults(Rs2Config *out);
bool rs2_config_equal(const Rs2Config *a, const Rs2Config *b);

bool rs2_config_parse(const char *text, Rs2Config *out, char *err, size_t errn);
bool rs2_config_write(const Rs2Config *in, std::string *out);
bool rs2_config_load_file(const char *path, Rs2Config *out, char *err, size_t errn);
bool rs2_config_load_or_defaults(const char *path, Rs2Config *out, bool *used_defaults,
                                 char *err, size_t errn);

bool rs2_plugin_header_parse(const char *text, const char *expect_type, Rs2PluginHeader *out,
                             char *err, size_t errn);
bool rs2_plugin_header_load_file(const char *path, const char *expect_type, Rs2PluginHeader *out,
                                 char *err, size_t errn);

bool rs2_language_header_parse(const char *text, Rs2LanguageHeader *out, char *err, size_t errn);
bool rs2_language_header_load_file(const char *path, Rs2LanguageHeader *out, char *err,
                                   size_t errn);
