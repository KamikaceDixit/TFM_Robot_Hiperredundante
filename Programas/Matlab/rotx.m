function [ T ] = rotx( alpha )

alpha=deg2rad(alpha);
T = [1, 0, 0, 0; 
               0, cos(alpha), -sin(alpha), 0;
               0, sin(alpha), cos(alpha),0;
               0, 0, 0, 1];

end

