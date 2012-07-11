% Convert sensor positions to angles
% Angle is number of radians to the right of straight ahead (ie angle increases clockwise)
% Position (spos) is number of pixels where smallest number is farthest left
function spos=angle2spos(sa,cam)
spos=interp1(cam.anglemap,1:cam.hpixels,sa);
