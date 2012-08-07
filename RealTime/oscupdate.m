% Update OSC destinations with a new set of locations for the targets
function [ok,needcal]=oscupdate(p,sampnum,snap,prevsnap)
global oscsetup;

debug=false;
needcal=false;
ok=true;
apps={'cseq'};   % Can be any of guitar,lcd,cseq
plotguitar=false;

if isempty(oscsetup)
  % Need to init OSC
  oscinit;
end

% Check for incoming messages to server
dodump=nargin<4;   % First call - do a dump

while true
  % Non-blocking receive
  msgin=osc_recv(oscsetup.server,0.0);
  if isempty(msgin)
    % No message 
    break;
  end
  for i=1:length(msgin)
    m=msgin{i};
    fprintf('Got message %s from %s\n', m.path, m.src);
    if strcmp(m.path,'/pf/dest/add/port')
      [host,port,proto]=spliturl(m.src);
      url=sprintf('%s://%s:%d',proto,host,m.data{1});
      oscadddest(url);
      dodump=true;   % Always start with a dump
    elseif strcmp(m.path,'/pf/dest/remove/port')
      [host,port,proto]=spliturl(m.src);
      url=sprintf('%s://%s:%d',proto,host,m.data{1});
      oscrmdest(url);
    elseif strcmp(m.path,'/pf/dump')
      dodump=true;
    elseif strcmp(m.path,'/pf/calibrate')
      if m.data{1}>0.5
        % Don't respond to button up event (with data=0.0), only button down (1.0)
        needcal=true;
      end
    else
      fprintf('Unknown OSC message: %s\n', m.path);
    end
  end
end

% Setup client messages
m={};

% Just began
global juststarted
running=true;
if sampnum==1
  m{end+1}=msg('/pf/started',{});
  oscsetup.starttime=snap.when;
  juststarted=true;
elseif nargin<3
  m{end+1}=msg('/pf/stopped',{});
  running=false;
  juststarted=false;
end

if dodump
  active=single(p.layout.active);
  m{end+1}=msg('/pf/set/minx',min(active(:,1)));
  m{end+1}=msg('/pf/set/maxx',max(active(:,1)));
  m{end+1}=msg('/pf/set/miny',min(active(:,2)));
  m{end+1}=msg('/pf/set/maxy',max(active(:,2)));
end

if running
  elapsed=(snap.when-oscsetup.starttime)*24*3600;
  
  ids=[snap.hypo.id];
  if nargin>=4
    previds=[prevsnap.hypo.id];
  else
    previds=[];
  end
  
  % Compute entries, exits
  entries=setdiff(ids,previds);
  exits=setdiff(previds,ids);
  updates=intersect(ids,previds);
  
  m{end+1}=msg('/pf/frame',{int32(sampnum) });
  
  for i=entries
    m{end+1}=msg('/pf/entry',{int32(sampnum),elapsed,int32(i)});
    m{end+1}=msg(sprintf('/touchosc/loc/%d/color',mod(i-1,6)+1),{'green'});
  end
  for i=exits
    m{end+1}=msg('/pf/exit',{int32(sampnum),elapsed,int32(i)});
    m{end+1}=msg(sprintf('/touchosc/loc/%d/color',mod(i-1,6)+1),{'red'});
  end
  
  people=length(ids);
  prevpeople=length(previds);
  if people~=prevpeople
    m{end+1}=msg('/pf/set/npeople',int32(people));
  end
  for i=1:length(snap.hypo)
    h=snap.hypo(i);
    % Compute groupid -- lowest id in group, or 0 if not in a group
    sametnum=find(h.tnum==[snap.hypo.tnum]);
    groupid=snap.hypo(min(sametnum)).id;
    m{end+1}=msg('/pf/update',{int32(sampnum), elapsed,int32(h.id),h.pos(1),h.pos(2),h.velocity(1),h.velocity(2),h.majoraxislength,h.minoraxislength,int32(groupid),int32(length(sametnum))});
    xypos=h.pos ./max(abs(p.layout.active))
    m{end+1}=msg(sprintf('/touchosc/loc/%d',mod(h.id-1,6)+1),{xypos(2),xypos(1)});
  end
