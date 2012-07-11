% Model power usage
mode=1;

Vps=5.07;
ledstate=ones(Nled,1);
Nled=160;
clear meas;

if mode==1
  % Conditions under which data was collected for all LED's of all 4 strips
  Rps=.016*4;  % Since base data was using all 4 strips together
               % Resistance of 12 gauge wire is 1.6215 ohms/1000ft
  Lcable=16*2;  % Power and ground
  meas.PWM=[0,1,2,4,8,16,32,64,127];
  meas.Iavg=[0,1.27,4.45,5.72,10.68,18.36,28.79,40.92,51.88];
elseif mode==2
  % Driving one strip per supply with a short cable
  Rps=.016;
  Lcable=2*2;
elseif mode==3
  % Test of 50 LEDs in a single strip
  Rps=.016;
  Lcable=16*2;  % Power and ground
  ledstate(:)=0;
  ledstate(51:51+25-1)=1;
  meas.PWM=[0,1,2,4,8,16,32,64,127];
  meas.Iavg=[0,3.86,2.99,.85,14.57,16.69,26.53,58.41,92.66];
elseif mode==4
  % One LED at a time
  Rps=.016;
  Lcable=16*2;  % Power and ground
  ledstate(:)=0;
  ledstate(80:90)=1;
end

Rcable=1.6215/1000*Lcable;
% Resistance of a strip element (1 LED)
Rstrip=(4.10-3.08)/(8.3/2)/160;
Rstrip=Rstrip*1.2;  % To match data
% LED voltage drop
Vled=2.9;
% Resistance driving LED
Rled=((4.82+4.51)/2-Vled)/.09266*0.55;  % To match data
fprintf('LED driver to %.1f V through %.1f ohms equiv.\n', Vled, Rled);

% LED current in mA
PWM=0:127;
for j=1:length(PWM)
  frac=PWM(j)/127;
  % V(1..Nled) is the voltage at each LED strip position (above resistor feeding LED)
  % I(1..Nled) is the current entering each node
  % Basic equation is I(k)=(V(k)-Vled)/Rled+(V(k)-V(k+1))/Rstrip
  % From system of equations 
  A=zeros(Nled,Nled);
  % b is the fixed current flowing out of a node
  b=-Vled/Rled*frac*ledstate;  % Offset reduced current flowing into LED due to Vled
  % Values in A are positive if current is flowing into node
  for i=1:Nled
    if i>1
      A(i,i-1)=1/Rstrip;  % Current flowing from last strip
      A(i,i)=-1/Rstrip;
    end
    if i<Nled
      A(i,i+1)=1/Rstrip;  % Current flowing to next strip
      A(i,i)=A(i,i)-1/Rstrip;
    end
    if ledstate(i)==1
      A(i,i)=A(i,i)-1/Rled*frac;   % Current into LED
    end
  end
  b(1)=b(1)-Vps/(Rps+Rcable);  % Extra current flowing into node 1
  A(1,1)=A(1,1)-1/(Rps+Rcable);
  if 0
    % HACK: Force V(1)to 4.10V to match observed data
    A(1,:)=0;
    A(1,1)=1;
    b(1)=4.1;
  end
  
  V=A\b;
  Iled=(V-Vled)/Rled*frac;
  Iled(ledstate==0)=0;
  Iavg(j)=mean(Iled(ledstate==1));
  if j==127
    setfig('LED');
    clf;
    subplot(211);
    plot(V);
    hold on;
    plot(0,Vps,'o');
    xlabel('LED'); ylabel('Voltage');
    subplot(212);
    plot([0:length(Iled)],[nan;Iled]*1000);
    xlabel('LED'); ylabel('Current (mA)');
    Itotal=sum(Iled);
    Vpsout=Vps-Itotal*Rps;
    fprintf('Total load current = %.2f A (%.1f mA/LED), Vpsout=%.2f, V(1)=%.2f, V(%d)=%.2f \n', Itotal, 1000*mean(Iled(ledstate==1)),Vpsout, V(1), Nled, V(end));
  end
end

setfig('PWM');
clf;
plot(PWM,Iavg*1000);
hold on;
if exist('meas')
  plot(meas.PWM,meas.Iavg,'g');
  mse=sqrt(sum((Iavg(ismember(PWM,meas.PWM))*1000-meas.Iavg).^2));
  fprintf('MSE=%.1f mA\n',mse);
  legend('Model','Measured');
end
xlabel('PWM');
ylabel('Current/LED (mA)');