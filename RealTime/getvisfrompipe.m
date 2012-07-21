% Get vis struct from pipe
function vis=getvisfrompipe(p,randseed)
global pipefd pipestatus
if isempty(pipefd) 
  filename='/tmp/pulsefield.pipe';
  fprintf('Opening %s\n', filename);
  pipefd=fopen(filename,'r');
  pipestatus=0;
end
if isempty(pipestatus)
  pipestatus=0;
end

if pipestatus==0
  % Waiting for main header
  magic='VISD';
  try
    hdr=fread(pipefd,4,'uint8=>char')';
  catch me
    pipefd=[];
    error('Error reading from pipe: %s\n', me.message);
    fclose(pipefd);   % Just in case
  end
  if ~strcmp(hdr,magic)
    fprintf('Bad header.  Expected "%s", got "%s"\n', magic,hdr);
    fprintf('Searching for header...');
    pos=1;
    seekcnt=0;
    while pos<=length(magic)
      ch=fread(pipefd,1,'*uint8');
      if ch==magic(pos)
        pos=pos+1;
      elseif ch==magic(1)
        pos=2;
      else
        pos=1;
      end
      seekcnt=seekcnt+1;
    end
    fprintf('synced to header after skipping %d bytes\n',seekcnt);
  end
  vstring=fread(pipefd,8,'uint8=>char');
  if ~strncmp(vstring(1:4),'-VER',4)
    error('Expected "-VERnnnn", found "%s"\n', vstring);
  end
  version=sscanf(vstring(5:end),'%d');
  fprintf('Data file version %d\n', version);
  checkseed=fread(pipefd,1,'*double');
  if checkseed~=randseed
    fprintf('Warning: stream has seed %f (expected %f)\n', checkseed, randseed);
  end
  ncamera=fread(pipefd,1,'uint16');
  nled=fread(pipefd,1,'uint16');
  fprintf('Have %d cameras, %d leds\n', ncamera, nled);
  if length(p.camera)~=ncamera
    error('Expected %d cameras, got %d from pipe\n', length(p.camera),ncamera);
  end
  if length(p.led)~=nled
    error('Expected %d leds, got %d from pipe\n', length(p.led),nled);
  end
  pipestatus=1;
end

% Pipestatus=1
% Waiting for next frame
while true
  [fhdr,cnt]=fread(pipefd,4,'uint8=>char');
  if cnt~=4
    pipestatus=0;
    error('Bad read while looking for frame header; cnt=%d\n',cnt);
  end
  fhdr=fhdr';
  if ~strcmp(fhdr,'FRME')
    pipestatus=0;
    error('Frame header sync failure, expected "FRME" got "%s"\n',fhdr);
  end
  fnum=fread(pipefd,1,'uint32');
  when=fread(pipefd,1,'double');
  fprintf('Loading: %d@%.2f ', fnum, (now-when)*24*3600);
  ncamera=length(p.camera);
  nled=length(p.led);
  [binary,cnt]=fread(pipefd,ncamera*nled,'*uint8');
  if cnt~=ncamera*nled
    pipestatus=0;
    error('Only read %d/%d data points\n', cnt, ncamera*nled);
  end
  %fprintf('Read %d data points\n', cnt);
  latency=(now-when)*24*3600;
  if latency > 0.2
    fprintf('*');
    % Skip the frame
  else
    break;
  end
end
fprintf('\n');
  vis=struct('v',reshape(binary,ncamera,nled),'when',when);
