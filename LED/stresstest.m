% Stress ethernet I/O
s1=arduino_ip(1);
for k=1:20
  sync(s1);
end
nsend=250;
nrddata=200;

data=1:nsend;
ncopy=3;
rpt=50;
for j=1:ncopy
  awrite(s1,['E',uint8(nsend),uint8(nrddata),uint8(data)]);
end
fprintf('Testing with sending %d bytes to Arduino, receiving %d bytes\n', nsend+3, nrddata);

while true
  tic
  for k=1:rpt
    awrite(s1,['E',uint8(nsend),uint8(nrddata),uint8(data)]);
    readback=portread(s1,nrddata);
    ncmp=min(nsend,nrddata);
    mismatch=find(data(1:ncmp)~=readback(1:ncmp));
    if ~isempty(mismatch)
      i=mismatch(1);
      error('Data mismatch: data(%d)=0x%02x, readback=0x%02x\n', i, data(i), readback(i));
    end
  end
  tm=toc;
  fprintf('Total (%dR+%dW): %.0f bytes/sec\n',nrddata,nsend+3, (nsend+3+nrddata)*rpt/tm);
  pause(rand);
end
