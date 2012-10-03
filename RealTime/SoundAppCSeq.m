% Circular step sequencer
classdef SoundAppCSeq < SoundApp
  properties
    nsteps=16;
    npitchsteps=16;
    minr=0.1;
    touchsteps=16;
    touchpitches=16;
    maxr;
    pitches;
  end
  
  methods
    function obj=SoundAppCSeq(uiposition)
      obj=obj@SoundApp(uiposition);
      obj.name='CSeq';
    end
    
    function info=start(obj,p,info)
      info.max=1;   % Need MAX for this
      info.ableton=0;
      info.cm.setnumchannels(8);   % Force to 8 channels

      obj.pitches=makescale(info.scales{info.scale},info.keys{info.key},obj.npitchsteps);
      obj.maxr=max(p.layout.active(:,1))*0.9;
      oscmsgout('MAX','/pf/pass/seqt',{'clear','cseq'});
      oscmsgout('MAX','/pf/pass/seqt',{'seq','cseq'});
      oscmsgout('MAX','/pf/pass/seqt',{'play',int32(1)});
    end
    
    function stop(obj,p,info)
      oscmsgout('MAX','/pf/pass/seqt',{'clear','cseq'});
      oscmsgout('MAX','/pf/pass/seqt',{'seq','cseq'});
      oscmsgout('MAX','/pf/pass/seqt',{'play',int32(0)});
    end

    function refresh(obj,p,info)
      fprintf('%s.refresh\n', obj.name);
      
      oscmsgout('TO','/touchosc/seq/scale/value',{[num2str(info.scale),': ',info.scales{info.scale}]});
      oscmsgout('TO','/touchosc/seq/key/value',{info.keys{info.key}});
      oscmsgout('MAX','/pf/pass/seqt/tempo',{info.tempo});
      
      obj.pitches=makescale(info.scales{info.scale},info.keys{info.key},obj.npitchsteps);

      y=sort(obj.pstep2y(1:length(obj.pitches)),'descend');
      for i=1:obj.touchpitches
        midinote=obj.pitches(y==i);
        if isempty(midinote)
          label='';
        elseif length(midinote)==1
          label=midinotename(midinote);
        elseif i==1
          label=['<= ',midinotename(max(midinote))];
        else
          label=['<= ',midinotename(min(midinote))];
        end
        oscmsgout('TO',sprintf('/touchosc/seq/note/%d',i),{label});
      end
    end
      
    function info=update(obj,p,info)
      %fprintf('cseq: start=%s, stop=%s, cont=%s\n',shortlist(info.entries), shortlist(info.exits), shortlist(info.updates));
      if isempty(info.snap)
        ids=[];
      else
        ids=[info.snap.hypo.id];
      end

      if isempty(info.prevsnap)
        previds=[];
      else
        previds=[info.prevsnap.hypo.id];
      end
      
      for i=info.exits
        [pphase,pstep,ppitch,ppitchstep]=obj.pos2cseq(info.prevsnap.hypo(previds==i).pos);
        oscmsgout('MAX','/pf/pass/seqt',{'delete','cseq',0.0,1.0,'playmidinote',int32(i)});
        obj.tsmsg(ppitchstep,pstep,0);
      end

      for i=info.entries
        [phase,step,pitch,pitchstep]=obj.pos2cseq(info.snap.hypo(ids==i).pos);
        if isfinite(phase)
          velocity=info.velocity;
          duration=info.duration;
          channel=info.cm.id2channel(i);
          oscmsgout('MAX','/pf/pass/seqt',{'add','cseq',phase,'playmidinote',int32(i),int32(pitch), int32(velocity), int32(duration),  int32(channel) });
          obj.tsmsg(pitchstep,step,1);
        end
      end

      for i=info.updates
        [phase,step,pitch,pitchstep]=obj.pos2cseq(info.snap.hypo(ids==i).pos);
        [pphase,pstep,ppitch,ppitchstep]=obj.pos2cseq(info.prevsnap.hypo(previds==i).pos);
        if pstep~=step || pitch~=ppitch
          if isfinite(phase)
            % Remove any old step for this ID
            oscmsgout('MAX','/pf/pass/seqt',{'delete','cseq',0.0,1.0,'playmidinote',int32(i)});
            obj.tsmsg(ppitchstep,pstep,0);
          end
          if isfinite(phase)
            % Add new step
            velocity=info.velocity;
            duration=info.duration;
            channel=info.cm.id2channel(i);
            oscmsgout('MAX','/pf/pass/seqt',{'add','cseq',phase,'playmidinote',int32(i),int32(pitch), int32(velocity), int32(duration),  int32(channel) });
            obj.tsmsg(pitchstep,step,1);
          end
        end
      end

      if ~isempty(info.exits) && isempty(info.entries) && isempty(info.updates)
        % Last person left
        oscmsgout('MAX','/pf/pass/seqt',{'play',int32(0)});
      end

      if isempty(info.exits) && ~isempty(info.entries) && isempty(info.updates)
        % First entry
        oscmsgout('MAX','/pf/pass/seqt',{'play',int32(1)});
      end
    end

    function [phase,step,pitch,pitchstep]=pos2cseq(obj,pos)
      [th,r]=cart2pol(pos(1),pos(2));
      th=mod(th+2*pi,2*pi);
      if r<obj.minr
        step=nan;
        phase=nan;
      else
        step=floor(obj.nsteps*th/(2*pi))+1;
        phase=(step-1)/obj.nsteps;
      end
      pitchstep=min(length(obj.pitches),max(1,ceil((r-obj.minr)/(obj.maxr-obj.minr)*(length(obj.pitches)+1))));
      pitch=obj.pitches(pitchstep);
      %fprintf('pos=(%.2f,%.2f), phase=%.3f, step=%d, pitch=%d, pitchstep=%d\n', pos,phase,step,pitch,pitchstep);
    end

    function tsmsg(obj,pitchstep,step,val)
      oscmsgout('TO','/touchosc/seq',{obj.pstep2y(pitchstep),obj.step2x(step),int32(val)});
    end

    function x=step2x(obj,step)
      x=int32(round(((step-1)*obj.touchsteps/obj.nsteps)+1));
    end

    function y=pstep2y(obj,pitchstep)
      y=zeros(1,length(pitchstep));
      for i=1:length(pitchstep)
        y(i)=int32(min(obj.touchpitches,max(1,round(pitchstep(i)+(-length(obj.pitches)+obj.touchpitches)/2))));
      end
    end
    
  end
end
