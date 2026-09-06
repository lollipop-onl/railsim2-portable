#include "rs2_config_plugin.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

namespace {

bool is_sjis_lead(unsigned char c) {
	return (c >= 0x81 && c <= 0x9F) || (c >= 0xE0 && c <= 0xFC);
}

const char *cp932_next(const char *p) {
	if (!p || !*p) return p;
	unsigned char c = static_cast<unsigned char>(*p);
	if (is_sjis_lead(c) && p[1]) return p + 2;
	return p + 1;
}

void set_err(char *err, size_t errn, const char *msg) {
	if (err && errn) std::snprintf(err, errn, "%s", msg ? msg : "parse error");
}

bool read_all(const char *path, std::string *out) {
	std::ifstream in(path, std::ios::binary);
	if (!in) return false;
	std::ostringstream ss;
	ss << in.rdbuf();
	*out = ss.str();
	return true;
}

struct Cur {
	const char *p;
	char errbuf[128];

	explicit Cur(const char *text) : p(text ? text : "") { errbuf[0] = 0; }

	void fail(const char *msg) { std::snprintf(errbuf, sizeof(errbuf), "%s", msg); }

	bool skip() {
		while (p && *p) {
			unsigned char c = static_cast<unsigned char>(*p);
			if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
				p++;
				continue;
			}
			if (c == 0x81 && static_cast<unsigned char>(p[1]) == 0x40) {
				p += 2;
				continue;
			}
			if (p[0] == '/' && p[1] == '/') {
				p += 2;
				while (*p && *p != '\n') p = cp932_next(p);
				continue;
			}
			if (p[0] == '/' && p[1] == '*') {
				p += 2;
				bool closed = false;
				while (*p) {
					if (p[0] == '*' && p[1] == '/') {
						p += 2;
						closed = true;
						break;
					}
					p = cp932_next(p);
				}
				if (!closed) {
					fail("comment end not found");
					return false;
				}
				continue;
			}
			break;
		}
		return true;
	}

	bool peek_ident(std::string *out) {
		if (!skip()) return false;
		if (!(('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z') || *p == '_')) return false;
		const char *s = p;
		p++;
		while (('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z') || *p == '_' ||
		       ('0' <= *p && *p <= '9'))
			p++;
		if (out) out->assign(s, p);
		return true;
	}

	bool ident_is(const char *want) {
		const char *save = p;
		std::string got;
		if (!peek_ident(&got) || got != want) {
			p = save;
			return false;
		}
		return skip();
	}

	bool ch(char c) {
		if (!skip()) return false;
		if (*p != c) return false;
		p++;
		return skip();
	}

	bool begin_block(const char *name) { return ident_is(name) && ch('{'); }

	bool end_block() { return ch('}'); }

	bool assignment(const char *key) { return ident_is(key) && ch('='); }

	bool number(int *outi, float *outf) {
		if (!skip()) return false;
		const char *s = p;
		if (*p == '-') p++;
		bool digit = false;
		while ('0' <= *p && *p <= '9') {
			digit = true;
			p++;
		}
		if (*p == '.') {
			p++;
			while ('0' <= *p && *p <= '9') {
				digit = true;
				p++;
			}
		}
		if (!digit) {
			p = s;
			return false;
		}
		char tmp[64];
		size_t n = static_cast<size_t>(p - s);
		if (n >= sizeof(tmp)) {
			p = s;
			return false;
		}
		std::memcpy(tmp, s, n);
		tmp[n] = 0;
		int i = 0;
		float f = 0.0f;
		std::sscanf(tmp, "%d", &i);
		std::sscanf(tmp, "%f", &f);
		if (outi) *outi = i;
		if (outf) *outf = f;
		return skip();
	}

	bool string_lit(std::string *out) {
		if (!skip()) return false;
		if (*p != '"') return false;
		p++;
		const char *s = p;
		while (*p) {
			if (*p == '\r' || *p == '\n') {
				fail("string exceeds line break");
				return false;
			}
			if (p[0] == '\\' && p[1] == '"') {
				p += 2;
				continue;
			}
			if (*p == '"') {
				if (out) out->assign(s, p);
				p++;
				return skip();
			}
			p = cp932_next(p);
		}
		fail("unterminated string");
		return false;
	}

