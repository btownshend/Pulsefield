
#ifndef DBG_H
#define DBG_H

static const char *debugFile="Debug.out";
// the default debug file output
extern char *dbgf__;

#ifdef NODEBUG
inline int DebugCheck(const char *dstr,int level) { return 0; }
inline void SetDebug(const char *dlev, const char *dbgf=debugFile) { ; }
inline void CloseDebugFiles(void) {;}
#else /* NODEBUG */
int DebugCheck (const char *dstr,int level);
void SetDebug (const char* dlev, const char *dbgf=debugFile);
void CloseDebugFiles(void);
#endif /* NODEBUG */

#ifdef __cplusplus
extern "C" {
#endif
    
void SetDebugDirectory(const char *dirName);
const char *GetDebugDirectory(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class ostream;

ostream& DbgFmt(ostream& s,const char* dstr, int level);
ostream& DbgFile(const char *fname, const char *dstr, int level);

// Output to a stream with no prefix
#define dbgsn(s,fn,level)  if(DebugCheck((fn),(level))) (s)
// Output to Debug.out with no prefix
#define dbgn(fn,level) dbgfn(dbgf__,fn,level)
// Output to a stream with prefix
#define dbgs(s,fn,level)  if (DebugCheck((fn),(level))) DbgFmt((s),(fn),(level))
// Output to Debug.out with prefix
#define dbg(fn,level)  dbgs(DbgFile(dbgf__,fn,level),fn,level)
// Output to file without prefix
#define dbgfn(fname,fn,level) if (DebugCheck((fn),(level))) DbgFile((fname),(fn),(level))

#endif /* cplusplus */
#endif /* !DBG_H */
