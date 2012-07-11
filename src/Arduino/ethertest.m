% Test ethernet interface
jtcpobj=jtcp('REQUEST','192.168.0.154',1500,'SERIALIZE',false);
cmd=setled(0,80,[100,0,0],0);
cmd(end+1)='V';
cmd(end+1)='1';
jtcp('WRITE',jtcpobj,uint8(cmd));
pause(1)
msg=jtcp('READ',jtcpobj);
jtcpobj=jtcp('CLOSE',jtcpobj);
