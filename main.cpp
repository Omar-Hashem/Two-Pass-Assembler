#include <cstring>
#include <map>
#include <deque>
#include <queue>
#include <stack>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <math.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <vector>
#include <set>
#include <list>
#include <climits>
#include <cctype>
#include <bitset>
#include <iostream>
#include <complex>
#include <fstream>
#include <regex>
#include <iterator>
#include <unordered_map>
#include <iomanip>

#include "section.h"
#include "Pass2.h"

using namespace std;

map<string, pair<int, pair<string, string> > > operation_table;
vector<pair<string, pair<long long, pair<long long, string> > > > LITTAB; // <name , <length , <address , value>>>

unordered_set<string> directive;
vector<section> sections;
string current_section = "";
string intermediate_file;
vector<string>temp;

long long hextodec(string hexa) {
	long long ret;
	stringstream ss;
	ss << std::hex << hexa;
	ss >> ret;
	// output it as a signed type
	return static_cast<int>(ret);
}

bool is_directive(string s) {
	if (directive.find(s) != directive.end()) {
		return true;
	}
	return false;
}

string repeate(int cnt, string item) {
	string ret = "";
	for (int i = 0; i < cnt; i++) {
		ret += item;
	}
	return ret;
}

string dectohex(long long dec) {
	stringstream ss;
	ss << hex << dec;
	string result(ss.str());
	return result;
}

string spaces(int cnt) {
	string ret = "";
	for (int i = 0; i < cnt; i++) {
		ret += " ";
	}
	return ret;
}
string zeros(int cnt) {
	string ret = "";
	for (int i = 0; i < cnt; i++) {
		ret += "0";
	}
	return ret;
}

bool con(string s) {
	return !(s.compare("WORD") && s.compare("BYTE"));
}

bool var(string s) {
	return !(s.compare("RESW") && s.compare("RESB"));
}

bool mem(string s) {
	return con(s) || var(s);
}

bool is_start(string s) {
	return !(s.compare("START"));
}

bool is_end(string s) {
	return !(s.compare("END"));
}

bool isRegister(string r) {
	if (r == "A" || r == "B" || r == "X" || r == "T" || r == "L" || r == "S"
			|| r == "PC" || r == "SW" || r == "F")
		return true;
	return false;
}

section* getsection() {
	for (int i = 0; i < (int) sections.size(); i++) {
		if (sections[i].name == current_section) {
			section* res = &sections[i];
			return res;
		}
	}
	section* res = &sections[0];
	return res;
}

bool isSection(string name) {
	for (int i = 0; i < (int) sections.size(); i++) {
		if (sections[i].name == name)
			return true;
	}
	return false;
}

bool isnum(string s) {
	for (int i = 0; i < (int) s.length(); i++) {
		if (!isdigit(s[i]))
			return false;
	}
	return true;
}

void load_file() {
	section def("");
	sections.push_back(def);
	////////////////////////////////
	ifstream stream("op_code.txt");
	for (int i = 0; i < 39; i++) {
		string operation;
		int format;
		string opCode;
		string paramter;
		stream >> operation >> format >> opCode >> paramter;
//		////////////cout << operation << format << opCode << paramter<<endl;;
		operation_table[operation] = make_pair(format,
				make_pair(opCode, paramter));
	}
	stream.close();
	/////////////////////////////////////
	ifstream in("directive.txt");
	for (int i = 0; i < 7; i++) {
		string dir;
		in >> dir;
		directive.insert(dir);
	}
	in.close();
}

void extract_memories(instruction &current) {
	if (current.error != "" || current.comment_only)
		return;
	if (current.label != "" && current.operation != "EQU") { //todo
		section* sec = getsection();
		if (sec->memory.find(current.label) == sec->memory.end()) {
			sec->memory.insert( { current.label, { (long long) 0, true } });
		} else {
			current.error = "****** duplicate label definition";
		}
	}
}

void valid_operation(instruction &current) {

	if (current.comment_only || current.error != "")
		return;

	if (directive.find(current.operation) != directive.end()
			|| is_start(current.operation) || is_end(current.operation)
			|| mem(current.operation) || current.comment_only
			|| operation_table.find(current.operation) != operation_table.end()
			|| (current.operation.size() >= 1
					&& (current.operation.at(0) == '+')
					&& (operation_table.find(current.operation.substr(1))
							!= operation_table.end())))
		return;
	current.error = " ****** unrecognized operation code ";
}

void validation_phase1(instruction &current) {
	extract_memories(current);
	valid_operation(current);
}

bool valid(string s) {
	regex em("\\s*");
	if (s == "" || regex_match(s, em)) {
		return false;
	}
	return true;
}

vector<string> extract_tokens(string content) {
	string label_regex = "\\s*(\\w*)?";
	string operation_regex = "\\s*([a-zA-Z\\+]+)?\\s*";
	string operand_regex = "(=?[Cc]'.+'|[^ \\t]+)?\\s*";
	regex l3(label_regex + operation_regex + operand_regex);
	regex l2(operation_regex + operand_regex);
	regex l1(operation_regex);
	smatch sm;
	vector<string> tokens;
//////////////cout << content << endl;
	string word;
	bool done = false;
	regex tee("(=?[Cc]'.+')");

	regex_match(content, sm, l1);
	if (sm.size() > 0) {
		for (int i = 1; i < sm.size(); i++)
			if (sm[i] != "") {
				done = true;
				word = sm[i];
				transform(word.begin(), word.end(), word.begin(), ::toupper);
				tokens.push_back(word);
			}
	} else {
		regex_match(content, sm, l2);
		if (sm.size() > 0) {
			done = true;
			for (int i = 1; i < sm.size(); i++)
				if (sm[i] != "") {
					word = sm[i];
					if (!regex_match(word, tee)) {
						transform(word.begin(), word.end(), word.begin(),
								::toupper);
					} else {
						regex bee("(=[Cc]'.+')");
						if (regex_match(word, bee)) {
							word[1] = toupper(word[1]);
						} else {
							word[0] = toupper(word[0]);
						}
					}
					tokens.push_back(word);
				}
		} else {
			regex_match(content, sm, l3);
			if (sm.size() > 0) {
				done = true;
				for (int i = 1; i < sm.size(); i++)
					if (sm[i] != "") {
						word = sm[i];
						if (!regex_match(word, tee)) {
							transform(word.begin(), word.end(), word.begin(),
									::toupper);
						} else {
							regex bee("(=[Cc]'.+')");
							if (regex_match(word, bee)) {
								word[1] = toupper(word[1]);
							} else {
								word[0] = toupper(word[0]);
							}
						}
						tokens.push_back(word);
					}
			}
		}
	}

	if (!done) {
		tokens.push_back(content);
//////////////cout <<  "->>>>>>>>> yes"<< endl;
	}

	return tokens;
}

