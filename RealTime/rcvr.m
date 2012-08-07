% Receiver of data from frontend (C++) that sends data via OSC
function vis=rcvr(p,varargin)
global visserver;
[frontendhost,frontendport]=getsubsysaddr('FE');

defaults=struct('init',false, 'stats',false,'flush',false,'nowait',false,'debug',false);
args=processargs(defaults,varargin);

if args.init
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
  sendmsg(addr,'/vis/start',{});   % (re)start it
  osc_free_address(addr);
end


if args.stats
  % Request stats from frontend
  rcvr(p,'flush');   % Flush any queued data
  addr=osc_new_address(frontendhost,frontendport);
  fprintf('Sending request for stats\n');
  sendmsg(addr,'/vis/get/corr',{int32(1)});
  sendmsg(addr,'/vis/get/refimage',{int32(1)});
  sendmsg(addr,'/vis/get/image',{int32(1)});
  %  osc_send(addr,struct('path',{'/vis/get/corr','/vis/get/refimage','/vis/get/image'},'data',{{int32(1)},{int32(1)},{int32(1)}}));
  osc_free_address(addr);
  % Now, go ahead and read next frames until we get one that has the requested fields
  % Might be a few without these fields
  % Could also be a race condition where only some of the above fields are filled, and the rest are in the next frame
  for i=1:3
    vis=rcvr(p)
    if isfield(vis,'im')
      if ~isfield(vis,'refim') || ~isfield(vis,'corr')
        break;
      end
      return;
    end
  end
  fprintf('Stats did not result in a vis struct that contained corr,refimage,image\n');
  return;
end

if isempty(visserver)
  fprintf('Not connected to server; use startfrontend()\n');
  return;
end

vis=[];
frame=-1;
while true
  if args.nowait || args.flush
    m=getnextfrontendmsg(0.0);
  else
    m=getnextfrontendmsg(1.0);
  end
  if isempty(m)
    if args.flush
      fprintf('Buffers flushed, now looking for vis\n');
      args.flush=false;  % Now continue normally
      frame=-1;
      continue;
    end
    % Nothing available
    if frame==-1 || ~any(filled)
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
  if strcmp(m.path,'/vis/beginframe')
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
        vis.frame=frame;
        vis.acquired(c)=(((sec+usec/1e6)/3600-7)/24)+datenum(1970,1,1);   % Convert to matlab datenum (assuming 7 hours offset from GMT)
        if args.debug
          fprintf('Got frame %d, camera %d with total latency of %.3f sec\n', frame, c, (now-vis.acquired(c))*24*3600);
        end
      else
        blob=m.data{5};
        if length(blob)~=size(vis.v,2)*4
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
