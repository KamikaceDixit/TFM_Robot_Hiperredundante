function [ T ] = rotz( gamma )
gamma=deg2rad(gamma);
          
T = [cos(gamma), -sin(gamma), 0, 0;
               sin(gamma), cos(gamma), 0, 0;
               0, 0, 1, 0;
               0, 0, 0, 1];
end

