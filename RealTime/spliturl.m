function [host,port,proto]=spliturl(url)
colons=strfind(url,':');
slashes=strfind(url,'/');
if length(colons)==2 && length(slashes)==3
  host=url(slashes(2)+1:colons(end)-1);
  port=url(colons(end)+1:slashes(end)-1);
  proto=url(1:colons(1)-1);
  if ~isempty(host) && ~isempty(proto) && ~isempty(port)
    return;
  end
end
fprintf('Unable to split URL "%s"\n', url);

