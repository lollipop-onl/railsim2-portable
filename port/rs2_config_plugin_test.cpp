// Config.txt smoke/roundtrip and *2.txt LoadHeader smoke (#58).

#include "path.h"
#include "rs2_config_plugin.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

namespace {

const int kSkip = 77;

const char kSaveFormatConfig[] =
    "/*\n"
    " *\tRailSim II Configuration Datafile\n"
    " */\n"
    "\n"
    "DatafileHeader{\n"
    "\tRailSimVersion = 2.15;\n"
    "\tDatafileType = Config;\n"
    "}\n"
    "\n"
    "ConfigMode{\n"
    "\tInterface{\n"
    "\t\tHideTopPanel = yes;\n"
    "\t\tHideRightPanel = no;\n"
    "\t\tWindowShadow = yes;\n"
    "\t}\n"
    "\tDevice{\n"
    "\t\tFullScreen = no;\n"
    "\t\tResolution = 800, 600;\n"
    "\t\tRailMipMap = yes;\n"
    "\t\tTrainMipMap = no;\n"
    "\t\tStructMipMap = no;\n"
    "\t\tSurfaceMipMap = yes;\n"
    "\t}\n"
    "\tAccessory{\n"
    "\t\tCompass = yes;\n"
    "\t\tWindMeter = yes;\n"
    "\t\tShowMap = no;\n"
    "\t}\n"
    "\tEffect{\n"
    "\t\tShadow = no;\n"
    "\t\tLinearFilter = yes;\n"
    "\t\tEnvMap = yes;\n"
    "\t\tSpecularLight = yes;\n"
    "\t\tSunLensFlare = yes;\n"
    "\t\tSunWhiteout = yes;\n"
    "\t\tMiscLensFlare = yes;\n"
    "\t\tMiscParticle = yes;\n"
    "\t\tWind = yes;\n"
    "\t}\n"
    "\tSound{\n"
    "\t\tInterfaceSound = yes;\n"
    "\t\tRailSound = yes;\n"
    "\t\tTrainSound = yes;\n"
    "\t\tStructSound = yes;\n"
    "\t\tSurfaceSound = yes;\n"
    "\t}\n"
    "\tMisc{\n"
    "\t\tUseUndo = yes;\n"
    "\t}\n"
    "\tStereoscopy{\n"
    "\t\tEnabled = no;\n"
    "\t\tMethod = 0;\n"
    "\t\tInterval = 1.00;\n"
    "\t}\n"
    "\tSelectedPlugin{\n"
    "\t\tRail = \"Default_JR_Narrow\";\n"
    "\t\tTie = \"Default_JRN_BallastPC\";\n"
    "\t\tGirder = \"Default_JRN_SinglePC\";\n"
    "\t\tPier = \"Default_SinglePC\";\n"
    "\t\tLine = \"Default_SimpleCatenary\";\n"
    "\t\tPole = \"Default_JRN_Single\";\n"
    "\t\tTrain = \"Aizentranza01\";\n"
    "\t\tStation = \"MM02\";\n"
    "\t\tStruct = \"Ship\";\n"
    "\t\tSurface = \"Default\";\n"
    "\t\tEnv = \"Default\";\n"
    "\t\tSkin = \"Default_Blue\";\n"
    "\t}\n"
    "}\n";

const char kOldConfig[] =
    "DatafileHeader{\n"
    "\tRailSimVersion = 2.00;\n"
    "\tDatafileType = Config;\n"
    "}\n"
    "ConfigMode{\n"
    "\tInterface{\n"
    "\t\tHideTopPanel = no;\n"
    "\t\tHideRightPanel = no;\n"
    "\t}\n"
    "\tDevice{\n"
    "\t\tFullScreen = yes;\n"
    "\t\tResolution = 640, 480;\n"
    "\t\tRailMipMap = yes;\n"
    "\t\tTrainMipMap = no;\n"
    "\t\tStructMipMap = no;\n"
    "\t\tSurfaceMipMap = yes;\n"
    "\t}\n"
    "\tAccessory{\n"
    "\t\tCompass = yes;\n"
    "\t\tWindMeter = yes;\n"
    "\t}\n"
    "\tEffect{\n"
    "\t\tShadow = no;\n"
    "\t\tLinearFilter = yes;\n"
    "\t\tEnvMap = yes;\n"
    "\t\tSpecularLight = yes;\n"
    "\t\tSunLensFlare = yes;\n"
    "\t\tSunWhiteout = yes;\n"
    "\t\tMiscLensFlare = yes;\n"
    "\t\tMiscParticle = yes;\n"
    "\t\tWind = yes;\n"
    "\t}\n"
    "\tSound{\n"
    "\t\tInterfaceSound = yes;\n"
    "\t\tRailSound = yes;\n"
    "\t\tTrainSound = yes;\n"
    "\t\tStructSound = yes;\n"
    "\t\tSurfaceSound = yes;\n"
    "\t}\n"
    "\tMisc{\n"
    "\t\tUseUndo = yes;\n"
    "\t}\n"
    "\tSelectedPlugin{\n"
    "\t\tRail = \"Default_JR_Narrow\";\n"
    "\t\tTie = \"Default_JRN_BallastPC\";\n"
    "\t\tGirder = \"Default_JRN_SinglePC\";\n"
    "\t\tPier = \"Default_SinglePC\";\n"
    "\t\tLine = \"Default_SimpleCatenary\";\n"
    "\t\tPole = \"Default_JRN_Single\";\n"
    "\t\tTrain = \"Aizentranza01\";\n"
    "\t\tStation = \"MM02\";\n"
    "\t\tStruct = \"Ship\";\n"
    "\t\tSurface = \"Default\";\n"
    "\t\tEnv = \"Default\";\n"
    "\t\tSkin = \"Default_Blue\";\n"
    "\t}\n"
    "}\n";

const char kRail2Header[] =
    "PluginHeader{\n"
    "\tRailSimVersion = 2.00;\n"
    "\tPluginType = Rail;\n"
    "\tPluginName = \"Default JR-narrow\";\n"
    "\tPluginAuthor = \"Okadu\";\n"
    "\tIconTexture = \"..\\\\..\\\\Train\\\\Aizentranza01\\\\Icon.png\";\n"
    "\tDescription = \"Popular narrow gauge in Japan.\";\n"
    "}\n";

bool expect(bool ok, const char *label) {
	if (!ok) std::fprintf(stderr, "self-test: %s\n", label);
	return ok;
}

bool parse_ok(const char *text, Rs2Config *out, const char *label) {
	char err[128];
	if (rs2_config_parse(text, out, err, sizeof(err))) return true;
	std::fprintf(stderr, "self-test: %s: %s\n", label, err);
	return false;
}

int self_test() {
	char err[128];
	Rs2Config cfg;
	bool used_defaults = false;
	if (!expect(rs2_config_load_or_defaults("/no/such/Config.txt", &cfg, &used_defaults, err,
	                                        sizeof(err)) &&
	                used_defaults && cfg.full_screen && cfg.resolution[0] == 640 &&
	                cfg.selected_rail == "Default_JR_Narrow",
	            "missing Config.txt uses defaults"))
		return 1;

	if (!parse_ok(kSaveFormatConfig, &cfg, "Save-format Config.txt")) return 1;
	if (!expect(cfg.hide_top_panel && !cfg.full_screen && cfg.resolution[0] == 800 &&
	                cfg.resolution[1] == 600 && cfg.selected_train == "Aizentranza01",
	            "Save-format field values"))
		return 1;

	if (!parse_ok(kOldConfig, &cfg, "2.00 Config.txt")) return 1;
	if (!expect(cfg.version == 2.00f && cfg.window_shadow && !cfg.show_map && !cfg.stereo_enabled &&
	                cfg.stereo_interval == 1.0f,
	            "2.00 keeps gated defaults"))
		return 1;

	Rs2Config mutated;
	rs2_config_defaults(&mutated);
	mutated.hide_top_panel = true;
	mutated.full_screen = false;
	mutated.resolution[0] = 1024;
	mutated.resolution[1] = 768;
	mutated.shadow = true;
	mutated.stereo_interval = 2.50f;
	mutated.selected_struct = "Fence";
	std::string written;
	if (!expect(rs2_config_write(&mutated, &written), "write Config.txt")) return 1;
	Rs2Config back;
	if (!parse_ok(written.c_str(), &back, "roundtrip parse")) return 1;
	mutated.version = RS2_CONFIG_VERSION;
	if (!expect(rs2_config_equal(&mutated, &back), "roundtrip fields")) return 1;

	Rs2PluginHeader plugin;
	if (!expect(rs2_plugin_header_parse(kRail2Header, "Rail", &plugin, err, sizeof(err)) &&
	                plugin.name == "Default JR-narrow" && plugin.author == "Okadu",
	            "inline Rail2.txt header"))
		return 1;

	Rs2LanguageHeader lang;
	const char *lang_txt =
	    "DatafileHeader{\n"
	    "\tRailSimVersion = 2.15;\n"
	    "\tDatafileType = Language;\n"
	    "}\n"
	    "Language{\n"
	    "\tName = \"English\";\n"
	    "}\n";
	if (!expect(rs2_language_header_parse(lang_txt, &lang, err, sizeof(err)) &&
	                lang.name == "English",
	            "inline Language.txt header"))
		return 1;

	return 0;
}

bool find_railsim2(const char *root, char *base, size_t n) {
	if (rs2_path_join(base, n, root, "en", "RailSim2") && rs2_is_dir(base)) return true;
	if (rs2_path_join(base, n, root, "RailSim2") && rs2_is_dir(base)) return true;
	return false;
}

int load_distribution(const char *root) {
	namespace fs = std::filesystem;
	std::error_code ec;
	if (!fs::is_directory(root, ec)) {
		std::fprintf(stderr, "skip: no fixture at %s\n", root);
		return kSkip;
	}

	char base[RS2_PATH_MAX];
	if (!find_railsim2(root, base, sizeof(base))) {
		std::fprintf(stderr, "skip: no RailSim2 under %s\n", root);
		return kSkip;
	}

	char err[128];
	char langpath[RS2_PATH_MAX];
	if (!rs2_path_join(langpath, sizeof(langpath), base, "Language.txt")) return 1;
	Rs2LanguageHeader lang;
	if (!rs2_language_header_load_file(langpath, &lang, err, sizeof(err))) {
		std::fprintf(stderr, "Language.txt: %s (%s)\n", err, langpath);
		return 1;
	}
	if (lang.name.empty()) {
		std::fprintf(stderr, "Language.txt: empty Name\n");
		return 1;
	}

	char rail2file[RS2_PATH_MAX];
	if (!rs2_path_join(rail2file, sizeof(rail2file), base, "Rail", "Default_JR_Narrow",
	                   "Rail2.txt"))
		return 1;
	Rs2PluginHeader plugin;
	if (!rs2_plugin_header_load_file(rail2file, "Rail", &plugin, err, sizeof(err))) {
		std::fprintf(stderr, "Rail2.txt: %s (%s)\n", err, rail2file);
		return 1;
	}
	if (plugin.name.empty()) {
		std::fprintf(stderr, "Rail2.txt: empty PluginName\n");
		return 1;
	}

	char cfgpath[RS2_PATH_MAX];
	if (!rs2_path_join(cfgpath, sizeof(cfgpath), base, "Config.txt")) return 1;
	Rs2Config cfg;
	bool used_defaults = false;
	if (!rs2_config_load_or_defaults(cfgpath, &cfg, &used_defaults, err, sizeof(err))) {
		std::fprintf(stderr, "Config.txt: %s (%s)\n", err, cfgpath);
		return 1;
	}
	if (used_defaults) {
		std::printf("config: defaults path (no Config.txt under %s)\n", base);
	} else {
		std::printf("config: parsed Config.txt under %s\n", base);
	}
	std::printf("plugin: loaded %s (%s)\n", rail2file, plugin.name.c_str());
	std::printf("language: %s (%s)\n", langpath, lang.name.c_str());
	return 0;
}

}  // namespace

int main(int argc, char **argv) {
	if (argc >= 2 && std::strcmp(argv[1], "--self-test") == 0) return self_test();
	if (argc < 2) {
		std::fprintf(stderr, "usage: rs2_config_plugin_test --self-test | <Distribution>\n");
		return 2;
	}
	int st = self_test();
	if (st) return st;
	return load_distribution(argv[1]);
}
