function n=getnumrcvd(s1)
n=-1;
awrite(s1,'N');
[resp,cnt]=portread(s1,5);
if cnt~=5
  fprintf('getnumrcvd: Received %d (%s) bytes in response to N command instead of 5\n', cnt, sprintf('%02x ',resp(1:cnt)));
elseif resp(1)~='N'
  fprintf('getnumrcvd: Received 0x%02x instead of "N"\n', resp(1));
else
  rr=double(resp(2:5));
  n=((rr(1)*256+rr(2))*256+rr(3))*256+rr(4);
end
