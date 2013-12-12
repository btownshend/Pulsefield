public class Patterns {
    16=>static int NUMPATTERNS;
    static Pattern @pats[NUMPATTERNS];
    
    fun void initrandom() {
//        <<<"initrandom(), NUMPATTERNS=",NUMPATTERNS>>>;
	for (0=>int i;i<NUMPATTERNS;i++) {
	    new Pattern @=> pats[i];
	    pats[i].setrandom();
	}
	<<<"First pattern:">>>;
	pats[0].print();
	<<<"Last pattern:">>>;
	pats[NUMPATTERNS-1].print();
    }

    fun static Pattern get(int pat) {
	return pats[pat];
    }
}

