#ifndef PARSE_OPTIONS_H_
#define PARSE_OPTIONS_H_

class ParseOptions {
private:
	// maps for option variables
	std::map<std::string, bool*> bool_map_;
	std::map<std::string, int*> int_map_;
	std::map<std::string, unsigned*> uint_map_;
	std::map<std::string, float*> float_map_;
	std::map<std::string, double*> double_map_;
	std::map<std::string, std::string*> string_map_;

	/**
	 * Structure for options' documentation
	 */
	struct DocInfo {
		DocInfo() {}
		DocInfo(const std::string& name, const std::string& usemsg)
			: name_(name), use_msg_(usemsg) {}

		std::string name_;
		std::string use_msg_;
	};
	typedef std::map<std::string, DocInfo> DocMapType;
	DocMapType doc_map_;  ///< map for the documentation

	bool print_args_;     ///< variable for the implicit --print-args parameter
	bool help_;           ///< variable for the implicit --help parameter
	std::string config_;  ///< variable for the implicit --config parameter
	std::vector<std::string> positional_args_;
	const char *usage_;
	int argc_;
	const char*const* argv_;

private:
	/// Reads the options values from a config file
	void ReadConfigFile(const std::string& filename);

	void SplitLongArg(std::string in, std::string* key, std::string* value);
	void NormalizeArgName(std::string* str);

	bool SetOption(const std::string& key, const std::string& value);

	bool ToBool(std::string str);
	int ToInt(std::string str);
	unsigned ToUInt(std::string str);
	float ToFloat(std::string str);
	double ToDouble(std::string str);

public:
	explicit ParseOptions(const char *usage) :
		print_args_(true), help_(false), usage_(usage), argc_(0), argv_(NULL) {
		
	}

	~ParseOptions() {}

	/// Register boolean variable
	void Register(const std::string& name, bool *b, const std::string& doc);
	/// Register int variable
	void Register(const std::string& name, int *i, const std::string& doc);
	/// Register unsigned int variable
	void Register(const std::string& name, unsigned* u, const std::string& doc);
	/// Register float variable
	void Register(const std::string& name, float* f, const std::string& doc);
	/// Register double variable
	void Register(const std::string& name, double* d, const std::string& doc);
	/// Register string variable
	void Register(const std::string& name, std::string* s, const std::string& doc);

	/**
	* Parses the command line options and fills the ParseOptions-registered
	* variables. This must be called after all the variables were registered!!!
	*
	* Initially the variables have implicit values,
	* then the config file values are set-up,
	* finally the command line vaues given.
	* Returns the first position in argv that was not used.
	* [typically not useful: use NumParams() and GetParam(). ]
	*/
	int Read(int argc, const char*const* argv);

	/// Prints the usage documentation [provided in the constructor].
	void PrintUsage(bool print_command_line = false);
	/// Prints the actual configuration of all the registered variables
	void PrintConfig(std::ostream& os);

	/// Number of positional parameters (c.f. argc-1).
	int NumArgs();

	/// Returns one of the positional parameters; 1-based indexing for argc/argv
	/// compatibility. Will crash if param is not >=1 and <=NumArgs().
	std::string GetArg(int param);

	std::string GetOptArg(int param) {
		return (param <= NumArgs() ? GetArg(param) : "");
	}

	/// The following function will return a possibly quoted and escaped
	/// version of "str", according to the current shell.  Currently
	/// this is just hardwired to bash.  It's useful for debug output.
	static std::string Escape(const std::string &str);
};



void ParseOptions::Register(const std::string& name, bool* b,
	const std::string& doc) {
	assert(b != NULL);
	std::string idx = name;
	NormalizeArgName(&idx);
	if (doc_map_.find(idx) != doc_map_.end()) {
		std::cout << "Registering option twice, ignoring second time: " << name;
		exit(-1);
	}
	bool_map_[idx] = b;
	doc_map_[idx] = DocInfo(name, doc + " (bool, default = "
		+ ((*b) ? "true)" : "false)"));
}

void ParseOptions::Register(const std::string& name, int* i,
	const std::string& doc) {
	assert(i != NULL);
	std::string idx = name;
	NormalizeArgName(&idx);
	if (doc_map_.find(idx) != doc_map_.end()) {
		std::cout << "Registering option twice, ignoring second time: " << name;
		exit(-1);
	}
	int_map_[idx] = i;
	std::ostringstream ss;
	ss << doc << " (int, default = " << *i << ")";
	doc_map_[idx] = DocInfo(name, ss.str());
}

