import processing.core.PApplet;
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
}
