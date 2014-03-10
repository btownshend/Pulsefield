% Model x,y locations as a leg and return struct holding:
%   center - center of leg
%   radius - radius
%   err - mean square error
% Assume scan origin is 0,0
function leg=legmodel(xy,varargin)
defaults=struct('maxlegdiam',0.3,...   % Maximum leg diameter
                'minlegdiam',0.1,...   % Minimum
                'debug',true...
                );
args=processargs(defaults,varargin);
avgradius=(args.maxlegdiam+args.minlegdiam)/4;
if size(xy,1)==0
  leg=struct('center',[nan,nan],'radius',avgradius,'err',0);
elseif size(xy,1)<2
  leg=struct('center',[xy(1),xy(2)+avgradius],'radius',avgradius,'err',0);
else
  leg=struct();
  [leg.center,leg.radius,leg.err]=fitcircle(xy,args.minlegdiam/2,args.maxlegdiam/2);
end