void ParseOptions::Register(const std::string& name, unsigned* u,
	const std::string& doc) {
	assert(u != NULL);
	std::string idx = name;
	NormalizeArgName(&idx);
	if (doc_map_.find(idx) != doc_map_.end()) {
		std::cout << "Registering option twice, ignoring second time: " << name;
		exit(-1);
	}
	uint_map_[idx] = u;
	std::ostringstream ss;
	ss << doc << " (uint, default = " << *u << ")";
	doc_map_[idx] = DocInfo(name, ss.str());
}

void ParseOptions::Register(const std::string& name, float* f,
	const std::string& doc) {
	assert(f != NULL);
	std::string idx = name;
	NormalizeArgName(&idx);
	if (doc_map_.find(idx) != doc_map_.end()) {
		std::cout << "Registering option twice, ignoring second time: " << name;
		exit(-1);
	}
	float_map_[idx] = f;
	std::ostringstream ss;
	ss << doc << " (float, default = " << *f << ")";
	doc_map_[idx] = DocInfo(name, ss.str());
}

void ParseOptions::Register(const std::string& name, double* f,
	const std::string& doc) {
	assert(f != NULL);
	std::string idx = name;
	NormalizeArgName(&idx);
	if (doc_map_.find(idx) != doc_map_.end()) {
		std::cout << "Registering option twice, ignoring second time: " << name;
		exit(-1);
	}
	double_map_[idx] = f;
	std::ostringstream ss;
	ss << doc << " (double, default = " << *f << ")";
	doc_map_[idx] = DocInfo(name, ss.str());
}

void ParseOptions::Register(const std::string& name, std::string* s,
	const std::string& doc) {
	assert(s != NULL);
	std::string idx = name;
	NormalizeArgName(&idx);
	if (doc_map_.find(idx) != doc_map_.end()) {
		std::cout << "Registering option twice, ignoring second time: " << name;
		exit(-1);
	}
	string_map_[idx] = s;
	doc_map_[idx] = DocInfo(name, doc + " (string, default = \"" + *s + "\")");
}

int ParseOptions::NumArgs() {
	return (int)positional_args_.size();
}

std::string ParseOptions::GetArg(int i) {
	if (i < 1 || i > static_cast<int>(positional_args_.size())) {
		std::cout << "ParseOptions::GetArg, invalid index " << i;  // code error
		exit(-1);
	}
	return positional_args_[i - 1];
}

enum ShellType { kBash = 0, kDos = 1 };

static ShellType kShellType = kBash;  // This can be changed in the
// code if it ever does need to be changed (as it's unlikely that one
// compilation of this tool-set would use both shells).

static bool MustBeQuoted(const std::string &str, ShellType st) {
	// returns true if we need to escape it before putting it into
	// a shell (mainly thinking of bash shell, but should work for others)
	// This is for the convenience of the user so command-lines that are
	// printed out by ParseOptions::Read (with --print-args=true) are
	// paste-able into the shell and will run.
	// If you use a different type of shell, it might be necessary to
	// change this function.
	// But it's mostly a cosmetic issue as it basically affects how
	// the program echoes its command-line arguments to the screen.

	assert(st == kBash || st == kDos);
	const char *c = str.c_str();
	if (*c == '\0') return true;  // Must quote empty string
	else {
		const char *ok_chars[2];
		ok_chars[kBash] = "[]~#^_-+=:., /";  // these seem not to be interpreted as long
		// as there are no other "bad" characters involved (e.g. ", " would be interpreted
		// as part of something like a{b, c}).
		ok_chars[kDos] = "\\[]~#^_-+=:., /";  // This may not be exact.

		for (; *c != '\0'; c++) {
			if (!isalnum(*c)) {
				// For non-alphanumeric characters we have a list of
				// characters which are OK.  All others are forbidden
				// (this is easier since the shell interprets most non-alphanumeric
				// characters).
				const char *d;
				for (d = ok_chars[st]; *d != '\0'; d++) if (*c == *d) break;
				if (*d == '\0') return true;  // Was not alphanumeric, or
				// one of the "ok_chars".  So must be escaped
			}
		}
		return false;  // The string was OK: no escaping.
	}
}


// returns a quoted and escaped version of "str"
// which has previously been determined to need escaping.
static std::string QuoteAndEscape(const std::string &str, ShellType st) {
	char quote_char;
	const char *escape_str = "";  // the sequence of characters we insert
	// when we encounter a quote character.

	if (st == kBash) {
		quote_char = '\''; escape_str = "'\\''";  // e.g. echo 'a'\''b' returns a'b
	}
	else if (st == kDos) {
		quote_char = '"'; escape_str = "\"\"";   // not sure about this.  Must test.
	}
	else assert(0);

	char buf[2];
	buf[1] = '\0';

	buf[0] = quote_char;
	std::string ans = buf;
	const char *c = str.c_str();
	for (; *c != '\0'; c++) {
		if (*c == quote_char) {
			ans += escape_str;
		}
		else {
			buf[0] = *c;
			ans += buf;
		}
	}
	buf[0] = quote_char;
	ans += buf;
	return ans;
}

