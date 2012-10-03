main=zeros(699,3);
left=zeros(129,3);
main(:,1)=1;
main(:,2)=floor((1:699)/10);
main(:,3)=(1:699)'-main(:,2)*10;
left(:,1)=51;
left(:,2)=floor((1:129)/10);
left(:,3)=(1:129)'-left(:,2)*10;
right=left;
right(:,1)=101;
[cmd,mcolor]=setallleds(s1,{main,left,right});
show(s1);
plot(mcolor);

