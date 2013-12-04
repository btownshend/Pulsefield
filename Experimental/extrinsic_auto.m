function [r,status]=extrinsic_auto(I,target,d,dorectify,debug)
if nargin<4
  dorectify=0;
end
if nargin<5
  debug=0;
end
if isstruct(I) 
  im=I.im;
else
  im=I;
end

if size(im,3)>1
  im=rgb2gray(im);
end
im=im2double(im);
Iorig=im;

if isstruct(I) 
  if isfield(I,'bounds')  && ~dorectify
    bounds=I.bounds;
    im=im(bounds(1,2):bounds(2,2),bounds(1,1):bounds(2,1),:);
    lh=stretchlim(im,.2);
    im=imadjust(im,lh,[0,1]);
  else
    bounds(2,:)=[size(I.im,1),size(I.im,2)];
    bounds(1,:)=1;
  end
end
if debug
  setfig('extrinsic_auto.orig');clf;
  imshow(Iorig);
  pause(0.1);
end

if dorectify
  fprintf('Rectifying image...');
  im=rectify(im,d,1.5);
  fprintf('done\n');
  if debug
    setfig('extrinsic_auto.undistorted');clf;
    imshow(im);
    pause(0.1);
  end
end
fprintf('Finding corners...');
corners=findcorners(im,debug);

if corners.fail
  fprintf('Failed corner detection\n');
  status=1;
  r=[];
  return;
end
corners.grid(:,:,1)=corners.grid(:,:,1)+bounds(1,2)-1;
corners.grid(:,:,2)=corners.grid(:,:,2)+bounds(1,1)-1;
nopts_mat=size(corners.grid,1)*size(corners.grid,2);
grid_pts=reshape(corners.grid(:,:,1),1,nopts_mat);
grid_pts=[reshape(corners.grid(:,:,2),1,nopts_mat);grid_pts];
nX=size(corners.grid,1)-1;
nY=size(corners.grid,2)-1;
if nX~=target.nX || nY~=target.nY
  fprintf('Corner detection returned grid of size %dx%d, but expected %dx%d',nX,nY,target.nX, target.nY);
  status=1;
  r=[];
  return;
end
Np = (nX+1)*(nY+1);

Xi = reshape(([0:nX]*target.dX)'*ones(1,nY+1),Np,1)';
Yi = reshape(ones(nX+1,1)*[nY:-1:0]*target.dY,Np,1)';
Zi = zeros(1,Np);

Xgrid = [Xi;Yi;Zi];

fprintf('done, found %d x %d grid\n', nX, nY);

if dorectify
  if debug
    setfig('extrinsic_auto.undistorted');
    hold on;
    plot(grid_pts(1,:),grid_pts(2,:),'r.');
  end
  fprintf('Mapping grid back to orig device space ...');
  offset=(size(im)-size(Iorig))/2;
  doffset=d;
  doffset.cc(1)=d.cc(1)+offset(2);
  doffset.cc(2)=d.cc(2)+offset(1);
  for i=1:size(grid_pts,2)
    xd=(grid_pts(:,i)-doffset.cc)./doffset.fc;
    grid_pts(:,i)=distort(doffset,xd')';
  end
  grid_pts(1,:)=grid_pts(1,:)-offset(2);
  grid_pts(2,:)=grid_pts(2,:)-offset(1);
  keyboard;
  fprintf('done\n');
end

fprintf('Computing extrinsics...');
r=struct('corners',corners,'grid_pts',grid_pts,'Xgrid',Xgrid,'distortion',d);
[r.omc,r.Tc,r.Rc,r.H,r.reproj,r.error] = compute_extrinsic_fisheye(grid_pts,Xgrid,d.fc,d.cc,d.kc,d.alpha_c);
fprintf('done\n');

if debug
  setfig('extrinsic_auto.orig');
  hold on;
  plot(r.grid_pts(1,:),r.grid_pts(2,:),'r.');
  plot(r.corners.grid(1,1,2),r.corners.grid(1,1,1),'rx','MarkerSize',20);
  plot(r.corners.grid(end,end,2),r.corners.grid(end,end,1),'ro','MarkerSize',20);
  plot(r.reproj(1,:),r.reproj(2,:),'g.');
  meanerr=sqrt(mean(r.error(1,:).^2+r.error(2,:).^2));
  title(sprintf('Reprojection (RMS error=%.1f pixels)',meanerr));
end

status=0;
