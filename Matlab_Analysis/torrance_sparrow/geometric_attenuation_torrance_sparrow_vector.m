function [geometricFactor] = geometric_attenuation_torrance_sparrow_vector(v_i, v_r, n)
%GEOMETRIC_ATTENUATION_TORRANCE_SPARROW_VECTOR process the given two
%vectors and output the goemetric factor based on them.
%   Detailed explanation goes here
A = A_torrance_sparrow_simp(v_i, v_r, n);
geometricFactor = 1 - (1-sqrt(1-A*A))./A;
% geometricFactor = geometricFactor;
temp = geometricFactor/dot(v_r, n);
fprintf('Geometric Attenuation for A=%f , G=%f, G/cos(R)=%f\n', A, geometricFactor, temp);
end

