% Guitar app
classdef SoundAppGuitar < SoundApp
  properties
    plotguitar=true;
    nfrets=10;
    fretpitches=[64,59,55,50,45,40];  % E4,B3,G3,D3,A2,E2
    stringx=[];
    frety=[];
  end
  
  methods

    function obj=SoundAppGuitar(uiposition)
      obj=obj@SoundApp(uiposition);
      obj.name='Guitar';
    end

    function info=start(obj,p,info)
    % Change to guitar sounds
      info.max=1;   % Need MAX for this
      info.ableton=0;
      info.cm.setnumchannels(8);   % Force to 8 channels

      for i=1:length(info.pgm)
        info.pgm(i)=25+(i-1)*2;
      end
      info.max=1;   % Need MAX
      info.ableton=0;
      
      nstrings=length(obj.fretpitches);
      firststringx=max(p.layout.active(:,1))*0.6;
      laststringx=-firststringx;
      obj.stringx=(0:nstrings-1)/(nstrings-1)*(laststringx-firststringx)+firststringx;
      topfrety=max(p.layout.active(:,2))*0.8;
      lastfrety=-topfrety;
      obj.frety=(0:obj.nfrets-1)/(obj.nfrets-1)*(lastfrety-topfrety)+topfrety;
      if obj.plotguitar
        setfig('guitar');
        fprintf('Clearing figure\n');
        clf; 
        plotlayout(p.layout,0);
        hold on;
        for j=1:length(obj.frety)
          plot([firststringx,laststringx],[obj.frety(j),obj.frety(j)],'k');
        end
        plot([firststringx,laststringx],[obj.frety(1),obj.frety(1)]+.05,'k');  % Darken top 
        for j=1:length(obj.stringx)
          plot([obj.stringx(j),obj.stringx(j)],[topfrety,lastfrety],'k');
        end
      end
    end

    function stop(obj,p,info)
    end
    
    function info=update(obj,p,info)
      if ~info.max
        return;
      end

      for i=1:length(info.snap.hypo)
        ph=find([info.prevsnap.hypo.id]==info.snap.hypo(i).id);
        if ~isempty(ph)
          id=info.snap.hypo(i).id;
          pos=info.snap.hypo(i).pos;
          ppos=info.prevsnap.hypo(ph).pos;
          rightofstring=pos(1)>obj.stringx;
          prevros=ppos(1)>obj.stringx;
          allplucked=find(rightofstring~=prevros);    % May be multiple
          for pl=1:length(allplucked)
            plucked=allplucked(pl);
            velocity=max(5,min(127,round(norm(info.snap.hypo(i).velocity)/1.3*127)));
            fret=find(pos(2)>[obj.frety,-inf],1)-1;
            pitch=obj.fretpitches(plucked)+fret;
            if obj.debug
              fprintf('Plucked string %d at fret %d: pitch=%d, vel=%d, dur=%d\n',plucked,fret,pitch,velocity,info.duration);
            end
            oscmsgout('MAX','/pf/pass/playmidinote',{int32(id),int32(pitch),int32(velocity),int32(info.duration),int32(info.cm.id2channel(id))});
            if obj.plotguitar
              setfig('guitar');
              col=id2color(info.snap.hypo(i).id,p.colors);
              plot(ppos(1),ppos(2),'o','Color',col);
              plot([ppos(1),pos(1)],[ppos(2),pos(2)],'-','Color',col);
              plot(pos(1),pos(2),'x','Color',col);
            end
          end
        end
      end
    end
  end
end