//string repeate(int cnt, string item) {
//	string ret = "";
//	for (int i = 0; i < cnt; i++) {
//		ret += item;
//	}
//	return ret;
//}

string trim(string in) {
	in.erase(in.begin(),
			find_if(in.begin(), in.end(), bind1st(not_equal_to<char>(), ' ')));
	in.erase(
			find_if(in.rbegin(), in.rend(), bind1st(not_equal_to<char>(), ' ')).base(),
			in.end());
	return in;
}

string addd(string s) {
	string in = s;
	int pl = in.find("+");
	if (pl != string::npos) {
		regex com("\\+\\s*");
		in = regex_replace(in, com, "+");
	}
	s = in;
	transform(in.begin(), in.end(), in.begin(), ::toupper);
//////////////cout << s << "<----------"<< endl;
	smatch m;
	regex e("(\\w+)(\\s*)\\+(\\w+)");
	while (regex_search(in, m, e)) {
		if (m.size() == 0)
			break;
		string pre = m[1];
		string post = m[3];

		if (!(operation_table.find(pre) != operation_table.end()
				|| operation_table.find(post) != operation_table.end()
				|| is_start(pre) || is_end(pre) || mem(pre) || is_directive(pre)
				|| is_start(post) || is_end(post) || mem(post)
				|| is_directive(post))) {

			in = in.substr(m.prefix().str().size());

			regex ee("\\s+");
			int sizeBefore = in.size();
			in = regex_replace(in, ee, "");
			in = repeate(sizeBefore - in.size(), " ") + in;
			s = s.substr(0, s.size() - in.size()) + in;
//            ////////////cout <<pre <<" "<< post << endl;
			return s;
//            ////////////cout << in << endl;
//            ////////////cout << in << endl;
		}
//       regex_search (in,m,ee);
		in = m.suffix().str();
		if (in == "") {
			break;
		}
	}
	return s;
}

instruction read_input_free(string line) {
	instruction current = instruction("", "");
	if (!valid(line)) {
		return instruction("", " ");
	}

	string content = "";
	string comment = "";

	int dot = line.find(".");
	if (dot != string::npos) {
		comment = line.substr(dot, line.size() - dot);
		content = line.substr(0, dot);
	} else {
		content = line;
		comment = " ";
	}
//    ////////////cout <<"***********************"<< endl <<  content << endl <<  comment << endl <<"*******************************" << endl ;
	if (content == "") {
		return instruction("", comment);
	}
	int comma = content.find(",");
	if (comma != string::npos) {
		regex com("\\s*,\\s*");
		content = regex_replace(content, com, ",");
	}
	int quote = content.find("'");
	if (quote != string::npos) {
		regex com("([CX])\\s*'");
		content = regex_replace(content, com, "$1'");
	}

	int equ = content.find("=");
	if (equ != string::npos) {
		regex com("=\\s*([CX])");
		content = regex_replace(content, com, "=$1");
	}

	int div = content.find("/");
	if (div != string::npos) {
		regex com("\\s*/\\s*");
		content = regex_replace(content, com, "/");
	}

	int imm = content.find("#");
	if (imm != string::npos) {
		regex com("#\\s*");
		content = regex_replace(content, com, "#");
	}

	int ind = content.find("@");
	if (ind != string::npos) {
		regex com("@\\s*");
		content = regex_replace(content, com, "@");
	}

	int mul = content.find("*");
	if (mul != string::npos) {
		regex isMul(".*\\s*\\*\\s*\\w+");
		if (regex_match(content, isMul)) {
			regex com("\\s*\\*\\s*");
			content = regex_replace(content, com, "*");
		}
	}
	int sub = content.find("-");
	if (sub != string::npos) {
		regex com("\\s*-\\s*");
		content = regex_replace(content, com, "-");
	}

	int eqst = content.find("=");
	if (eqst != string::npos) {
		regex com("=\\s*\\*");
		content = regex_replace(content, com, "=*");
	}

	int bracket1 = content.find("(");
	int bracket2 = content.find(")");
	if (bracket1 != string::npos || bracket2 != string::npos) {
		regex com("\\s+");
		int bSize = content.size();
		string temp = content;
		int bracket;
		if (bracket1 != string::npos) {
			if (bracket2 != string::npos) {
				bracket = min(bracket1, bracket2);
			} else {
				bracket = bracket1;
			}
		} else {
			if (bracket2 != string::npos) {
				bracket = bracket2;
			} else {
				bracket = bSize;
			}
		}
//        int bracket = max(min(bracket1, bracket2), 0);
		////////////cout << "=============>" << bracket;
		content = content.substr(bracket);
		content = regex_replace(content, com, "");
//        ////////////cout << content << endl;
		content = temp.substr(0, bracket)
				+ repeate(bSize - bracket - content.size(), " ") + content;
	}

	content = addd(content);

	vector<string> tokens = extract_tokens(content);

//    for(auto i : tokens){
//        ////////////cout << i<<" " ;
//    }
//
//////////////cout << endl;
	if (tokens.size() == 0) {
//    ////////////cout <<"************matched***********"<< endl <<  content << endl <<  comment << endl <<"*******************************" << endl ;
		//error couldn't recognize instruction parts
		current = instruction("", line);

		return current;
	}

	if (tokens.size() == 1) {
		if (operation_table.find(tokens[0]) != operation_table.end()
				|| is_end(tokens[0]) || is_start(tokens[0])
				|| is_directive(tokens[0])) {
			current = instruction("", tokens[0], "", comment, "");
		} else {
			current = instruction("", tokens[0], "", comment,
					"****** unrecognized operation code");
		}
	}

// \s*([A-Z]\w*)?\s*(\w+)?\s*(C'[\w\s]+'|[^ \t]+)?\s*
//need modification for literals
/////////////////////////////////////////////////////////////////

	if (tokens.size() == 2) {
		if (operation_table.find(tokens[0]) != operation_table.end()
				|| is_start(tokens[0]) || is_end(tokens[0]) || mem(tokens[0])
				|| is_directive(tokens[0])) {

			current = instruction("", tokens[0], tokens[1], comment, "");
		} else if (operation_table.find(tokens[1]) != operation_table.end()
				|| is_end(tokens[1]) || is_directive(tokens[1])) {
			current = instruction(tokens[0], tokens[1], "", comment, "");
			//can be in format 4 ????
		} else if (operation_table.find(
				tokens[0].substr(1, tokens[0].size() - 1))
				!= operation_table.end()) {
			current = instruction("", tokens[0], tokens[1], comment, "");
		} else if (operation_table.find(
				tokens[1].substr(1, tokens[1].size() - 1))
				!= operation_table.end()) {

			current = instruction(tokens[0], tokens[1], "", comment, "");
		} else {
			current = instruction("", tokens[0], tokens[1], comment,
					"****** unrecognized operation code");
			return current;
		}
	}
///////////////////////////////////////////////////////////////////////
	if (tokens.size() == 3) {
		if (operation_table.find(tokens[0]) != operation_table.end()
				|| mem(tokens[0]) || is_end(tokens[0]) || is_start(tokens[0])) {
//operation operand1 operand2 ***error***
			current = instruction(tokens[0], tokens[1], tokens[2], comment,
					"missplaced mnemonic");
			return current;
		} else if (operation_table.find(tokens[1]) != operation_table.end()
				|| is_start(tokens[1]) || is_end(tokens[1]) || mem(tokens[1])
				|| is_directive(tokens[1])) {

			current = instruction(tokens[0], tokens[1], tokens[2], comment, "");
		} else if (operation_table.find(
				tokens[0].substr(1, tokens[0].size() - 1))
				!= operation_table.end()) {
			current = instruction(tokens[0], tokens[1], tokens[2], comment,
					"missplaced mnemonic");
			return current;
		} else if (operation_table.find(
				tokens[1].substr(1, tokens[1].size() - 1))
				!= operation_table.end()) {
			current = instruction(tokens[0], tokens[1], tokens[2], comment, "");
		} else {

			current = instruction(tokens[0], tokens[1], tokens[2], comment,
					"****** unrecognized operation code");
			return current;
		}
	}

	if (tokens.size() > 3) {
		current = instruction("****** elements > 4 ", line);
		return current;
	}

	if (current.operand != "") {
		//( - 8)  ( -kjhg  )
//		////cout <<"==>" <<current.operand <<"<==="<< endl;
		regex dd(
				"=?\\*|(\\w{1,8},\\d{1,4})|(=?C'.{0,15}')|(=?W'-?\\d{1,4}')|(=?X'[0-9A-F]{0,15}')|((\\w|PC|SW),(\\w|PC|SW))|(\\w{1,16},X)|#\\w{1,17}|@\\w{1,17}|(\\w+(,\\w+)*)|\\w{1,17}|\\w+[\\+\\/\\*\\-]\\w+|[#@]?\\(*-?\\w+\\(*\\)*([\\/\\*\\-\\+]-?\\w+)*\\)*");
		string check = current.operand;
		if (!regex_match(check, dd)) {
			current.error = "****** unrecognized operand";
			return current;
		}

	}

	if (current.operation.compare("BYTE") == 0) {
		regex dd("(C'.{0,15}')|(X'[0-9A-F]{0,15}')|([0-9A-F]{0,15})");
		string check = current.operand;
		if (!regex_match(check, dd)) {
			current.error = "****** unrecognized operand";
			return current;
		}
	}

	if (current.operation.compare("WORD") == 0) {
		regex dd(
				"\\d+(,\\d+)*|\\w+|\\w+[\\+\\/\\*\\-]\\w+|[#@]?\\(*-?\\w+\\(*\\)*([\\/\\*\\-\\+]-?\\w+)*\\)*");
		string check = current.operand;
		if (!regex_match(check, dd)) {
			current.error = "****** unrecognized operand";
			return current;
		}
	}

	if (current.label != "") {
		regex dd("[A-Z]\\w{0,7}");
		string check = current.label;
		if (!regex_match(check, dd)) {
			current.error = "****** unrecognized label";
			return current;
		}
	}
	return current;
}