end


% Various pass-throughs to target devices
if running && ismember('lcd',apps)
  % Update LCD
  lcdsize=400;
  for i=1:length(snap.hypo)
    pos=snap.hypo(i).pos;
    lp=int32((pos+2.5)*lcdsize/5);
    sz=2;
    lp=lp-sz/2;
    le=lp+sz;
    col=id2color(snap.hypo(i).id);
    if all(col==127)
      col=[0 0 0];
    end
    col=int32(col*255);
    m{end+1}=msg('/pf/pass/lcd',{'frgb',col(1),col(2),col(3)});
    m{end+1}=msg('/pf/pass/lcd',{'paintoval',lp(1),lp(2),le(1),le(2)});
  end	
end

% Circular step sequencer
if ismember('cseq',apps)
  if running
    cseqsetup=struct('nsteps',16,'pitches',makescale('major','C',2),'minr',0.1,'maxr',2.5,'velocity',127,'duration',120,'touchsteps',16,'touchpitches',16);

    if juststarted
      m{end+1}=msg('/pf/pass/seqt',{'clear','cseq'});
      m{end+1}=msg('/pf/pass/seqt',{'seq','cseq'});
      m{end+1}=msg('/pf/pass/seqt',{'play',int32(1)});
    end
    
    %fprintf('cseq: start=%s, stop=%s, cont=%s\n',shortlist(entries), shortlist(exits), shortlist(updates));

    for i=exits
      [pphase,pstep,ppitch,ppitchstep]=pos2cseq(cseqsetup,prevsnap.hypo(previds==i).pos);
      m{end+1}=msg('/pf/pass/seqt',{'delete','cseq',0.0,1.0,'playmidinote',int32(i)});
      m{end+1}=tsmsg(cseqsetup,ppitchstep,pstep,0);
    end

    for i=entries
      [phase,step,pitch,pitchstep]=pos2cseq(cseqsetup,snap.hypo(ids==i).pos);
      if isfinite(phase)
        velocity=cseqsetup.velocity;
        duration=cseqsetup.duration;
        channel=mod(i,16)+1;
        m{end+1}=msg('/pf/pass/seqt',{'add','cseq',phase,'playmidinote',int32(i),int32(pitch), int32(velocity), int32(duration),  int32(channel) });
        m{end+1}=tsmsg(cseqsetup,pitchstep,step,1);
      end
    end
      
    for i=updates
      [phase,step,pitch,pitchstep]=pos2cseq(cseqsetup,snap.hypo(ids==i).pos);
      [pphase,pstep,ppitch,ppitchstep]=pos2cseq(cseqsetup,prevsnap.hypo(previds==i).pos);
      if pstep~=step || pitch~=ppitch
        if isfinite(phase)
          % Remove any old step for this ID
          m{end+1}=msg('/pf/pass/seqt',{'delete','cseq',0.0,1.0,'playmidinote',int32(i)});
          m{end+1}=tsmsg(cseqsetup,ppitchstep,pstep,0);
        end
        if isfinite(phase)
          % Add new step
          velocity=cseqsetup.velocity;
          duration=cseqsetup.duration;
          channel=mod(i,16)+1;
          m{end+1}=msg('/pf/pass/seqt',{'add','cseq',phase,'playmidinote',int32(i),int32(pitch), int32(velocity), int32(duration),  int32(channel) });
          m{end+1}=tsmsg(cseqsetup,pitchstep,step,1);
        end
      end
    end

    if ~isempty(exits) && isempty(entries) && isempty(updates)
      % Last person left
      m{end+1}=msg('/pf/pass/seqt',{'play',int32(0)});
    end

    if isempty(exits) && ~isempty(entries) && isempty(updates)
      % First entry
      m{end+1}=msg('/pf/pass/seqt',{'play',int32(1)});
    end
  end
end