// static function
std::string ParseOptions::Escape(const std::string &str) {
	if (!MustBeQuoted(str, kShellType)) return str;
	else return QuoteAndEscape(str, kShellType);
}

void Trim(std::string *str) {
	const char* white_chars = " \t\n\r\f\v";

	std::string::size_type pos = str->find_last_not_of(white_chars);
	if (pos != std::string::npos)  {
		str->erase(pos + 1);
		pos = str->find_first_not_of(white_chars);
		if (pos != std::string::npos) str->erase(0, pos);
	}
	else {
		str->erase(str->begin(), str->end());
	}
}

int ParseOptions::Read(int argc, const char* const argv[]) {
	argc_ = argc;
	argv_ = argv;
	std::string key, value;
	int i;

	// first pass: look for config parameter, look for priority
	for (i = 1; i < argc; i++) {
		if (std::strncmp(argv[i], "--", 2) == 0) {
			SplitLongArg(argv[i], &key, &value);
			NormalizeArgName(&key);
			Trim(&value);
			if (key.compare("config") == 0) {
				ReadConfigFile(value);
			}
			if (key.compare("help") == 0) {
				PrintUsage();
				exit(0);
			}
		}
	}

	// second pass: add the command line options
	for (i = 1; i < argc; i++) {
		if (std::strncmp(argv[i], "--", 2) == 0) {
			SplitLongArg(argv[i], &key, &value);
			NormalizeArgName(&key);
			Trim(&value);
			if (!SetOption(key, value)) {
				std::cerr << "Invalid option " << argv[i] << '\n';
				PrintUsage(true);
				exit(1);
			}
		}
		else {
			// first non-long option finishes the options // was: return i;
			for (; i < argc; i++) {
				positional_args_.push_back(std::string(argv[i]));
			}
		}
	}

	if (print_args_) {  // if the user did not suppress this with --print-args = false....
		for (int j = 0; j < argc; j++)
			std::cerr << Escape(argv[j]) << " ";
		std::cerr << '\n';
		std::cerr << std::flush;
	}
	return i;
}

void ParseOptions::PrintUsage(bool print_command_line) {
	std::cerr << '\n' << usage_ << '\n';
	DocMapType::iterator it;
	std::cerr << "Allowed options:" << '\n';
	for (it = doc_map_.begin(); it != doc_map_.end(); ++it) {
		std::cerr << "  --" << std::setw(25) << std::left << it->second.name_
			<< " : " << it->second.use_msg_ << '\n';
	}
	std::cerr << '\n';
	if (print_command_line) {
		std::cerr << "Command line was: ";
		for (int j = 0; j < argc_; j++)
			std::cerr << Escape(argv_[j]) << " ";
		std::cerr << '\n';
	}
}

void ParseOptions::PrintConfig(std::ostream& os) {
	os << '\n' << "[[ Configuration of UI-Registered options ]]"
		<< '\n';
	std::string key;
	DocMapType::iterator it;
	for (it = doc_map_.begin(); it != doc_map_.end(); ++it) {
		key = it->first;
		os << it->second.name_ << " = ";
		if (bool_map_.end() != bool_map_.find(key)) {
			os << (*bool_map_[key] ? "true" : "false");
		}
		else if (int_map_.end() != int_map_.find(key)) {
			os << (*int_map_[key]);
		}
		else if (uint_map_.end() != uint_map_.find(key)) {
			os << (*uint_map_[key]);
		}
		else if (float_map_.end() != float_map_.find(key)) {
			os << (*float_map_[key]);
		}
		else if (double_map_.end() != double_map_.find(key)) {
			os << (*double_map_[key]);
		}
		else if (string_map_.end() != string_map_.find(key)) {
			os << "'" << *string_map_[key] << "'";
		}
		else {
			std::cout << "PrintConfig: unrecognized option " << key << "[code error]";
			exit(1);
		}
		os << '\n';
	}
	os << '\n';
}


