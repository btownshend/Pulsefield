function overlay(cam,recvis,varargin)
defaults=struct('firstframe',1);
args=processargs(defaults,varargin);
% Overlay an analysis and video
setfig('video');clf;
lastframe=-1;
vr=cam.vr;

for vframe=args.firstframe:vr.NumberOfFrames
  % Find closest time
  frametime=cam.capttime+((vframe-1)/vr.FrameRate-cam.cameralate)/3600/24;
  closest=1;
  for i=1:length(recvis.vis)
    if abs(recvis.vis(i).when-frametime) < abs(recvis.vis(closest).when-frametime)
      closest=i;
    end
  end
  if closest==lastframe
    continue;
  end
  lastframe=closest;
  fprintf('Closest time to video frame %d is at vis(%d) with an offset of %.1f seconds\n', vframe, closest, (recvis.vis(closest).when-frametime)*24*3600);

  frame=read(vr,vframe);
  imshow(frame);
  hold on;

  % Draw blocked lines
  vorig=recvis.vis(closest).vorig;
  colors='rgymck';
  for i=1:size(vorig,1)
    for j=1:size(vorig,2)
      if vorig(i,j)==0
        overlayline(cam,[recvis.p.layout.cpos(i,:),recvis.p.layout.cposz(i)],[recvis.p.layout.lpos(j,:),recvis.p.layout.lposz(j)],colors(vorig(i,j)+1));
      end
    end
  end
  pause(0.1);
  
  snap=analyze(recvis.p,recvis.vis(closest),0);

  for i=1:length(snap.tgts)
    tgt=[snap.tgts(i).pos';0];
    fprintf('Target at (%.1f,%.1f,%.1f)\n',tgt);
    cd=mappoint(cam,tgt);
    plot(cd(1),cd(2),'xr');
    text(cd(1),cd(2),sprintf('%d',i));
  end
  pause(0.1);
end


% Draw a line on an image from camera 
function overlayline(cam,start,finish,color)
%fprintf('Line from (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f)\n', start, finish);
res=0.1;   % Resolution of line in meters
npts=norm(finish-start)/res;
x=(0:npts)/npts*(finish(1)-start(1))+start(1);
y=(0:npts)/npts*(finish(2)-start(2))+start(2);
z=(0:npts)/npts*(finish(3)-start(3))+start(3);
pt=[];
for i=1:length(x)
  pt(i,:)=mappoint(cam,[x(i);y(i);z(i)]);
end
%fprintf('Mapped to (%.1f,%.1f) to (%.1f,%.1f)\n', pt(1,:),pt(end,:));
plot(pt(:,1),pt(:,2),color);


function ptm=mappoint(cam,pt)
c=grid2cam(cam.extcal.extrinsic,pt);
ptm=distort(cam.distortion,c(1:2)'/c(3));
