% Hotspots app
classdef SoundAppHotspots < SoundApp
  properties
    nhotspots=10;
    radius=1.2;
    pos;
  end

  methods
    function obj=SoundAppHotspots(uiposition)
      obj=obj@SoundApp(uiposition);
      obj.name='Hotspot';
    end
    
    function info=start(obj,p,info)
      obj.pos=(rand(obj.nhotspots,2)*2-1)*max(p.layout.active(:))*0.9;   % TODO - make this depend on layout, check if inside, none too close together
    end

    function stop(obj,p,info)
    end
    
    function plot(obj,p,info)
      setfig('hotspots');
      clf;
      plotlayout(p.layout,0);
      hold on;
      plot(obj.pos(:,1),obj.pos(:,2),'*');
    end

    function info=update(obj,p,info)
      for i=1:length(info.updates)
        id=info.updates(i);
        ids=[info.snap.hypo.id];
        previds=[info.prevsnap.hypo.id];
        oldpos=info.snap.hypo(ids==id).pos;
        newpos=info.prevsnap.hypo(previds==id).pos;
        for j=1:size(obj.pos,1)
          dist=norm(newpos-obj.pos(j,:));
          %fprintf('dist(%d,%d)=%.2f\n',i,j,dist);
          if dist<=obj.radius
            pdist=norm(oldpos-obj.pos(j,:));
            if pdist>obj.radius
              % Just moved into place
              if info.max
                pitch=j+35;   % Drum kit goes from 35 to 81
                oscmsgout('MAX','/pf/pass/playmidinote',{int32(info.snap.hypo(i).id),int32(pitch),int32(info.velocity),int32(info.duration),10});
              end
              if info.ableton
                channel=info.cm.id2channel(id);
                info.al.playclip(channel,j);
              end
            end
          end
        end
      end
    end
    
  end
end