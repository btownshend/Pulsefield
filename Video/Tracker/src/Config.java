import processing.core.PApplet;
import processing.core.PMatrix3D;
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
	}

	public static void saveIfModified(PApplet parent) {
		if (modified)
			save(parent);
	}

	public static void save(PApplet parent) {
		parent.saveJSONObject(data, configFileName);
		modified=false;
	}

	public static float getFloat(String group, String param, float defVal) {
		if (! data.hasKey(group))
			data.setJSONObject(group, new JSONObject());
		JSONObject grp=data.getJSONObject(group);
		if (! grp.hasKey(param))
			setFloat(group,param,defVal);
		return data.getJSONObject(group).getFloat(param);
	}

	public static void setFloat(String group, String param, float value) {
		if (! data.hasKey(group))
			data.setJSONObject(group, new JSONObject());
		JSONObject grp=data.getJSONObject(group);
		grp.setFloat(param, value);
		data.setJSONObject(group, grp);
		modified=true;
	}
	public static int getInt(String group, String param, int defVal) {
		if (! data.hasKey(group))
			data.setJSONObject(group, new JSONObject());
		JSONObject grp=data.getJSONObject(group);
		if (! grp.hasKey(param))
			setFloat(group,param,defVal);
		return data.getJSONObject(group).getInt(param);
	}
	public static void setInt(String group, String param, int value) {
		if (! data.hasKey(group))
			data.setJSONObject(group, new JSONObject());
		JSONObject grp=data.getJSONObject(group);
		grp.setInt(param, value);
		data.setJSONObject(group, grp);
		modified=true;
	}
	public static void set(String group, String param, PMatrix3D mat) {
		if (! data.hasKey(group))
			data.setJSONObject(group, new JSONObject());
		JSONObject grp=data.getJSONObject(group);
		if (! grp.hasKey(param))
			grp.setJSONObject(param, new JSONObject());
		JSONObject m=grp.getJSONObject(param);
		m.setFloat("m00", mat.m00);
		m.setFloat("m01", mat.m01);
		m.setFloat("m02", mat.m02);
		m.setFloat("m03", mat.m03);
		m.setFloat("m10", mat.m10);
		m.setFloat("m11", mat.m11);
		m.setFloat("m12", mat.m12);
		m.setFloat("m13", mat.m13);
		m.setFloat("m20", mat.m20);
		m.setFloat("m21", mat.m21);
		m.setFloat("m22", mat.m22);
		m.setFloat("m23", mat.m23);
		m.setFloat("m30", mat.m30);
		m.setFloat("m31", mat.m31);
		m.setFloat("m32", mat.m32);
		m.setFloat("m33", mat.m33);
		grp.setJSONObject(param, m);
		data.setJSONObject(group, grp);
		modified=true;
	}
}
