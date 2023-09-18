//#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#include <iostream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <cli11/CLI11.hpp>
#include <filesystem>
#include <cstring>

//#define ALLOW_WINDOWS_OPTIONS

#define FILTER_FLAG(shortname, longname) filterGroup->add_flag_callback("-" shortname ",--" longname, [&] { selectedFilters.push_back(longname); }, "remove " longname );
#define FILTER_FLAG_PLURAL(shortname, longname) filterGroup->add_flag_callback("-" shortname ",--" longname, [&] { selectedFilters.push_back(longname); }, "remove " longname "s" );

using std::string;
using std::vector;
using std::cout;
using std::filesystem::path;
using rapidjson::Document;

const path defaultPath = R"(Content\MyStuff\JSON\exported-entities.json)";
path realPath;

// soooo as it turns out, if you copy an object and then deallocate the original, it crashes
// why it doesn't do a deep copy (or at least refcount) on assignment into a DIFFERENT object i will never understand
Document data, output;

void filterJSON(path& filePath, vector<string>& filters) {
	using namespace rapidjson;

	std::ifstream json_file(filePath);
	IStreamWrapper json_isw(json_file);

	const int n_filters = filters.size();

	cout << "Loading file " << filePath << '\n';
	data.ParseStream(json_isw);
	
	auto entities = data["entities"].GetArray();
	cout << "Found " << entities.Size() << " entities\n";

	output.SetObject();
	auto& allocator = output.GetAllocator();

	// copy everything except entities
	for (auto it = data.MemberBegin(); it != data.MemberEnd(); it++) {
		if (!strcmp(it->name.GetString(), "entities")) {
			Value copiedEntities(kArrayType);
			copiedEntities.Reserve(entities.Size(), allocator);
			output.AddMember("entities", copiedEntities, allocator);
		} else {
			output.AddMember(it->name, it->value, allocator);
		}
	}

	cout << "Removing every instance of '";
	for (int i = 0; i < n_filters - 1; i++) cout << filters[i] << ", ";
	cout << filters.back() << "'\n";

	// copy every entity except those that match our filters
	auto newEntityArray = output["entities"].GetArray();
	for (auto& entity : entities) {
		auto name = entity["name"].GetString();
		bool shouldAdd = true;

		for (int j = 0; j < n_filters; j++) {
			if (strstr(name, filters[j].c_str())) {
				shouldAdd = false;
				break;
			}
		}

		if (shouldAdd) newEntityArray.PushBack(entity, allocator);
	}

	cout << "Copied " << newEntityArray.Size() << " entities\n";
}

int main(int argc, const char** argv) {
	CLI::App app(
		"Filters FUE5 (Factorio in Unreal Engine 5) exported JSON and removes unnecessary entities.\n"
		"Specify either the FUE5 project base directory, and the program will automatically find the files, "
		"or specify the input and output files.",
		"FUE5 JSON filter"
	);

#ifdef ALLOW_WINDOWS_OPTIONS
	app.allow_windows_style_options();
#endif

	auto g_files = app.add_option_group("files")->required();
	path baseDir, outPath;
	auto opt_base = g_files->add_option("--base", baseDir, "FUE5 base directory");
	auto opt_inF = g_files->add_option("--in", realPath, "input file");
	auto opt_outF = g_files->add_option("--out", outPath, "output file");

	opt_base->excludes(opt_inF, opt_outF);
	opt_inF->excludes(opt_base)->needs(opt_outF);
	opt_outF->excludes(opt_base)->needs(opt_inF);

	auto filterGroup = app.add_option_group("filters")->required();
	vector<string> selectedFilters;
	selectedFilters.reserve(5);

	FILTER_FLAG("f", "fish");
	FILTER_FLAG_PLURAL("t", "tree");
	FILTER_FLAG_PLURAL("b", "biter");
	FILTER_FLAG_PLURAL("s", "spitter");
	FILTER_FLAG_PLURAL("w", "worm");
	FILTER_FLAG_PLURAL("c", "cliff");
	FILTER_FLAG_PLURAL("r", "rock");
	filterGroup->add_flag_callback("-i,--item", [&] { selectedFilters.push_back("item-on-ground"); }, "remove ground items");

	try {
		app.parse(argc, argv);
	} catch (const CLI::ParseError& e) {
		return app.exit(e);
	}

	if (!opt_base->empty()) {
		outPath = realPath = baseDir / defaultPath;
	}

	filterJSON(realPath, selectedFilters);
	
	cout << "Writing file " << outPath << '\n';
	std::ofstream outfile(outPath);
	rapidjson::OStreamWrapper json_osw(outfile);
	rapidjson::Writer<rapidjson::OStreamWrapper> writer(json_osw);
	output.Accept(writer);

	return 0;
}