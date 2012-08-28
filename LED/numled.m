function n=numled(strip)
nperstrip=[30,160,160,160,160,29];
if nargin<1
  n=sum(nperstrip);  % Total number
else
  n=nperstrip(strip);
end
