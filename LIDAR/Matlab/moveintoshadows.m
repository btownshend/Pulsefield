% Move all tracker states that are invisible into the nearest shadow
function tracker=moveintoshadows(tracker,vis)
for i=1:length(tracker.tracks)
  t=tracker.tracks(i);
  if t.consecutiveInvisibleCount>0
    % Not visible
    [theta,range]=xy2range(t.predictedLoc);
    if theta<vis.angle(1) || theta>vis.angle(end)
      fprintf('Frame %d, track %d at (%.2f,%.2f) is outside FOV\n',vis.frame,t.id, t.predictedLoc);
      if theta<vis.angle(1)
        theta=vis.angle(1);
      else
        theta=vis.angle(end);
      end
      newpos=range2xy(theta,range);
    else
      [~,cpos]=min(abs(vis.angle-theta));
      % Actually, don't make background special, could have a near background (like a chair) that targets go behind
      if vis.range(cpos)<range % && bg.range(cpos)>range
        % Already shadowed
        continue;
      else
        % Current prediction is not shadowed
        fprintf('Frame %d, track %d at (%.2f,%.2f) is invisible, but not shadowed\n',vis.frame,t.id, t.predictedLoc);
        rangetmp=max(vis.range+0.2,range);   % Nearest posible locations of each shadow
        %rangetmp(vis.class==0)=vis.range(vis.class==0); % Don't make background a shadow possibility, but could stick to background
        closexy=range2xy(vis.angle,rangetmp);
        distxy=closexy;
        distxy(:,1)=distxy(:,1)-t.predictedLoc(1);
        distxy(:,2)=distxy(:,2)-t.predictedLoc(2);
        [~,cpos]=min(distxy(:,1).^2+distxy(:,2).^2);
        newpos=closexy(cpos,:);
      end
    end
    distmoved=norm(newpos-t.predictedLoc);
    fprintf('Moving to closest valid point at (%.2f,%.2f) with a distance of %.2f\n', newpos, distmoved);
    t.predictedLoc=newpos;
    newstate=t.kalmanFilter.State;
    newstate([1,3])=newpos;
    %newstate([2,4])=newstate([2,4])/2;   % Also decrease velocity
    t.kalmanFilter.State=newstate;
    tracker.tracks(i)=t;
    if distmoved>1
      keyboard
    end
  end
end

  