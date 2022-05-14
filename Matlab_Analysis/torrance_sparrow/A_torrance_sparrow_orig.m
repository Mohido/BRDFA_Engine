function [A] = A_torrance_sparrow_orig(psi,theta)
%A_TORRANCE_SPARROW_ORIG Summary of this function goes here
%   Detailed explanation goes here
C2 = cos((theta-psi)/2);
nom = sin(theta).*sin(theta) - C2.*C2;
den =  C2.*C2 - cos(theta-psi).*sin(theta).*sin(theta);
A = nom/den;
end

