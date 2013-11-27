function xd=distort(d,x)
x=apply_fisheye_distortion(x',d.kc)';
xd=x'.*d.fc+d.cc;
