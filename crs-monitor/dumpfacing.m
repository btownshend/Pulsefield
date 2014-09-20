fkeys=facing.keys;
buids=body.keys
minframe=[]; maxframe=[];
for i=1:length(buids)
  bperson=body(buids{i});
  bkeys=bperson.keys;
  for k=1:length(bkeys)
    d=bperson(bkeys{k});
    frame=d{1};
    minframe=min([minframe,frame]);
    maxframe=max([maxframe,frame]);
  end
end
fprintf('Frames %d-%d\n', minframe, maxframe);

for ik=1:length(fkeys)
  uids=fkeys{ik};
  fprintf('%s: ', uids);
  fa=facing(uids);
  frames=fa.keys;
  setfig(uids);clf; 
  subplot(313);
  for j=1:length(frames)
    d=fa(frames{j});
    fprintf('%d:%.2f,%.2f ',frames{j},d{6},d{7});
    plot(frames{j},d{6});
    hold on;
  end
  c=axis;axis([minframe,maxframe,0,c(4)]);
  fprintf('\n');
  uid=sscanf(uids,'%d-%d');
  for j=1:length(uid)
    subplot(3,1,j);
    bperson=body(uid(j));
    bkeys=bperson.keys;
    for ib=1:length(bkeys)
      bb=bperson(bkeys{ib});
      plot(bb{1},bb{11});
      hold on;
    end
    c=axis;axis([minframe,maxframe,-180,180]);
  end
end
