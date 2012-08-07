% Load simultaneously from multiple cameras
% id is an array of ids to load
% result is a cell array of images
function im=arefps(id,roi)

p.captstart=now;
arecont_set(id,'autoexp','on');
arecont_set(id,'exposure','on');
arecont_set(id,'brightness',-50);
arecont_set(id,'lowlight','highspeed');
arecont_set(id,'shortexposures',10);
pause(2);
arecont_set(id,'autoexp','off');
arecont_set(id,'exposure','off');
arecont_set(id,'sensorleft',roi(1));
arecont_set(id,'sensorwidth',roi(2)-roi(1)+1);
arecont_set(id,'sensortop',roi(3));
arecont_set(id,'sensorheight',roi(4)-roi(3)+1);

fprintf('FPS setting = %.2f\n', arecont_get(id,'fps'));
[h,p]=getsubsysaddr(sprintf('CA%d',id));
url=sprintf('http://%s/image?res=full&quality=21&doublescan=0&x0=%d&x1=%d&y0=%d&y1=%d',h,roi);
cmd=sprintf('curl -s ''%s'' >/tmp/im%d.jpg', url,id);
cnt=50;
tic
for i=1:cnt
  system(cmd);
  t(i)=toc;
end
stop=toc;
fprintf('FPS=%.2f MIN=%.2f, MAX=%.2f\n',(cnt-2)/(t(end)-t(2)),1/max(diff(t(2:end))),1/min(diff(t(2:end))));
figure(1);
clf;
subplot(211);
plot(t(2:end));
subplot(212);
plot(diff(t(2:end)));
