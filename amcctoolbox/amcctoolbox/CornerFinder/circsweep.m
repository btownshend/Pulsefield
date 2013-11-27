function [theta,values,thetasmd,valuessmd]=circsweep(img,x,y)
% CIRCSWEEP sums the intensity of the input image along rays at all angles.
% 
% CIRCSWEEP sums the intensity pixels lying along a ray at a certain angle.
% This is done for all angles. The outputs theta and values correspond to
% the summation with the rays going from 0 to 360 degrees from the centre
% of the image to the border. The other two outputs correspond to the 
% summation along the entire ray passing through the centre with the angles
% going from 0 to 180.
% 
% USAGE:
%     [theta,values,thetasmd,valuessmd]=circsweep(img,x,y);
% 
% INPUTS:
%     img: input grayscale image of class double
% 
%     x: the x coordinates of the pixels, this is fed from the output of
%     SWEEPMATRIX
% 
%     y: the y coordinates of the pixels, this is fed from the output of
%     SWEEPMATRIX
% 
% OUTPUTS:
%     theta: angles from 0 to 360
% 
%     values: the sum of intensity values from centre to border
% 
%     thetasmd: angles from 0 to 180
% 
%     valuessmd: the sum of intensity values from border to border along
%     the centre of the image


win=floor(size(img,1)/2);
cen=win+1;

theta=pi()/90:pi()/90:2*pi();


x=x+cen;
y=y+cen;
xn=find(x<1 | x>size(img,1));
yn=find(y<1 | y>size(img,2));
xn=xn';
yn=yn';

x(xn)=1;
y(yn)=1;

indcs=sub2ind(size(img),x,y);
values=img(indcs);
values([xn,yn])=0;
values=mean(values);
n180=length(theta)/2;
thetasmd=theta(1:n180);
valuessmd=(values(1:n180)+values(n180+1:length(theta)))/2;