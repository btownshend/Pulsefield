/* $Id: dbg.cc,v 1.15 1998/07/14 22:30:18 eryk Exp $
 * Townshend Computer Tools
 * Montreal, Quebec
 *
 * Wed Jun 8 17:03:07 EDT 1994
 *
 */
#include <assert.h>
#include <iostream>
#include <fstream>
#include <ostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <assert.h>
#include <string.h>
extern "C" {
    //#include <sys/time.h>
}
//#include <ctype.h>
#include "dbg.h"

static int PrintTiming = 0;

#if !defined(sgi) && !defined(M_I86) && !defined(__sparc)
extern "C" int gettimeofday (struct timeval *tp, struct timezone *tzp);
#endif

// the default debug file output
char *dbgf__;

std::ostream& DbgFmt(std::ostream &s, const char* dstr, int level)
{
    if (PrintTiming) {
	char fullfmt[100];
	struct timeval rt;
	gettimeofday(&rt,0);
	rt.tv_sec = rt.tv_sec%1000;
	sprintf(fullfmt,"%4ld.%03ld: ",(long)rt.tv_sec,(long)rt.tv_usec/1000);
	s << fullfmt;
    }
    s << dstr;
    int i;
    for (i=strlen(dstr); i<25; i++)
	s << " ";
    for (i=0; i<level; i++)
	s << ".";
    return s;
}

struct Files {
    char *fileName;
    std::ostream *s;
    Files *next;
};

static Files *openFiles = 0;
static char *dbgDir = 0;

std::ostream& DbgFile(const char *fname, const char *dstr, int level)
{
    for (Files *p=openFiles;p;p=p->next)
	if (strcmp(p->fileName,fname) == 0)
	    return *p->s;
    Files *newFile = new Files;
    newFile->next = openFiles;
    openFiles = newFile;
    newFile->fileName = new char[strlen(fname)+1];
    strcpy(newFile->fileName,fname);
    if (dbgDir == 0)
	SetDebugDirectory(".");
    char *fullFilename;
    if (strcmp(fname,"-") == 0) {
      fullFilename = new char[strlen(fname)+1];
      sprintf(fullFilename,"%s",fname);
      newFile->s = &std::cout;
    } else {
      fullFilename = new char[strlen(fname)+strlen(dbgDir)+2];
      sprintf(fullFilename,"%s/%s",dbgDir,fname);
      newFile->s = new std::ofstream(fullFilename,std::ofstream::trunc);
    }
    dbg(dstr,level) << "Writing to '" << fullFilename << "'." << std::endl;
    delete [] fullFilename;
    return *newFile->s;
}

void CloseDebugFiles()
{
    Files *p=openFiles,*n;
    for (;p;p=n) {
	n=p->next;
	delete [] p->fileName;
	if (p->s != &std::cout)
	  delete p->s;
	delete p;
    }
    openFiles=0;
}

    
void SetDebugDirectory(const char *dirName)
{
    if (dbgDir != 0)
	delete [] dbgDir;
    dbgDir = new char[strlen(dirName)+1];
    strcpy(dbgDir,dirName);
}

const char *GetDebugDirectory()
{
    if (dbgDir == 0)
	SetDebugDirectory(".");
    return dbgDir;
}


struct DTable {
    DTable *next;
    char *str;
    int level;
    static DTable *Root,*PushedRoot;
    DTable(const char *s, int lvl);
    ~DTable();
};

inline DTable::~DTable()
{
    if (next) {
	delete next;
	next = 0;
    }
    if (str)
	delete [] str;
}

DTable *DTable::Root = 0;
DTable *DTable::PushedRoot = 0;

DTable::DTable(const char *s, int lvl)
{
    str = new char[strlen(s)+1];
    strcpy(str,s);
    level = lvl;
    next = Root;
    Root = this;
}


// Find the first entry in DTable that matches 'dstr'.  A match occurs
// if either the entry is identical or is prefix of 'dstr' followed by
// a '.' or the end of the entry.  This gives the following results, as examp:
//
//   Entry	dstr		result
//   DELIM	DELIM.test	match
//   DELIM	DELIM		match
//   DELIM	DELIMITER	no match
//   DELIMITER	DELIM		no match
//   DELIMITER  DELIM.test	no match
//   D.TEST	D		no match
//   D.TEST	D.TEST		match
//   D.TEST	D.test		no match
//   ""		anything	match
//
// DebugCheck() returns after the first match it finds.  Since the table is
// formed by appending to the head, this results in returning the results 
// for the matching entry that was last added to the table.	
//
int DebugCheck(const char* dstr,int level)
{
    register const char *s, *q;
    for (DTable *p = DTable::Root; p; p=p->next) {
	if (*p->str == 0) {
	    // Default entry 
	    return p->level >= level;
	}
	for (s=dstr,q=p->str;*s;s++,q++)
	    if (*s != *q)
		break;
	if ((*q == 0) && ((*s == 0) || (*s == '.')))
	    return p->level >= level;
    }
    return 0;
}

// Add an entry to the debug table
// If 's' is an integer, then it serves as a default
void SetDebug(const char* s, const char* dbgf)
{
    if (dbgf__)
      delete dbgf__;
    dbgf__=new char[strlen(dbgf)];
    strcpy(dbgf__,dbgf);
    if (strncmp(s,"TIMING",6) == 0) {
	// Special case so we don't clog debug table
	PrintTiming = 1;
	return;
    }

    if (isdigit(*s))
	(void)new DTable("",atoi(s));
    else {
	const char *colon = strchr(s,':');
	if (colon) {
	    char *s2 = new char[colon-s+1];
	    if (colon != s)
		strncpy(s2,s,colon-s);
	    s2[colon-s] = 0;
	    (void)new DTable(s2,atoi(colon+1));
	    delete [] s2;
	}
	else {
	    (void)new DTable(s,999);
	}
    }
}

void PushDebugSettings() {
    dbg("dbg.PushDebugSettings",1) << "Pushing settings" << std::endl;
    assert (DTable::PushedRoot==0);
    DTable::PushedRoot=DTable::Root;
    DTable::Root=0;
}

void PopDebugSettings() {
    assert (DTable::PushedRoot!=0);
    DTable::Root=DTable::PushedRoot;
    DTable::PushedRoot=0;
    dbg("dbg.PopDebugSettings",1) << "Popped settings" << std::endl;
}
