% Set exposure window same as sensor window
for i=1:6
  arecont_set(i,'expwndwidth',arecont_get(i,'sensorwidth'));
  arecont_set(i,'expwndheight',arecont_get(i,'sensorheight'));
  arecont_set(i,'expwndleft',arecont_get(i,'sensorleft'));
  arecont_set(i,'expwndtop',arecont_get(i,'sensortop'));
end
