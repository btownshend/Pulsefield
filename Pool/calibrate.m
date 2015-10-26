while true
    pause(1)
   oscmsgout('PHYSICS','/cue',{cue_center(1), cue_center(2), cue_vector(1), cue_vector(2)}); 
end

% calibration=load('arecont/C2.fisheye/c2-Results_left.mat');
% 
% for i=1:1
%     
%     
%     input('next image ready?');
%     a = arecont(2);
%     rectified = rectify(a.im,calibration,1);
%     imwrite(rectified, sprintf('test-%d.tiff', i), 'tiff');
%     
% end