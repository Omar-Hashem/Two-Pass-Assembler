/*
 * section.h
 *
 *  Created on: Apr 24, 2015
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
#include <unordered_set>

#ifndef SECTION_H_
#define SECTION_H_


namespace std {

class section {
public:
	section(string name);
	string name;
	// <name , <location ,relative> >
	map<string, pair<long long,bool > > memory;
	set<string> def;
	set<string> ref;
};

}

#endif
