% Move all tracker states that are invisible into the nearest shadow
function tracker=moveintoshadows(tracker,vis,bg)
for i=1:length(tracker.tracks)
  t=tracker.tracks(i);
  if t.consecutiveInvisibleCount>0
    % Not visible
    [theta,range]=xy2range(t.updatedLoc);
    if theta<vis.angle(1) || theta>vis.angle(end)
      fprintf('Track %d at (%.2f,%.2f) is outside FOV\n',t.id, t.updatedLoc);
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
        fprintf('Track %d at (%.2f,%.2f) is invisible, but not shadowed\n',t.id, t.updatedLoc);
        rangetmp=max(vis.range+0.2,range);   % Nearest posible locations of each shadow
        %rangetmp(vis.class==0)=vis.range(vis.class==0); % Don't make background a shadow possibility, but could stick to background
        closexy=range2xy(vis.angle,rangetmp);
        distxy=closexy;
        distxy(:,1)=distxy(:,1)-t.updatedLoc(1);
        distxy(:,2)=distxy(:,2)-t.updatedLoc(2);
        [~,cpos]=min(distxy(:,1).^2+distxy(:,2).^2);
        newpos=closexy(cpos,:);
      end
    end
    distmoved=norm(newpos-t.updatedLoc);
    fprintf('Moving to closest valid point at (%.2f,%.2f) with a distance of %.2f\n', newpos, distmoved);
    t.updatedLoc=newpos;
    newstate=t.kalmanFilter.State;
    newstate([1,3])=newpos;
    newstate([2,4])=newstate([2,4])/2;   % Also decrease velocity
    t.kalmanFilter.State=newstate;
    tracker.tracks(i)=t;
  end
end

  