void ParseOptions::ReadConfigFile(const std::string& filename) {
	std::ifstream is(filename.c_str(), std::ifstream::in);
	if (!is.good()) {
		std::cerr << "Cannot open config file " << filename << '\n';
		exit(1);
	}

	std::string line, key, value;
	while (!std::getline(is, line).eof()) {
		// trim out the comments
		size_t pos;
		if ((pos = line.find_first_of('#')) != std::string::npos) {
			line.erase(pos);
		}
		// skip empty lines
		Trim(&line);
		if (line.length() == 0) continue;

		// parse option
		SplitLongArg(line, &key, &value);
		NormalizeArgName(&key);
		Trim(&value);
		if (!SetOption(key, value)) {
			std::cerr << "Invalid option " << line << " in config file "
				<< filename << '\n';
			PrintUsage(true);
			exit(1);
		}
	}
}

void ParseOptions::SplitLongArg(std::string in, std::string* key,
	std::string* value) {
	assert(in.substr(0, 2) != " == ");  // precondition.
	size_t pos = in.find_first_of('=', 0);
	if (std::string::npos == pos ||
		std::string::npos != in.find_first_of('=', pos + 1)) {
		// defaults to empty.  We handle this differently in different cases.
		*key = in.substr(2, in.size() - 2);  // 2 because starts with --.
		*value = "";
	}
	else {
		*key = in.substr(2, pos - 2);  // 2 because starts with --.
		*value = in.substr(pos + 1);
	}
}

void ParseOptions::NormalizeArgName(std::string* str) {
	std::string out;
	std::string::iterator it;

	for (it = str->begin(); it != str->end(); ++it) {
		if (*it == '_') out += '-';  // convert _ to -
		else
			out += std::tolower(*it);
		// if (strip.find(*it) == std::string::npos) {
		// out += std::tolower(*it);
		//}
	}
	*str = out;

	assert(str->length() > 0);
}

bool ParseOptions::SetOption(const std::string& key, const std::string& value) {
	if (bool_map_.end() != bool_map_.find(key)) {
		*(bool_map_[key]) = ToBool(value);
	}
	else if (int_map_.end() != int_map_.find(key)) {
		*(int_map_[key]) = ToInt(value);
	}
	else if (uint_map_.end() != uint_map_.find(key)) {
		*(uint_map_[key]) = ToUInt(value);
	}
	else if (float_map_.end() != float_map_.find(key)) {
		*(float_map_[key]) = ToFloat(value);
	}
	else if (double_map_.end() != double_map_.find(key)) {
		*(double_map_[key]) = ToDouble(value);
	}
	else if (string_map_.end() != string_map_.find(key)) {
		*(string_map_[key]) = value;
	}
	else {
		return false;
	}
	return true;
}


bool ParseOptions::ToBool(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);

	// allow "" as a valid option for "true", so that --x is the same as --x=true
	if ((str.compare("true") == 0) || (str.compare("t") == 0)
		|| (str.compare("1") == 0) || (str.compare("") == 0)) {
		return true;
	}
	if ((str.compare("false") == 0) || (str.compare("f") == 0)
		|| (str.compare("0") == 0)) {
		return false;
	}
	std::cerr << "Invalid format for boolean argument [expected true or false]: " << str << '\n';
	PrintUsage(true);
	exit(1);
	return false;
}


int ParseOptions::ToInt(std::string str) {
	char* end_pos;
	// strtol is cheaper than stringstream...
	// strtol accepts decimal 438143, hexa 0x1f2d3 and octal 067123
	int ret = (int)std::strtol(str.c_str(), &end_pos, 0);
	if (str.c_str() == end_pos) {
		std::cerr << "Invalid integer option \"" << str << "\"\n";
		PrintUsage(true);
		exit(1);
	}
	return ret;
}

unsigned ParseOptions::ToUInt(std::string str) {
	char* end_pos;
	// strtol is cheaper than stringstream...
	// strtol accepts decimal 438143, hexa 0x1f2d3 and octal 067123
	unsigned ret = (unsigned)std::strtoul(str.c_str(), &end_pos, 0);
	if (str.c_str() == end_pos) {
		std::cerr << "Invalid integer option  \"" << str << "\"\n";
		PrintUsage(true);
		exit(1);
	}
	return ret;
}


float ParseOptions::ToFloat(std::string str) {
	char* end_pos;
	// strtod is cheaper than stringstream...
	float ret = std::strtod(str.c_str(), &end_pos);
	if (str.c_str() == end_pos) {
		std::cerr << "Invalid floating-point option \"" << str << "\"\n";
		PrintUsage(true);
		exit(1);
	}
	return ret;
}

double ParseOptions::ToDouble(std::string str) {
	char* end_pos;
	// strtod is cheaper than stringstream...
	double ret = std::strtod(str.c_str(), &end_pos);
	if (str.c_str() == end_pos) {
		std::cerr << "Invalid floating-point option  \"" << str << "\"\n";
		PrintUsage(true);
		exit(1);
	}
	return ret;
}

#endif
