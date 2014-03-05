% Receiver of data from frontend (C++) that sends data via OSC
function vis=sickrcvr(varargin)
defaults=struct('flush',false,'nowait',false,'debug',false,'skipstale',true,'timeout',1.0,'nsick',1);
args=processargs(defaults,varargin);

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
    filled=false(1,args.nsick);
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
          vis.when=max(vis.acquired);
          vis.whenrcvd=now;
          if args.skipstale
            peek=oscmsgin('MPV',0.0,true);   % Peak at next messages
            havemore=false;
            for i=1:length(peek)
              if strcmp(peek{i}.path,'/vis/endframe')
                %fprintf('Skipping visframe %d since another /vis/endframe is queued\n', frame);
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
    elseif strcmp(m.path,'/vis/range')  || strcmp(m.path,'/vis/reflect')
      c=m.data{1}+1;   %  Sensor
      if filled(c)
        if vis.cframe(c)~=m.data{2} || vis.nmeasure~=m.data{6}
          fprintf('Different parameters from previously received frame for sensor %d (prior frame %d, cur frame %d)\n', c, vis.cframe(c), m.data{2});
        end
      end
      vis.cframe(c)=m.data{2};
      sec=m.data{3};
      usec=m.data{4};
      echo=m.data{5}+1;
      vis.echo(echo)=1;
      vis.nmeasure=m.data{6};
      vis.angle=(-95+(0:vis.nmeasure-1)/(vis.nmeasure-1)*190)*pi/180;
      blob=m.data{7};
      if length(blob)~=vis.nmeasure*4
        error('/vis/range message has incorrect number of measurements: expected %d, got %d\n', vis.nmeasure,length(blob)/4);
      end
      bptr=1;
      blob=uint32(blob);
      res=((blob(4:4:end)*256+blob(3:4:end))*256+blob(2:4:end))*256+blob(1:4:end);
      if strcmp(m.path,'/vis/range')
        vis.range(c,echo,:)=double(res)/1000;
      elseif strcmp(m.path,'/vis/reflect')
        vis.reflect(c,echo,:)=res;
      end
      filled(c)=true;
      vis.frame=frame;
      vis.acquired(c)=(((sec+usec/1e6)/3600-8)/24)+datenum(1970,1,1);   % Convert to matlab datenum (assuming 7 hours offset from GMT)
      if args.debug
        fprintf('Got visframe %d, camera %d with total latency of %.3f sec\n', frame, c, (now-vis.acquired(c))*24*3600);
      end
      if args.debug
        fprintf('Got %s blob for frame %d, camera %d: cnt=%d/%d/%d\n', m.path, frame, c, sum(blob==0), sum(blob==1), sum(blob==2));
      end
    end
  end
end