	bool asgn_yesno(const char *key, bool *out) {
		if (!assignment(key)) return false;
		std::string yn;
		if (!peek_ident(&yn) || (yn != "yes" && yn != "no")) return false;
		*out = yn == "yes";
		return ch(';');
	}

	bool asgn_int(const char *key, int *out, int n) {
		if (!assignment(key)) return false;
		for (int i = 0; i < n; i++) {
			if (i && !ch(',')) return false;
			if (!number(out + i, nullptr)) return false;
		}
		return ch(';');
	}

	bool asgn_float(const char *key, float *out, int n) {
		if (!assignment(key)) return false;
		for (int i = 0; i < n; i++) {
			if (i && !ch(',')) return false;
			if (!number(nullptr, out + i)) return false;
		}
		return ch(';');
	}

	bool asgn_ident(const char *key, std::string *out) {
		if (!assignment(key)) return false;
		if (!peek_ident(out)) return false;
		return ch(';');
	}

	bool asgn_string(const char *key, std::string *out) {
		if (!assignment(key)) return false;
		if (!string_lit(out)) return false;
		return ch(';');
	}

	bool try_asgn_string(const char *key, std::string *out) {
		const char *save = p;
		if (asgn_string(key, out)) return true;
		p = save;
		return false;
	}

	bool try_asgn_float(const char *key, float *out, int n) {
		const char *save = p;
		if (asgn_float(key, out, n)) return true;
		p = save;
		return false;
	}
};

const char *yesno(bool v) { return v ? "yes" : "no"; }

}  // namespace

void rs2_config_defaults(Rs2Config *out) {
	if (!out) return;
	*out = Rs2Config{};
	out->version = RS2_CONFIG_VERSION;
	out->hide_top_panel = false;
	out->hide_right_panel = false;
	out->window_shadow = true;
	out->full_screen = true;
	out->resolution[0] = 640;
	out->resolution[1] = 480;
	out->rail_mipmap = true;
	out->train_mipmap = false;
	out->struct_mipmap = false;
	out->surface_mipmap = true;
	out->compass = true;
	out->wind_meter = true;
	out->show_map = false;
	out->shadow = false;
	out->linear_filter = true;
	out->env_map = true;
	out->specular_light = true;
	out->sun_lens_flare = true;
	out->sun_whiteout = true;
	out->misc_lens_flare = true;
	out->misc_particle = true;
	out->wind = true;
	out->interface_sound = true;
	out->rail_sound = true;
	out->train_sound = true;
	out->struct_sound = true;
	out->surface_sound = true;
	out->use_undo = true;
	out->stereo_enabled = false;
	out->stereo_method = 0;
	out->stereo_interval = 1.0f;
	out->selected_rail = "Default_JR_Narrow";
	out->selected_tie = "Default_JRN_BallastPC";
	out->selected_girder = "Default_JRN_SinglePC";
	out->selected_pier = "Default_SinglePC";
	out->selected_line = "Default_SimpleCatenary";
	out->selected_pole = "Default_JRN_Single";
	out->selected_train = "Aizentranza01";
	out->selected_station = "MM02";
	out->selected_struct = "Ship";
	out->selected_surface = "Default";
	out->selected_env = "Default";
	out->selected_skin = "Default_Blue";
}

