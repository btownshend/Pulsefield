% Test ethernet interface
[h,p]=getsubsysaddr('AR');
jtcpobj=jtcp('REQUEST',h,p,'SERIALIZE',false);
cmd=setled(0,80,[100,0,0],0);
cmd(end+1)='V';
cmd(end+1)='1';
jtcp('WRITE',jtcpobj,uint8(cmd));
pause(1)
msg=jtcp('READ',jtcpobj);
jtcpobj=jtcp('CLOSE',jtcpobj);
