% Convert sensor positions to angles
function sa=spos2angle(spos,cam)
sa=interp1(1:cam.hpixels,cam.anglemap,spos);
