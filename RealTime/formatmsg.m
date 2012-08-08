% Format an OSC message into a string for printing
function s=formatmsg(path,data)
s=path;
for i=1:length(data)
  if ischar(data{i})
    s=[s,',',data{i}];
  else
    s=[s,',',num2str(data{i})];
  end
end