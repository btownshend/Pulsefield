% Make a movie out of snapshots
function snapmovie(snap,varargin)
params=getparams();
defaults=struct('frame',[],...
                'filename','snap.avi',...
                'maxrange',params.maxrange,...
                'fps',29.97,...
                'width',1280,...
                'height',720,...
                'crop',false,...
                'debug',false,...
                'sync',false...
                );
args=processargs(defaults,varargin);

if isempty(args.frame)
  args.frame=snap(1).vis.frame;
end
if length(args.frame)==1
  args.frame(2)=snap(end).vis.frame;
end

% Prepare the new file.
vidObj = VideoWriter(args.filename);
vidObj.FrameRate=args.fps;
open(vidObj);

if args.sync
  srcfps=(snap(end).vis.frame-snap(1).vis.frame)/((snap(end).vis.acquired-snap(1).vis.acquired)*24*3600);
  incr=srcfps/args.fps;
  fprintf('Source FPS=%f, target=%f, advancing by %f frames/video frame\n', srcfps, args.fps, incr);
else
  incr=1;
end
for f=args.frame(1):incr:args.frame(2)
  plotsnap(snap,'frame',round(f),'maxrange',args.maxrange,'crop',args.crop,'setfig',false,'showhits',false);
  axis normal
  % Write each frame to the file.
  set(gca,'Position',[0,0,1,1]);
  pos=get(gcf,'Position');
  set(gcf,'Position',[pos(1),pos(2),args.width,args.height]);
  currFrame = getframe;
  writeVideo(vidObj,currFrame);
end
  
% Close the file.
close(vidObj);
  
