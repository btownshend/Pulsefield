% Receiver of data from frontend (C++) that sends data via OSC
function vis=rcvr(p,varargin)
defaults=struct('stats',false,'flush',false,'nowait',false,'debug',false,'skipstale',true,'timeout',1.0);
args=processargs(defaults,varargin);

if args.stats
  % Request stats from frontend
  rcvr(p,'flush',true,'skipstale',false);   % Flush any queued data (turn off skipstale to avoid confusing messages)
  if args.debug
    fprintf('Sending request for stats\n');
  end
  oscmsgout('FE','/vis/get/corr',{int32(1)});
  oscmsgout('FE','/vis/get/refimage',{int32(1)});
  oscmsgout('FE','/vis/get/image',{int32(1)});

  % Now, go ahead and read next frames until we get one that has the requested fields
  % Might be a few without these fields
  % Could also be a race condition where only some of the above fields are filled, and the rest are in the next frame
  for i=1:3
    vis=rcvr(p,'skipstale',false);
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

vis=[];
frame=-1;
while true
  if args.nowait || args.flush
    m=oscmsgin('MPV',0.0);
  else
    m=oscmsgin('MPV',args.timeout);
  end
  if isempty(m)
    if args.flush
      if args.debug
        fprintf('Buffers flushed, now looking for vis\n');
      end
      args.flush=false;  % Now continue normally
      frame=-1;
      continue;
    end
    % Nothing available
    if frame==-1 || ~any(filled)
      if args.debug
        fprintf('No frame available\n');
      end
      break;   % Return empty vis[]
    end
    if args.debug
      fprintf('Partial frame\n');
    end
    % Wait a little longer to get a full frame
    if args.timeout==0.0
      args.timeout=0.001;  
    end
    continue;
  end
  if strcmp(m.path,'/vis/beginframe')
    if frame~=-1
      fprintf('Missed /vis/endframe messages, skipping to next frame\n');
    end
    frame=m.data{1};
    if args.debug
      fprintf('New frame %d\n', frame);
    end
    filled=false(1,length(p.camera));
  elseif frame==-1
    if ~strcmp(m.path,'/ack')
      fprintf('Skipping message %s while looking for /vis/beginframe\n', m.path);
    end
  else
    if strcmp(m.path,'/vis/endframe')
      if frame ~= m.data{1}
        fprintf('Got %s for frame %d while reading frame %d\n', m.path, m.data{1}, frame);
      end
      if all(filled)
        if args.flush 
          frame=-1;   % Continue and look for another one
          vis=[];
          if args.debug
            fprintf('Skipping frame to flush buffers\n');
          end
        else
          if args.skipstale
            vis.when=max(vis.acquired);
            peek=oscmsgin('MPV',0.0,true);   % Peak at next messages
            havemore=false;
            for i=1:length(peek)
              if strcmp(peek{i}.path,'/vis/endframe')
                fprintf('Skipping visframe %d since another /vis/endframe is queued\n', frame);
                havemore=true;
                break;
              end
            end
            if havemore
              frame=-1;
              vis=[];
            else
              break;  % Done
            end
          else
            break;
          end
        end
      else
        fprintf('End of frame without having received data from cameras %s, waiting for next frame\n', shortlist(find(~filled)));
        vis=[];
      end
    elseif strcmp(m.path,'/vis/visible')  || strcmp(m.path,'/vis/corr')
      c=m.data{1}+1;   % Camera
      sec=m.data{3};
      usec=m.data{4};
      if strcmp(m.path,'/vis/visible')
        vis.cframe(c)=m.data{2};
        vis.mincorr=m.data{5};
        blob=m.data{6};
        if length(blob)~=length(p.led)
          error('/vis/visible message has incorrect number of LEDS: expected %d, got %d\n', length(p.led),length(blob));
        end
        vis.v(c,:)=single(blob);
        vis.vorig(c,:)=vis.v(c,:);
        vis.v(c,blob>=2)=nan;
        % Turn off indicators for LEDs we're not using
        vis.v(c,~p.camera(c).viscache.inuse)=nan;
        filled(c)=true;
        vis.frame=frame;
        vis.acquired(c)=(((sec+usec/1e6)/3600-8)/24)+datenum(1970,1,1);   % Convert to matlab datenum (assuming 7 hours offset from GMT)
        if args.debug
          fprintf('Got visframe %d, camera %d with total latency of %.3f sec\n', frame, c, (now-vis.acquired(c))*24*3600);
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
    elseif strcmp(m.path,'/vis/image') || strcmp(m.path,'/vis/refimage')  || strcmp(m.path,'/vis/refimage2') || strcmp(m.path,'/vis/frame')
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
        for retry=1:3
          [fd,errmsg]=fopen(filename,'r');
          if fd==-1
              fprintf('Unable to open %s: %s\n', filename,errmsg);
              rawimage=[];
              break;
          end
          assert(type(1)=='b' || type(1)=='f');
          if type(1)=='b'
            rawimage=fread(fd,'*uint8');
          elseif type(1)=='f'
            rawimage=fread(fd,'*float');
          else
            fprintf('Bad type for %s: %s\n', m.path, type);
          end
          fclose(fd);
          if length(rawimage)==depth*width*height
            rawimage=permute(reshape(rawimage,depth,width,height),[3 2 1]);
            break;
          else
            % Even though the frontend app closes the file before sending the packet, seems that the filesystem introduces
            % some delay before this process sees the full file.  
            fprintf('Image read from %s has %d/%d bytes on attempt %d\n',filename,length(rawimage),depth*width*height, retry);
            if retry<3
              pause(0.2);
            else
              fprintf('Failed read of image at %s\n', filename);
              rawimage=[];
            end
          end
        end
        if strcmp(m.path,'/vis/image')
          vis.im{c}=rawimage;
        elseif strcmp(m.path,'/vis/refimage')
          vis.refim{c}=rawimage;
        elseif strcmp(m.path,'/vis/refimage2')
          vis.refim2{c}=rawimage;
        else
          fprintf('Bad message: %s with file %s\n', m.path, filename);
        end
      end
    end
  end
end