instruction read_input_fixed(string line) {

	if (!valid(line)) {
		return instruction("", " ");
	}

	instruction current = instruction("", "");

	if (line[0] == '.') {
		transform(line.begin(), line.end(), line.begin(), ::toupper);

		current = instruction("", line);
		return current;
	}
	vector<string> tokens;
	int len = line.length();
	regex fe("\t");
	regex_replace(line, fe, " ");

	for (int i = len; i < 66; i++) {
		line += " ";
	}
	string error = "";

	string label = line.substr(0, 8);
	string mnemonic = line.substr(8, 7);
	string operand = line.substr(15, 20);
	string comment = line.substr(35);

	transform(label.begin(), label.end(), label.begin(), ::toupper);
	transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);
	transform(comment.begin(), comment.end(), comment.begin(), ::toupper);

	if (operand != "") {
		regex tee("  ([Cc]'[\\w\\s]+')\\s*");
		if (!regex_match(operand, tee)) {
			transform(operand.begin(), operand.end(), operand.begin(),
					::toupper);
		} else {
			operand[2] = toupper(operand[2]);
		}
	}

	regex lbl("[A-Z]\\w* *| {8}");
	regex operation("[+ ][A-Z]+ *");
	regex oper(
			"  ( *|\\*|(C'.*')|(X'[0-9A-F]*')|((\\w|PC|SW),(\\w|PC|SW))|(\\w+,X)|[#@]\\w+|(\\d+(,\\d+)*)|\\w+) *");

	if (len > 66) {
		error = "****** extra characters at end of statement";
		//program.push_back(instruction(label, mnemonic, operand, comment, error));
		//continue;
	} else if (!regex_match(label, lbl)) {
		error = "****** unrecognized label";
		//program.push_back(instruction(label, mnemonic, operand, comment, error));
		//continue;
	} else if (!regex_match(mnemonic, operation)) {
		error = "****** unrecognized operation code";
		//program.push_back(instruction(label, mnemonic, operand, comment, error));
		//continue;
	} else if (!regex_match(operand, oper)) {
		error = "****** unrecognized operand";
		//program.push_back(instruction(label, mnemonic, operand, comment, error));
		//continue;
	}

	label = trim(label);
	mnemonic = trim(mnemonic);
	operand = trim(operand);
	comment = trim(comment);

	if (error != "") {
//		program.push_back(
//				instruction(label, mnemonic, operand, comment, error));
		current = instruction(label, mnemonic, operand, comment, error);
		return current;
	}

	if (!(operation_table.find(mnemonic) != operation_table.end()
			|| operation_table.find(mnemonic.substr(1, mnemonic.size() - 1))
					!= operation_table.end() || is_start(mnemonic)
			|| is_end(mnemonic) || mem(mnemonic))) {
		error = "****** unrecognized operation code";
//		program.push_back(
//				instruction(label, mnemonic, operand, comment, error));
//		continue;
		current = instruction(label, mnemonic, operand, comment, error);
		return current;

	} else if (mnemonic.compare("BYTE") == 0) {
		regex dd("(C'.{0,15}')|(X'[0-9A-F]{0,15}')|([0-9A-F]{0,15})");
		if (!regex_match(operand, dd)) {
			error = "****** unrecognized operand";
//			program.push_back(
//					instruction(label, mnemonic, operand, comment, error));
//			continue;
			current = instruction(label, mnemonic, operand, comment, error);
			return current;

		}
	} else if (mnemonic.compare("WORD") == 0) {
		regex dd("\\d+(,\\d+)*|\\w+");
		if (!regex_match(operand, dd)) {
			error = "****** unrecognized operand";
//			program.push_back(
//					instruction(label, mnemonic, operand, comment, error));
//			continue;
			current = instruction(label, mnemonic, operand, comment, error);
			return current;
		}
	}

