// An operator - a function that a person can take on (such as carrier, modulator, ...)
public class Operator {
    false => int stopped;

    fun void update() {
	<<<"Operator-update()">>>;
    }

    fun void run() {
	while (!stopped) {
	    1::second=>now;
	}
    }
 
    fun void stop() {
	true=>stopped;
    }

    spork ~run();
}
