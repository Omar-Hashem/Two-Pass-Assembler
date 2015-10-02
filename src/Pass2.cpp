/*
 * Pass2.cpp
 *
 *  Created on: Apr 30, 2015
 *      Author: magedmilad
 */

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
#include "Pass2.h"

namespace std {

//struct instruction {
//
//	string label;
//	string operation;
//	string operand;
//	string error;
//	string comment;
//	long long pc_counter = -1;
//	bool comment_only;
//	string registers = "000000";
//	bool relocatable = -1;
//	string object_code = "";
//
//	instruction(string l, string op, string oper, string comm, string err) {
//		label = l;
//		operation = op;
//
//		operand = oper;
//		comment = comm;
//		error = err;
//		pc_counter = -1;
//		comment_only = false;
//		relocatable = -1;
//		registers = "000000";
//		string object_code = "";
//	}
//
//	instruction(string e, string com) {
//		label = "";
//		operation = "";
//		operand = "";
//		error = e;
//		comment = com;
//		if (comment.compare("") != 0)
//			comment_only = true;
//		else
//			comment_only = false;
//		pc_counter = -1;
//		relocatable = -1;
//		registers = "000000";
//		string object_code = "";
//	}
//};

//map<string, pair<int, pair<string, string> > > operation_table;
//unordered_set<string> directive;
//vector<section> sections;
//string current_section = "";

Pass2::Pass2(map<string, pair<int, pair<string, string> > > operation_table,
		unordered_set<string> directive, vector<section> sections,
		string intermediate_file,
		vector<pair<string, pair<long long, pair<long long, string> > > > LITTAB) {
	this->current_section = "";
	this->operation_table = operation_table;
	this->directive = directive;
	this->sections = sections;
	this->intermediate_file = intermediate_file;
	this->LITTAB = LITTAB;
	tr.push_back("");
}

Pass2::~Pass2() {
}

long long hextodec(string hexa) {
	long long ret;
	stringstream ss;
	ss << std::hex << hexa;
	ss >> ret;
	// output it as a signed type
	return static_cast<int>(ret);
}

bool Pass2::is_directive(string s) {

	if (directive.find(s) != directive.end()) {
		return true;
	}
	return false;

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

section* Pass2::getsection() {
	for (int i = 0; i < (int) sections.size(); i++) {
		if (sections[i].name == current_section) {
			section* res = &sections[i];
			return res;
		}
	}
	section* res = &sections[0];
	return res;
}

bool Pass2::isSection(string name) {
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

int get_register_number(string r) {
	if (r == "A")
		return 0;
	if (r == "X")
		return 1;
	if (r == "L")
		return 2;
	if (r == "B")
		return 3;
	if (r == "S")
		return 4;
	if (r == "T")
		return 5;
	if (r == "F")
		return 6;
	if (r == "PC")
		return 8;
	if (r == "SW")
		return 9;
}

//bool valid(string s) {
//	regex em("\\s*");
//	if (s == "" || regex_match(s, em)) {
//		return false;
//	}
//
//	return true;
//}

int tonum(string s) {
	int res = 0;
	for (int i = 0; i < (int) s.length(); i++) {
		res *= 10;
		res += s[i] - '0';
	}
	return res;
}

string repeate(int cnt, string item) {
	string ret = "";
	for (int i = 0; i < cnt; i++) {
		ret += item;
	}
	return ret;
}

string trim(string in) {
	in.erase(in.begin(),
			find_if(in.begin(), in.end(), bind1st(not_equal_to<char>(), ' ')));
	in.erase(
			find_if(in.rbegin(), in.rend(), bind1st(not_equal_to<char>(), ' ')).base(),
			in.end());
	return in;
}

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

void Pass2::validExpression(instruction& current) {
	bool format4 = false;
	if (current.operation[0] == '+') {
		format4 = true;
	}
	if (!format4) {
		bool ok = true;
		section* sec = getsection();
		vector<string> vars = getLabels(current.operand);
		for (int i = 0; i < (int) vars.size(); i++) {
			if (sec->memory.find(vars[i]) == sec->memory.end()
					&& sec->ref.find(vars[i]) != sec->ref.end()) {
				ok = false;
				break;
			}
		}
		if (!ok) {
			current.error = "****** external reference must be format 4";
			return;
		}
	}
	return;
}

//string trim(string in) {
//	in.erase(in.begin(),
//			find_if(in.begin(), in.end(), bind1st(not_equal_to<char>(), ' ')));
//	in.erase(
//			find_if(in.rbegin(), in.rend(), bind1st(not_equal_to<char>(), ' ')).base(),
//			in.end());
//	return in;
//}

bool ended = false;

string Pass2::print_instruction(instruction current) {
	stringstream out;
	string tmp = dectohex(current.pc_counter);
	transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
	if (current.comment_only || current.operation.compare("") == 0) {
		if (current.comment != "") {
			string tmp1 = repeate(6, " ");
			out << tmp1 << current.comment
					<< repeate(66 - current.comment.size(), " ") << "\n";
		}
		if (current.error != "") {
			out << current.error << repeate(72 - current.error.size(), " ")
					<< "\n";
		}
//		error = true;
		return out.str();
	}
	string tmp1 = repeate(6 - tmp.length(), "0") + tmp;
	if (current.pc_counter == -1) {
		tmp1 = repeate(6, " ");
	}
	string tmp2 = current.label + repeate(8 - current.label.length(), " ");
	string tmp3 = current.operation
			+ repeate(7 - current.operation.length(), " ");
//			//cout << current.operand  << endl;
	string tmp4 = current.operand + repeate(18 - current.operand.length(), " ");
	string tmp5 = current.comment + repeate(30 - current.comment.length(), " ");
	string tmp6;
	bool large = false;
	//cout << current.object_code << current.object_code.size() << endl;
	if (current.object_code.compare("") == 0) {
		tmp6 = repeate(6, " ");
	} else {
		if (current.object_code[current.object_code.size() - 1] == '\n')
			current.object_code = current.object_code.substr(0,
					current.object_code.size() - 1);
		string toup = current.object_code;
		transform(toup.begin(), toup.end(), toup.begin(), ::toupper);
		current.object_code = toup;
		if (current.object_code.size() > 6) {
			large = true;
			tmp6 = repeate(6, " ");
		} else {
			tmp6 = current.object_code
					+ repeate(6 - current.object_code.length(), " ");
		}
	}

	//error = false;
//        //cout << current.operand << endl;
	bool flags = true;
	if (tmp2.size() >= 1 && tmp2[0] == '*') {
		flags = false;
		int len = 0;
		for (auto s : LITTAB) {
//            //cout << s.first <<" "<< tmp3 <<" "<< (s.first == tmp3)<< endl;
			if (s.first == current.operation) {
				len = s.second.first * 2;
				break;
			}
		}
		//cout << len << " " << current.operand.size() << " " << current.operand
//				<< endl;
		tmp4 = repeate(len - current.operand.size(), "0") + current.operand
				+ repeate(6 - max(len, (int) current.operand.size()), " ");
		transform(tmp4.begin(), tmp4.end(), tmp4.begin(), ::toupper);
		if (tmp4.size() > 6) {
			current.object_code = tmp4;
			tmp4 = repeate(6, " ");
			large = true;
		}
		out << tmp1 << " " << tmp4 << " " << tmp2 << tmp3 << "\n";
//        out << tmp1  << " " << tmp6 << " " << tmp2 << tmp3 <<  tmp4 << tmp5 << "\n";

	} else {
		out << tmp1 << " " << tmp6 << " " << tmp2 << tmp3 << tmp4 << tmp5
				<< "\n";
	}

	if (current.error.compare("") != 0) {
		out << current.error << repeate(72 - current.error.size(), " ") << "\n";
		//error = true;
	}

	if (large) {
		out << "object code : " << current.object_code
				<< repeate(30 - current.object_code.length(), " ") << " ";
	} else {
		out << repeate(45, " ");
	}

	if (current.registers.compare("-1") != 0
			&& current.registers.compare("000000") != 0 && flags) {
		out << (current.relocatable ? "RELOC" : "ABS  ") << " n="
				<< current.registers[0] << " i=" << current.registers[1]
				<< " x=" << current.registers[2] << " b="
				<< current.registers[3] << " p=" << current.registers[4]
				<< " e=" << current.registers[5];
	}

	out << "\n\n";
	return out.str();
}

bool valid(string s) {
	regex em("\\s*");
	if (s == "" || regex_match(s, em)) {
		return false;
	}
	return true;
}

instruction Pass2::read_input(string line) {
	instruction current = instruction("", "");
	if (!valid(line)) {
		return instruction("", " ");
	}

	if (line == ">>    e n d    o f   p a s s   1 ") {
		ended = true;
	}
	if (ended) {
		return instruction("", "");
	}

	for (int i = line.size(); i < 70; i++) {
		line += " ";
	}
	if (line[6] == '.' || line[0] == '>') {
		return instruction("", line);
	}
	string error = "";

	string locctr = line.substr(0, 6);

	string label = line.substr(7, 8);
	string mnemonic = "";
	string operand = "";
	int last = 0;
	if (label[0] == '*') {
		string temp = line.substr(18);
		int lastquote = temp.find("'");
		if (lastquote != string::npos) {
			mnemonic = line.substr(16, lastquote + 3);
//            //cout << mnemonic << endl;
			operand = "";
			for (auto s : LITTAB) {
//                    //cout <<  mnemonic << " "<< s.first << endl;
				if (s.first == mnemonic) {
					operand = s.second.second.second;
//                    //cout << operand << endl;
					break;
				}
			}
//            //cout << operand << endl;
			last = lastquote + 19;
		} else {
			mnemonic = line.substr(15, 7);
			last = 22;
			operand = line.substr(last, 20);
		}
	} else {
		mnemonic = line.substr(15, 7);
		last = 22;
		operand = line.substr(last, 20);
	}

	string comment = line.substr(last + 20);
	label = trim(label);
	mnemonic = trim(mnemonic);
	operand = trim(operand);
	comment = trim(comment);
	current = instruction(label, mnemonic, operand, comment, "");
	istringstream iss(locctr);
	iss >> hex >> current.pc_counter;
////cout << current.pc_counter << current.label << " " << current.operation<< " " << current.operand<< " " << current.comment<< " " << current.error<<(current.comment_only?" comment only":"") <<endl;
	return current;
}

int get_reg_val(string r) {
	int res = 0;
	int base = 1;
	for (int i = r.size() - 1; i >= 2; i--) {
		if (r[i] == '1')
			res += base;
		base *= 2;
	}
	return res;
}

bool Pass2::isInLITTAB(string s) {
	for (int i = 0, n = LITTAB.size(); i < n; i++)
		if (s == LITTAB[i].first)
			return true;
	return false;
}

int Pass2::LITTAB_IDX(string s) {
	for (int i = 0, n = LITTAB.size(); i < n; i++)
		if (s == LITTAB[i].first)
			return i;
	return -1;
}

pair<vector<string>, pair<int, int> > Pass2::testExpression(instruction &inst) /// takes string expression returns vector, value and boolean (REL/ABS)
		{
	int countBracket = 0;
	bool indexed = false;
	bool immediate = false;
	string qwertyuiop = inst.operand;
	vector<string> qwertyu, g;

	if (qwertyuiop.size() == 1 && qwertyuiop == "*") {
//        cout << "STAR" << endl;
		return make_pair(g, make_pair(inst.pc_counter, 1));          //return PC counter  STAR
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
		return make_pair(g, make_pair((1 << 30), 0));
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
				return make_pair(g, make_pair((1 << 30), 0));
			}
		} else if (qwertyuiop[i] == '-') {
			if (i + 1 < qwertyuiop.size()
					&& (qwertyuiop[i + 1] == '*' || qwertyuiop[i + 1] == '+'
							|| qwertyuiop[i + 1] == '/')) {
				inst.error = " ******  illegal operad";
				return make_pair(g, make_pair((1 << 30), 0));
			}

			if (i + 1 < qwertyuiop.size() && (qwertyuiop[i + 1] == '-')) {
				if ((i + 2 < qwertyuiop.size())
						&& (qwertyuiop[i + 2] == '-' || qwertyuiop[i + 2] == '*'
								|| qwertyuiop[i + 2] == '+'
								|| qwertyuiop[i + 2] == '/')) {
					inst.error = " ******  illegal operad";
					return make_pair(g, make_pair((1 << 30), 0));
				}
				++i;
				asdfghjkl = '+';
			}
		} else if (qwertyuiop[i] == '*') {
			if (i + 1 < qwertyuiop.size()
					&& (qwertyuiop[i + 1] == '*' || qwertyuiop[i + 1] == '+'
							|| qwertyuiop[i + 1] == '/')) {
				inst.error = " ******  illegal operad";
				return make_pair(g, make_pair((1 << 30), 0));
			}
			if (i + 1 < qwertyuiop.size() && qwertyuiop[i + 1] == '-') {
				if ((i + 2 < qwertyuiop.size())
						&& (qwertyuiop[i + 2] == '-' || qwertyuiop[i + 2] == '*'
								|| qwertyuiop[i + 2] == '+'
								|| qwertyuiop[i + 2] == '/')) {
					inst.error = " ******  illegal operad";
					return make_pair(g, make_pair((1 << 30), 0));
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
				return make_pair(g, make_pair((1 << 30), 0));
			}
			if (i + 1 < qwertyuiop.size() && qwertyuiop[i + 1] == '-') {
				if ((i + 2 < qwertyuiop.size())
						&& (qwertyuiop[i + 2] == '-' || qwertyuiop[i + 2] == '*'
								|| qwertyuiop[i + 2] == '+'
								|| qwertyuiop[i + 2] == '/')) {
					inst.error = " ******  illegal operad";
					return make_pair(g, make_pair((1 << 30), 0));
				}
				qwertyu.push_back("/");
				asdfghjkl = '_';
				++i;
			}
		} else {
			inst.error = " ******  illegal operad";
			return make_pair(g, make_pair((1 << 30), 0));
		}
		if (countBracket < 0) {
			inst.error = " ******  illegal operad";
			return make_pair(g, make_pair((1 << 30), 0));
		}
		qwertyu.push_back(asdfghjkl);
	}

	if (countBracket) {
		inst.error = " ******  illegal operad";
		return make_pair(g, make_pair((1 << 30), 0));
	}

	vector<string> extRef;
	section* sec = getsection();
	for (int i = 0; i < qwertyu.size(); ++i)
		if (isalpha(qwertyu[i][0])
				&& sec->ref.find(qwertyu[i]) != sec->ref.end()) {
			string b = "";
			if (i && qwertyu[i - 1] == "-")
				b += "-";

			else
				b += "+";

			b += qwertyu[i];
			extRef.push_back(b);
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
			return make_pair(g, make_pair((1 << 30), 0));
		}
		if (isalpha(qwertyu[j][0])) {
			///if( found in symbol table || found in extref )
			///     push_back(qwertyu[j]);

			///else
			///     inst.error = " ****** undefined symbol";          /// name the error
			///     return make_pair(g , make_pair(1 << 30 , 0));

			section* sec = getsection();
			if (sec->memory.find(qwertyu[j]) == sec->memory.end()
					&& sec->ref.find(qwertyu[j]) == sec->ref.end()) {
				inst.error = " ****** undefined symbol";
				return make_pair(g, make_pair(1 << 30, 0));
			} else {
				postfix.push_back(qwertyu[j]);
			}
		} else if (isdigit(qwertyu[j][0])) {
			if (qwertyu[j].size() > 1048575)       ///number of available digits
					{
				inst.error = " ****** Address out of range";
				return make_pair(g, make_pair((1 << 30), 0));
			}
			postfix.push_back(qwertyu[j]);
		} else if (qwertyu[j] == "_")
			stackk.push(qwertyu[j]);

		else if (qwertyu[j] == "+" || qwertyu[j] == "-") {
			if (qwertyu[j - 1] == "(" || (j + 2 > qwertyu.size())
					|| qwertyu[j + 1] == ")") {
				inst.error = " ******  illegal operad";
				return make_pair(g, make_pair((1 << 30), 0));
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
				return make_pair(g, make_pair((1 << 30), 0));
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
			///if( found in symbol table )
			///     then push_back(value , type)

			///else
			///     then push_back(0 , type)             /// I've checked its existance
			section* sec = getsection();
			if (sec->memory.find(postfix[j]) != sec->memory.end()) {
				value = sec->memory[postfix[j]].first;
				stackkk.push(make_pair(value, sec->memory[postfix[j]].second));
			} else {
				value = 0;
				stackkk.push(make_pair(value, 0));
			}
		}

		else if (postfix[j] == "_") {
			pair<int, int> value = stackkk.top();
			stackkk.pop();
			value.first = -value.first;
			stackkk.push(value);
		} else {
			if (stackkk.size() < 2) {
				inst.error = " ******  illegal operad";
				return make_pair(g, make_pair((1 << 30), 0));
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
					return make_pair(g, make_pair((1 << 30), 0));
				}
				value[3].second = value[0].second + value[1].second;
			}

			else if (postfix[j] == "*") {
				value[3].first = value[0].first * value[1].first;
				if (value[0].second + value[1].second) {
					inst.error = " ****** relative multiple relative"; ///name the error
					return make_pair(g, make_pair((1 << 30), 0));
				}
				value[3].second = value[0].second + value[1].second;
			}

			else if (postfix[j] == "/") {
				if (value[1].first == 0) {
					inst.error = " ****** DIVISION BY ZERO";
					return make_pair(g, make_pair((1 << 30), 0));
				}
				value[3].first = value[0].first / value[1].first;
				if (value[0].second + value[1].second) {
					inst.error = " ****** relative over relative"; ///name the error
					return make_pair(g, make_pair((1 << 30), 0));
				}
				value[3].second = value[0].second + value[1].second;
			} else {
				inst.error = " ****** illegal operand";
				return make_pair(g, make_pair((1 << 30), 0));
			}
			stackkk.push(value[3]);
		}
	}
	if (stackkk.size() != 1) {
		inst.error = " ****** illegal operand";
		return make_pair(g, make_pair((1 << 30), 0));
	}
	pair<vector<string>, pair<int, int> > temp;
	temp.second = stackkk.top();
	temp.second.first = temp.second.first * sign;
	temp.first = extRef;
	return temp;
}

void Pass2::set_registers(instruction& current) {
	if (current.comment_only || current.error != "") {
		current.registers = "-1";
		return;
	}
	///////////////////////////////////////////////////////

	if (current.operation == "CSECT") {
		current_section = current.label;
	}

	if (is_directive(current.operation) || is_start(current.operation)
			|| is_end(current.operation))
		return;
	///////////////////////////////////////////////////////

	string operand = current.operand;
	string operation = current.operation;
	bool set = false;
	current.relocatable = true;
	if (current.operation[0] == '+') {
		operation = current.operation.substr(1, current.operation.length() - 1);
		//current.relocatable = false;
		current.registers[5] = '1';
		current.registers[0] = '1';
		current.registers[1] = '1';
		set = true;
	}
	if (current.operand[0] == '#') {
		operand = operand.substr(1, operand.length() - 1);
//		current.relocatable = false;
		set = true;
		current.registers[0] = '0';
		current.registers[1] = '1';
	}
	if (current.operand[0] == '@') {
		operand = operand.substr(1, operand.length() - 1);
		//current.relocatable = true;
		set = true;
		current.registers[0] = '1';
		current.registers[1] = '0';
		current.registers[4] = '1';
	}
	if (operand.length() > 2) {
		string suffex = operand.substr(operand.length() - 2, 2);
		if (suffex == ",X") {
			operand = operand.substr(0, operand.length() - 2);
			//current.relocatable = true;
			set = true;
			current.registers[2] = '1';
			current.registers[0] = '1';
			current.registers[1] = '1';
			current.registers[4] = '1';
		}

	}
	if (current.relocatable == -1) {
		current.relocatable = true;
	}
	////////////////////////////////////////////////////////////////
	if (!set) {
		current.registers[0] = '1';
		current.registers[1] = '1';
		current.registers[4] = '1';
		current.relocatable = true;
		set = true;
	}
	////////////////////////////////////////////////////////////////
	if (var(current.operation)) {
		current.registers = "-1";
	}
	/////////////////////////////////////////////////////////////////
	else if (con(current.operation)) {
		current.registers = "-1";
		if (current.operation == "BYTE") {
			current.registers = "-1";
			if (current.operand[0] == 'X') {
				current.object_code = current.operand.substr(2,
						current.operand.length() - 3);
			} else if (current.operand[0] == 'C') {
				stringstream ss;
				string operand = current.operand.substr(2,
						current.operand.length() - 3);
				for (int i = 0; i < (int) operand.size(); i++) {
					int c = operand[i];
					ss << hex << c;
				}
				current.object_code = ss.str();
				////cout <<ss.str() <<"------" << endl;
			}
		} else {
			// may get wrong
			current.registers = "-1";
			istringstream ss(current.operand);
			string tokenizer;
			vector<string> arr;
			while (getline(ss, tokenizer, ',')) {
				arr.push_back(tokenizer);
			}
			stringstream sss;
//			for (int i = 0; i < (int) arr.size(); i++) {
//				int num = 0;
//				for (int j = 0; j < (int) arr[i].size(); j++) {
//					num *= 10;
//					num += arr[i][j] - '0';
//				}
//				string val = dectohex(num);
//				string res = repeate(6 - val.length(), "0") + val;
//				sss << res;
//				sss << "\n";
//			}
			if (arr.size() == 1) {
				pair<int, int> res = testExpression(current).second;
//				if (res.second) {
//					current.error = "****** can't be relative expression";
//				}
				if (current.error == "") {
					current.extref = testExpression(current).first;
					current.relocatable = res.second;
					int num = res.first;
					string val = dectohex(num);
					string res = repeate(6 - val.length(), "0") + val;
					sss << res;

				}
			} else {
				for (int i = 0; i < (int) arr.size(); i++) {
					int num = 0;
					for (int j = 0; j < (int) arr[i].size(); j++) {
						num *= 10;
						num += arr[i][j] - '0';
					}
					string val = dectohex(num);
					string res = repeate(6 - val.length(), "0") + val;
					sss << res;
					sss << "\n";
				}
			}
			current.object_code = sss.str();
		}
		////////////////////////////////////////////////////////////////

	} else if (operation_table[operation].second.second == "m") {
		section* sec = getsection();

		if (current.registers[0] == '0') { // #
			stringstream ss;
			string temp = operation_table[operation].second.first;
			long long opcode = hextodec(temp);
			opcode += 1;
			string operand = current.operand.substr(1,
					current.operand.length() - 1);
			if (isnum(
					current.operand.substr(1, current.operand.length() - 1))) { // # number
				int num = tonum(operand);
				if (opcode <= 15) {
					ss << "0";
				}
				ss << hex << opcode;
				ss << hex << get_reg_val(current.registers);
				if (ss.str().size() == 2)
					ss << "0";
				ss << repeate(3 - operand.size(), "0") << num;
				current.object_code = ss.str();
				current.relocatable = false;
			} else {
//				if (sec->memory.find(operand) == sec->memory.end()) { // # var todo may wrong
//					current.error = "****** undefined symbol in operand";
//					return;
//				}
				//todo
				pair<int, int> ret = testExpression(current).second;

				validExpression(current);
				if (current.error != "")
					return;

				//
				current.extref = testExpression(current).first;
				//current.relocatable = false;
				current.registers[4] = '1';

//				long long address = sec->memory[operand].first; // todo need edit by osama
				long long address = ret.first;
				long long pc = current.pc_counter;
				long long res = address - pc;
				if (current.registers[5] == '1') { // # var with format 4
					res -= 4;
					current.relocatable = ret.second;
					if (address > 1048575) {
						current.error = "****** address out of range";
						return;
					}
					if (opcode <= 15) {
						ss << "0";
					}
					ss << hex << opcode;
					ss << hex << get_reg_val(current.registers);
					if (ss.str().size() < 3) {
						ss << "0";
					}
					string disp = dectohex(address);
					if (disp.size() == 1) {
						ss << "0000" << disp;
					} else if (disp.size() == 2) {
						ss << "000" << disp;
					} else if (disp.size() == 3) {
						ss << "00" << disp;
					} else if (disp.size() == 4) {
						ss << "0" << disp;
					} else {
						ss << disp.substr(disp.size() - 5, 5);
					}
					current.object_code = ss.str();

				} else { // # var with format 3
					res -= 3;
					if (res > 2047 || res < -2048) {
						current.error = "****** address out of range";
						return;
					}
					if (opcode <= 15) {
						ss << "0";
					}
					ss << hex << opcode;
					ss << hex << get_reg_val(current.registers);
					if (ss.str().size() < 3) {
						ss << "0";
					}
					current.relocatable = ret.second;
					string disp = dectohex(res);
					if (disp.size() == 1) {
						ss << "00" << disp;
					} else if (disp.size() == 2) {
						ss << "0" << disp;
					} else {
						ss << disp.substr(disp.size() - 3, 3);
					}
					current.object_code = ss.str();
				}

			}
		} else {
			if (current.registers[1] == '0') { // @
				//i don't know what to do
				if (current.registers[5] == '0') { // @ with format 3
					stringstream ss;
					string temp = operation_table[operation].second.first;
					long long opcode = hextodec(temp);
					opcode += 2;

					pair<int, int> ret = testExpression(current).second;

					validExpression(current);
					if (current.error != "")
						return;
					current.extref = testExpression(current).first;
					current.relocatable = ret.second;
//					long long address = sec->memory[operand].first;
					long long address = ret.first;
					long long pc = current.pc_counter + 3;
					long long res = address - pc;
					if (res > 2047 || res < -2048) {
						current.error = "****** address out of range";
						return;
					}
					if (opcode <= 15) {
						ss << "0";
					}
					ss << hex << opcode;
					ss << hex << get_reg_val(current.registers);
					if (ss.str().size() < 3) {
						ss << "0";
					}

					string disp = dectohex(res);
					if (disp.size() == 1) {
						ss << "00" << disp;
					} else if (disp.size() == 2) {
						ss << "0" << disp;
					} else {
						ss << disp.substr(disp.size() - 3, 3);
					}
					current.object_code = ss.str();
				} else { // @ with format 4
					stringstream ss;
					string temp = operation_table[operation].second.first;
					long long opcode = hextodec(temp);
					opcode += 2;

					pair<int, int> ret = testExpression(current).second;

					validExpression(current);
					if (current.error != "")
						return;
					current.extref = testExpression(current).first;
					current.relocatable = ret.second;
					long long pc = current.pc_counter + 4;
					long long res = ret.first - pc;
					if (ret.first > 1048575) {
						current.error = "****** address out of range";
						return;
					}
					if (opcode <= 15) {
						ss << "0";
					}
					ss << hex << opcode;
					ss << hex << get_reg_val(current.registers);
					if (ss.str().size() < 3) {
						ss << "0";
					}

					string disp = dectohex(ret.first);
					if (disp.size() == 1) {
						ss << "0000" << disp;
					} else if (disp.size() == 2) {
						ss << "000" << disp;
					} else if (disp.size() == 3) {
						ss << "00" << disp;
					} else if (disp.size() == 4) {
						ss << "0" << disp;
					} else {
						ss << disp.substr(disp.size() - 5, 5);
					}
					current.object_code = ss.str();
				}
			} else {
				if (current.operand[0] == '=') { // literal
					if (!isInLITTAB(current.operand.substr(1))) {
						current.error = "****** undefined symbol in operand";
						return;
					}
					stringstream ss;
					string temp = operation_table[operation].second.first;
					long long opcode = hextodec(temp);
					opcode += 3;

					long long address = LITTAB[LITTAB_IDX(
							current.operand.substr(1))].second.second.first;
					long long pc = current.pc_counter + 3;
					if (current.registers[5] == '1') { // todo // literal with format 4
						pc++;
						long long res = address - pc;
						if (address > 1048575) {
							current.error = "****** address out of range";
							return;
						}
						if (opcode <= 15) {
							ss << "0";
						}
						ss << hex << opcode;
						ss << hex << get_reg_val(current.registers);
						if (ss.str().size() < 3) {
							ss << "0";
						}

						string disp = dectohex(address);
						if (disp.size() == 1) {
							ss << "0000" << disp;
						} else if (disp.size() == 2) {
							ss << "000" << disp;
						} else if (disp.size() == 3) {
							ss << "00" << disp;
						} else if (disp.size() == 4) {
							ss << "0" << disp;
						} else {
							ss << disp.substr(disp.size() - 5, 5);
						}
					} else { // literal with format 3
						long long res = address - pc;
//					////cout <<address;
						if (res > 2047 || res < -2048) {
							current.error = "****** address out of range";
							return;
						}
						if (opcode <= 15) {
							ss << "0";
						}
						ss << hex << opcode;
						ss << hex << get_reg_val(current.registers);
						if (ss.str().size() < 3) {
							ss << "0";
						}

						string disp = dectohex(res);
						if (disp.size() == 1) {
							ss << "00" << disp;
						} else if (disp.size() == 2) {
							ss << "0" << disp;
						} else {
							ss << disp.substr(disp.size() - 3, 3);
						}
					}
					current.object_code = ss.str();
				} else {
					if (sec->memory.find(operand) != sec->memory.end()) { // memory
						if (current.registers[5] == '0') { // memory with format 3
							stringstream ss;
							string temp =
									operation_table[operation].second.first;
							long long opcode = hextodec(temp);
							opcode += 3;

							if (operand == "*") {
								ss << hex << opcode;
								ss << hex << get_reg_val(current.registers);
								if (ss.str().size() < 3) {
									ss << "0";
								}
								ss << "000";
								current.object_code = ss.str();
								current.relocatable = true;
								return;
							}

							pair<int, int> ret = testExpression(current).second;

							validExpression(current);
							if (current.error != "")
								return;
							current.extref = testExpression(current).first;
							long long address = ret.first;
							long long pc = current.pc_counter + 3;
							current.relocatable = ret.second;
							long long res = address - pc;
							if (res > 2047 || res < -2048) {
								current.error = "****** address out of range";
								return;
							}
							if (opcode <= 15) {
								ss << "0";
							}
							ss << hex << opcode;
							ss << hex << get_reg_val(current.registers);
							if (ss.str().size() < 3) {
								ss << "0";
							}

							string disp = dectohex(res);
							if (disp.size() == 1) {
								ss << "00" << disp;
							} else if (disp.size() == 2) {
								ss << "0" << disp;
							} else {
								ss << disp.substr(disp.size() - 3, 3);
							}
							current.object_code = ss.str();
						} else { // memory with format 4

							stringstream ss;
							string temp =
									operation_table[operation].second.first;

							pair<int, int> ret = testExpression(current).second;

							validExpression(current);
							if (current.error != "")
								return;
							current.extref = testExpression(current).first;
//							if(current.operand == "RDREC"){
//								cout << "yes"<<endl;
//							}
							long long opcode = hextodec(temp);
							opcode += 3;
							long long pc = current.pc_counter + 4;
							long long res = ret.first - pc;
							current.relocatable = ret.second;
							if (ret.first > 1048575) {
								current.error = "****** address out of range";
								return;
							}
							if (opcode <= 15) {
								ss << "0";
							}
							//current.relocatable = true;
							ss << hex << opcode;
							ss << hex << get_reg_val(current.registers);
							if (ss.str().size() < 3) {
								ss << "0";
							}

							string disp = dectohex(ret.first);
							if (disp.size() == 1) {
								ss << "0000" << disp;
							} else if (disp.size() == 2) {
								ss << "000" << disp;
							} else if (disp.size() == 3) {
								ss << "00" << disp;
							} else if (disp.size() == 4) {
								ss << "0" << disp;
							} else {
								ss << disp.substr(disp.size() - 5, 5);
							}
							current.object_code = ss.str();
						}
					} else if (sec->ref.find(operand) != sec->ref.end()) { // external reference
						stringstream ss;
						string temp = operation_table[operation].second.first;
						long long opcode = hextodec(temp);
						opcode += 3;
						if (current.registers[5] == '0') {
							current.error =
									"****** external reference must be format 4";
							return;
						}
						current.extref = testExpression(current).first;
						current.registers[4] = '0';
						ss << hex << opcode;
						ss << hex << get_reg_val(current.registers);
						ss << "00000";
						current.object_code = ss.str();
						current.relocatable = false;
						current.object_code = ss.str();

					} else {
//						current.error = "****** Undefined symbol in operand";
//						return;
						stringstream ss;
						pair<int, int> ret = testExpression(current).second;

						validExpression(current);
						if (current.error != "")
							return;
						current.extref = testExpression(current).first;
						string temp = operation_table[operation].second.first;

						long long opcode = hextodec(temp);
						opcode += 3;
						long long pc = current.pc_counter + 3;
						if (current.registers[5] == '1') { // expression with format 4
							pc++;
							long long res = ret.first;
//							stringstream ss;
							if (res > 1048575) {
								current.error = "****** address out of range";
								return;
							}
							if (opcode <= 15) {
								ss << "0";
							}
							current.relocatable = ret.second;
							ss << hex << opcode;
							ss << hex << get_reg_val(current.registers);
							if (ss.str().size() < 3) {
								ss << "0";
							}
							current.extref = testExpression(current).first;
							string disp = dectohex(res);
							if (disp.size() == 1) {
								ss << "0000" << disp;
							} else if (disp.size() == 2) {
								ss << "000" << disp;
							} else if (disp.size() == 3) {
								ss << "00" << disp;
							} else if (disp.size() == 4) {
								ss << "0" << disp;
							} else {
								ss << disp.substr(disp.size() - 5, 5);
							}
//							current.object_code = ss.str();
						} else { // expression with format 3
							long long res = ret.first - pc;
							if(current.operand[0] == '*')
								res=0;

							if (res > 2047 || res < -2048) {
								current.error = "****** address out of range";
								return;
							}
							if (opcode <= 15) {
								ss << "0";
							}
							ss << hex << opcode;
							ss << hex << get_reg_val(current.registers);
							if (ss.str().size() < 3) {
								ss << "0";
							}
							current.extref = testExpression(current).first;
							current.relocatable = ret.second;

							string disp = dectohex(res);
							if (disp.size() == 1) {
								ss << "00" << disp;
							} else if (disp.size() == 2) {
								ss << "0" << disp;
							} else {
								ss << disp.substr(disp.size() - 3, 3);
							}
							//current.object_code = ss.str();
						}
						current.object_code = ss.str();
					}
				}
			}
		}
	}
//////////////////////////////////////////////////////////////////////
	else if (operation_table[operation].second.second == "2r") {
		istringstream ss(current.operand);
		string tokenizer;
		vector<string> arr;
		while (getline(ss, tokenizer, ',')) {
			arr.push_back(tokenizer);
		}
		stringstream sss;
		string temp = operation_table[current.operation].second.first;
		long long opcode = hextodec(temp);
		sss << hex << opcode;
		sss << get_register_number(arr[0]) << get_register_number(arr[1]);
		current.object_code = sss.str();
	}
////////////////////////////////////////////////////////////////////////
	else if (operation_table[operation].second.second == "r") {
		stringstream ss;
		string temp = operation_table[current.operation].second.first;
		long long opcode = hextodec(temp);
		ss << hex << opcode;
		ss << get_register_number(current.operand);
		ss << "0";
		current.object_code = ss.str();
	}
////////////////////////////////////////////////////////////////////////
	else if (operation_table[operation].second.second == "-") {
		stringstream ss;
		string temp = operation_table[current.operation].second.first;
		long long opcode = hextodec(temp);
		opcode += 3;
		ss << hex << opcode;
		ss << "0000";
//		ss << hex << get_reg_val(current.registers);
		current.object_code = ss.str();
	}
}

int ind = 0;
bool flagcsec = false;

void Pass2::objectProgram(instruction& current) {
//	cout << current.extref.size() << endl;
//	if(current.extref.size()){
//		cout <<current.label << "  "  <<current.operation << "  " << current.operand << endl;
//	}
	if (current.label == "*") {
		current.object_code = current.operand;
	}
	if (current.error != "" || current.comment_only) {
		return;
	}
	if (current.operation == "START") {
		hr += "H";
		hr += current.label + repeate(6 - current.label.size(), " ");
		hr += repeate(6 - dectohex(current.pc_counter).size(), "0")
				+ dectohex(current.pc_counter);
	} else if (current.operation == "CSECT") {
		flagcsec = true;
		hr += repeate(6 - dectohex(current.pc_counter).size(), "0")
				+ dectohex(current.pc_counter);
		er += "E";
		op += hr + "\n";

		if (dr != "") {
			op += dr + "\n";
		}
		if (rr != "") {
			op += rr + "\n";
		}
		for (int i = 0; i < tr.size(); i++) {
			if (tr[i] != "") {
				string tt = tr[i];
				tr[i] = tt.substr(0, 7)
						+ repeate(2 - dectohex((tt.size() - 7) / 2).size(), "0")
						+ dectohex((tt.size() - 7) / 2)
						+ tt.substr(7, tt.size() - 7);
				replace(tr[i].begin(), tr[i].end(), '\n', ' ');
				op += tr[i];
				op += "\n";
			}
		}
		for (int i = 0; i < mr.size(); i++) {
			if (mr[i] != "") {
				op += mr[i];
				op += "\n";
			}
		}
		op += er + "\n\n\n";
		if (ind == 0) {
			ind = op.size() - 3;
		}

		hr = "";
		er = "";
		dr = "";
		rr = "";
		mr.clear();
		tr.clear();
		tr.push_back("");
		hr += "H";
		hr += current.label + repeate(6 - current.label.size(), " ");
		hr += "000000";
	} else if (current.operation == "END2") {
		if (flagcsec == false) {
			long long endvalue = 0;
			for (int i = 0; i < sections.size(); i++) {
				if (sections[i].memory.find(current.operand)
						!= sections[i].memory.end()) {
					endvalue = sections[i].memory[current.operand].first;
					break;
				}
			}
			er += "E";
			er += repeate(6 - dectohex(endvalue).size(), "0")
					+ dectohex(endvalue);

		} else {
			long long endvalue = 0;
			for (int i = 0; i < sections.size(); i++) {
				if (sections[i].memory.find(current.operand)
						!= sections[i].memory.end()) {
					endvalue = sections[i].memory[current.operand].first;
					break;
				}
			}
			op = op.substr(0, ind) + repeate(6 - dectohex(endvalue).size(), "0")
					+ dectohex(endvalue) + op.substr(ind, op.size() - ind);
			er += "E";
		}
		hr += repeate(6 - dectohex(current.pc_counter).size(), "0")
				+ dectohex(current.pc_counter);
		op += hr + "\n";
		if (dr != "") {
			op += dr + "\n";
		}
		if (rr != "") {
			op += rr + "\n";
		}
		for (int i = 0; i < tr.size(); i++) {
			if (tr[i] != "") {
				string tt = tr[i];
				tr[i] = tt.substr(0, 7)
						+ repeate(2 - dectohex((tt.size() - 7) / 2).size(), "0")
						+ dectohex((tt.size() - 7) / 2)
						+ tt.substr(7, tt.size() - 7);
				replace(tr[i].begin(), tr[i].end(), '\n', ' ');
				op += tr[i];
				op += "\n";
			}
		}
		for (int i = 0; i < mr.size(); i++) {
			if (mr[i] != "") {
				op += mr[i];
				op += "\n";
			}
		}
		op += er;

	} else if (current.operation == "EXTDEF") {
		dr += "D";
		section* sec = getsection();
		set<string>::iterator it;
		for (it = sec->def.begin(); it != sec->def.end(); ++it) {
			dr += *it + repeate(6 - (*it).size(), " ")
					+ repeate(6 - dectohex((sec->memory[*it]).first).size(),
							"0") + dectohex(sec->memory[*it].first);
		}
	} else if (current.operation == "EXTREF") {
		rr += "R";
		section* sec = getsection();
		set<string>::iterator it;
		for (it = sec->ref.begin(); it != sec->ref.end(); ++it) {
			rr += *it + repeate(6 - (*it).size(), " ");
		}
	} else if (var(current.operation) && tr[tr.size() - 1] != "") {
		tr.push_back("");
	} else if (current.object_code != "") {
		if (current.operation[0] == '+' && current.relocatable == true) { //edit
			mr.push_back(
					"M"
							+ repeate(
									6 - dectohex(current.pc_counter + 1).size(),
									"0") + dectohex(current.pc_counter + 1)
							+ "05");
		}
		if (current.operation == "WORD" && current.relocatable == true) {
			mr.push_back(
					"M" + repeate(6 - dectohex(current.pc_counter).size(), "0")
							+ dectohex(current.pc_counter) + "06");
		}
		if (current.operation[0] == '+' && current.extref.size() != 0
				&& current.operation != "WORD") {
			for (int q = 0; q < current.extref.size(); q++) {
				mr.push_back(
						"M"
								+ repeate(
										6
												- dectohex(
														current.pc_counter + 1).size(),
										"0") + dectohex(current.pc_counter + 1)
								+ "05" + current.extref[q]);
			}
		}
		if (current.extref.size() != 0 && current.operation == "WORD") {
			for (int q = 0; q < current.extref.size(); q++) {
				mr.push_back(
						"M"
								+ repeate(
										6 - dectohex(current.pc_counter).size(),
										"0") + dectohex(current.pc_counter)
								+ "06" + current.extref[q]);
			}
		}
		string tmp = tr[tr.size() - 1];
		if (tmp == "") {
			tr[tr.size() - 1] += "T";
			tr[tr.size() - 1] += repeate(
					6 - dectohex(current.pc_counter).size(), "0")
					+ dectohex(current.pc_counter);
//                ////cout << dectohex(current.pc_counter) <<endl  ;
			tr[tr.size() - 1] += current.object_code;

		} else if (tmp.size() - 9 + current.object_code.size() <= 60) {
			tr[tr.size() - 1] += current.object_code;
		} else if (tmp.size() - 9 + current.object_code.size() > 60) {

			string str = "";
			str += "T";
			str += repeate(6 - dectohex(current.pc_counter).size(), "0")
					+ dectohex(current.pc_counter);
			str += current.object_code;
			tr.push_back(str);
		}

	}
}

bool Pass2::pass2_logic() {
//	cout<<"--------------"<<endl;
//	for(int i=0;i<overLit.size();i++){
//		cout<<overLit[i]<<endl;
//	}
//	cout<<"--------------"<<endl;

	/////////////
	long long pctemp = 0;
	string operandtemp = "";
	bool endreached = false;
	bool error = false;
	///////////

	string file_name = "LISTFILE.txt";
	ifstream stream(file_name);
	string line;
	intermediate_file +=
			">>   *****************************************************\n>>   S t a r t   o f    P a s s   I I\n\n";
	intermediate_file +=
			">>   A s s e m b l e d    p r o g r a m     l i s t i n g\n\n";
	intermediate_file += " LC    Code         Source Statement\n\n";
	while (getline(stream, line)) {
		instruction i = read_input(line);
		set_registers(i);
		intermediate_file += print_instruction(i);
		////////////
		if (i.operation == "END") {
			pctemp = i.pc_counter;
			operandtemp = i.operand;
			endreached = true;
		}
		if (endreached && i.operation != "END") {
			pctemp += (i.operand.size()) / 2;
		}
		///////////////
		objectProgram(i);
		if (i.error != "")
			error = true;

//		cout<<"-------------" <<endl;
//		cout <<i.operation << endl;
//		cout <<i.object_code << endl;
//		cout<<"-------------" <<endl;
//		if(i.label=="" && i.operation=="" && i.operand=="" && !i.comment_only)
//			cout<<"asdasd";
//		if(i.operation=="END"){
//			cout<<i.litpool.size();
//		}
	}

	stream.close();

	//////////////
	instruction ii = read_input("");
	ii.error = "";
	ii.comment_only = false;
	ii.operation = "END2";
	ii.operand = operandtemp;
	ii.pc_counter = pctemp;
//    cout <<dectohex(pctemp) ;
	objectProgram(ii);
	/////////////

	ofstream out("LISTFILE.txt");
	out << intermediate_file;
	out.close();
	ofstream out2("OBJFILE.txt");
	transform(op.begin(), op.end(), op.begin(), ::toupper);
	out2 << op;
	out2.close();
	return error;
}

}

