/*
 * Pass2.h
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
#include <unordered_set>
#include <iomanip>

#ifndef PASS2_H_
#define PASS2_H_

#include "section.h"

namespace std {

struct instruction {

	string label;
	string operation;
	string operand;
	string error;
	string comment;
	long long pc_counter = -1;
	bool comment_only;
	string registers = "000000";
	bool relocatable = -1;
	string object_code = "";
	vector<string> litpool;
	vector<string> extref ;

	instruction(string l, string op, string oper, string comm, string err) {
		label = l;
		operation = op;

		operand = oper;
		comment = comm;
		error = err;
		pc_counter = -1;
		comment_only = false;
		relocatable = -1;
		registers = "000000";
		string object_code = "";
	}

	instruction(string e, string com) {
		label = "";
		operation = "";
		operand = "";
		error = e;
		comment = com;
		if (comment.compare("") != 0)
			comment_only = true;
		else
			comment_only = false;
		pc_counter = -1;
		relocatable = -1;
		registers = "000000";
		string object_code = "";
	}
};

class Pass2 {

public:

	map<string, pair<int, pair<string, string> > > operation_table;
	vector<pair<string, pair<long long, pair<long long, string> > > > LITTAB; // <name , <length , <address , value>>>
	unordered_set<string> directive;
	vector<section> sections;
	string current_section = "";
	string intermediate_file;
	///////////////////////////////
	string op = "";
	string hr = "";
	string dr = "";
	string rr = "";
	string er = "";
	vector<string> tr;
	vector<string> mr;
	////////////////////////////////
	vector<string>overLit;

	Pass2(map<string, pair<int, pair<string, string> > > operation_table,
			unordered_set<string> directive, vector<section> sections,
			string intermediate_file,
			vector<pair<string, pair<long long, pair<long long, string> > > > LITTAB);
	virtual ~Pass2();
	bool pass2_logic();
	void set_registers(instruction& current);
	instruction read_input(string line);
	bool is_directive(string s);
	section* getsection();
	bool isSection(string name);
	bool isInLITTAB(string s);
	int LITTAB_IDX(string s);
	string print_instruction(instruction current);
	//////////////////////////////
	void objectProgram(instruction& current);
	////////////////////////////
	pair < vector <string>, pair <int , int> > testExpression(instruction &inst);
	void validExpression(instruction& current);

};

}

#endif
