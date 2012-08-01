% Update MAX with a new set of locations for the targets
function ok=oscupdate(p,sampnum,snap,prevsnap)
global oscsetup;

apps={'guitar','lcd'};   % Can be any of guitar,lcd,seq

if isempty(oscsetup)
  % Need to init OSC
  oscinit;
end

% Check for incoming messages to server
dodump=false;
while true
  % Non-blocking receive
  msgin=osc_recv(oscsetup.server,0.0);
  if isempty(msgin)
    % No message 
    break;
  end
  for i=1:length(msgin)
    m=msgin{i};
    fprintf('Got message %s\n', m.path);
    if strcmp(m.path,'/pf/adddest')
      host=m.data{1}; port=m.data{2};
      addr=osc_new_address(host,port);
      oscsetup.clients=[oscsetup.clients, struct('host',m.data{1},'port',m.data{2},'addr',addr)];
      fprintf('Added OSC client at %s:%d\n', host, port);
      dodump=true;   % Always start with a dump
    elseif strcmp(m.path,'/pf/rmdest')
      clients=oscsetup.clients;
      toremove=zeros(1,length(clients));
      for c=1:length(clients)
        if strcmp(clients(c).host,m.data{1}) && clients(c).port==m.data{2}
          toremove(c)=1;
          osc_free_address(clients(c).addr);
        end
      end
      if ~any(toremove)
        fprintf('Unable to remove OSC destination %s:%d - not found\n', m.data{1},m.data{2});
      else
        fprintf('Removed %d OSC clients at %s:%d\n',sum(toremove),clients(c).host,clients(c).port);
        oscsetup.clients=clients(~toremove);
      end
    elseif strcmp(m.path,'/pf/dump')
      dodump=true;
    else
      fprintf('Unknown OSC message: %s\n', m.path);
    end
  end
end

% Setup client messages
m={};

% Just began
global firstpluck
running=true;
if sampnum==1
  m{end+1}=msg('/pf/started',{});
  oscsetup.starttime=snap.when;
  firstpluck=true;
elseif nargin<3
  m{end+1}=msg('/pf/stopped',{});
  running=false;
  firstpluck=false;
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
  
  % Compute entries, exits
  if nargin<4
    % No prev, assume everything is an entry
    entries=[snap.hypo.id];
    exits=[];
  else
    % Check for entries, exits
    entries=setdiff([snap.hypo.id],[prevsnap.hypo.id]);
    exits=setdiff([prevsnap.hypo.id],[snap.hypo.id]);
  end
  for i=1:length(entries)
    m{end+1}=msg('/pf/entry',{int32(sampnum),elapsed,int32(entries(i))});
  end
  for i=1:length(exits)
    m{end+1}=msg('/pf/exit',{int32(sampnum),elapsed,int32(exits(i))});
  end
  
  m{end+1}=msg('/pf/set/npeople',int32(length(snap.hypo)));
  for i=1:length(snap.hypo)
    h=snap.hypo(i);
    % Compute groupid -- lowest id in group, or 0 if not in a group
    sametnum=find(h.tnum==[snap.hypo.tnum]);
    groupid=snap.hypo(min(sametnum)).id;
    m{end+1}=msg('/pf/update',{int32(sampnum), elapsed,int32(h.id),h.pos(1),h.pos(2),h.velocity(1),h.velocity(2),h.majoraxislength,h.minoraxislength,int32(groupid),int32(length(sametnum))});
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

% Update step sequencer
if ismember('seq',apps)
  if running
    nsteps=8;
    rng=[-2,2];
    stepsize=diff(rng)/(nsteps-1);
    stepedges=rng(1)+(0:nsteps-2)*stepsize;
    velocity=127;
    duration=120;

    if length(snap.hypo)>0
      pitches=[]; step=[]; channel=[];
      for i=1:length(snap.hypo)
        step(i)=max([1,find(snap.hypo(i).pos(1)>stepedges)]);
        pitches(i)=pos2pitch(snap.hypo(i).pos(2));
        channel(i)=i;
      end
      [step,ord]=sort(step);
      pitches=pitches(ord);
      channel=channel(ord);
      pv=zeros(1,nsteps);
      for i=1:nsteps
        sel=find(step==i);
        if length(sel)>1
          step(sel(2:end))=step(sel(2:end))+1;
          sel=find(step==i);
        end
        if isempty(sel)
          pi=0;
          ch=0;
        else
          pi=pitches(sel);
          ch=channel(sel);
        end
        
        if pi>0
          m{end+1}=msg('/pf/pass/seq',{'step',int32(i),int32(pi), int32(velocity), int32(duration), int32(ch)});
        else
          m{end+1}=msg('/pf/pass/seq',{'step',int32(i),int32(pi), int32(0), int32(duration), int32(ch)});
        end
        if pi>0
          fprintf('step %d: pitch=%d\n', i, pi);
        end
      end
      disprange=[min([pitches(pitches>0)-4,36]),max([pitches+4,86])];
      m{end+1}=msg('/pf/pass/seq',{'zoom',int32(disprange(1)),int32(disprange(2))});
      m{end+1}=msg('/pf/pass/seq',{'loop',int32(1),int32(nsteps)});
      m{end+1}=msg('/pf/pass/seq',{'active',int32(1)});
    else
      m{end+1}=msg('/pf/pass/seq',{'active',int32(0)});
    end
  else
    % Not running
    m{end+1}=msg('/pf/pass/seq',{'active',int32(0)});
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
        fret=min(find(pos(2)>[frety,-inf]))-1;
        pitch=fretpitches(plucked)+fret;
        fprintf('Plucked string %d at fret %d: pitch=%d, vel=%d, dur=%d\n',plucked,fret,pitch,velocity,duration);
        m{end+1}=msg('/pf/pass/guitar',{int32(snap.hypo(i).id),int32(pitch),int32(velocity),int32(duration)});
        setfig('guitar');
        if firstpluck
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
          firstpluck=false;
        end
        plot(ppos(1),ppos(2),'o','Color',id2color(snap.hypo(i).id));
        plot([ppos(1),pos(1)],[ppos(2),pos(2)],'-','Color',id2color(snap.hypo(i).id));
        plot(pos(1),pos(2),'x','Color',id2color(snap.hypo(i).id));
      end
    end
  end
end

% Send to all clients
for c=1:length(oscsetup.clients)
  cl=oscsetup.clients(c);
  ok=osc_send(cl.addr,m);
  if ~ok
    fprintf('Failed send of message to OSC target at %s:%d\n',cl.host,cl.host);
  end
end

% Map a position in meters to a MIDI pitch val
function pitch=pos2pitch(pos)
pitch=int32(60+pos*10);   % 10 notes/meter centered at middle-C

function m=msg(path,data)
if iscell(data)
  m=struct('path',{path},'data',{data});
else
  m=struct('path',{path},'data',{{data}});
end
  
