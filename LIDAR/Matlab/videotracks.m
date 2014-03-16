%% Display Tracking Results
% The |displayTrackingResults| function draws a bounding box and label ID 
% for each track on the video frame. It then 
% displays the frame  in their respective video players. 

function vp=videotracks(snap,vp,speedFactor)
  winbounds=[-8,5,-0.5,6];
  if nargin<2
    width=800;
    height=round((winbounds(4)-winbounds(3))/(winbounds(2)-winbounds(1))*width);
    vp = vision.VideoPlayer('Position', [20, 20, width, height]);
  else
    vp.reset();
  end
  
  if nargin<3
    speedFactor=1.0;
  end
  im=255*zeros(vp.Position(4),vp.Position(3),3,'uint8');

  starttime=now;
  totalwait=0;
  ww=[];
  tic
  for is=1:length(snap)
    s=snap(is);
    v=s.vis;
    frame=im;
    
    obj=snap(is).tracker;
    if ~isempty(obj.tracks)
      
      % noisy detections tend to result in short-lived tracks
      % only display tracks that have been visible for more than 
      % a minimum number of frames.
      reliableTrackInds =[obj.tracks(:).totalVisibleCount] > obj.minVisibleCount;
      
      % display the objects. If an object has not been detected
      % in this frame, display its predicted bounding box.
      if ~isempty(obj.tracks)
        predictedTrackInds = [obj.tracks.consecutiveInvisibleCount] > 0;
        
        % draw on the frame
        for i=1:length(obj.tracks)
          t=obj.tracks(i);
          if reliableTrackInds(i)
            col='green';
          else
            col='yellow';
          end
          l=t.legs;
          c1=mappt(l.c1,winbounds,size(frame));
          rtmp=mappt(l.c1+[l.radius,0],winbounds,size(frame));
          radius=rtmp(1)-c1(1);
          frame=insertObjectAnnotation(frame,'circle',[c1,radius],'L','Color',col);
          if all(isfinite(l.c2))
            c2=mappt(l.c2,winbounds,size(frame));
            frame=insertObjectAnnotation(frame,'circle',[c2,radius],'R','Color',col);
            radius=radius+norm(c2-c1)/2;
          end
          pos=mappt(t.updatedLoc,winbounds,size(frame));
          if predictedTrackInds(i)
            label=sprintf('[%d]',t.id);
          else
            label=sprintf('%d',t.id);
          end
          frame=insertObjectAnnotation(frame,'circle',[pos,radius],label,'Color',col);
        end
      end
    end
    frame=vis2image(v,frame,winbounds,0);
    frame=vis2image(snap(is).bg,frame,winbounds,1);

    waittime=(snap(is).vis.when-snap(1).vis.when)/speedFactor-(now-starttime); 
    ww(end+1)=waittime;
    if waittime>0
      pause(waittime*3600*24);
    end
    vp.step(frame);
  end
  endtime=toc;
  fprintf('Played at %.2fx real time with %d late frames\n', (snap(end).vis.when-snap(1).vis.when)*24*3600/endtime,sum(ww<0));
end

function mapped=mappt(xy,winbounds,framesz)
  mapped(1)=round((xy(1)-winbounds(1))./(winbounds(2)-winbounds(1))*framesz(2)+1);
  mapped(2)=round((winbounds(4)-xy(2))./(winbounds(4)-winbounds(3))*framesz(1)+1);
end

  