% Load and solve the mappings in matlab
if ~exist('mats')
  fd=fopen('settings_proj.json');
  x=char(fread(fd,inf))';  % Read the file
                           % Replace numeric strings with values
  x=regexprep(x,'"([0-9.e-]*)"','$1');
  j=jsondecode(x);
  c=j.calibration;
  mats=reshape(c.homographies,[],3,3);
  x=pack(mats);
  m1=unpack(x);
  x2=pack(m1);
  assert(all(abs(x-x2)<1e-10));
end
fprintf('Initial error=%f\n',allerror(c,x,2));
options=optimset('display','final','maxfunevals',100000,'maxiter',100000,'tolx',1e-10,'tolfun',1e-8);
lasterr=0;
while true
  fprintf('\nMinimizing...');
  x=fminsearch(@(z) allerror(c,z), x, options);
  finerr=allerror(c,x,2);
  fprintf('Final error=%f\n',finerr);
  mats=unpack(x);
  x2=pack(mats);
  assert(all(abs(x-x2)<1e-10));
  if abs(finerr-lasterr)<1e-8
    break;
  end
  lasterr=finerr;
end

function mats=unpack(x)
% Last 2 units are translation & rotation only (3 parameters)
mats=nan(4,3,3);
for i=1:size(mats,1)
  mats(i,:,:)=reshape([x((i-1)*8+(1:8)),1],3,3);
end
j=33;
for k=1:2
  mats(end+1,:,:)=eye(3);
  mats(end,1:2,3)=x(j:j+1); j=j+2;
  theta=x(j);j=j+1;
  mats(end,1:2,1:2)=[cos(theta) sin(theta); -sin(theta) cos(theta)];
end
end

function x=pack(mats)
x=nan(1,4*8);
for i=1:4
  m=mats(i,:,:);
  x((i-1)*8+(1:8))=m(1:8);
end
for k=5:6
  x((end+1):(end+2))=mats(k,1:2,3);
  x(end+1)=atan2(mats(k,1,2),mats(k,1,1));
end
end


function err=allerror(c,x,verbose)
mats=unpack(x);
err=0;
totalpts=0;
if nargin<3
  verbose=0;
end
for i=1:length(c.mappings)
  m=c.mappings(i);
  if m.unit1<c.nunits && m.unit2<c.nunits
    npts=sum([m.pairs.locked]);
    if npts>0
      ei=maperror(m,squeeze(mats(m.unit1+1,:,:)),squeeze(mats(m.unit2+1,:,:)),verbose);
      if verbose
        fprintf('%d:%d has %d points with RMS error of %.3f, e^2=%.4f\n', m.unit1, m.unit2, npts, sqrt(ei/npts),ei);
      end
      err=err+ei;
      totalpts=totalpts+npts;
    end
  end
end
err=sqrt(err/totalpts);
if nargin>=3 && verbose
  fprintf('RMS Error = %.3f\n', err);
end
end


function e=maperror(m,mat1,mat2,verbose)
e=0;
for i=1:length(m.pairs)
  p=m.pairs(i);
  if p.locked
    pt1=[p.pt1.x,p.pt1.y,1]';
    pt2=[p.pt2.x,p.pt2.y,1]';
    w1=mat1*pt1;
    w1=w1(1:2)/w1(3);
    w2=mat2*pt2;
    w2=w2(1:2)/w2(3);
    if nargin>=4 && verbose>1
      fprintf('[%.1f,%.1f]->[%.2f,%.2f];  [%.1f,%.1f]->[%.2f,%.2f]; e=%.2f, e^2=%.4f\n', pt1(1:2),w1,pt2(1:2),w2,norm(w1-w2),sum((w1-w2).^2));
    end
    e=e+sum((w1-w2).^2);
  end
end
end

    
