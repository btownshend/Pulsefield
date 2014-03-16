%% Display Tracking Results
% The |displayTrackingResults| function draws a bounding box and label ID 
% for each track on the video frame. It then 
% displays the frame  in their respective video players. 

function videotracks(snap)
  winbounds=[-5,5,-0.5,8];
  vp = vision.VideoPlayer('Position', [20, 400, 600, 600]);
  im=255*zeros(600,600,3,'uint8');

  for is=1:length(snap)
    s=snap(is);
    v=s.vis;
    im2=vis2image(v,im,winbounds,0);
    frame=vis2image(snap(is).bg,im2,winbounds,1);

    obj=snap(is).tracker;
    if ~isempty(obj.tracks)
      
      % noisy detections tend to result in short-lived tracks
      % only display tracks that have been visible for more than 
      % a minimum number of frames.
      reliableTrackInds =[obj.tracks(:).totalVisibleCount] > obj.minVisibleCount;
      reliableTracks = obj.tracks(reliableTrackInds);
      
      % display the objects. If an object has not been detected
      % in this frame, display its predicted bounding box.
      if ~isempty(reliableTracks)
        % get bounding boxes
        % bboxes = cat(1, reliableTracks.bbox);
        % for i=1:size(bboxes,1)
        %   bboxes(i,1)=(bboxes(i,1)-winbounds(1))/(winbounds(2)-winbounds(1))*(size(frame,2)-1)+1;
        %   bboxes(i,2)=(winbounds(4)-(bboxes(i,2)+bboxes(i,4)))/(winbounds(4)-winbounds(3))*(size(frame,1)-1)+1;
        %   bboxes(i,3)=bboxes(i,3)/(winbounds(2)-winbounds(1))*(size(frame,2)-1);
        %   bboxes(i,4)=bboxes(i,4)/(winbounds(4)-winbounds(3))*(size(frame,1)-1);
        % end
        % bboxes=round(bboxes);
        
        % get ids
        ids = int32([reliableTracks(:).id]);
        
        % create labels for objects indicating the ones for 
        % which we display the predicted rather than the actual 
        % location
        labels = cellstr(int2str(ids'));
        predictedTrackInds = [reliableTracks(:).consecutiveInvisibleCount] > 0;
        isPredicted = cell(size(labels));
        isPredicted(predictedTrackInds) = {' predicted'};
        labels = strcat(labels, isPredicted);
        
        % draw on the frame
        % for i=1:length(labels)
        %   fprintf('Annotating with %s at (%f,%f,%f,%f)\n', labels{i},bboxes (i,:));
        % end
        %      frame = insertObjectAnnotation(frame, 'rectangle', ...
        %                                    bboxes, labels);
        
      end
    end
    
    vp.step(frame);
    if is<length(snap)
      pause((snap(is+1).vis.when-snap(is).vis.when)*24*3600);
    end
  end
end