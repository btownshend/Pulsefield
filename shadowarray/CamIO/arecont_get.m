function val=arecont_get(id,param)
if strncmp(param,'reg',3)
  v=sscanf(param,'reg_%d_%d');
  url=sprintf('http://192.168.0.%d/getreg?page=%d&reg=%d',70+id,v(1),v(2));
else  
  url=sprintf('http://192.168.0.%d/get?%s',70+id,param);
end
[resp,status]=urlread(url);
if status~=1
  error('Failed read of %s',url);
end
httpstr=strfind(resp,'HTTP');
if ~isempty(httpstr)
%  fprintf('Bad response: "%s"\n', resp);
  resp=resp(1:httpstr(1)-1);
end

eq=strfind(resp,'=');
if isempty(eq)
  error('Missing "=" in response from %s: %s', url, resp);
end
val=resp(eq+1:end);
x=str2double(val);
if ~isnan(x)
  val=x;
end    
