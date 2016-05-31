import java.util.Date;

import processing.core.PApplet;
import processing.core.PMatrix3D;
import processing.core.PVector;
import processing.data.JSONArray;
import processing.data.JSONObject;

public class Config {
	private static String configFileName;
	private static JSONObject data;
	private static boolean modified;

	public Config(String fname) {
		configFileName=fname;
		modified=false;
		data=new JSONObject();
	}
	public static void load(PApplet parent) {
		data=parent.loadJSONObject(configFileName);
		PApplet.println("Loaded config from "+configFileName);
	}

	public static void saveIfModified(PApplet parent) {
		if (modified)
			save(parent);
	}

	public static void save(PApplet parent) {
		data.setString("dateSaved", (new Date()).toString());
		parent.saveJSONObject(data, configFileName);
		modified=false;
		PApplet.println("Saved config to "+configFileName);
	}

	private static JSONObject get(String group) {
		if (! data.hasKey(group)){
			PApplet.println("JSON file doesn't contain "+group);
			data.setJSONObject(group, new JSONObject());
		}
		return data.getJSONObject(group);
	}
	
	public static float getFloat(String group, String param, float defVal) {
		JSONObject grp=get(group);
		if (! grp.hasKey(param)){
			PApplet.println("JSON file doesn't contain "+group+"/"+param);
			return defVal;
		}
		return data.getJSONObject(group).getFloat(param);
	}

	public static void setFloat(String group, String param, float value) {
		JSONObject grp=get(group);
		grp.setFloat(param, value);
		data.setJSONObject(group, grp);
		modified=true;
	}
	public static int getInt(String group, String param, int defVal) {
		JSONObject grp=get(group);
		if (! grp.hasKey(param)){
			PApplet.println("JSON file doesn't contain "+group+"/"+param);
			return defVal;
		}
		return data.getJSONObject(group).getInt(param);
	}
	public static void setInt(String group, String param, int value) {
		JSONObject grp=get(group);
		grp.setInt(param, value);
		data.setJSONObject(group, grp);
		modified=true;
	}
	public static void setMat(String group, String param, PMatrix3D mat) {
		JSONObject grp=get(group);
		JSONArray array=new JSONArray();
		array.append(mat.m00);
		array.append(mat.m01);
		array.append(mat.m02);
		array.append(mat.m03);
		array.append(mat.m10);
		array.append(mat.m11);
		array.append(mat.m12);
		array.append(mat.m13);
		array.append(mat.m20);
		array.append(mat.m21);
		array.append(mat.m22);
		array.append(mat.m23);
		array.append(mat.m30);
		array.append(mat.m31);
		array.append(mat.m32);
		array.append(mat.m33);
		grp.setJSONArray(param, array);
		modified=true;
	}
	
	public static PMatrix3D getMat(String group, String param, PMatrix3D defMat) {
		JSONObject grp=get(group);
		if (! grp.hasKey(param)) {
			PApplet.println("JSON file doesn't contain "+group+"/"+param);
			return defMat;
		}
		JSONArray m=grp.getJSONArray(param);
		if (m.size() != 16) {
			PApplet.println("Bad size for "+group+"/"+param+": "+m.size());
			return defMat;
		}
		return new PMatrix3D(m.getFloat(0),m.getFloat(1),m.getFloat(2),m.getFloat(3),
				m.getFloat(4),m.getFloat(5),m.getFloat(6),m.getFloat(7),
				m.getFloat(8),m.getFloat(9),m.getFloat(10),m.getFloat(11),
				m.getFloat(12),m.getFloat(13),m.getFloat(14),m.getFloat(15)
				);
	}
	
	public static void setVec(String group, String param, PVector vec) {
		JSONObject grp=get(group);
		JSONArray array=new JSONArray();
		array.append(vec.x);
		array.append(vec.y);
		array.append(vec.z);
		grp.setJSONArray(param, array);
		modified=true;
	}
	
	public static PVector getVec(String group, String param, PVector defVec) {
		JSONObject grp=get(group);
		if (! grp.hasKey(param)){
			PApplet.println("JSON file doesn't contain "+group+"/"+param);
			return defVec;
		}
		JSONArray m=grp.getJSONArray(param);
		if (m.size() != 3) {
			PApplet.println("Bad size for "+group+"/"+param+": "+m.size());
			return defVec;
		}
		return new PVector(m.getFloat(0),m.getFloat(1),m.getFloat(2));
	}
}
