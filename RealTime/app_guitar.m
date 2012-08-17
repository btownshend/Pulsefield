% Guitar app
function info=app_guitar(info,op)
if strcmp(op,'start')
  % Change to guitar sounds
  for i=1:length(info.pgm)
    info.pgm(i)=25+(i-1)*2;
  end
  info.max=1;   % Need MAX
end

if ~strcmp(op,'update')
  return;
end
plotguitar=true;
debug=true;
nstrings=6;
firststringx=1.5; laststringx=-1.5;
stringx=(0:nstrings-1)/(nstrings-1)*(laststringx-firststringx)+firststringx;
nfrets=10;
topfrety=1.7; lastfrety=-1.7;
frety=(0:nfrets-1)/(nfrets-1)*(lastfrety-topfrety)+topfrety;
fretpitches=[40,45,50,55,59,64];   % E2, A2, D3, G3, B3, E4 
fretpitches=fretpitches(end:-1:1);  % String 1 is lowest
for i=1:length(info.snap.hypo)
  ph=find([info.prevsnap.hypo.id]==info.snap.hypo(i).id);
  if ~isempty(ph)
    pos=info.snap.hypo(i).pos;
    ppos=info.prevsnap.hypo(ph).pos;
    rightofstring=pos(1)>stringx;
    prevros=ppos(1)>stringx;
    allplucked=find(rightofstring~=prevros);    % May be multiple
    for pl=1:length(allplucked)
      plucked=allplucked(pl);
      velocity=max(5,min(127,round(norm(info.snap.hypo(i).velocity)/1.3*127)));
      fret=find(pos(2)>[frety,-inf],1)-1;
      pitch=fretpitches(plucked)+fret;
      fprintf('Plucked string %d at fret %d: pitch=%d, vel=%d, dur=%d\n',plucked,fret,pitch,info.velocity,info.duration);
      oscmsgout('MAX','/pf/pass/playmidinote',{int32(info.snap.hypo(i).id),int32(pitch),int32(info.velocity),int32(info.duration)});
      if plotguitar
        setfig('guitar');
        if info.juststarted
          fprintf('Clearing figure\n');
          clf; 
          plotlayout(info.p.layout,0);
          hold on;
          for j=1:length(frety)
            plot([firststringx,laststringx],[frety(j),frety(j)],'k');
          end
          plot([firststringx,laststringx],[frety(1),frety(1)]+.05,'k');  % Darken top 
          for j=1:length(stringx)
            plot([stringx(j),stringx(j)],[topfrety,lastfrety],'k');
          end
          info.juststarted=false;
        end
        col=id2color(info.snap.hypo(i).id,info.p.colors);
        plot(ppos(1),ppos(2),'o','Color',col);
        plot([ppos(1),pos(1)],[ppos(2),pos(2)],'-','Color',col);
        plot(pos(1),pos(2),'x','Color',col);
      end
    end
  end
end
