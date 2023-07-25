std::unordered_map <std::string, element *> 		elementsLookup;
std::unordered_map <std::string, slider *> 		slidersLookup;
std::unordered_map <std::string, toggle *> 		togglesLookup;
std::unordered_map <std::string, radio *> 		radiosLookup;
std::unordered_map <std::string, inspector *> 	inspectorsLookup;

element * getElement(const std::string & n) {
	return elementsLookup.find(n) != elementsLookup.end() ? elementsLookup[n] : NULL;
}

// and other kinds
slider * getSlider(const std::string & n) {
	return slidersLookup.find(n) != slidersLookup.end() ? slidersLookup[n] : NULL;
}

toggle * getToggle(const std::string & n) {
	return togglesLookup.find(n) != togglesLookup.end() ? togglesLookup[n] : NULL;
}

radio * getRadio(const std::string & n) {
	return radiosLookup.find(n) != radiosLookup.end() ? radiosLookup[n] : NULL;
}

inspector * getInspector(const std::string & n) {
	return inspectorsLookup.find(n) != inspectorsLookup.end() ? inspectorsLookup[n] : NULL;
}

bool updatedRect = false;
std::string loadedTextFile { "" };
std::string createdLines { "" };
// 16 02 2021 - testing to solve future ui element issues
bool uiIsCreated = false;

void reload() {
	std::cout << "ofxMicroUI Reload" << uiName << std::endl;
	clear();
	createFromText(loadedTextFile);
}

void updateRect();

void createFromLine(std::string l);
void createFromText(const std::string & fileName);
void createFromLines(const std::string & line);
void createFromLines(const std::vector<std::string> & lines, bool complete = true);
