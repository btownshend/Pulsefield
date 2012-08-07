function setval=arecont_set(id,param,val)
[h,p]=getsubsysaddr(sprintf('CA%d',id));
if strncmp(param,'reg',3)
  v=sscanf(param,'reg_%d_%d');
  url=sprintf('http://%s/setreg?page=%d&reg=%d&value=%d',h,v(1),v(2),val);
elseif ischar(val)
  url=sprintf('http://%s/set?%s=%s',h,param,val);
else
  url=sprintf('http://%s/set?%s=%d',h,param,val);
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