//	program.push_back(instruction(label, mnemonic, operand, comment, ""));
	return instruction(label, mnemonic, operand, comment, "");
}

pair<int, int> testEQU(instruction& inst) /// takes expression returns value and boolean (REL/ABS)
		{
	int countBracket = 0;
	bool indexed = false;
	bool immediate = false;
	string qwertyuiop = inst.operand;
	vector<string> qwertyu;

	if (qwertyuiop.size() == 1 && qwertyuiop == "*") {
//        ////////cout << "STAR" << endl;
		return make_pair(0, 1);             //return PC counter  STAR
	}

	if (qwertyuiop.size() > 0
			&& (qwertyuiop[0] == '#' || qwertyuiop[0] == '@')) {
		immediate = true;
		qwertyuiop = qwertyuiop.substr(1, qwertyuiop.size() - 1);
	}

	if (qwertyuiop.size() > 1) {
		if (qwertyuiop[qwertyuiop.size() - 2] == ','
				&& qwertyuiop[qwertyuiop.size() - 1] == 'X') {
			qwertyuiop = qwertyuiop.substr(0, qwertyuiop.size() - 2);
			indexed = true;
		}
	}

	if (indexed & immediate) {
		inst.error = " ****** illegal addressing mode";
		return make_pair(1 << 30, 0);
	}

	for (int i = 0; i < qwertyuiop.size(); ++i) {
		string asdfghjkl = qwertyuiop.substr(i, 1);

		if (qwertyuiop[i] == '(')
			++countBracket;

		else if (qwertyuiop[i] == ')')
			--countBracket;

		else if (isalpha(qwertyuiop[i])) {
			while (i + 1 < qwertyuiop.size()
					&& (isdigit(qwertyuiop[i + 1])
							|| (isalpha(qwertyuiop[i + 1])))) {
				asdfghjkl += qwertyuiop[i + 1];
				++i;
			}
		} else if (isdigit(qwertyuiop[i])) {
			asdfghjkl = qwertyuiop.substr(i, 1);
			while (i + 1 < qwertyuiop.size()
					&& (qwertyuiop[i + 1] >= '0' && qwertyuiop[i + 1] <= '9')) {
				asdfghjkl += qwertyuiop[i + 1];
				++i;
			}
		} else if (qwertyuiop[i] == '+') {
			if (i + 1 < qwertyuiop.size()
					&& (qwertyuiop[i + 1] == '-' || qwertyuiop[i + 1] == '+'
							|| qwertyuiop[i + 1] == '/'
							|| qwertyuiop[i + 1] == '*')) {
				inst.error = " ****** illegal operand";
				return make_pair(1 << 30, 0);
			}
		} else if (qwertyuiop[i] == '-') {
			if (i + 1 < qwertyuiop.size()
					&& (qwertyuiop[i + 1] == '*' || qwertyuiop[i + 1] == '+'
							|| qwertyuiop[i + 1] == '/')) {
				inst.error = " ******  illegal operad";
				return make_pair(1 << 30, 0);
			}

			if (i + 1 < qwertyuiop.size() && (qwertyuiop[i + 1] == '-')) {
				if ((i + 2 < qwertyuiop.size())
						&& (qwertyuiop[i + 2] == '-' || qwertyuiop[i + 2] == '*'
								|| qwertyuiop[i + 2] == '+'
								|| qwertyuiop[i + 2] == '/')) {
					inst.error = " ******  illegal operad";
					return make_pair(1 << 30, 0);
				}
				++i;
				asdfghjkl = '+';
			}
		} else if (qwertyuiop[i] == '*') {
			if (i + 1 < qwertyuiop.size()
					&& (qwertyuiop[i + 1] == '*' || qwertyuiop[i + 1] == '+'
							|| qwertyuiop[i + 1] == '/')) {
				inst.error = " ******  illegal operad";
				return make_pair(1 << 30, 0);
			}
			if (i + 1 < qwertyuiop.size() && qwertyuiop[i + 1] == '-') {
				if ((i + 2 < qwertyuiop.size())
						&& (qwertyuiop[i + 2] == '-' || qwertyuiop[i + 2] == '*'
								|| qwertyuiop[i + 2] == '+'
								|| qwertyuiop[i + 2] == '/')) {
					inst.error = " ******  illegal operad";
					return make_pair(1 << 30, 0);
				}
				qwertyu.push_back("*");
				asdfghjkl = '_';
				++i;
			}
		} else if (qwertyuiop[i] == '/') {
			if (i + 1 < qwertyuiop.size()
					&& (qwertyuiop[i + 1] == '*' || qwertyuiop[i + 1] == '+'
							|| qwertyuiop[i + 1] == '/')) {
				inst.error = " ******  illegal operad";
				return make_pair(1 << 30, 0);
			}
			if (i + 1 < qwertyuiop.size() && qwertyuiop[i + 1] == '-') {
				if ((i + 2 < qwertyuiop.size())
						&& (qwertyuiop[i + 2] == '-' || qwertyuiop[i + 2] == '*'
								|| qwertyuiop[i + 2] == '+'
								|| qwertyuiop[i + 2] == '/')) {
					inst.error = " ******  illegal operad";
					return make_pair(1 << 30, 0);
				}
				qwertyu.push_back("/");
				asdfghjkl = '_';
				++i;
			}
		} else {
			inst.error = " ******  illegal operad";
			return make_pair(1 << 30, 0);
		}
		if (countBracket < 0) {
			inst.error = " ******  illegal operad";
			return make_pair(1 << 30, 0);
		}
		qwertyu.push_back(asdfghjkl);
	}

	if (countBracket) {
		inst.error = " ******  illegal operad";
		return make_pair(1 << 30, 0);
	}

	vector<string> postfix;
	stack<string> stackk;
	int sign = 1;
	if (qwertyu.size() > 1 && qwertyu[0] == "-") {
		sign = -1;
		qwertyu.erase(qwertyu.begin(), qwertyu.begin() + 1);
	}
	for (int j = 0; j < qwertyu.size(); ++j) {
		if ((!j)
				&& (qwertyu[j] == "+" || qwertyu[j] == "-" || qwertyu[j] == "*"
						|| qwertyu[j] == "/")) {
			inst.error = " ******  illegal operad";
			return make_pair(1 << 30, 0);
		}
		if (isalpha(qwertyu[j][0])) {
			///check symbol table of qwertyu[j]
			section* sec = getsection();
			if (sec->memory.find(qwertyu[j]) == sec->memory.end()) {
				inst.error = "****** undefined symbol in operand";
				return make_pair(1 << 30, 0);
			}
			postfix.push_back(qwertyu[j]);
		} else if (isdigit(qwertyu[j][0])) {
			if (qwertyu[j].size() > 1048575)       ///number of available digits
					{
				inst.error = " ****** Address out of range";
				return make_pair(1 << 30, 0);
			}
			postfix.push_back(qwertyu[j]);
		} else if (qwertyu[j] == "_")
			stackk.push(qwertyu[j]);

		else if (qwertyu[j] == "+" || qwertyu[j] == "-") {
			if (qwertyu[j - 1] == "(" || (j + 2 > qwertyu.size())
					|| qwertyu[j + 1] == ")") {
				inst.error = " ******  illegal operad";
				return make_pair(1 << 30, 0);
			}
			if (stackk.empty() || stackk.top() == "(")
				stackk.push(qwertyu[j]);

			else {
				while (!stackk.empty() && stackk.top() != "(") {
					postfix.push_back(stackk.top());
					stackk.pop();
				}
				stackk.push(qwertyu[j]);
			}
		} else if (qwertyu[j] == "*" || qwertyu[j] == "/") {
			if (qwertyu[j - 1] == "(" || (j + 2 > qwertyu.size())
					|| qwertyu[j + 1] == ")") {
				inst.error = " ******  illegal operad";
				return make_pair(1 << 30, 0);
			}
			while (!stackk.empty()
					&& (stackk.top() == "*" || stackk.top() == "/"
							|| stackk.top() == "_")) {
				postfix.push_back(stackk.top());
				stackk.pop();
			}
			stackk.push(qwertyu[j]);
		} else if (qwertyu[j] == "(")
			stackk.push(qwertyu[j]);

		else if (qwertyu[j] == ")") {
			while (stackk.top() != "(") {
				postfix.push_back(stackk.top());
				stackk.pop();
			}
			stackk.pop();
		}
	}
	while (!stackk.empty()) {
		postfix.push_back(stackk.top());
		stackk.pop();
	}
	stack<pair<int, int> > stackkk;
	for (int j = 0; j < postfix.size(); ++j) {
		if (isdigit(postfix[j][0])) {
			int value = 0;
			for (int k = 0; k < postfix[j].size(); ++k)
				value = value * 10 + postfix[j][k] - '0';

			stackkk.push(make_pair(value, 0));
		} else if (isalpha(postfix[j][0])) {
			int value = 5; ///get value and type (rel / abs) from symbol table of postfix[j]
			section* sec = getsection();
			if (sec->memory.find(postfix[j]) != sec->memory.end()) {
				value = sec->memory[postfix[j]].first;
			}
			stackkk.push(make_pair(value, 1));
		}

		else if (postfix[j] == "_") {
			pair<int, int> value = stackkk.top();
			stackkk.pop();
			value.first = -value.first;
			stackkk.push(value);
		} else {
			if (stackkk.size() < 2) {
				inst.error = " ******  illegal operad";
				return make_pair(1 << 30, 0);
			}
			pair<int, int> value[3];
			value[1] = stackkk.top();
			stackkk.pop();
			value[0] = stackkk.top();
			stackkk.pop();

			if (postfix[j] == "-") {
				value[3].first = value[0].first - value[1].first;
				value[3].second = (value[0].second + value[1].second) & 1;
			}

			else if (postfix[j] == "+") {
				value[3].first = value[0].first + value[1].first;
				if (value[0].second == 1 && value[1].second == 1) {
					inst.error = " ****** relative plus relative"; ///name the error
					return make_pair(1 << 30, 0);
				}
				value[3].second = value[0].second + value[1].second;
			}

			else if (postfix[j] == "*") {
				value[3].first = value[0].first * value[1].first;
				if (value[0].second + value[1].second) {
					inst.error = " ****** relative multiple relative"; ///name the error
					return make_pair(1 << 30, 0);
				}
				value[3].second = value[0].second + value[1].second;
			}

			else if (postfix[j] == "/") {
				if (value[1].first == 0) {
//					////////cout << " ***** DIVISION BY ZERO" << endl;
					return make_pair(1 << 30, 0);
				}
				value[3].first = value[0].first / value[1].first;
				if (value[0].second + value[1].second) {
					inst.error = " ****** relative over relative"; ///name the error
					return make_pair(1 << 30, 0);
				}
				value[3].second = value[0].second + value[1].second;
			} else {
				inst.error = " ****** illegal operand";
				return make_pair(1 << 30, 0);
			}
			stackkk.push(value[3]);
		}
	}
	if (stackkk.size() != 1) {
		inst.error = " ****** illegal operand";
		return make_pair(1 << 30, 0);
	}
	pair<int, int> temp = stackkk.top();
	temp.first = temp.first * sign;
	return temp;
}

