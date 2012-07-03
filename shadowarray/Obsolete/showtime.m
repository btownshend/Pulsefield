% Display time in a window
window=2/60/60/24;
triggertime=now+window;
done=false;
url='http://192.168.0.70/cgi-bin/viewer/video.jpg?quality=5';
cmd=sprintf('curl -s %s >/tmp/dcs.jpg &', url);
pos=0.1;
while now<triggertime+window
  if pos>1
    pos=0.1;
    clf
  end
  text(0.1,pos,datestr(now,'MM:SS.FFF'),'FontSize',120)
  if ~done && now>=triggertime
    tstart=now;
    system(cmd);
    tend=now;
    done=true;
  end
  pause(0.001)
  pos=pos+0.2;
end
fprintf('Triggered at %s - %s\n',datestr(tstart,'MM:SS.FFF'),datestr(tend,'MM:SS.FFF'));
im=imread('/tmp/dcs.jpg');
imshow(im);


