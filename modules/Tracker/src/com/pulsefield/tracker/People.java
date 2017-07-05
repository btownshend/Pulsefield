package com.pulsefield.tracker;
import java.util.HashMap;
import java.util.logging.Logger;

import processing.core.PVector;


public class People {
	HashMap<Integer, Person> pmap;
    private final static Logger logger = Logger.getLogger(People.class.getName());

	People() {
		pmap=new HashMap<Integer,Person>();
	}

	public void add(int id, int channel) {
		Person ps=new Person(new PVector(0f,0f),channel, id);
		pmap.put(id,ps);
	}

	public void exit(int id) {
		Person ps=pmap.get(id);
		if (ps==null)
			logger.warning("Unable to locate user "+id+" at exit, ignoring.");
		else
			pmap.remove(id);
	}

	public void clear() {
		pmap.clear();
	}

	public void setnpeople(int n) {
		if (n!=pmap.size()) {
			logger.warning("Had "+pmap.size()+" people, but got message that there are "+n+" .. cleared.");
			pmap.clear();
		}
	}
	
	public Person get(int id) {
		return pmap.get(id);
	}
	
	public Person getOrCreate(int id, int channel) {
		Person ps=pmap.get(id);
		if (ps==null) {
			logger.info("Unable to locate user "+id+", creating it.");
			add(id,channel);
			ps=pmap.get(id);
		}
		return ps;
	}
}