pair<string, pair<long long, pair<long long, string> > > get_literal(string s) {
	for (int i = 0, n = LITTAB.size(); i < n; i++)
		if (s == LITTAB[i].first)
			return LITTAB[i];
}

string print_literals(vector<string> lit) {
	string ret = "";
	stringstream out(ret);
	pair<string, pair<long long, pair<long long, string> > > literal;
	for (int i = 0; i < lit.size(); i++) {
		literal = get_literal(lit[i]);
		string address = dectohex(literal.second.second.first);
		transform(address.begin(), address.end(), address.begin(), ::toupper);
		string name = literal.first;
		out << repeate(6 - address.size(), "0") << address << " *       " << "="
				<< name << repeate(6 - name.size(), " ")
				<< (name.size() > 7 ? " " : "") << endl;
	}
	return out.str();
}

void print_prgram(instruction &current, string &intermediate_file,
		bool &error) {

//	string str = "+" + repeate(73, "-") + "+";
//	string tr = "+" + repeate(6, "-") + "+" + repeate(8, "-") + "+"
//			+ repeate(7, "-") + "+" + repeate(18, "-") + "+" + repeate(30, "-")
//			+ "+";
//	////////////cout << current.operation << endl;
//	intermediate_file += tr + "\n";
//	////////////cout << current.label <<"  " << current.operation <<" " <<current.operand << endl;
	string tmp = dectohex(current.pc_counter);
	transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
	if (current.comment_only || current.operation.compare("") == 0) {
		if (current.comment != "") {
			string tmp1 = repeate(6, " ");
			intermediate_file += tmp1 + current.comment
					+ repeate(66 - current.comment.size(), " ") + "\n";
		}
		if (current.error != "") {
			intermediate_file += current.error
					+ repeate(72 - current.error.size(), " ") + "\n";
		}
		error = true;
		return;
	}
	string tmp1 = repeate(6 - tmp.length(), "0") + tmp;
	if (current.pc_counter == -1) {
		tmp1 = "      ";
	}
	string tmp2 = current.label + repeate(8 - current.label.length(), " ");
	string tmp3 = current.operation
			+ repeate(7 - current.operation.length(), " ");
	string tmp4 = current.operand + repeate(18 - current.operand.length(), " ");
	string tmp5 = current.comment + repeate(30 - current.comment.length(), " ");
	error = false;
	intermediate_file += tmp1 + " " + tmp2 + tmp3 + tmp4 + tmp5 + "\n";
	if (current.error.compare("") != 0) {
		intermediate_file += current.error
				+ repeate(72 - current.error.size(), " ") + "\n";
		error = true;
	}
	if (current.operation == "LTORG" || current.operation == "END") {
		intermediate_file += print_literals(current.litpool);
	}
}

