% Circular step sequencer
function info=app_cseq(info,op)
  debug=true;
  
  if ~isfield(info,'seq') || strcmp(op,'start')
    info.seq=struct('nsteps',16,'npitchsteps',16,'minr',0.1,'maxr',2.5,'touchsteps',16,'touchpitches',16);
    info.seq.pitches=makescale(info.scales{info.scale},info.keys{info.key},info.seq.npitchsteps);
  end

  if strcmp(op,'start') || strcmp(op,'stop')
    oscmsgout('MAX','/pf/pass/seqt',{'clear','cseq'});
    oscmsgout('MAX','/pf/pass/seqt',{'seq','cseq'});
    if strcmp(op,'start')
      oscmsgout('MAX','/pf/pass/seqt',{'play',int32(1)});
    else
      oscmsgout('MAX','/pf/pass/seqt',{'play',int32(0)});
    end
  end

  if info.refresh
    oscmsgout('MAX','/pf/pass/seqt/tempo',{info.tempo});
    
    info.seq.pitches=makescale(info.scales{info.scale},info.keys{info.key},info.seq.npitchsteps);

    oscmsgout('TO','/touchosc/seq/scale/val',{ info.scales{info.scale} });
    oscmsgout('TO','/touchosc/seq/key/val',{ info.keys{info.key} });
    y=sort(pstep2y(info,1:length(info.seq.pitches)),'descend');
    for i=1:info.seq.touchpitches
      midinote=info.seq.pitches(y==i);
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
  %fprintf('cseq: start=%s, stop=%s, cont=%s\n',shortlist(info.entries), shortlist(info.exits), shortlist(info.updates));

  if ~strcmp(op,'update')
    return;
  end
  
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
    [pphase,pstep,ppitch,ppitchstep]=pos2cseq(info,info.prevsnap.hypo(previds==i).pos);
    oscmsgout('MAX','/pf/pass/seqt',{'delete','cseq',0.0,1.0,'playmidinote',int32(i)});
    tsmsg(info,ppitchstep,pstep,0);
  end

  for i=info.entries
    [phase,step,pitch,pitchstep]=pos2cseq(info,info.snap.hypo(ids==i).pos);
    if isfinite(phase)
      velocity=info.velocity;
      duration=info.duration;
      channel=id2channel(info,i);
      oscmsgout('MAX','/pf/pass/seqt',{'add','cseq',phase,'playmidinote',int32(i),int32(pitch), int32(velocity), int32(duration),  int32(channel) });
      tsmsg(info,pitchstep,step,1);
    end
  end

  for i=info.updates
    [phase,step,pitch,pitchstep]=pos2cseq(info,info.snap.hypo(ids==i).pos);
    [pphase,pstep,ppitch,ppitchstep]=pos2cseq(info,info.prevsnap.hypo(previds==i).pos);
    if pstep~=step || pitch~=ppitch
      if isfinite(phase)
        % Remove any old step for this ID
        oscmsgout('MAX','/pf/pass/seqt',{'delete','cseq',0.0,1.0,'playmidinote',int32(i)});
        tsmsg(info,ppitchstep,pstep,0);
      end
      if isfinite(phase)
        % Add new step
        velocity=info.velocity;
        duration=info.duration;
        if info.multichannel
          channel=mod(i-1,6)+1;
        else
          channel=1;
        end
        oscmsgout('MAX','/pf/pass/seqt',{'add','cseq',phase,'playmidinote',int32(i),int32(pitch), int32(velocity), int32(duration),  int32(channel) });
        tsmsg(info,pitchstep,step,1);
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

function [phase,step,pitch,pitchstep]=pos2cseq(info,pos)
  [th,r]=cart2pol(pos(1),pos(2));
  th=mod(th+2*pi,2*pi);
  if r<info.seq.minr
    step=nan;
    phase=nan;
  else
    step=floor(info.seq.nsteps*th/(2*pi))+1;
    phase=(step-1)/info.seq.nsteps;
  end
  pitchstep=min(length(info.seq.pitches),max(1,ceil((r-info.seq.minr)/(info.seq.maxr-info.seq.minr)*(length(info.seq.pitches)+1))));
  pitch=info.seq.pitches(pitchstep);
  %fprintf('pos=(%.2f,%.2f), phase=%.3f, step=%d, pitch=%d, pitchstep=%d\n', pos,phase,step,pitch,pitchstep);
end

function m=tsmsg(info,pitchstep,step,val)
  oscmsgout('TO','/touchosc/seq',{pstep2y(info,pitchstep),step2x(info,step),int32(val)});
end

function x=step2x(info,step)
  x=int32(round(((step-1)*info.seq.touchsteps/info.seq.nsteps)+1));
end

function y=pstep2y(info,pitchstep)
  y=zeros(1,length(pitchstep));
  for i=1:length(pitchstep)
    y(i)=int32(min(info.seq.touchpitches,max(1,round(pitchstep(i)+(-length(info.seq.pitches)+info.seq.touchpitches)/2))));
  end
end
  