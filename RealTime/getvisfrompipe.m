% Get vis struct from pipe
function [vis,fnum]=getvisfrompipe(p,randseed)
global pipefd
if isempty(pipefd) 
  filename='/tmp/pulsefield.pipe';
  fprintf('Opening %s\n', filename);
  pipefd=fopen(filename,'r');
end

magic_hdr='VISD';
magic_frm='FRME';
newhdr=false;

while true
  [hdr,cnt]=piperead(pipefd,4,'uint8=>char');
  hdr=hdr';
  if ~strcmp(hdr,magic_hdr) && ~strcmp(hdr,magic_frm)
    fprintf('Bad header.  Expected %s or %s, got "%s"\n', magic_hdr, magic_frm, hdr);
    fprintf('Searching for header...');
    pos=1;
    fpos=1;
    seekcnt=0;
    while pos<=length(magic_hdr) && fpos<=length(magic_frm)
      ch=piperead(pipefd,1,'*uint8');
      if ch==magic_hdr(pos)
        pos=pos+1;
      elseif ch==magic_hdr(1)
        pos=2;
      else
        pos=1;
      end
      if ch==magic_frm(fpos)
        fpos=fpos+1;
      elseif ch==magic_frm(1)
        fpos=2;
      else
        fpos=1;
      end
      seekcnt=seekcnt+1;
    end
    if pos>length(magic_hdr)
      hdr=magic_hdr;
    else
      hdr=magic_frm;
    end
    fprintf('synced to header "%s" after skipping %d bytes\n',hdr,seekcnt);
  end

  if strcmp(hdr,magic_hdr)
    vstring=piperead(pipefd,8,'uint8=>char');
    if ~strncmp(vstring(1:4),'-VER',4)
      error('Expected "-VERnnnn", found "%s"\n', vstring);
    end
    version=sscanf(vstring(5:end),'%d');
    checkseed=piperead(pipefd,1,'*double');
    fprintf('\nGot new data file header: version %d, seed=%f, ', version,checkseed);
    if checkseed~=randseed
      fprintf('***WARNING*** Stream has seed %f (expected %f), ', checkseed, randseed);
    end
    ncamera=piperead(pipefd,1,'uint16');
    nled=piperead(pipefd,1,'uint16');
    fprintf('%d cameras, %d leds', ncamera, nled);
    if length(p.camera)~=ncamera
      error('Expected %d cameras, got %d from pipe\n', length(p.camera),ncamera);
    end
    if length(p.led)~=nled
      error('Expected %d leds, got %d from pipe\n', length(p.led),nled);
    end
    fprintf('\n');
    newhdr=true;
  elseif strcmp(hdr,magic_frm)
    fnum=piperead(pipefd,1,'uint32');
    when=piperead(pipefd,1,'double');
    fprintf('Loading: %d@%.2f ', fnum, (now-when)*24*3600);
    ncamera=length(p.camera);
    nled=length(p.led);
    [binary,cnt]=piperead(pipefd,ncamera*nled,'*uint8');
    if cnt~=ncamera*nled
      error('Only read %d/%d data points\n', cnt, ncamera*nled);
    end
    %fprintf('Read %d data points\n', cnt);
    latency=(now-when)*24*3600;
    if latency > 0.2
      fprintf('*');
      % Skip the frame
    else
      vis=struct('v',double(reshape(binary,ncamera,nled)),'when',when,'frame',fnum);
      vis.v(vis.v==2)=nan;  % Sent as 2 over wire
      fprintf('\n');
      return;
    end
  end
end

function [d,cnt]=piperead(pipefd,n,t)
try 
  [d,cnt]=fread(pipefd,n,t);
catch me
  fclose(pipefd);   % Just in case
  error('Error reading from pipe: %s\n', me.message);
end