bool ishex(string num) {
	for (int i = 0; i < (int) num.length(); i++) {
		if (!(isdigit(num[i]) || num[i] == 'A' || num[i] == 'B' || num[i] == 'C'
				|| num[i] == 'D' || num[i] == 'E' || num[i] == 'F'))
			return false;
	}
	return true;
}

//int litpool_index=0;

vector<string> getLabels(string in) {
	vector<string> ans;
	smatch m;
	regex e("[A-Z]\\w+");
	while (regex_search(in, m, e)) {
		if (m.size() == 0)
			break;
		string res = m[0];
//		////////////cout << res << " ";

		ans.push_back(res);
		in = m.suffix().str();
		if (in == "") {
			break;
		}
	}
//	////////////cout << endl;
	return ans;
}

void validation_directive(instruction &current) {
	if (current.comment_only || current.error != "")
		return;
	///////////////////////////////////////////////////////
	bool format4 = false;
	bool index = false;
	bool indirect = false;
	string operand = current.operand;
	string operation = current.operation;
	if (current.operation[0] == '+') {
		operation = current.operation.substr(1, current.operation.length() - 1);
		format4 = true;
	}
	if (current.operand[0] == '#' || current.operand[0] == '@') {
		operand = operand.substr(1, operand.length() - 1);
		indirect = true;
	}
	if (operand.length() > 2) {
		string suffex = operand.substr(operand.length() - 2, 2);
		if (suffex == ",X") {
			operand = operand.substr(0, operand.length() - 2);
			index = true;
		}
	}
	///////////////////////////////////////////////////////
	if (current.operation == "LTORG") {
		if (format4) {
			current.error = "****** can not be format 4 instrcction";
			return;
		}
		if (current.operand != "") {
			current.error = "****** illegal operand";
			return;
		}

		return;
	}
	////////////////////////////////////////////////////////
	else if (current.operation == "ORG") {
		section* sec = getsection();
		vector<string> var = getLabels(current.operand);
		for (int i = 0; i < (int) var.size(); i++) {
			if (sec->memory.find(var[i]) == sec->memory.end()) {
				current.error =
						"****** not allow for forward reference or undifined";
			}
		}
	}
	/////////////////////////////////////////////////////
	else if (current.operation == "EQU") {
		section* sec = getsection();
		vector<string> var = getLabels(current.operand);
		for (int i = 0; i < (int) var.size(); i++) {
			if (sec->memory.find(var[i]) == sec->memory.end()) {
				current.error =
						"****** not allow for forward reference or undifined";
			}
		}
	}
	////////////////////////////////////////////////////
	else if (current.operation == "CSECT") {
		if (format4) {
			current.error = "****** can not be format 4 instrcction";
			return;
		}
		if (index || indirect) {
			current.error = "****** illegal operand";
			return;
		}
		section sec(current.label);
		sections.push_back(sec);
		current_section = current.label;
	}
	////////////////////////////////////////////////////
	else if (current.operation == "EXTDEF") {
		if (format4) {
			current.error = "****** can not be format 4 instruction";
			return;
		}
		if (index || indirect) {
			current.error = "****** illegal operand";
			return;
		}
		section *sec = getsection();
		istringstream ss(current.operand);
		string tokenizer;
		vector<string> arr;
		while (getline(ss, tokenizer, ',')) {
			arr.push_back(tokenizer);
		}
//		bool ok = true;
//		for (int i = 0; i < (int) arr.size(); i++) {
//			if (sec->memory.find(arr[i]) == sec->memory.end()) {
//				ok = false;
//				break;
//			}
//		}
//		if (ok) {
		for (int i = 0; i < (int) arr.size(); i++) {
			sec->def.insert(arr[i]);
		}
//		}
	}
	////////////////////////////////////////////////////////
	else if (current.operation == "EXTREF") {
		if (format4) {
			current.error = "****** can not be format 4 instrcction";
			return;
		}
		if (index || indirect) {
			current.error = "****** illegal operand";
			return;
		}
		section *sec = getsection();
		istringstream ss(current.operand);
		string tokenizer;
		vector<string> arr;
		while (getline(ss, tokenizer, ',')) {
			arr.push_back(tokenizer);
		}

		for (int i = 0; i < (int) arr.size(); i++) {
			sec->ref.insert(arr[i]);
		}

	}

}

