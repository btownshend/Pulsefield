function oh=loadoh(filename, capttime,recvisfile);
recvis=load(['../../Recordings/',recvisfile]);
vr=VideoReader(filename);
fprintf('Loaded video from %s:  %dx%d @%.1f FPS, nframes=%d\n', filename, vr.Width, vr.Height, vr.FrameRate, vr.NumberOfFrames);
frame1=read(vr,1);
setfig('loadoh');clf;
imshow(frame1);
hold on;
cpos=[recvis.p.layout.cpos,recvis.p.layout.cposz'];
refs=[30/3,0,0];
for i=1:size(cpos,1)+size(refs,1)
  if i<=size(cpos,1)
    fprintf('Click on camera %d...',i);
  else
    fprintf('Click on reference %d at (%.1f,%.1f,%.1f)...',i-size(cpos,1),refs(i-size(cpos,1),:));
  end
  while true
    [x,y]=ginput(1);
    if ~isempty(x)
      break;
    end
  end
  fprintf('done\n');
  cpixel(i,:)=[x,y];
  plot(x,y,'xr');
end

worldpos=[cpos;refs];

% Load intrinsics of camera
c15=load('canon-15mm.mat');

% Correct for current image size
scale=size(frame1,2)/c15.nx;
c15.fc=c15.fc*scale;
c15.cc=(c15.cc-[c15.nx;c15.ny]/2)*scale+[size(frame1,2);size(frame1,1)]/2;

% Calculate extrinsics of camera
r=struct();
[r.omc,r.Tc,r.Rc,r.H,r.reproj,r.error] = compute_extrinsic_fisheye(cpixel',worldpos',c15.fc,c15.cc,c15.kc,c15.alpha_c);

position=cam2grid(r,[0;0;0]);
fprintf('Camera position is [%.2f,%.2f,%.2f]\n', position);
extcal=struct('extrinsic',r,'Rcw',inv(r.Rc),'Tcw',-inv(r.Rc)*r.Tc);

oh=struct('vr',vr,'capttime',capttime,'cameralate',0,'distortion',c15,'extcal',extcal,'position',position);

% Test undistortion on rectified image
frame1ud=rectify(frame1,c15);
setfig('undistorted');clf;
imshow(frame1ud);
hold on;
for i=1:length(worldpos)
  c=grid2cam(r,worldpos(i,:)');
  cn=(c(1:2)/c(3)).*c15.fc+c15.cc;
  plot(cn(1),cn(2),'xr');
end

% Test on distorted image
setfig('distorted');clf;
imshow(frame1);
hold on;
for i=1:length(worldpos)
  c=grid2cam(r,worldpos(i,:)');
  cd=distort(c15,c(1:2)'/c(3));
  plot(cd(1),cd(2),'xr');
end

% Fake a 7th camera and plot the world
ptmp=recvis.p;
ptmp.camera(7)=ptmp.camera(6);
ptmp.camera(7).id=7;
% Change it into chessboard coordinates like the other extrinsics are
ptmp.camera(7).extcal=extcal;
ptmp.camera(7).distortion=c15;
ptmp.layout.cpos(7,:)=ptmp.camera(7).extcal.Tcw(1:2);
ptmp.layout.cposz(7)=ptmp.camera(7).extcal.Tcw(3);
cdir=ptmp.camera(7).extcal.Rcw*[0 0 1]';
ptmp.layout.cdir(7,:)=cdir(1:2);
ptmp.layout.cdirz(7)=cdir(3);
setfig('world');clf;
plotworld2(ptmp);
plot3(refs(:,1),refs(:,2),refs(:,3),'go');
