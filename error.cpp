#include <iostream>
#include "error.h"
using namespace std;

// Legacy Code from CSE504

static int errorCount = 0;
static int warningCount = 0;

void msg(const string& type, const string& errorMsg, int line, int col, string file) {
	if (line == 0) {
    line = yylinenum;
    file = yyfilename;
  }
  cerr << file << ':' << line;
  if (col != 0)
    cerr << '.' << col;
  cerr << ':';
  cerr << type << errorMsg << endl;

}

void 
internalError(const string& s, int line, string file) {
  msg("Internal error: ", s, line, 0, file);
}

void errMsg(string errorMsg, int line, int col, string file) {
  msg("Error: ", errorMsg, line, col, file);
  errorCount++;
  if (errorCount > MAX_ERROR_BEFORE_QUIT) {
    cerr << endl << "Too many errors.  Exiting ... " << endl;
    exit(-1);
  }
}


void warnMsg(string warningMsg, int line, int col, string file) {
  msg("Warning: ", warningMsg, line, col, file);
  warningCount++;
}



void resetErrs() {
  warningCount = errorCount = 0;
}

void resetWarns() {
  warningCount = 0;
}

int warnCount() {
  return warningCount;
}

int errCount() {
  return errorCount;
}



