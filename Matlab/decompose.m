% Decompose a complete transformation matrix (P*C*M) into its parts
% P - projection matrix based on camera frustum (e.g. set in Processing by frustum(left,right,bottom,top,near,far);
% C - camera (view) matrix.  Consists of a rotation and translation
% M - model matrix.  Maps from world coordinates to model

function [proj,c,m]=decompose(pcm,zknown)
if nargin<2
  zknown=true;
end
% Assume p for EH320est
proj=[ 0.2851  0.0000  0.0000  0.0000
 0.0000 -0.5068  1.2027  0.0000
 0.0000  0.0000 -1.0408 -2.0408
 0.0000  0.0000 -1.0000  0.0000];
% assume identity for m
m=eye(4,4);

if zknown
  % Compute c
  c=inv(proj)*pcm*inv(m);
else
  % Unknown z (3rd row and column of pcm indeterminate)
  % Solve numerically
  eye0=[1 2 3]; eye0=[3 2 1];
  center0=[4 7 0 ];
  upangle=pi/4;
  p0=[eye0,center0(1:2),upangle];
  opts=optimset('display','iter','tolx',1e-10,'tolfun',1e-10,'maxfunevals',10000);
  p=fminsearch(@(z) errtest(pcm,proj,m,z), p0,opts);
  eyepos=p(1:3)
  center=p(4:5);
  upangle=p(6);
  center(3)=0
  up=[cos(upangle)*.5, sin(upangle)*.5, 0.5];
  c=camera(eyepos,center,up);
end
end


function e=errtest(pcm,proj,m,p)
  eyepos=p(1:3);
  center=p(4:5);
  upangle=p(6);
  center(3)=0;
  up=[cos(upangle)*.5, sin(upangle)*.5, 0.5];
  c=camera(eyepos,center,up);
  recon=proj*c*m;
  err=recon-pcm;
  err(:,3)=0; err(3,:)=0;   % don't care about z
  e=sum(err(:).^2);
end

 
  

  
