% Display time in a window
guard=2;
window=guard/60/60/24;
done=false;
url='http://192.168.0.70/cgi-bin/viewer/video.jpg?quality=5';
cmd=sprintf('( sleep %d; for i in 1 2 3 4; do; ./time_ms >/tmp/d$i; curl -s %s >/tmp/dcs$i.jpg; sleep 1; done; ) &', guard, url, url);
starttime=now;
system(cmd);
figure(1);
while now<starttime+4*window
  clf;
  text(0.1,0.5,datestr(now,'MM:SS.FFF'),'FontSize',120)
  pause(0.001)
end
for i=1:4
  fd=fopen(sprintf('/tmp/d%d',i));
  d{i}=fgetl(fd);
  t(i)=fscanf(fd,'%f');
  im{i}=imread(sprintf('/tmp/dcs%d.jpg',i));
  fprintf('Loaded #%d at %s: %f\n', i, d{i}, t(i));
  fclose(fd);
end
figure(2);
for i=1:4
  subplot(2,2,i);
  imshow(im{i});
  axis([603,879,813,1088]);
end
