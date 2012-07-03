% Calibrate positions of camera's LED's
% Use spos(c,l) - sensor position (linear) of led l in camera c, null if not visible
% Normalize to distance between LED's
% Unknowns are:
%  cp(ncameras,2) - camera positions
%  cd(ncameras,2) - camera direction
%  lp(nled, 2) - led positions
% Data is:
%  spos(ncameras,led) - position of LED on sensor in pixels
%  sp2ang(sensorpos) - mapping from sensor position in pixels to angle relative to cd()
% Approach:
%  initial estimate of all variables
%  use global minimizer to match spos
function calibrate2(spos,p,cpos,cdir,lpos)
ncamera=size(spos,1);
nled=size(spos,2);

% initial guess 
% initialize camera positions randomly offset from actual ones with 5cm stdev
cp=cpos+rand(size(cpos))*.01*p.scale;
cd=cdir+rand(size(cdir))*.00;
% normalize direction vectors
for i=1:ncamera
  cd(i,:)=cd(i,:)/norm(cd(i,:));
end
% initialize LED positions randomly offset from actual ones with 1cm stdev
lp=lpos+randn(size(lpos))*.0*p.scale;
p.lp=lp;
p.cd=cd;
% Check error
x0=pack(cp,cd,lp);
err=minfn(x0,p,spos);
fprintf('Initial RMS error in lpos=%f, cpos=%f, cdir=%f, spos=%f\n', norm(lpos-lp), norm(cpos-cp), norm(cdir-cd),err);

options=optimset('Display','iter','PlotFcns',@optimplotfval);
x=fminsearch(@(x) minfn(x,p,spos),x0,options);
[cp,cd,lp]=unpack(x);

err=minfn(x,p,spos);
fprintf('Final RMS error in lpos=%f, cpos=%f, cdir=%f, spos=%f\n', norm(lpos-lp), norm(cpos-cp), norm(cdir-cd), err);

function err=minfn(x,p,spos)
[cp,cd,lp]=unpack(x,p);
curspos=calcspos(cp,cd,p.lp,p,1);
sel=isfinite(spos) & isfinite(curspos);
err=norm(spos(sel)-curspos(sel));

function x=pack(cp,cd,lp)
%x=[cp(:);cd(:);lp(:)];
x=[cp(:)];

function [cp,cd,lp]=unpack(x,p)
cp=reshape(x(1:p.ncameras*2),p.ncameras,2);x=x(p.ncameras*2+1:end);
%cd=reshape(x(1:p.ncameras*2),p.ncameras,2);x=x(p.ncameras*2+1:end);
cd=p.cd;
%lp=reshape(x,[],2);
lp=p.lp;

