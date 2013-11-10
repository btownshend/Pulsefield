s1=arduino_ip();
while true
  awrite(s1,'G');
  [resp,cnt]=portread(s1,1);
  if cnt>0
    fprintf('Got: <%s>\n',resp);
  end
end
