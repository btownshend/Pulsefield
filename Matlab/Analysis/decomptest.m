% Test decomposition
up=[0.1,0.1,1]; up=up/norm(up);
c=camera([1,2,3],[4,7,0],up);
% Projector (from java code)
p=[ 0.2851  0.0000  0.0000  0.0000
 0.0000 -0.5068  1.2027  0.0000
 0.0000  0.0000 -1.0408 -2.0408
 0.0000  0.0000 -1.0000  0.0000];
% model
m=[ 1.0000  0.0000  0.0000  0.0000
 0.0000  1.0000  0.0000  0.0000
 0.0000  0.0000  1.0000  0.0000
 0.0000  0.0000  0.0000  1.0000];
pcm=p*c*m;

[dp,dc,dm]=decompose(pcm)
[dp,dc,dm]=decompose(pcm,false)
fprintf('RMS error of p: %f\n', norm(dp(:)-p(:)));
fprintf('RMS error of c: %f\n', norm(dc([1,2,4],[1,2,4])-c([1,2,4],[1,2,4])));
fprintf('RMS error of m: %f\n', norm(dm(:)-m(:)));

unk=1;
full=[ -0.0629 -0.2586  unk  0.7324
 0.3501 -0.0578 unk  0.3968
-0.0970  0.0493  unk -1.0000
 0.0970 -0.0493  unk  1.0000]
[fp,fc,fm]=decompose(full,false)
[eye,aim,up]=decomposecamera(fc)
