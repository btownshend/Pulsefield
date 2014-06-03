% Analyze data in an OSC file

file='../Recordings/mah-ballroom.osc';
frame=2;
uid=3;
x=4;
y=5;
facing=12;

cmd=sprintf('grep /body %s | cut -d " " -f%d,%d,%d,%d,%d >/tmp/t1.txt',file,frame,uid,x,y,facing);
system(cmd);

x=load('/tmp/t1.txt');
fprintf('Loaded %d entries\n',size(x,1));
uids=unique(x(:,2));
col='rgbcmy';
lh={};
setfig('facing');clf;
subplot(211);
for i=1:length(uids)
  uid=uids(i);
  sel=x(:,2)==uid;
  plot(x(sel,1),mod(x(sel,5)+180,360)-180,col(mod(i-1,length(col))+1));
  hold on;
  lh{i}=sprintf('UID %d',uid);
end
legend(lh);
xlabel('Frame');
ylabel('Facing');

subplot(212);
p2=[nan,nan];p1=[nan,nan];
pnum=1;
lh={};
for i=1 % :length(uids)-1
  for j=i+1 %:length(uids)
    dir=[];
    for k=1:size(x,1)
      if x(k,2)==uids(i)
        p1=x(k,3:4);
      elseif x(k,2)==uids(j)
        p2=x(k,3:4);
      end
      rel=p2-p1;
      dir(k)=cart2pol(rel(1),rel(2));
    end
    plot(x(:,1),dir*180/pi,col(mod(pnum,length(col))+1));
    lh{pnum}=sprintf('UID %d to %d',uids(i),uids(j));
    pnum=pnum+1;
    hold on;
  end
end
legend(lh);
xlabel('Frame');
ylabel('Direction between UIDs');
