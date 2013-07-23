function [vlow,vin,iin,V,I]=stripmodel(model,levels,doplot)
  A=zeros(2*(model.NLed));
  b=zeros(2*(model.NLed),1);
  % A(:,1) refers to I1 (current into LED 1)
  % A(:,2) refers to V1 (voltage at stage 1)
  % A(1,:) is the first nodal equation that the sum of currents = 0
  % A(2,:) is the first ohms law equation
  for i=1:2:model.NLed*2
    % Sum of currents out of this node = 0
    A(i,i)=-1;
    if i+2<=size(A,2)
      A(i,i+2)=1;
    end
    A(i,i+1)=sum(1 ./model.RLed)*levels((i+1)/2)/255;
    b(i)=-model.IOff+sum(model.Vf./model.RLed)*levels((i+1)/2)/255;
    % Current into node based on Voltage difference from last node
    A(i+1,i)=model.RPerSegment;
    A(i+1,i+1)=1;
    if i>1
      A(i+1,i-1)=-1;
    else
      b(i+1)=model.VIn;
      A(i+1,i)=A(i+1,i)+model.RIn;
    end
  end
  if (model.Double)
    % Also have connection to power supply at other end
    % Sum of currents out of this node includes p/s
    A(end-1,end)=A(end-1,end)+1/(model.RPerSegment+model.RIn);
    b(end-1)=b(end-1)+(model.VIn)/(model.RPerSegment+model.RIn);
  end
  
  x=A\b;
  V=x(2:2:end);
  I=x(1:2:end);
  
  if model.Double
    IRight=(model.VIn-V(end))/(model.RPerSegment+model.RIn);
  else
    IRight=0;
  end

  if doplot
    ti=sprintf('NumLeds=%d, Level=%d, Double=%d',sum(levels>0),max(levels),model.Double);
    setfig(ti);clf;
    subplot(211);
    plot(1:model.NLed,[V]);
    hold on;
    plot(0,model.VIn,'o');
    if (model.Double)
      plot(model.NLed+1,model.VIn,'o');
    end
    hold on;
    col='rgb';
    for k=1:3
      plot([1,model.NLed],model.Vf(k)*[1,1],[':',col(k)]);
    end
    c=axis;
    xlabel('Led');
    ylabel('Voltage (V)');
    subplot(212);
    plot(1:model.NLed,-1000*diff([I;-IRight]),'k');
    c2=axis;
    c2(1:2)=c(1:2);
    xlabel('Led');
    ylabel('Current (mA)');
    title(sprintf('Total current = %.1f mA', (I(1)+IRight)*1000));
    hold on
    col='rgb';
    for k=1:3
      ILed=(V-model.Vf(k))./model.RLed(k).*levels'/255;
      if any(ILed<0)
        fprintf('Warning - have reverse current through %s led\n', col(k));
      end
      plot(1:model.NLed,1000*ILed,col(k));
    end
    suptitle(ti);
  end
  vlow=min(V);
  vin=V(1)+I(1)*model.RPerSegment;
  iin=I(1)+IRight;
end