bool rs2_config_equal(const Rs2Config *a, const Rs2Config *b) {
	if (!a || !b) return false;
	return a->version == b->version && a->hide_top_panel == b->hide_top_panel &&
	       a->hide_right_panel == b->hide_right_panel && a->window_shadow == b->window_shadow &&
	       a->full_screen == b->full_screen && a->resolution[0] == b->resolution[0] &&
	       a->resolution[1] == b->resolution[1] && a->rail_mipmap == b->rail_mipmap &&
	       a->train_mipmap == b->train_mipmap && a->struct_mipmap == b->struct_mipmap &&
	       a->surface_mipmap == b->surface_mipmap && a->compass == b->compass &&
	       a->wind_meter == b->wind_meter && a->show_map == b->show_map && a->shadow == b->shadow &&
	       a->linear_filter == b->linear_filter && a->env_map == b->env_map &&
	       a->specular_light == b->specular_light && a->sun_lens_flare == b->sun_lens_flare &&
	       a->sun_whiteout == b->sun_whiteout && a->misc_lens_flare == b->misc_lens_flare &&
	       a->misc_particle == b->misc_particle && a->wind == b->wind &&
	       a->interface_sound == b->interface_sound && a->rail_sound == b->rail_sound &&
	       a->train_sound == b->train_sound && a->struct_sound == b->struct_sound &&
	       a->surface_sound == b->surface_sound && a->use_undo == b->use_undo &&
	       a->stereo_enabled == b->stereo_enabled && a->stereo_method == b->stereo_method &&
	       a->stereo_interval == b->stereo_interval && a->selected_rail == b->selected_rail &&
	       a->selected_tie == b->selected_tie && a->selected_girder == b->selected_girder &&
	       a->selected_pier == b->selected_pier && a->selected_line == b->selected_line &&
	       a->selected_pole == b->selected_pole && a->selected_train == b->selected_train &&
	       a->selected_station == b->selected_station && a->selected_struct == b->selected_struct &&
	       a->selected_surface == b->selected_surface && a->selected_env == b->selected_env &&
	       a->selected_skin == b->selected_skin;
}

