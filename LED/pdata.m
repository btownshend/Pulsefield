% Power data for WS2811 300 LED string
function pdata()
% Vf of LED's is 2.1V @ maximum current; WS2811 low output voltage is 0.4V, so total drop is 2.1+0.4
% Vf for LEDs is hv (planck's constant * frequencey)
  Vf=4.136e-15*3e8./([620,515,460]*1e-9);
  
% RIn is power supply + cabling effective resistance
  
  lvl=127;
  % Lvl, numleds, Iin, Vin, Vout
  data=[lvl 0 .274 4.971 4.779
        lvl 1 .296 4.966 4.773
        lvl 2 .317 4.960 4.766
        lvl 4 .361 4.950 4.751
        lvl 8 .448 4.929 4.721
        lvl 16 .620 4.888 4.656
        lvl 32 .941 4.808 4.513
        lvl 64 1.460 4.670 4.210
        lvl 128 2.065 4.501 3.660
        lvl 256 2.452 4.387 2.965
        lvl 299 2.491 4.374 2.817
        0 299 nan 4.970 4.783
         1 299 nan 4.969 4.777
         2 299 nan 4.966 4.764
         4 299 nan 4.959 4.738
         8 299 nan 4.946 4.687
         16 299 nan 4.896 4.499
         32 299 nan 4.754 3.977
         64 299 nan 4.559 3.332
         128 299 nan 4.369 2.815
         255 299 nan 4.190 2.451];
         
         
  d=struct('level',data(:,1),'first',0,'nled',data(:,2),'iin',data(:,3), 'vin', data(:,4), 'vout', data(:,5));
  
  model=struct('Double',true,'NLed',45,'RPerSegment',.005,'Vf',Vf,'IMax',.0185*3);
  model.RLed=(5-model.Vf)/model.IMax*3;

  fit=polyfit(d.iin(d.level==lvl),d.vin(d.level==lvl),1);
  model.RIn=-fit(1);
  model.VIn=fit(2);
  model.IOff=d.iin(2)/model.NLed;

  % Additional 0.112 ohms due to pigtail
  %model.RIn=model.RIn+.112;
  
  % Test model with a close power supply delivering 6V
  %  model.RIn=.112;
  model.VIn=6;
  model.RIn=.05;
  
  model
  
  % Test model
  testmodel=model;
  testmodel.NLed=3;
  stripmodel(testmodel,[255,255,0],1);
  
  vlow=nan(1,length(d.level));
  iin=vlow;
  vin=vlow;
  for i=1:model.NLed;
    levels=zeros(1,model.NLed);
    levels(1:i)=lvl;
    doplot=(i==1 || i==model.NLed || i==128);
    [vlow(i),vin(i),iin(i)]=stripmodel(model,levels,doplot);
  end
  m=struct('level',lvl*ones(1,model.NLed),'first',zeros(1,model.NLed),'nled',1:model.NLed,'iin',iin,'vin',vin,'vlow',vlow);

  vlow=nan(1,256);
  iin=vlow;
  vin=vlow;
  for i=1:256
    levels=(i-1)*ones(1,model.NLed);
    doplot=ismember(i,[1,128,256]);
    [vlow(i),vin(i),iin(i)]=stripmodel(model,levels,doplot);
  end
  m2=struct('level',0:255,'first',zeros(1,256),'nled',model.NLed*ones(1,256),'iin',iin,'vin',vin,'vlow',vlow);
  
  vdrop=d.vin-d.vout;

  figure(1);clf;
  plot(d.nled,d.iin*1000,'o');
  xlabel('NLED');
  ylabel('Current (mA)');
  hold on;
  plot(m.nled,(m.iin*1000)','-g');
  legend('Observed','Model(led 0..n)')
  
  figure(2);clf;
  sel=isfinite(vdrop);
  plot(d.iin(sel)*1000,vdrop(sel),'o');
  xlabel('Current (mA)');
  ylabel('VDrop (V)');
  hold on;
  plot(m.iin*1000,m.vin-m.vlow,'-g');
  plot(m2.iin*1000,m2.vin-m2.vlow,'-r');
  legend('Observed','Model(led 0..n)','Model (level 0..255)')

  figure(3);clf;
  plot(d.iin*1000,d.vin,'o');
  xlabel('Current (mA)');
  ylabel('Input Voltage (V)');
  hold on;
  plot(m.iin*1000,m.vin,'-g');
  plot(m2.iin*1000,m2.vin,'-r');
  legend('Observed','Model(led 0..n)','Model (level 0..255)')

  figure(4);clf;
  sel=d.nled==max(d.nled);
  plot(d.level(sel),d.iin(sel),'o');
  xlabel('Level');
  ylabel('Current (mA)');
  hold on;
  plot(m2.level,m2.iin,'-r');
  plot(m2.level,m2.level/255*model.IMax*model.NLed,':k');
  legend('Observed','Model','Maximum');
end


