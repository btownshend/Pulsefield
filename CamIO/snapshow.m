function snapshow(snapshot)
  setfig('snapshot');
  for i=1:length(snapshot)
    subplot(2,ceil(length(snapshot)/2),i)
    imshow(snapshot(i).im);
    title(datestr(snapshot(i).when,'mm/dd/yyyy HH:MM:SS.FFF'));
  end
end
