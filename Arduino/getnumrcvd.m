function n=getnumrcvd(s1)
n=-1;
awrite(s1,'N');
[resp,cnt]=portread(s1,3);
if cnt~=3
  fprintf('getnumrcvd: Received %d (%s) bytes in response to N command instead of 3\n', cnt, sprintf('%02x ',resp(1:cnt)));
elseif resp(1)~='N'
  fprintf('getnumrcvd: Received 0x%02x instead of "N"\n', resp(1));
else
  n=double(resp(2))*256+double(resp(3));
end
