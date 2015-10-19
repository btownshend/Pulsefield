% numled() - get number of LEDs in given strip
% with no arg, get total active LEDS (not including outer ones)
% with strip==0, get number in outer area (outside entry)
function n=numled(strip)
%nouter=129;  % Number in outer parts
%nperstrip=[30,160,160,160,160,29];
nouter=0;
%nperstrip=[160,160,160,160];
nperstrip=[160,160,160,160,160,160];	% 8/23/15 - enable all LEDs 
if nargin<1
  n=sum(nperstrip);  % Total number
elseif strip==0
  n=nouter;
else
  n=nperstrip(strip);
end
