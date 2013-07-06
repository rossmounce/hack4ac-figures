% Replicating the first two steps of the ReVision method
% http://vis.stanford.edu/files/2011-ReVision-UIST.pdf

close all
clear all

%------------ loading image
name = 'test.png'
name = 'fig1.png'
%name = 'fig3.png'

I = imread(name);

%------------ reducing image size
step = 4;
I = I(1:step:end,1:step:end);
I_init = I;

%------------ applying biliteral filter (some kind of edge preserving filter)
% Set bilateral filter parameters.
w     = 1;       % bilateral filter half-width
sigma = [1 0.1]; % bilateral filter standard deviations
% Apply bilateral filter to each image.
Ianalog = double(I)./255; % from 0-255 to 0-1
bflt_I = bfilter2(Ianalog,w,sigma); % applying filter

figure(1)
subplot(3,1,1)
imagesc(I)
subplot(3,1,2)
imagesc(bflt_I)
I =floor(255*bflt_I);

%% try connnected items

% some hard-coded parameters
MIN_WIDTH = 5; % minimum width of rectangle
MIN_NUM_COMP = 100; % minimum area of rectangle
v_step = 5; % grouping integer values by blocks 




Nx = size(I,2);Ny = size(I,1);
VALUES = 0:v_step:255;

rectangles = [];  % x,y,width,height

i_item = 0;
for i =  1:length(VALUES)-1 % for all colors
    [i,length(VALUES)]
       
    I_v_indices = [];
    for v_int = VALUES(i):VALUES(i+1)
        I_v_indices = [I_v_indices;find(I == v_int)];
    end
    
    I_v = zeros(size(I));
    I_v(I_v_indices) = 1;
    [L,num] = bwlabel(I_v,4); % extract connected items

    for i = 1 : num % for all connected items
        
        x = find(sum(L==i,1)>0); % sum on y axis
        xmin = x(1); xmax = x(end) ;
        width = xmax-xmin;
        
        % discard too narrow items
        % discard too small items
        if ( sum(sum(L==i)) > MIN_NUM_COMP ) & (width > MIN_WIDTH)
            
            
            x = find(sum(L==i,1)>0); % sum on y axis
            d1 = max(x)-min(x);
            y = find(sum(L==i,2)>0); % sum on y axis
            d2= max(y)-min(y);
            Area = d1*d2;
            px_in_item = sum(sum(L==i));
            
             
            if px_in_item > 0.9*Area % discard if not rectangular like
              rectangles = [rectangles;[min(x),min(y),d1,d2]];
    
 
            end           
        end
    end
    
    
end

%% Process items to only keep rectangular ones

I_final = I.*0;
for i = 1 : size(rectangles,1)
   xinit = rectangles(i,1);
      yinit = rectangles(i,2);
      width = rectangles(i,3);
      height = rectangles(i,4);
      
    I_final(yinit:yinit+height,xinit:xinit+width)= I_final(yinit:yinit+height,xinit:xinit+width) + i;
end

figure
imagesc(I_final)

%%
figure(6)
figure
subplot(2,1,1)
imagesc(I_init)
subplot(2,1,2)
imagesc(I_final)
colormap hot

