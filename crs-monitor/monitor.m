fcnt=0;
body=containers.Map('KeyType','int32','ValueType','any');  % indexed by uid, frame
facing=containers.Map('KeyType','char','ValueType','any');  % indexed by 'uid1-uid2',frame
fprintf('Flushing...');
while ~isempty(oscmsgin('REC',0.0))
end
fprintf('ready\n');
while true
  m=oscmsgin('REC',1.0);
  if isempty(m) 
    if fcnt>0
      break;
    end
    continue;
  end
  if strcmp(m.path,'/pf/frame')
    frame=m.data{1};
  elseif strcmp(m.path,'/pf/body')
    if ~isKey(body,m.data{2})
      body(m.data{2})=containers.Map('KeyType','int32','ValueType','any');
    end
    map=body(m.data{2});
    map(m.data{1})=m.data;
    body(m.data{2})=map;
  elseif strcmp(m.path,'/conductor/conx') & strcmp(m.data{2},'facing')
    if ~isKey(facing,m.data{3})
      facing(m.data{3})=containers.Map('KeyType','int32','ValueType','any');
    end
    f=facing(m.data{3});
    f(frame)=m.data;
    facing(m.data{3})=f;
    fcnt=fcnt+1;
    fprintf('.');
  end
end
fprintf('\n');

  