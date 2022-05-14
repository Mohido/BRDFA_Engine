function [D] = ward_distribution_vector(V, L, N, H, alpha, beta, rho)
%WARD_DISTRIBUTION_VECTOR Summary of this function goes here
%   Detailed explanation goes here
cosRho = H(1)./alpha;   % h.x / alpha
sinRho = H(2)./beta;    % h.y / beta
HN = dot(H,N);
LN = dot(L,N);
VN = dot(V,N);

exponentTerm = exp(-(sinRho.*sinRho + cosRho.*cosRho)./(HN.*HN) );
den = 4.*pi.*alpha.*beta.*sqrt(LN.*VN);

D = (rho.* exponentTerm)./den;
end