void validation_phase2(instruction &current) {
	if (current.comment_only || current.error != "")
		return;
	/////////////////////////////////////////////////////
	bool format4 = false;
	bool index = false;
	bool indirect = false;
	string operand = current.operand;
	string operation = current.operation;
	if (current.operation[0] == '+') {
		operation = current.operation.substr(1, current.operation.length() - 1);
		format4 = true;
	}
	if (current.operand[0] == '#' || current.operand[0] == '@') {
		operand = operand.substr(1, operand.length() - 1);
		indirect = true;
	}
	if (operand.length() > 2) {
		string suffex = operand.substr(operand.length() - 2, 2);
		if (suffex == ",X") {
			operand = operand.substr(0, operand.length() - 2);
			index = true;
		}
	}
	////////////////////////////////////////////////////
	if (var(operation)) {
		if (index) {
			current.error = "****** extra characters at end of statement";
			return;
		}
		if (format4) {
			current.error = "****** can not be format 4 instrcction";
			return;
		}
		pair<int, int> res = testEQU(current);
		if (current.error == "") {
			if (isnum(operand) && operand.length() > 4) {
				current.error = "****** extra characters at end of statement";
				return;
			}
//			if (!isnum(current.operand)) {
//				current.error = "****** illegal operand";
//			}
		}
		return;
	}
	/////////////////////////////////////////////////
	if (con(operation)) {
		if (format4) {
			current.error = "****** can not be format 4 instrcction";
			return;
		}
		if (operation == "WORD") {
			istringstream ss(current.operand);
			string tokenizer;
			vector<string> arr;
			while (getline(ss, tokenizer, ',')) {
				arr.push_back(tokenizer);
			}
			if (arr.size() == 1) {
				if (isnum(arr[0]) && arr[0].length() > 4) {
					current.error =
							"****** extra characters at end of statement";
					return;
				}
				if (arr[0][0] == '-'
						&& isnum(arr[0].substr(1, arr[0].length() - 1))
						&& arr[0].length() > 4) {
					current.error =
							"****** extra characters at end of statement";
					return;
				}
				if (isdigit(arr[0][0]) && !isnum(arr[0])) {
					current.error =
							"****** extra characters at end of statement";
					return;
				}
				return;
			}
			for (int j = 0; j < (int) arr.size(); j++) {
				if (arr[j] == "" && j == 0) {
					current.error = "****** illegal operand";
					break;
				}
				if (arr[j] == "")
					return;
				if (arr[j][0] == '-') {
					if (!isnum(arr[j].substr(1, arr[j].length() - 1))) {
						current.error =
								"****** extra characters at end of statement";
					}
					if (isnum(arr[j].substr(1, arr[j].length() - 1))) {
						if (arr[j].length() - 1 > 4) {
							current.error =
									"****** extra characters at end of statement";
						}
					}
					break;
				}
				if (!isnum(arr[j])) {
					current.error =
							"****** extra characters at end of statement";
					break;
				}
				if (isnum(arr[j]) && arr[j].length() > 4) {
					current.error =
							"****** extra characters at end of statement";
					break;
				}
			}
		} else {
			if (operand[0] == 'C') {
				if (operand[1] == '\''
						&& operand[operand.length() - 1] == '\'') {
					return;
				} else {
					current.error =
							"****** extra characters at end of statement";
				}
			} else if (operand[0] == 'X') {
				if (operand[1] == '\''
						&& operand[operand.length() - 1] == '\'') {
					operand = operand.substr(2, operand.length() - 3);
					if (operand.length() % 2 == 1) {
						current.error = "****** odd length for hex string";
					} else {
						for (int j = 0; j < (int) operand.length(); j++) {
							if (!isdigit(operand[j]) && operand[j] != 'A'
									&& operand[j] != 'B' && operand[j] != 'C'
									&& operand[j] != 'D' && operand[j] != 'E'
									&& operand[j] != 'F') {
								current.error =
										"****** not a hexadecimal string";
							}
						}
					}

				} else {
					current.error = "****** illegal operand";
				}
			} else {
				current.error = "****** illegal operand";
			}
		}
		return;
	}
	//////////////////////////////////////////////////
	if (operation == "START") {
		if (indirect) {
			current.error = "****** illegal operand";
		} else if (index) {
			current.error = "****** extra characters at end of statement";
		} else if (current.operand.length() > 4) {
			current.error = "****** extra characters at end of statement";
		} else if (current.operand.length() > 1 && !ishex(current.operand)) {
			current.error = "****** illegal operand";
		}

		return;
	}
	//////////////////////////////////////////////////
	if (operation == "END") {
//		return; // edit
		if (index && indirect) {
			current.error = "****** illegal addressing mode";
			return;
		}
		if (current.operand == "")
			return;
		bool ok = false;
		for (section sec : sections) {
			if (sec.memory.find(current.operand) != sec.memory.end()) {
				ok = true;
			}
		}
		if (!ok) {
			current.error = "****** unrecognized symbol in operand";
		}
		return;
	}
	/////////////////////////////////////////////////////
	if (operation_table[operation].second.second == "m") {
		if (index && indirect) {
			current.error = "****** illegal addressing mode";
			return;
		}
		if (operand == "*")
			return;
		if (isnum(operand) && operand.length() <= 4)
			return;
		if (isnum(operand) && operand.length() > 4) {
			current.error = "****** extra characters at end of statement";
			return;
		}
		return;

		///////////////////////////////////////////////////////////////////////
	} else if (operation_table[operation].second.second == "2r") {
		if (format4) {
			current.error = "****** can not be format 4 instrcction";
			return;
		}
		istringstream ss(current.operand);
		string tokenizer;
		vector<string> registers;
		while (getline(ss, tokenizer, ',')) {
			registers.push_back(tokenizer);
		}
		if (registers.size() == 1) {
			current.error = "****** missing comma in operand field";
			return;
		}
		if (registers.size() > 2) {
			current.error = "****** extra characters at end of statement";
			return;
		}
		if (!isRegister(registers[0]) || !isRegister(registers[1]))
			current.error = "****** illegal address for a register";
		////////////////////////////////////////////////////////
	} else if (operation_table[operation].second.second == "r") {
		if (format4) {
			current.error = "****** can not be format 4 instrcction";
			return;
		}
		if (!isRegister(current.operand))
			current.error = "****** illegal address for a register";
		////////////////////////////////////////////////////////////
	} else if (operation_table[operation].second.second == "-") {
		if (index && indirect) {
			current.error = "****** illegal addressing mode";
			return;
		}
		if ((isnum(operand) && operand.length() > 4) || operand.length() > 8) {
			current.error = "****** extra characters at end of statement";
			return;
		}
		if (isdigit(operand[0]) && !isnum(operand)) {
			current.error = "****** extra characters at end of statement";
			return;
		}
		////////////////////////////////////////////////////////////////
	} else if (operation_table[operation].second.second == "rn") {
		if (format4) {
			current.error = "****** can not be format 4 instrcction";
			return;
		}
		if (index || indirect) {
			current.error = "****** illegal addressing mode";
			return;
		}
		istringstream ss(current.operand);
		string tokenizer;
		vector<string> token;
		while (getline(ss, tokenizer, ',')) {
			token.push_back(tokenizer);
		}
		if (token.size() == 1) {
			current.error = "****** missing comma in operand field";
		} else if (token.size() > 2) {
			current.error = "****** extra characters at end of statement";
		} else {
			if (!isRegister(token[0]) || !isnum(token[1])) {
				current.error = "****** illegal operand";
			}
		}
	}
	//////////////////////////////////////////////////////////
}

int tonum(string s) {
	int res = 0;
	for (int i = 0; i < (int) s.length(); i++) {
		res *= 10;
		res += s[i] - '0';
	}
	return res;
}

