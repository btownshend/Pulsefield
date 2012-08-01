% Dump VISD file
function visddump(filename)
if nargin<1
  filename='/tmp/pulsefield.pipe';
  fprintf('Reading from %s\n', filename);
end
magic='VISD';
fd=fopen(filename,'r');
hdr=fread(fd,4,'uint8=>char')';
if ~strcmp(hdr,magic)
  fprintf('Bad header.  Expected "%s", got "%s"\n', magic,hdr);
  fprintf('Searching for header...');
  pos=1;
  seekcnt=0;
  while pos<=length(magic)
    ch=fread(fd,1,'*uint8');
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
vstring=fread(fd,8,'uint8=>char');
if ~strncmp(vstring(1:4),'-VER',4)
  error('Expected "-VERnnnn", found "%s"\n', vstring);
end
version=sscanf(vstring(5:end),'%d');
fprintf('Data file version %d\n', version);
checkseed=fread(fd,1,'*double');
fprintf('Rand seed = %f\n', checkseed);
ncamera=fread(fd,1,'uint16');
nled=fread(fd,1,'uint16');
fprintf('Have %d cameras, %d leds\n', ncamera, nled);
while true
  [fhdr,cnt]=fread(fd,4,'uint8=>char');
  if cnt~=4
    fprintf('Bad read while looking for frame header; cnt=%d\n',cnt);
    break;
  end
  fhdr=fhdr';
  if ~strcmp(fhdr,'FRME')
    error('Frame header sync failure, expected "FRME" got "%s"\n',fhdr);
  end
  fnum=fread(fd,1,'uint32');
  when=fread(fd,1,'double');
  fprintf('Loading frame %d from %s\n', fnum, datestr(when));
  [binary,cnt]=fread(fd,ncamera*nled,'*uint8');
  if cnt~=ncamera*nled
    error('Only read %d/%d data points\n', cnt, ncamera*nled);
  end
  fprintf('Read %d data points\n', cnt);
  binary=reshape(binary,ncamera,nled);
  for c=1:ncamera
    fprintf('C%d: ',c);
    fprintf('%1d',binary(c,:));
    fprintf('\n');
  end
end