% Update guitar strings
if running && nargin>=4 && ismember('guitar',apps)
  nstrings=6;
  firststringx=1.5; laststringx=-1.5;
  stringx=(0:nstrings-1)/(nstrings-1)*(laststringx-firststringx)+firststringx;
  nfrets=10;
  topfrety=1.7; lastfrety=-1.7;
  frety=(0:nfrets-1)/(nfrets-1)*(lastfrety-topfrety)+topfrety;
  fretpitches=[40,45,50,55,59,64];   % E2, A2, D3, G3, B3, E4 
  fretpitches=fretpitches(end:-1:1);  % String 1 is lowest
  for i=1:length(snap.hypo)
    ph=find([prevsnap.hypo.id]==snap.hypo(i).id);
    if ~isempty(ph)
      pos=snap.hypo(i).pos;
      ppos=prevsnap.hypo(ph).pos;
      rightofstring=pos(1)>stringx;
      prevros=ppos(1)>stringx;
      allplucked=find(rightofstring~=prevros);    % May be multiple
      for pl=1:length(allplucked)
        plucked=allplucked(pl);
        duration=120;
        velocity=max(5,min(127,round(norm(snap.hypo(i).velocity)/1.3*127)));
        fret=find(pos(2)>[frety,-inf],1)-1;
        pitch=fretpitches(plucked)+fret;
        fprintf('Plucked string %d at fret %d: pitch=%d, vel=%d, dur=%d\n',plucked,fret,pitch,velocity,duration);
        m{end+1}=msg('/pf/pass/playmidinote',{int32(snap.hypo(i).id),int32(pitch),int32(velocity),int32(duration)});
        if plotguitar
          setfig('guitar');
          if juststarted && i==1 && pl==1
            clf; 
            plotlayout(p.layout,0);
            hold on;
            for j=1:length(frety)
              plot([firststringx,laststringx],[frety(j),frety(j)],'k');
            end
            plot([firststringx,laststringx],[frety(1),frety(1)]+.05,'k');  % Darken top 
            for j=1:length(stringx)
              plot([stringx(j),stringx(j)],[topfrety,lastfrety],'k');
            end
          end
          plot(ppos(1),ppos(2),'o','Color',id2color(snap.hypo(i).id));
          plot([ppos(1),pos(1)],[ppos(2),pos(2)],'-','Color',id2color(snap.hypo(i).id));
          plot(pos(1),pos(2),'x','Color',id2color(snap.hypo(i).id));
        end
      end
    end
  end
end

% Send to all clients
toremove=zeros(1,length(oscsetup.clients));
if debug
  for i=1:length(m)
    fprintf('Msg: %s ',m{i}.path);
    disp(m{i}.data);
  end
end

for c=1:length(oscsetup.clients)
  cl=oscsetup.clients(c);
  ok=osc_send(cl.addr,m);
  if ~ok
    fprintf('Failed send of message to OSC target at %s - removing\n',cl.url);
    osc_free_address(cl.addr);
    toremove(c)=1;
  end
end
if any(toremove)
  oscsetup.clients=oscsetup.clients(~toremove);
end

% Clear juststarted flag
if juststarted
  juststarted=false;
end

function m=msg(path,data)
if iscell(data)
  m=struct('path',{path},'data',{data});
else
  m=struct('path',{path},'data',{{data}});
end
  
function [phase,step,pitch,pitchstep]=pos2cseq(s,pos)
[th,r]=cart2pol(pos(1),pos(2));
th=mod(th+2*pi,2*pi);
if r<s.minr
  step=nan;
  phase=nan;
else
  step=floor(s.nsteps*th/(2*pi))+1;
  phase=(step-1)/s.nsteps;
end
pitchstep=min(length(s.pitches),max(1,ceil((r-s.minr)/(s.maxr-s.minr)*(length(s.pitches)+1))));
pitch=s.pitches(pitchstep);
fprintf('pos=(%.2f,%.2f), phase=%.3f, step=%d, pitch=%d, pitchstep=%d\n', pos,phase,step,pitch,pitchstep);

function m=tsmsg(q,pitchstep,step,val)
tsstep=int32(round(((step-1)*q.touchsteps/q.nsteps)+1));
tspstep=int32(min(q.touchpitches,max(1,round(pitchstep+(length(q.pitches)-q.touchpitches)/2))));
m=msg('/touchosc/seq',{tspstep,tsstep,int32(val)});
