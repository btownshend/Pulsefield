function setval=arecont_set(id,param,val)
if strncmp(param,'reg',3)
  v=sscanf(param,'reg_%d_%d');
  url=sprintf('http://192.168.0.%d/setreg?page=%d&reg=%d&value=%d',70+id,v(1),v(2),val);
elseif ischar(val)
  url=sprintf('http://192.168.0.%d/set?%s=%s',id+70,param,val);
else
  url=sprintf('http://192.168.0.%d/set?%s=%d',id+70,param,val);
end  
[resp,status]=urlread(url);
if status~=1
  error('Failed set of %s',url);
end
eq=strfind(resp,'=');
if isempty(eq)
  error('Missing "=" in response from %s: %s', url, resp);
end
setval=resp(eq+1:end);
x=str2double(val);
if ~isnan(x)
  setval=x;
end    
