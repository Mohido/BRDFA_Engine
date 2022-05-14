function [geometricFactor] = geometric_attenuation_torrance_sparrow(psi,theta)
% GEOMETRIC_ATTENUATION_TORRANCE_SPARROW Returns the Geometric Attenuation
% Factor
%   This model function is implemented based on Torranec-Sparrow Geometric
%   Attenuation derivation considering V-Grooves cavities. The function
%   details is explained in "theory off-specular peak" for Torrance and
%   Sparrow
fprintf('Geometric Attenuation Angles for I=%f , R=%f to the surface normal\n', rad2deg(psi), rad2deg(theta));
C2 = cos((theta-psi)/2);
nom = sin(theta).*sin(theta) - C2.*C2;
den =  C2.*C2 - cos(theta-psi).*sin(theta).*sin(theta);
A = nom/den;
geometricFactor = 1 - (1-sqrt(1-A*A))./A;
% geometricFactor = geometricFactor;
temp = geometricFactor/cos(theta);
fprintf('Geometric Attenuation for A=%f , G=%f, G/cos(R)=%f\n', A, geometricFactor, temp);
end

