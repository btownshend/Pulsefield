function x=undistort(d,xd)
xd=(xd-d.cc)./d.fc;
x=comp_fisheye_distortion(xd',d.kc)';

