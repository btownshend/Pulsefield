function n=numled(strip)
nperstrip=[160,51,160,160];
if nargin<1
  n=sum(nperstrip);  % Total number
else
  n=nperstrip(strip);
end