long long last_pc = -1;
void output(bool &started, bool &endf, long long &lc, instruction &current) {

	if (current.comment_only || current.error != "")
		return;

	if (is_directive(current.operation) && current.operation != "CSECT") {
		current.pc_counter = lc;
	}

	if (current.error == "") {

		if (current.operation == "START" && !started) {
			lc = hextodec(current.operand);
			current.pc_counter = lc;
		} else if (current.operation == "START" && started) { //start not in the correct place
			current.error += " ****** duplicate or misplaced START statement ";
			current.pc_counter = lc;
		} else if (current.operation == "END" && !endf) {
			current.pc_counter = lc;
			endf = true;
		} else if (current.operation == "END" && endf) { // end not in the correct place
			current.error +=
					"\n****** statement should not follow END statement";
			current.pc_counter = lc;

		} else if (current.operation == "RESW") {
			current.pc_counter = lc;
			if (lc > 1048575) {
				current.error += "****** address out of range";
			}
			lc += testEQU(current).first * 3;
		} else if (current.operation == "RESB") {
			current.pc_counter = lc;
			if (lc > 1048575) {
				current.error += "****** address out of range";
			}
			lc += testEQU(current).first;
		} else if (current.operation == "WORD") {
			current.pc_counter = lc;
			if (lc > 1048575) {
				current.error += "****** address out of range";
			}
			if (current.operand.find(",") != std::string::npos) {
				lc += (count(current.operand.begin(), current.operand.end(),
						',') + 1) * 3;
			} else {
				lc += 3;
			}
		} else if (current.operation == "BYTE") {
			current.pc_counter = lc;
			if (lc > 1048575) {
				current.error += "****** address out of range";
			}
			if (current.operand[0] == 'C') {
				lc +=
						current.operand.substr(2, current.operand.length() - 3).length();
			} else if (current.operand[0] == 'X') {
				lc +=
						current.operand.substr(2, current.operand.length() - 3).length()
								/ 2;
			}
		} else if (current.operation[0] != '+'
				&& operation_table[current.operation].first == 2) {
			current.pc_counter = lc;
			if (lc > 1048575) {
				current.error += "****** address out of range";
			}
			lc += 2;
		} else if (current.operation[0] != '+'
				&& operation_table[current.operation].first == 3) {
			current.pc_counter = lc;
			if (lc > 1048575) {
				current.error += "****** address out of range";
			}
			lc += 3;
		} else if (current.operation[0] == '+') {
			current.pc_counter = lc;
			if (lc > 1048575) {
				current.error += "****** address out of range";
			}
			lc += 4;

		} else if (current.operation == "ORG") {
			if (current.operand != "*") {
				last_pc = current.pc_counter;
				if (current.operand != "") {
					pair<int, bool> p = testEQU(current);
					if (p.first > (1 << 25))
						current.error =
								"****** illegal address expression in operand field";
					else if (!p.second)
						current.error =
								"****** address expression is not relocatable";
					else {
						current.pc_counter = p.first;
						if (last_pc == -1)
							last_pc = lc;
						lc = p.first;
					}
				} else {
					lc = last_pc;
					last_pc = -1;
				}
			}
		} else if (current.operation == "EQU") {
			section * sec = getsection();
			if (current.operand == "*")
				sec->memory.insert( { current.label,
						{ current.pc_counter, true } });
			else {
				pair<int, bool> p = testEQU(current);
				if (p.first > (1 << 25))
					current.error =
							"****** illegal address expression in operand field";
				else
					sec->memory.insert( { current.label, { (long long) p.first,
							p.second } });
			}
		}
	} else {
		current.pc_counter = lc;
	}
	started = true;
	if (current.label != "" && current.operation != "CSECT"
			&& current.operation != "EQU") {
		section* sec = getsection();
		sec->memory[current.label].first = current.pc_counter;
	}
}

void print_SYMTAP(string &intermediate_file, bool &error) {

	for (section sec : sections) {
		string name = sec.name;
		if (name == "") {
			name = "default";
		}
		intermediate_file +=
				">>    s y m b o l     t a b l e (values in decimal) " + name
						+ " Section" + "\n\n";
		intermediate_file +=
				"        name         value     Absol/Reloc\n        ----------------------------------\n";
		stringstream ss;
		for (auto& mem : sec.memory) {
			ss << "        " << std::setw(10) << left << mem.first
					<< std::setw(8) << right << mem.second.first << "     ";
			if (mem.second.second) {
				ss << "Relocatable \n";
			} else {
				ss << "Absolute \n";
			}
		}
		intermediate_file += ss.str() + "\n\n";
	}
}

int pointer = 0;

string getLitVal(string literal) {
	string res = "";
	//////////////cout << literal[0] << endl;
	if (literal[0] == 'C') {
		for (int i = 2; literal[i] != '\''; i++)
			res += dectohex((int) literal[i]);
	} else if (literal[0] == 'X') {
		res = literal.substr(2, literal.size() - 3);
	} else {
		long long val = 0;
		for (int i = 2; literal[i] != '\''; i++)
			val = val * 10 + (literal[i] - '0');
		res = dectohex(val);
	}
	return res;
}

bool isInLITTAB(string s) {
	for (int i = 0, n = LITTAB.size(); i < n; i++) {
		// maged
		////cout << s << endl;
		if (getLitVal(s) == LITTAB[i].second.second.second)
			return true;
	}
	return false;
}

void pass1LitHandle(instruction& current, long long& litPool) {
	if (current.comment_only || current.error != "")
		return;
	if (current.operand != "" && current.operand[0] == '=') {
		if (!isInLITTAB(current.operand.substr(1)))
			if (current.operand[1] == 'C') {
				string s = "";
				for (int i = 3; current.operand[i] != '\''; i++)
					s += dectohex((int) current.operand[i]);

				LITTAB.push_back(
						make_pair(current.operand.substr(1),
								make_pair(current.operand.size() - 4,
										make_pair(0, s))));
			} else if (current.operand[1] == 'X') {
				LITTAB.push_back(
						make_pair(current.operand.substr(1),
								make_pair((current.operand.size() - 4) / 2,
										make_pair(0,
												current.operand.substr(3,
														current.operand.size()
																- 4)))));
			} else {
				long long val = 0;
				for (int i = 3; current.operand[i] != '\''; i++)
					val = val * 10 + (current.operand[i] - '0');
				LITTAB.push_back(
						make_pair(current.operand.substr(1),
								make_pair(3, make_pair(0, dectohex(val)))));
			}
	}
	if (current.operation == "LTORG" || current.operation == "END") {
		for (; pointer < (int) LITTAB.size(); pointer++) {
			LITTAB[pointer].second.second.first = litPool;
			litPool += LITTAB[pointer].second.first;
			current.litpool.push_back(LITTAB[pointer].first); // <-- edit
		}
		if(current.operation == "END"){
			temp = current.litpool;
		}
	}
}

bool pass1(string file_name) {
	ifstream stream(file_name);
	string line;
	long long pc = 0;
	bool started = false;
	bool ended = false;
	bool error = false;
	bool flag = false;
	intermediate_file =
			">>  Source Program statements with value of LC indicated \n\n";

	while (getline(stream, line)) {
		instruction current = instruction("", "");

		current = read_input_free(line);

		validation_directive(current);
		if (current.error == "" && current.operation == "CSECT") {
			current.pc_counter = pc;
			pc = 0;
		}
		validation_phase1(current);
		validation_phase2(current);

		output(started, ended, pc, current);

		pass1LitHandle(current, pc);
		print_prgram(current, intermediate_file, flag);

		if (current.error != "")
			error = true;
	}

	stream.close();
	if (!ended) {
		intermediate_file += "****** this statement can not have a label\n";
		error = true;
	}
	intermediate_file +=
			"\n>>    e n d    o f   p a s s   1 \n\n>>   *****************************************************\n";

	print_SYMTAP(intermediate_file, flag);
	ofstream out("LISTFILE.txt");
	out << intermediate_file;
	out.close();

	return error;
}

int main(int argc, char* argv[]) {
	load_file();
	bool error = pass1("prog.txt");
	if (!error) {
		Pass2 p(operation_table, directive, sections, intermediate_file,
				LITTAB);
		p.overLit = temp;
		error  = p.pass2_logic();
		if(error)
			return 1;
		return 0;
	}
	return 1;
}