bool rs2_config_parse(const char *text, Rs2Config *out, char *err, size_t errn) {
	if (!text || !out) {
		set_err(err, errn, "null arg");
		return false;
	}
	rs2_config_defaults(out);
	Cur cur(text);
	std::string dtype;
	if (!cur.skip() || !cur.begin_block("DatafileHeader") ||
	    !cur.asgn_float("RailSimVersion", &out->version, 1) ||
	    !cur.asgn_ident("DatafileType", &dtype) || !cur.end_block()) {
		set_err(err, errn, cur.errbuf[0] ? cur.errbuf : "DatafileHeader");
		return false;
	}
	if (dtype != "Config") {
		set_err(err, errn, "DatafileType != Config");
		return false;
	}
	if (out->version > RS2_CONFIG_VERSION) {
		set_err(err, errn, "unsupported Config version");
		return false;
	}
	if (!cur.begin_block("ConfigMode")) {
		set_err(err, errn, "ConfigMode");
		return false;
	}
	if (!cur.begin_block("Interface") || !cur.asgn_yesno("HideTopPanel", &out->hide_top_panel) ||
	    !cur.asgn_yesno("HideRightPanel", &out->hide_right_panel)) {
		set_err(err, errn, "Interface");
		return false;
	}
	if (out->version >= 2.03f && !cur.asgn_yesno("WindowShadow", &out->window_shadow)) {
		set_err(err, errn, "WindowShadow");
		return false;
	}
	if (!cur.end_block()) {
		set_err(err, errn, "Interface end");
		return false;
	}
	if (!cur.begin_block("Device") || !cur.asgn_yesno("FullScreen", &out->full_screen) ||
	    !cur.asgn_int("Resolution", out->resolution, 2) ||
	    !cur.asgn_yesno("RailMipMap", &out->rail_mipmap) ||
	    !cur.asgn_yesno("TrainMipMap", &out->train_mipmap) ||
	    !cur.asgn_yesno("StructMipMap", &out->struct_mipmap) ||
	    !cur.asgn_yesno("SurfaceMipMap", &out->surface_mipmap) || !cur.end_block()) {
		set_err(err, errn, "Device");
		return false;
	}
	if (!cur.begin_block("Accessory") || !cur.asgn_yesno("Compass", &out->compass) ||
	    !cur.asgn_yesno("WindMeter", &out->wind_meter)) {
		set_err(err, errn, "Accessory");
		return false;
	}
	if (out->version >= 2.03f && !cur.asgn_yesno("ShowMap", &out->show_map)) {
		set_err(err, errn, "ShowMap");
		return false;
	}
	if (!cur.end_block()) {
		set_err(err, errn, "Accessory end");
		return false;
	}
	if (!cur.begin_block("Effect") || !cur.asgn_yesno("Shadow", &out->shadow) ||
	    !cur.asgn_yesno("LinearFilter", &out->linear_filter) ||
	    !cur.asgn_yesno("EnvMap", &out->env_map) ||
	    !cur.asgn_yesno("SpecularLight", &out->specular_light) ||
	    !cur.asgn_yesno("SunLensFlare", &out->sun_lens_flare) ||
	    !cur.asgn_yesno("SunWhiteout", &out->sun_whiteout) ||
	    !cur.asgn_yesno("MiscLensFlare", &out->misc_lens_flare) ||
	    !cur.asgn_yesno("MiscParticle", &out->misc_particle) || !cur.asgn_yesno("Wind", &out->wind) ||
	    !cur.end_block()) {
		set_err(err, errn, "Effect");
		return false;
	}
	if (!cur.begin_block("Sound") || !cur.asgn_yesno("InterfaceSound", &out->interface_sound) ||
	    !cur.asgn_yesno("RailSound", &out->rail_sound) ||
	    !cur.asgn_yesno("TrainSound", &out->train_sound) ||
	    !cur.asgn_yesno("StructSound", &out->struct_sound) ||
	    !cur.asgn_yesno("SurfaceSound", &out->surface_sound) || !cur.end_block()) {
		set_err(err, errn, "Sound");
		return false;
	}
	if (!cur.begin_block("Misc") || !cur.asgn_yesno("UseUndo", &out->use_undo) || !cur.end_block()) {
		set_err(err, errn, "Misc");
		return false;
	}
	if (out->version >= 2.12f) {
		if (!cur.begin_block("Stereoscopy") || !cur.asgn_yesno("Enabled", &out->stereo_enabled) ||
		    !cur.asgn_int("Method", &out->stereo_method, 1) ||
		    !cur.asgn_float("Interval", &out->stereo_interval, 1) || !cur.end_block()) {
			set_err(err, errn, "Stereoscopy");
			return false;
		}
	}
	if (!cur.begin_block("SelectedPlugin") || !cur.asgn_string("Rail", &out->selected_rail) ||
	    !cur.asgn_string("Tie", &out->selected_tie) ||
	    !cur.asgn_string("Girder", &out->selected_girder) ||
	    !cur.asgn_string("Pier", &out->selected_pier) ||
	    !cur.asgn_string("Line", &out->selected_line) ||
	    !cur.asgn_string("Pole", &out->selected_pole) ||
	    !cur.asgn_string("Train", &out->selected_train) ||
	    !cur.asgn_string("Station", &out->selected_station) ||
	    !cur.asgn_string("Struct", &out->selected_struct) ||
	    !cur.asgn_string("Surface", &out->selected_surface) ||
	    !cur.asgn_string("Env", &out->selected_env) || !cur.asgn_string("Skin", &out->selected_skin) ||
	    !cur.end_block() || !cur.end_block()) {
		set_err(err, errn, "SelectedPlugin");
		return false;
	}
	return true;
}

