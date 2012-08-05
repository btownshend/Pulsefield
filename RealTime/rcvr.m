% Receiver of data from frontend (C++) that sends data via OSC
function vis=rcvr(p,varargin)
global visserver;
frontendport=7770;
frontendhost='localhost';
myhost='localhost';
myport=7773;

defaults=struct('init',false,'connect', false, 'stats',false,'flush',false,'nowait',false,'debug',false,'sendquit',false);
args=processargs(defaults,varargin);

if args.init
  % Start frontend
  cmd=sprintf('../FrontEnd/frontend >/tmp/frontend.log 2>&1 & sleep 1; cat /tmp/frontend.log; ');
  fprintf('Running cmd: "%s"\n', cmd);
  [s,r]=system(cmd);
  fprintf('%s\n',r);

  % Initialize
  addr=osc_new_address(frontendhost,frontendport);
  sendmsg(addr,'/vis/stop',{});
  sendmsg(addr,'/vis/set/fps',{p.analysisparams.fps});
  sendmsg(addr,'/vis/set/corrthresh',{0.5});
  for i=1:length(p.camera)
    c=p.camera(i);
    if strcmp(c.type,'av10115')
      res='full';
    elseif strcmp(c.type,'av10115-half')
      res='half';
    else
      fprintf('Unable to determine resolution of %s: assuming full res\n', c.type);
      res='full';
    end
    sendmsg(addr,'/vis/set/res',{int32(i-1),res});
    if isfield(p.camera(i),'viscache') && isfield(p.camera(i).viscache,'refim')
      sendmsg(addr,'/vis/set/updatetc',{ 0.0 });    % Turn off updates of reference image
      refim = im2single(p.camera(i).viscache.refim);
      filename=sprintf('/tmp/refim_%d.raw',i);
      fd=fopen(filename,'wb');
      fwrite(fd,permute(refim,[3,2,1]),'single');  % Write it in normal RGB order 
      fclose(fd);
      fprintf('Sending reference image to frontend in %s\n',filename);
      sendmsg(addr,'/vis/set/refimage',{ int32(i-1), size(refim,2), size(refim,1), size(refim,3), filename});
    else
      fprintf('Frontend is automatically updating reference with a time constant of %f seconds\n', p.analysisparams.updatetc);
      sendmsg(addr,'/vis/set/updatetc',{ p.analysisparams.updatetc });
    end
      
    % setRoi expects x0, y0, x1, y1 (with 1-based indexing)
    sendmsg(addr,'/vis/set/roi',{int32(i-1),int32(c.roi(1)),int32(c.roi(3)),int32(c.roi(2)),int32(c.roi(4))});
    
    for j=1:size(c.viscache.tlpos,1)
      tlpos=c.viscache.tlpos(j,:);
      brpos=c.viscache.brpos(j,:);
      if isnan(tlpos(1))
        sendmsg(addr,'/vis/set/pos',{int32(i-1),int32(j-1),int32(-1),int32(-1),int32(-1),int32(-1)});
      else
        sendmsg(addr,'/vis/set/pos',{int32(i-1),int32(j-1),int32(tlpos(1)-1),int32(tlpos(2)-1),int32(brpos(1)-tlpos(1)+1),int32(brpos(2)-tlpos(2)+1)});   % On the wire uses 0-based indexing
      end
    end
    pause(1);   % Try not to overrun UDP stack
  end
  osc_free_address(addr);
end

if args.connect
  fprintf('Connecting to %s:%d from %s:%d\n', frontendhost, frontendport, myhost, myport);
  addr=osc_new_address(frontendhost,frontendport);
  sendmsg(addr,'/vis/dest/add',{myhost,myport});

  % Start a local server to receive vis messages
  port=myport;
  if ~isempty(visserver)
    fprintf('Freeing old OSC server at port %d. ', port);
    osc_free_server(visserver);
  end
    
  visserver=osc_new_server(port);
  fprintf('Started new OSC server at port %d\n', port);
  sendmsg(addr,'/vis/start',{});
  fprintf('Started front end;  waiting for /started reply...');
  osc_free_address(addr);
  args.flush=false;   % This is not supported below;  if connect is set, will flush up until /vis/started is received anyway
  args.nowait=false;  % Wait for acknowledge
end

if args.stats
  % Request stats from frontend
  rcvr(p,'flush');   % Flush any queued data
  addr=osc_new_address(frontendhost,frontendport);
  sendmsg(addr,'/vis/get/corr',{});
  sendmsg(addr,'/vis/get/refimage',{});
  sendmsg(addr,'/vis/get/image',{});
  osc_free_address(addr);
  % Now, go ahead and read next frame
  % Could have a race though and next frame would not have the results (but a later one would)
end

if args.sendquit
  addr=osc_new_address(frontendhost,frontendport);
  sendmsg(addr,'/quit',{});
  osc_free_address(addr);
end

if isempty(visserver)
  fprintf('Not connected to server; use rcvr(p,''connect'')\n');
  return;
end

global msgin;