bool rs2_config_write(const Rs2Config *in, std::string *out) {
	if (!in || !out) return false;
	char ver[32];
	char interval[32];
	std::snprintf(ver, sizeof(ver), "%.2f", RS2_CONFIG_VERSION);
	std::snprintf(interval, sizeof(interval), "%.2f", in->stereo_interval);
	std::ostringstream ss;
	ss << "/*\n *\tRailSim II Configuration Datafile\n */\n\n";
	ss << "DatafileHeader{\n";
	ss << "\tRailSimVersion = " << ver << ";\n";
	ss << "\tDatafileType = Config;\n";
	ss << "}\n\n";
	ss << "ConfigMode{\n";
	ss << "\tInterface{\n";
	ss << "\t\tHideTopPanel = " << yesno(in->hide_top_panel) << ";\n";
	ss << "\t\tHideRightPanel = " << yesno(in->hide_right_panel) << ";\n";
	ss << "\t\tWindowShadow = " << yesno(in->window_shadow) << ";\n";
	ss << "\t}\n";
	ss << "\tDevice{\n";
	ss << "\t\tFullScreen = " << yesno(in->full_screen) << ";\n";
	ss << "\t\tResolution = " << in->resolution[0] << ", " << in->resolution[1] << ";\n";
	ss << "\t\tRailMipMap = " << yesno(in->rail_mipmap) << ";\n";
	ss << "\t\tTrainMipMap = " << yesno(in->train_mipmap) << ";\n";
	ss << "\t\tStructMipMap = " << yesno(in->struct_mipmap) << ";\n";
	ss << "\t\tSurfaceMipMap = " << yesno(in->surface_mipmap) << ";\n";
	ss << "\t}\n";
	ss << "\tAccessory{\n";
	ss << "\t\tCompass = " << yesno(in->compass) << ";\n";
	ss << "\t\tWindMeter = " << yesno(in->wind_meter) << ";\n";
	ss << "\t\tShowMap = " << yesno(in->show_map) << ";\n";
	ss << "\t}\n";
	ss << "\tEffect{\n";
	ss << "\t\tShadow = " << yesno(in->shadow) << ";\n";
	ss << "\t\tLinearFilter = " << yesno(in->linear_filter) << ";\n";
	ss << "\t\tEnvMap = " << yesno(in->env_map) << ";\n";
	ss << "\t\tSpecularLight = " << yesno(in->specular_light) << ";\n";
	ss << "\t\tSunLensFlare = " << yesno(in->sun_lens_flare) << ";\n";
	ss << "\t\tSunWhiteout = " << yesno(in->sun_whiteout) << ";\n";
	ss << "\t\tMiscLensFlare = " << yesno(in->misc_lens_flare) << ";\n";
	ss << "\t\tMiscParticle = " << yesno(in->misc_particle) << ";\n";
	ss << "\t\tWind = " << yesno(in->wind) << ";\n";
	ss << "\t}\n";
	ss << "\tSound{\n";
	ss << "\t\tInterfaceSound = " << yesno(in->interface_sound) << ";\n";
	ss << "\t\tRailSound = " << yesno(in->rail_sound) << ";\n";
	ss << "\t\tTrainSound = " << yesno(in->train_sound) << ";\n";
	ss << "\t\tStructSound = " << yesno(in->struct_sound) << ";\n";
	ss << "\t\tSurfaceSound = " << yesno(in->surface_sound) << ";\n";
	ss << "\t}\n";
	ss << "\tMisc{\n";
	ss << "\t\tUseUndo = " << yesno(in->use_undo) << ";\n";
	ss << "\t}\n";
	ss << "\tStereoscopy{\n";
	ss << "\t\tEnabled = " << yesno(in->stereo_enabled) << ";\n";
	ss << "\t\tMethod = " << in->stereo_method << ";\n";
	ss << "\t\tInterval = " << interval << ";\n";
	ss << "\t}\n";
	ss << "\tSelectedPlugin{\n";
	ss << "\t\tRail = \"" << in->selected_rail << "\";\n";
	ss << "\t\tTie = \"" << in->selected_tie << "\";\n";
	ss << "\t\tGirder = \"" << in->selected_girder << "\";\n";
	ss << "\t\tPier = \"" << in->selected_pier << "\";\n";
	ss << "\t\tLine = \"" << in->selected_line << "\";\n";
	ss << "\t\tPole = \"" << in->selected_pole << "\";\n";
	ss << "\t\tTrain = \"" << in->selected_train << "\";\n";
	ss << "\t\tStation = \"" << in->selected_station << "\";\n";
	ss << "\t\tStruct = \"" << in->selected_struct << "\";\n";
	ss << "\t\tSurface = \"" << in->selected_surface << "\";\n";
	ss << "\t\tEnv = \"" << in->selected_env << "\";\n";
	ss << "\t\tSkin = \"" << in->selected_skin << "\";\n";
	ss << "\t}\n";
	ss << "}\n\n";
	*out = ss.str();
	return true;
}

bool rs2_config_load_file(const char *path, Rs2Config *out, char *err, size_t errn) {
	std::string text;
	if (!path || !read_all(path, &text)) {
		set_err(err, errn, "open Config.txt failed");
		return false;
	}
	return rs2_config_parse(text.c_str(), out, err, errn);
}

bool rs2_config_load_or_defaults(const char *path, Rs2Config *out, bool *used_defaults, char *err,
                                 size_t errn) {
	if (!out) return false;
	std::string text;
	if (!path || !read_all(path, &text)) {
		rs2_config_defaults(out);
		if (used_defaults) *used_defaults = true;
		return true;
	}
	if (used_defaults) *used_defaults = false;
	return rs2_config_parse(text.c_str(), out, err, errn);
}

bool rs2_plugin_header_parse(const char *text, const char *expect_type, Rs2PluginHeader *out,
                             char *err, size_t errn) {
	if (!text || !out) {
		set_err(err, errn, "null arg");
		return false;
	}
	*out = Rs2PluginHeader{};
	Cur cur(text);
	if (!cur.skip() || !cur.begin_block("PluginHeader") ||
	    !cur.asgn_float("RailSimVersion", &out->version, 1) ||
	    !cur.asgn_ident("PluginType", &out->type) || !cur.asgn_string("PluginName", &out->name) ||
	    !cur.asgn_string("PluginAuthor", &out->author)) {
		set_err(err, errn, cur.errbuf[0] ? cur.errbuf : "PluginHeader");
		return false;
	}
	if (out->version < 2.00f || out->version > RS2_CONFIG_VERSION) {
		set_err(err, errn, "unsupported plugin version");
		return false;
	}
	if (expect_type && out->type != expect_type) {
		set_err(err, errn, "PluginType mismatch");
		return false;
	}
	float icon_rect[4] = {0, 0, 1, 1};
	cur.try_asgn_string("IconTexture", &out->icon);
	cur.try_asgn_float("IconRect", icon_rect, 4);
	std::string desc;
	while (cur.try_asgn_string("Description", &desc)) {
		if (!out->description.empty()) out->description += "\n";
		out->description += desc;
	}
	if (!cur.end_block()) {
		set_err(err, errn, "PluginHeader end");
		return false;
	}
	return true;
}

bool rs2_plugin_header_load_file(const char *path, const char *expect_type, Rs2PluginHeader *out,
                                 char *err, size_t errn) {
	std::string text;
	if (!path || !read_all(path, &text)) {
		set_err(err, errn, "open *2.txt failed");
		return false;
	}
	return rs2_plugin_header_parse(text.c_str(), expect_type, out, err, errn);
}

bool rs2_language_header_parse(const char *text, Rs2LanguageHeader *out, char *err, size_t errn) {
	if (!text || !out) {
		set_err(err, errn, "null arg");
		return false;
	}
	*out = Rs2LanguageHeader{};
	Cur cur(text);
	std::string dtype;
	if (!cur.skip() || !cur.begin_block("DatafileHeader") ||
	    !cur.asgn_float("RailSimVersion", &out->version, 1) ||
	    !cur.asgn_ident("DatafileType", &dtype) || !cur.end_block()) {
		set_err(err, errn, cur.errbuf[0] ? cur.errbuf : "Language DatafileHeader");
		return false;
	}
	if (dtype != "Language") {
		set_err(err, errn, "DatafileType != Language");
		return false;
	}
	if (out->version < 2.02f || out->version > RS2_CONFIG_VERSION) {
		set_err(err, errn, "unsupported Language version");
		return false;
	}
	if (!cur.begin_block("Language") || !cur.asgn_string("Name", &out->name) || !cur.end_block()) {
		set_err(err, errn, "Language Name");
		return false;
	}
	return true;
}

bool rs2_language_header_load_file(const char *path, Rs2LanguageHeader *out, char *err,
                                   size_t errn) {
	std::string text;
	if (!path || !read_all(path, &text)) {
		set_err(err, errn, "open Language.txt failed");
		return false;
	}
	return rs2_language_header_parse(text.c_str(), out, err, errn);
}