vis=[];
frame=-1;
while true
  if isempty(msgin)
    % Get some more messages, timeout after 100msec
    if args.nowait || args.flush
      msgin=osc_recv(visserver,0.0);
    else
      msgin=osc_recv(visserver,1);
    end
  end
  if isempty(msgin)
    if args.flush
      fprintf('Buffers flushed, now looking for vis\n');
      args.flush=false;  % Now continue normally
      continue;
    end
    % Nothing available
    if args.connect
      error('No /started acknowledge received');
    elseif frame==-1 || ~any(filled)
      if args.debug
        fprintf('No frame available\n');
      end
    else
      if args.debug
        fprintf('Partial frame\n');
      end
    end
    break;
  end
  if iscell(msgin)
    m=msgin{1};
    msgin=msgin(2:end);
  else
    m=msgin;
    msgin=[];
  end
  if args.debug
    fprintf('Have %d messages, next is %s\n', length(msgin)+1, m.path);
  end
  if args.connect 
    % Only waiting for started or stopped messages
    if strcmp(m.path,'/vis/started')
      fprintf('Got acknowledge from frontend that it has started\n');
      args.connect = false;   % No longer waiting for start acknowledge
    elseif strcmp(m.path,'/vis/stopped')
      error('Front end has stopped -- need to re-init\n');
      % TODO - handle this somehow
    end
  elseif strcmp(m.path,'/vis/beginframe')
    if frame~=-1
      fprintf('Missed /endframe messages, skipping to next frame\n');
    end
    frame=m.data{1};
    if args.debug
      fprintf('New frame %d\n', frame);
    end
    filled=false(1,length(p.camera));
  elseif frame==-1
    fprintf('Skipping message %s while looking for /vis/beginframe\n', m.path);
  else
    if strcmp(m.path,'/vis/endframe')
      if frame ~= m.data{1}
        fprintf('Got %s for frame %d while reading frame %d\n', m.path, m.data{1}, frame);
      end
      if all(filled)
        if args.flush 
          frame=-1;   % Continue and look for another one
          vis=[];
          fprintf('Skipping frame to flush buffers\n');
        else
          vis.when=max(vis.acquired);
          break;  % Done
        end
      else
        fprintf('End of frame without having received data from cameras %s, waiting for next frame\n', shortlist(find(~filled)));
        vis=[];
      end
    elseif strcmp(m.path,'/vis/visible')  || strcmp(m.path,'/vis/corr')
      c=m.data{1}+1;   % Camera
      if frame ~= m.data{2}
        fprintf('Got %s for frame %d while reading frame %d\n', m.path, m.data{2}, frame);
      end
      sec=m.data{3};
      usec=m.data{4};
      if strcmp(m.path,'/vis/visible')
        vis.mincorr=m.data{5};
        blob=m.data{6};
        if length(blob)~=length(p.led)
          error('/vis/visible message has incorrect number of LEDS: expected %d, got %d\n', length(p.led),length(blob));
        end
        vis.v(c,:)=double(blob);
        vis.v(c,blob==2)=nan;
        % Turn off indicators for LEDs we're not using
        vis.v(c,~p.camera(c).viscache.inuse)=nan;
        filled(c)=true;
        vis.frame(c)=frame;
        vis.acquired(c)=(((sec+usec/1e6)/3600-7)/24)+datenum(1970,1,1);   % Convert to matlab datenum (assuming 7 hours offset from GMT)
        if args.debug
          fprintf('Got frame %d, camera %d with total latency of %.3f sec\n', frame, c, (now-vis.acquired(c))*24*3600);
        end
      else
        blob=m.data{5};
        if length(blob)~=size(vis.v,2)
          error('/vis/corr message has incorrect number of LEDS: expected %d, got %d\n', size(vis.v,2),length(blob));
        end
        vis.corr(c,:)=typecast(blob,'SINGLE');
      end
      if args.debug
        fprintf('Got %s blob for frame %d, camera %d: cnt=%d/%d/%d\n', m.path, frame, c, sum(blob==0), sum(blob==1), sum(blob==2));
      end
    elseif strcmp(m.path,'/vis/image') || strcmp(m.path,'/vis/refimage')  || strcmp(m.path,'/vis/frame')
      c=m.data{1}+1;   % Camera
      if frame ~= m.data{2}
        fprintf('Got %s for frame %d while reading frame %d\n', m.path, m.data{2}, frame);
      end
      sec=m.data{3};
      usec=m.data{4};
      width=m.data{5};
      height=m.data{6};
      depth=m.data{7};
      type=m.data{8};
      filename=m.data{9};
      if args.debug
        fprintf('Loading image from %s\n', filename);
      end
      if strcmp(m.path,'/vis/frame')
        vis.frame{c}=imread(filename);
      else
        fd=fopen(filename,'r');
        assert(type(1)=='b' || type(1)=='f');
        if type(1)=='b'
          rawimage=fread(fd,'*uint8');
        elseif type(1)=='f'
          rawimage=fread(fd,'*float');
        else
          fprintf('Bad type for %s: %s\n', m.path, type);
        end
        rawimage=permute(reshape(rawimage,depth,width,height),[3 2 1]);
        fclose(fd);
        
        if strcmp(m.path,'/vis/image')
          vis.im{c}=rawimage;
        else  % /vis/refimage
          vis.refim{c}=rawimage;
        end
      end
    end
  end
end

function sendmsg(addr,path,data)
m=struct('path',path,'data',{data});
ok=osc_send(addr,m);
if ~ok
  error('Failed send of setup data to VIS frontend\n');
end
